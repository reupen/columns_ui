#include "pch.h"
#include "title_formatting.h"

namespace cui::tf {

class ValueVisitor {
public:
    explicit ValueVisitor(titleformat_text_out* out) : m_out{out} {}

    bool operator()(const bool& value) const
    {
        if (value) {
            m_out->write(titleformat_inputtypes::unknown, "1");
        }
        return value;
    }

    bool operator()(const std::string& value) const
    {
        m_out->write(titleformat_inputtypes::unknown, value.c_str(), value.size());
        return true;
    }

    bool operator()(const std::string_view& value) const
    {
        m_out->write(titleformat_inputtypes::unknown, value.data(), value.size());
        return true;
    }

    bool operator()(const pfc::string8& value) const
    {
        m_out->write(titleformat_inputtypes::unknown, value.c_str(), value.get_length());
        return true;
    }

    bool operator()(const std::function<std::string()>& value) const { return (*this)(value()); }

private:
    titleformat_text_out* m_out{};
};

bool FieldProviderTitleformatHook::process_field(
    titleformat_text_out* p_out, const char* p_name, size_t p_name_length, bool& p_found_flag)
{
    const auto iter = m_field_map.find(std::string_view(p_name, p_name_length));

    if (iter == m_field_map.end())
        return false;

    p_found_flag = true;

    return std::visit(ValueVisitor(p_out), iter->second);
}

} // namespace cui::tf
