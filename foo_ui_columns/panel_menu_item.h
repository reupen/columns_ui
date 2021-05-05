#pragma once

namespace cui::panel_helpers {

class CommandMenuNode : public uie::menu_node_command_t {
public:
    bool get_display_data(pfc::string_base& p_out, unsigned& p_displayflags) const override
    {
        p_out = m_title;
        p_displayflags = 0;
        return true;
    }
    bool get_description(pfc::string_base& p_out) const override { return false; }
    void execute() override { m_execute_callback(); }
    CommandMenuNode(const char* title, std::function<void()> execute_callback)
        : m_title(title)
        , m_execute_callback{std::move(execute_callback)}
    {
    }

private:
    pfc::string8 m_title;
    std::function<void()> m_execute_callback;
};

} // namespace cui::panel_helpers
