#include "stdafx.h"
#include "fcl.h"

static const char* g_help_text = u8"syntax: foobar2000 /columnsui:<command> \"<path>\"\n\n"
    "Available commands:\n"
    "help, ? – displays this command-line help\n"
    "import – imports an fcl file\n"
    "import-quiet – imports an fcl file without confirmation dialog boxes";

class HelpCommandLineHandler : public commandline_handler {
public:
    result on_token(const char * token) override
    {
        if (stricmp_utf8_partial(token, u8"/columnsui:help") && stricmp_utf8_partial(token, u8"/columnsui:?"))
            return RESULT_NOT_OURS;

        execute();
        return RESULT_PROCESSED;
    }
    static void execute()
    {
        HWND parent = core_api::get_main_window();
        static_api_ptr_t<ui_control>()->activate();
        uMessageBox(parent, g_help_text, u8"Columns UI command-line help", 0);
    }
};

class CommandLineSingleFileHelper {
public:
    CommandLineSingleFileHelper(const char* error_title, 
        const char* no_files_error,
        const char* too_many_files_error)
        : m_error_title{error_title}, m_no_files_error{no_files_error}, m_too_many_files_error{too_many_files_error} {};
    void reset()
    {
        m_files.remove_all();
    }
    void add_file(const char* url)
    {
        m_files.add_item(url);
    }
    bool validate_files()
    {
        const auto main_window = core_api::get_main_window();
        if (m_files.get_count() == 0) {
            static_api_ptr_t<ui_control>()->activate();
            fbh::show_info_box(
                main_window,
                m_error_title,
                m_no_files_error,
                OIC_ERROR);
            return false;
        }
        if (m_files.get_count() > 1) {
            static_api_ptr_t<ui_control>()->activate();
            fbh::show_info_box(
                main_window,
                m_error_title,
                m_too_many_files_error,
                OIC_ERROR);
            return false;
        }
        return true;
    }
    const char* get_file()
    {
        return m_files.get_count() ? m_files[0].get_ptr() : nullptr;
    }
private:
    pfc::list_t<pfc::string8> m_files;
    const char* m_error_title;
    const char* m_no_files_error;
    const char* m_too_many_files_error;
};

class ImportCommandLineHandler : public commandline_handler {
public:
    bool m_is_quiet = false;
    CommandLineSingleFileHelper m_single_file_helper;

    ImportCommandLineHandler()
        : m_single_file_helper{
            u8"Import configuration – Columns UI",
            u8"No file to import specified.",
            u8"Too many files to import specified. You can only import one file at a time, "
            "and should use double quotes around paths containing spaces."
        }
    {}

    result on_token(const char * token) override
    {
        m_is_quiet = !stricmp_utf8(token, u8"/columnsui:import-quiet");
        const auto is_import = m_is_quiet || !stricmp_utf8(token, u8"/columnsui:import");

        if (!is_import)
            return RESULT_NOT_OURS;

        m_single_file_helper.reset();

        return RESULT_PROCESSED_EXPECT_FILES;
    }

    void on_file(const char * url) override 
    {
        m_single_file_helper.add_file(url);
    }
    void on_files_done() override
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
            static_api_ptr_t<ui_control>()->activate();
            if (uMessageBox(main_window, formatter
                << u8"Are you sure you want to import "
                << pfc::string_filename_ext(path)
                << u8"? Your current Columns UI configuration will be lost.",
                u8"Import configuration",
                MB_YESNO) == IDNO) {
                return;
            }

        }
        g_import_layout(core_api::get_main_window(), path, is_quiet);
    }
};

class ExportCommandLineHandler : public commandline_handler {
public:
    bool m_is_quiet = false;
    CommandLineSingleFileHelper m_single_file_helper;

    ExportCommandLineHandler()
        : m_single_file_helper{
        u8"Export configuration – Columns UI",
        u8"No file to export to specified.",
        u8"Too many destination files specified. You must specify only one destination path, "
        "and should use double quotes around paths containing spaces."
    }
    {}

    result on_token(const char * token) override
    {
        m_is_quiet = !stricmp_utf8(token, u8"/columnsui:export-quiet");
        const auto is_import = m_is_quiet || !stricmp_utf8(token, u8"/columnsui:export");

        if (!is_import)
            return RESULT_NOT_OURS;

        m_single_file_helper.reset();

        return RESULT_PROCESSED_EXPECT_FILES;
    }

    void on_file(const char * url) override
    {
        m_single_file_helper.add_file(url);
    }
    void on_files_done() override
    {
        if (m_single_file_helper.validate_files())
            execute_export(m_single_file_helper.get_file(), m_is_quiet);
        m_single_file_helper.reset();
    }

    static void execute_export(const char* path, bool is_quiet)
    {
        if (!is_quiet) {
            static_api_ptr_t<ui_control>()->activate();
        }
        g_export_layout(core_api::get_main_window(), path, is_quiet);
    }
};

static commandline_handler_factory_t<HelpCommandLineHandler> help_cli_handler;
static commandline_handler_factory_t<ImportCommandLineHandler> import_cli_handler;
static commandline_handler_factory_t<ExportCommandLineHandler> export_cli_handler;
