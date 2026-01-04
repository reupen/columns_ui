#pragma once

namespace cui::tf {

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

} // namespace cui::tf
