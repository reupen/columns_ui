#pragma once

class GlobalVariable {
public:
    GlobalVariable(const char* p_name, t_size p_name_length, const char* p_value, t_size p_value_length)
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
    const char* find_by_name(const char* p_name, unsigned length)
    {
        unsigned count = get_count();
        for (unsigned n = 0; n < count; n++) {
            const char* ptr = get_item(n)->get_name();
            if (!stricmp_utf8_ex(p_name, length, ptr, pfc_infinite))
                return get_item(n)->get_value();
        }
        return nullptr;
    }
    void add_item(const char* p_name, unsigned p_name_length, const char* p_value, unsigned p_value_length)
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
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override
    {
        p_found_flag = false;
        return false;
    }

    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        p_found_flag = false;
        if (set && !stricmp_utf8_ex(p_name, p_name_length, "set_global", pfc_infinite)) {
            switch (p_params->get_param_count()) {
            case 2: {
                const char* name;
                const char* value;
                unsigned name_length, value_length;
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
                unsigned name_length;
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

    SetGlobalTitleformatHook(GlobalVariableList& vars) : p_vars(vars){};
};

class DateTitleformatHook : public titleformat_hook {
    const SYSTEMTIME* p_st;
    pfc::array_t<char> year, month, day, dayofweek, hour, julian;

public:
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;
    DateTitleformatHook(const SYSTEMTIME* st = nullptr) : p_st(st){};
};

class SplitterTitleformatHook : public titleformat_hook {
public:
    SplitterTitleformatHook(titleformat_hook* p_hook1, titleformat_hook* p_hook2, titleformat_hook* p_hook3,
        titleformat_hook* p_hook4 = nullptr)
        : m_hook1(p_hook1)
        , m_hook2(p_hook2)
        , m_hook3(p_hook3)
        , m_hook4(p_hook4){};
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;
    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override;

private:
    titleformat_hook *m_hook1, *m_hook2, *m_hook3, *m_hook4;
};

class PlaylistNameTitleformatHook : public titleformat_hook {
    bool m_initialised{false};
    pfc::string8 m_name;

public:
    void initialise();
    bool process_field(
        titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag) override;

    bool process_function(titleformat_text_out* p_out, const char* p_name, unsigned p_name_length,
        titleformat_hook_function_params* p_params, bool& p_found_flag) override
    {
        return false;
    };
    PlaylistNameTitleformatHook() = default;
};
