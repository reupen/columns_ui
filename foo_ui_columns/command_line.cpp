#include "pch.h"

#include "dark_mode_dialog.h"
#include "fcl.h"

static const char8_t* g_help_text
    = u8"Available commands:\n\n"
      u8"/columnsui:help, /columnsui:?\t\tdisplay this command-line help\n"
      u8"/columnsui:import <fcl path>\t\timport a Columns UI configuration\n"
      u8"/columnsui:import-quiet <fcl path>\timport a Columns UI configuration without confirming first\n"
      u8"/columnsui:export <fcl path>\t\texport selected parts of the current Columns UI configuration\n"
      u8"/columnsui:export-quiet <fcl path>\texport all parts of the current Columns UI configuration";

class HelpCommandLineHandler : public commandline_handler {
public:
    result on_token(const char* token) noexcept override
    {
        if (stricmp_utf8_partial(token, "/columnsui:help") != 0 && stricmp_utf8_partial(token, "/columnsui:?") != 0)
            return RESULT_NOT_OURS;

        execute();
        return RESULT_PROCESSED;
    }
    static void execute()
    {
        HWND parent = core_api::get_main_window();
        ui_control::get()->activate();
        cui::dark::modeless_info_box(parent, "Columns UI command-line help", reinterpret_cast<const char*>(g_help_text),
            uih::InfoBoxType::Neutral, true);
    }
};

class CommandLineSingleFileHelper {
public:
    CommandLineSingleFileHelper(const char* error_title, const char* no_files_error, const char* too_many_files_error)
        : m_error_title{error_title}
        , m_no_files_error{no_files_error}
        , m_too_many_files_error{too_many_files_error}
    {
    }
    void reset() { m_files.clear(); }
    void add_file(const char* url) { m_files.emplace_back(url); }
    bool validate_files()
    {
        const auto main_window = core_api::get_main_window();
        if (m_files.empty()) {
            ui_control::get()->activate();
            cui::dark::modeless_info_box(main_window, m_error_title, m_no_files_error, uih::InfoBoxType::Error);
            return false;
        }
        if (m_files.size() > 1) {
            ui_control::get()->activate();
            cui::dark::modeless_info_box(main_window, m_error_title, m_too_many_files_error, uih::InfoBoxType::Error);
            return false;
        }
        return true;
    }
    const char* get_file() { return m_files.empty() ? nullptr : m_files[0].get_ptr(); }

private:
    std::vector<pfc::string8> m_files;
    const char* m_error_title;
    const char* m_no_files_error;
    const char* m_too_many_files_error;
};

class ImportCommandLineHandler : public commandline_handler {
public:
    bool m_is_quiet = false;
    CommandLineSingleFileHelper m_single_file_helper;

    ImportCommandLineHandler()
        : m_single_file_helper{u8"Import configuration – Columns UI"_pcc, "No file to import specified.",
              "Too many files to import specified. You can only import one file at a time, "
              "and should use double quotes around paths containing spaces."}
    {
    }

    result on_token(const char* token) noexcept override
    {
        m_is_quiet = !stricmp_utf8(token, "/columnsui:import-quiet");
        const auto is_import = m_is_quiet || !stricmp_utf8(token, "/columnsui:import");

        if (!is_import)
            return RESULT_NOT_OURS;

        m_single_file_helper.reset();

        return RESULT_PROCESSED_EXPECT_FILES;
    }

    void on_file(const char* url) noexcept override { m_single_file_helper.add_file(url); }
    void on_files_done() noexcept override
    {
        if (m_single_file_helper.validate_files())
            execute_import(m_single_file_helper.get_file(), m_is_quiet);
        m_single_file_helper.reset();
    }

    static void execute_import(const char* path, bool is_quiet)
    {
        const auto main_window = core_api::get_main_window();
        pfc::string_formatter formatter;

        if (!is_quiet) {
            ui_control::get()->activate();
            const auto message
                = fmt::format("Are you sure you want to import {}? Your current Columns UI configuration will be lost.",
                    pfc::string_filename_ext(path).c_str());

            if (!cui::dark::modal_info_box(main_window, "Import configuration", message.c_str(),
                    uih::InfoBoxType::Neutral, uih::InfoBoxModalType::YesNo))
                return;
        }
        g_import_layout(core_api::get_main_window(), path, is_quiet);
    }
};

class ExportCommandLineHandler : public commandline_handler {
public:
    bool m_is_quiet = false;
    CommandLineSingleFileHelper m_single_file_helper;

    ExportCommandLineHandler()
        : m_single_file_helper{u8"Export configuration – Columns UI"_pcc, "No file to export to specified.",
              "Too many destination files specified. You must specify only one destination path, "
              "and should use double quotes around paths containing spaces."}
    {
    }

    result on_token(const char* token) noexcept override
    {
        m_is_quiet = !stricmp_utf8(token, "/columnsui:export-quiet");
        const auto is_import = m_is_quiet || !stricmp_utf8(token, "/columnsui:export");

        if (!is_import)
            return RESULT_NOT_OURS;

        m_single_file_helper.reset();

        return RESULT_PROCESSED_EXPECT_FILES;
    }

    void on_file(const char* url) noexcept override { m_single_file_helper.add_file(url); }
    void on_files_done() noexcept override
    {
        if (m_single_file_helper.validate_files())
            execute_export(m_single_file_helper.get_file(), m_is_quiet);
        m_single_file_helper.reset();
    }

    static void execute_export(const char* path, bool is_quiet)
    {
        if (!is_quiet) {
            ui_control::get()->activate();
        }
        g_export_layout(core_api::get_main_window(), path, is_quiet);
    }
};

static commandline_handler_factory_t<HelpCommandLineHandler> help_cli_handler;
static commandline_handler_factory_t<ImportCommandLineHandler> import_cli_handler;
static commandline_handler_factory_t<ExportCommandLineHandler> export_cli_handler;
