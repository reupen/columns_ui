#include "pch.h"

#include "colour_manager_data.h"

#include "colour_utils.h"
#include "dark_mode.h"

namespace cui::colours {

CommonColourCallbackManager common_colour_callback_manager;

ColourSet create_default_colour_set(bool is_dark, ColourScheme scheme)
{
    ColourSet colour_set;
    std::initializer_list<std::tuple<COLORREF&, colour_identifier_t>> colour_identifier_pairs
        = {{colour_set.text, colour_text}, {colour_set.background, colour_background},
            {colour_set.selection_text, colour_selection_text},
            {colour_set.selection_background, colour_selection_background},
            {colour_set.inactive_selection_text, colour_inactive_selection_text},
            {colour_set.inactive_selection_background, colour_inactive_selection_background},
            {colour_set.active_item_frame, colour_active_item_frame}};

    for (auto&& [colour, identifier] : colour_identifier_pairs)
        colour = dark::get_system_colour(get_system_colour_id(identifier), is_dark);

    colour_set.colour_scheme = scheme;

    return colour_set;
}

void CommonColourCallbackManager::s_on_common_bool_changed(uint32_t mask)
{
    // Copy the list of callbacks in case someone tries to add or remove one
    // while we're iterating through them
    for (const auto callbacks = m_callbacks; auto&& callback : callbacks) {
        if (m_callbacks.contains(callback))
            callback->on_bool_changed(mask);
    }
}

void CommonColourCallbackManager::s_on_common_colour_changed(uint32_t mask)
{
    // Copy the list of callbacks in case someone tries to add or remove one
    // while we're iterating through them
    for (const auto callbacks = m_callbacks; auto&& callback : callbacks)
        if (m_callbacks.contains(callback))
            callback->on_colour_changed(mask);
}

void CommonColourCallbackManager::deregister_common_callback(common_callback* p_callback)
{
    m_callbacks.erase(p_callback);
}

void CommonColourCallbackManager::register_common_callback(common_callback* p_callback)
{
    m_callbacks.emplace(p_callback);
}

ColourManagerData::ColourManagerData()
    : cfg_var(g_cfg_guid)
    , m_global_light_entry(std::make_shared<Entry>(false, true))
    , m_global_dark_entry(std::make_shared<Entry>(true, true))
{
}

Entry::Ptr ColourManagerData::get_entry(GUID id, bool is_dark)
{
    if (id == pfc::guid_null) {
        return get_global_entry(is_dark);
    }

    auto& entries = is_dark ? m_dark_entries : m_light_entries;

    for (auto&& entry : entries) {
        if (entry->id == id) {
            return entry;
        }
    }
    auto entry = std::make_shared<Entry>(is_dark);
    entry->id = id;
    entries.emplace_back(entry);
    return entry;
}

Entry::Ptr ColourManagerData::get_global_entry(bool is_dark) const
{
    return is_dark ? m_global_dark_entry : m_global_light_entry;
}

void ColourManagerData::set_data_raw(stream_reader* p_stream, size_t p_sizehint, abort_callback& p_abort)
{
    uint32_t version;
    p_stream->read_lendian_t(version, p_abort);
    if (version <= cfg_version) {
        m_global_light_entry->read(version, p_stream, p_abort);
        const auto light_count = p_stream->read_lendian_t<uint32_t>(p_abort);
        m_light_entries.clear();
        for (auto _ [[maybe_unused]] : ranges::views::iota(0u, light_count)) {
            auto ptr = std::make_shared<Entry>(false);
            ptr->read(version, p_stream, p_abort);
            m_light_entries.emplace_back(std::move(ptr));
        }

        try {
            m_global_dark_entry->read(version, p_stream, p_abort);
            const auto dark_count = p_stream->read_lendian_t<uint32_t>(p_abort);
            m_dark_entries.clear();
            for (auto _ [[maybe_unused]] : ranges::views::iota(0u, dark_count)) {
                auto ptr = std::make_shared<Entry>(true);
                ptr->read(version, p_stream, p_abort);
                m_dark_entries.emplace_back(std::move(ptr));
            }
        } catch (const exception_io_data_truncation&) {
        }
    }
}

void ColourManagerData::get_data_raw(stream_writer* p_stream, abort_callback& p_abort)
{
    std::unordered_set<GUID> clients;
    for (auto enumerator = client::enumerate(); !enumerator.finished(); ++enumerator) {
        clients.emplace((*enumerator)->get_client_guid());
    }

    const auto light_entries = m_light_entries | ranges::copy
        | ranges::actions::remove_if([&clients](auto&& entry) { return !clients.contains(entry->id); });
    const auto dark_entries = m_dark_entries | ranges::copy
        | ranges::actions::remove_if([&clients](auto&& entry) { return !clients.contains(entry->id); });

    p_stream->write_lendian_t(static_cast<uint32_t>(cfg_version), p_abort);

    m_global_light_entry->write(p_stream, p_abort);

    p_stream->write_lendian_t(gsl::narrow<uint32_t>(light_entries.size()), p_abort);
    for (auto&& entry : light_entries)
        entry->write(p_stream, p_abort);

    m_global_dark_entry->write(p_stream, p_abort);

    p_stream->write_lendian_t(gsl::narrow<uint32_t>(dark_entries.size()), p_abort);
    for (auto&& entry : dark_entries)
        entry->write(p_stream, p_abort);
}

Entry::Entry(bool is_dark, bool b_global)
    : colour_set(create_default_colour_set(is_dark, b_global ? ColourSchemeThemed : ColourSchemeGlobal))
{
}

void ColourSet::read(uint32_t version, stream_reader* stream, abort_callback& aborter)
{
    stream->read_lendian_t((uint32_t&)colour_scheme, aborter);
    stream->read_lendian_t(text, aborter);
    stream->read_lendian_t(selection_text, aborter);
    stream->read_lendian_t(inactive_selection_text, aborter);
    stream->read_lendian_t(background, aborter);
    stream->read_lendian_t(selection_background, aborter);
    stream->read_lendian_t(inactive_selection_background, aborter);
    stream->read_lendian_t(active_item_frame, aborter);
    stream->read_lendian_t(use_custom_active_item_frame, aborter);
}

void ColourSet::write(stream_writer* stream, abort_callback& aborter) const
{
    stream->write_lendian_t((uint32_t)colour_scheme, aborter);
    stream->write_lendian_t(text, aborter);
    stream->write_lendian_t(selection_text, aborter);
    stream->write_lendian_t(inactive_selection_text, aborter);
    stream->write_lendian_t(background, aborter);
    stream->write_lendian_t(selection_background, aborter);
    stream->write_lendian_t(inactive_selection_background, aborter);
    stream->write_lendian_t(active_item_frame, aborter);
    stream->write_lendian_t(use_custom_active_item_frame, aborter);
}

void Entry::read(uint32_t version, stream_reader* stream, abort_callback& aborter)
{
    stream->read_lendian_t(id, aborter);
    colour_set.read(version, stream, aborter);
}

void Entry::import(stream_reader* p_reader, size_t stream_size, uint32_t type, abort_callback& p_abort)
{
    fbh::fcl::Reader reader(p_reader, stream_size, p_abort);
    uint32_t element_id;
    uint32_t element_size;

    while (reader.get_remaining()) {
        reader.read_item(element_id);
        reader.read_item(element_size);

        switch (element_id) {
        case identifier_id:
            reader.read_item(id);
            break;
        case identifier_scheme:
            reader.read_item((uint32_t&)colour_set.colour_scheme);
            break;
        case identifier_text:
            reader.read_item(colour_set.text);
            break;
        case identifier_selection_text:
            reader.read_item(colour_set.selection_text);
            break;
        case identifier_inactive_selection_text:
            reader.read_item(colour_set.inactive_selection_text);
            break;
        case identifier_background:
            reader.read_item(colour_set.background);
            break;
        case identifier_selection_background:
            reader.read_item(colour_set.selection_background);
            break;
        case identifier_inactive_selection_background:
            reader.read_item(colour_set.inactive_selection_background);
            break;
        case identifier_custom_active_item_frame:
            reader.read_item(colour_set.active_item_frame);
            break;
        case identifier_use_custom_active_item_frame:
            reader.read_item(colour_set.use_custom_active_item_frame);
            break;
        default:
            reader.skip(element_size);
            break;
        }
    }
}

void Entry::_export(stream_writer* p_stream, abort_callback& p_abort)
{
    fbh::fcl::Writer out(p_stream, p_abort);
    out.write_item(identifier_id, id);
    out.write_item(identifier_scheme, (uint32_t)colour_set.colour_scheme);
    if (colour_set.colour_scheme == ColourSchemeCustom) {
        out.write_item(identifier_text, colour_set.text);
        out.write_item(identifier_selection_text, colour_set.selection_text);
        out.write_item(identifier_inactive_selection_text, colour_set.inactive_selection_text);
        out.write_item(identifier_background, colour_set.background);
        out.write_item(identifier_selection_background, colour_set.selection_background);
        out.write_item(identifier_inactive_selection_background, colour_set.inactive_selection_background);
    }
    out.write_item(identifier_use_custom_active_item_frame, colour_set.use_custom_active_item_frame);
    out.write_item(identifier_custom_active_item_frame, colour_set.active_item_frame);
}

void Entry::write(stream_writer* p_stream, abort_callback& p_abort)
{
    p_stream->write_lendian_t(id, p_abort);
    p_stream->write_lendian_t((uint32_t)colour_set.colour_scheme, p_abort);
    p_stream->write_lendian_t(colour_set.text, p_abort);
    p_stream->write_lendian_t(colour_set.selection_text, p_abort);
    p_stream->write_lendian_t(colour_set.inactive_selection_text, p_abort);
    p_stream->write_lendian_t(colour_set.background, p_abort);
    p_stream->write_lendian_t(colour_set.selection_background, p_abort);
    p_stream->write_lendian_t(colour_set.inactive_selection_background, p_abort);
    p_stream->write_lendian_t(colour_set.active_item_frame, p_abort);
    p_stream->write_lendian_t(colour_set.use_custom_active_item_frame, p_abort);
}

} // namespace cui::colours
