#include "pch.h"
#include "playlist_view_tfhooks.h"

bool PlaylistNameTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    if (p_name_length && *p_name == '_') {
        p_name++;
        p_name_length--;
    }
    if (!stricmp_utf8_ex(p_name, p_name_length, "playlist_name", pfc_infinite)) {
        initialise();
        p_out->write(titleformat_inputtypes::unknown, m_name);
        p_found_flag = true;
        return true;
    }
    return false;
}

void PlaylistNameTitleformatHook::initialise()
{
    if (!m_initialised) {
        playlist_manager_v3::get()->activeplaylist_get_name(m_name);
        m_initialised = true;
    }
}

int date_to_julian(const SYSTEMTIME* st)
{
    int year = st->wYear - 1;
    int hour = st->wHour;
    int day = st->wDay - (st->wHour > 11 ? 0 : 1);
    int monthdays = 0;
    switch (st->wMonth) {
    case 2:
        monthdays = 31;
        break;
    case 3:
        monthdays = 59;
        break;
    case 4:
        monthdays = 90;
        break;
    case 5:
        monthdays = 120;
        break;
    case 6:
        monthdays = 151;
        break;
    case 7:
        monthdays = 181;
        break;
    case 8:
        monthdays = 212;
        break;
    case 9:
        monthdays = 243;
        break;
    case 10:
        monthdays = 273;
        break;
    case 11:
        monthdays = 304;
        break;
    case 12:
        monthdays = 334;
        break;
    }

    int yeardays = monthdays;
    if ((st->wYear % 4) && (!(st->wYear % 100) || (st->wYear % 400)))
        yeardays++;

    return (year / 4 - year / 100 + year / 400) + (year * 365) + yeardays + day + 1721395 + 29;
}

bool DateTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;

    if (p_st) {
        if (p_name_length > 8 && !stricmp_utf8_ex(p_name, 8, "_system_", pfc_infinite)) {
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "year", pfc_infinite)) {
                if (!year.get_ptr()) {
                    year.set_size(33);
                    year.fill(0);
                    _ultoa_s(p_st->wYear, year.get_ptr(), year.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, year.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "month", pfc_infinite)) {
                if (!month.get_ptr()) {
                    month.set_size(33);
                    month.fill(0);
                    _ultoa_s(p_st->wMonth, month.get_ptr(), month.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, month.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "day", pfc_infinite)) {
                if (!day.get_ptr()) {
                    day.set_size(33);
                    day.fill(0);
                    _ultoa_s(p_st->wDay, day.get_ptr(), day.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, day.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "day_of_week", pfc_infinite)) {
                if (!dayofweek.get_ptr()) {
                    dayofweek.set_size(33);
                    dayofweek.fill(0);
                    _ultoa_s(p_st->wDayOfWeek + 1, dayofweek.get_ptr(), dayofweek.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, dayofweek.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "julian_day", pfc_infinite)) {
                if (!julian.get_ptr()) {
                    julian.set_size(33);
                    julian.fill(0);
                    _ultoa_s(date_to_julian(p_st), julian.get_ptr(), julian.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, julian.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
            if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "hour", pfc_infinite)) {
                if (!hour.get_ptr()) {
                    hour.set_size(33);
                    hour.fill(0);
                    _ultoa_s(p_st->wHour, hour.get_ptr(), hour.get_size(), 10);
                }
                p_out->write(titleformat_inputtypes::unknown, hour.get_ptr(), pfc_infinite);
                p_found_flag = true;
                return true;
            }
        }
    }

    return false;
}

bool SplitterTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    p_found_flag = false;
    if (m_hook1 && m_hook1->process_field(p_out, p_name, p_name_length, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook2 && m_hook2->process_field(p_out, p_name, p_name_length, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook3 && m_hook3->process_field(p_out, p_name, p_name_length, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook4 && m_hook4->process_field(p_out, p_name, p_name_length, p_found_flag))
        return true;
    p_found_flag = false;
    return false;
}

bool SplitterTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    if (m_hook1 && m_hook1->process_function(p_out, p_name, p_name_length, p_params, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook2 && m_hook2->process_function(p_out, p_name, p_name_length, p_params, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook3 && m_hook3->process_function(p_out, p_name, p_name_length, p_params, p_found_flag))
        return true;
    p_found_flag = false;
    if (m_hook4 && m_hook4->process_function(p_out, p_name, p_name_length, p_params, p_found_flag))
        return true;
    p_found_flag = false;
    return false;
}

bool DateTitleformatHook::process_function(titleformat_text_out* p_out, const char* p_name, size_t p_name_length,
    titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    return false;
}
