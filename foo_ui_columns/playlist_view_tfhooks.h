#pragma once

class GlobalVariable {
public:
    GlobalVariable(const char* p_name, size_t p_name_length, const char* p_value, size_t p_value_length)
        : m_name(p_name, p_name_length)
        , m_value(p_value, p_value_length)
    {
    }

    const char* get_name() const { return m_name; }
    const char* get_value() const { return m_value; }

private:
    pfc::string_simple m_name, m_value;
};

class GlobalVariableList : public pfc::ptr_list_t<GlobalVariable> {
public:
    const char* find_by_name(const char* p_name, size_t length)
    {
        const auto count = get_count();
        for (size_t n = 0; n < count; n++) {
            const char* ptr = get_item(n)->get_name();
            if (!stricmp_utf8_ex(p_name, length, ptr, pfc_infinite))
                return get_item(n)->get_value();
        }
        return nullptr;
    }
    void add_item(const char* p_name, size_t p_name_length, const char* p_value, size_t p_value_length)
    {
        auto var = new GlobalVariable(p_name, p_name_length, p_value, p_value_length);
        ptr_list_t<GlobalVariable>::add_item(var);
    }
    ~GlobalVariableList() { delete_all(); }
};

template <bool set = true, bool get = true>
class SetGlobalTitleformatHook : public titleformat_hook {
    GlobalVariableList& p_vars;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override
    {
        p_found_flag = false;
        return false;
    }

    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        p_found_flag = false;
        if (set && !stricmp_utf8_ex(p_name, p_name_length, "set_global", pfc_infinite)) {
            switch (p_params->get_param_count()) {
            case 2: {
                const char* name;
                const char* value;
                size_t name_length, value_length;
                p_params->get_param(0, name, name_length);
                p_params->get_param(1, value, value_length);
                p_vars.add_item(name, name_length, value, value_length);
                p_found_flag = true;
                return true;
            }
            default:
                return false;
            }
        }
        if (get && !stricmp_utf8_ex(p_name, p_name_length, "get_global", pfc_infinite)) {
            switch (p_params->get_param_count()) {
            case 1: {
                const char* name;
                size_t name_length;
                p_params->get_param(0, name, name_length);
                const char* ptr = p_vars.find_by_name(name, name_length);
                if (ptr) {
                    p_out->write(titleformat_inputtypes::unknown, ptr, pfc_infinite);
                    p_found_flag = true;
                } else
                    p_out->write(titleformat_inputtypes::unknown, "[unknown variable]", pfc_infinite);
                return true;
            }
            default:
                return false;
            }
        }
        return false;
    }

    explicit SetGlobalTitleformatHook(GlobalVariableList& vars) : p_vars(vars) {}
};

class DateTitleformatHook : public titleformat_hook {
    const SYSTEMTIME* p_st;
    pfc::array_t<char> year, month, day, dayofweek, hour, julian;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;
    explicit DateTitleformatHook(const SYSTEMTIME* st = nullptr) : p_st(st) {}
};

template <typename... Hooks>
class SplitterTitleformatHook : public titleformat_hook {
public:
    explicit SplitterTitleformatHook(Hooks*... hooks) : m_hooks(hooks...) {}

    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override
    {
        p_found_flag = false;

        return std::apply(
            [&](auto*... hooks) {
                return ((hooks && hooks->process_field(p_out, p_name, p_name_length, p_found_flag)) || ...);
            },
            m_hooks);
    }

    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        p_found_flag = false;

        return std::apply(
            [&](auto*... hooks) {
                return (
                    (hooks && hooks->process_function(p_out, p_name, p_name_length, p_params, p_found_flag)) || ...);
            },
            m_hooks);
    }

private:
    std::tuple<Hooks*...> m_hooks;
};

class PlaylistNameTitleformatHook : public titleformat_hook {
    bool m_initialised{false};
    pfc::string8 m_name;

public:
    void initialise();
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        return false;
    }
    PlaylistNameTitleformatHook() = default;
};
