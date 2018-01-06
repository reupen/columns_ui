#include "stdafx.h"
#include "fcl.h"

static const char* g_help_text = u8"syntax: foobar2000 /columnsui:<command> \"<path>\"\n\n"
    "Available commands:\n"
    "help, ? – displays this command-line help\n"
    "import – imports an fcl file\n"
    "import-quiet – imports an fcl file without confirmation dialog boxes";

class commandline_handler_help : public commandline_handler
{
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
        if (parent)
            static_api_ptr_t<ui_control>()->activate();
        uMessageBox(parent, g_help_text, u8"Columns UI command-line help", 0);
    }
};

class commandline_handler_columns : public commandline_handler
{
public:
    bool m_is_quiet = false;
    pfc::list_t<pfc::string8> m_files;
    result on_token(const char * token) override
    {
        m_is_quiet = !stricmp_utf8(token, u8"/columnsui:import-quiet");
        const auto is_import = m_is_quiet || !stricmp_utf8(token, u8"/columnsui:import");

        if (!is_import)
            return RESULT_NOT_OURS;

        m_files.remove_all();

        return RESULT_PROCESSED_EXPECT_FILES;
    }

    void on_file(const char * url) override 
    {
        m_files.add_item(url);
    }
    void on_files_done() override
    {
        const auto main_window = core_api::get_main_window();
        if (m_files.get_count() == 0) {
            uMessageBox(
                main_window,
                u8"No file to import specified.", 
                u8"Import configuration – Columns UI", 
                MB_ICONERROR);
            return;
        }
        if (m_files.get_count() > 1) {
            uMessageBox(
                main_window,
                u8"Too many files to import specified. You can only import one file at a time, "
                "and should use double quotes around paths containing spaces.",
                u8"Import configuration – Columns UI",
                MB_ICONERROR);
            m_files.remove_all();
            return;
        }
        execute_import(m_files[0], m_is_quiet);
        m_files.remove_all();
    }

    static void execute_import(const char* path, bool is_quiet)
    {
        const auto main_window = core_api::get_main_window();
        if (main_window)
            static_api_ptr_t<ui_control>()->activate();

        pfc::string_formatter formatter;
        if (stricmp_utf8("fcl", pfc::string_extension(path))) {
            uMessageBox(main_window, formatter
                << u8"\""
                << pfc::string_filename_ext(path)
                << u8"\" is not an FCL file. An FCL file should have the .fcl extension.", 
                u8"Import configuration",
                MB_ICONERROR);
            return;
        }
        if (is_quiet || uMessageBox(main_window, formatter
            << u8"Are you sure you want to import \""
            << pfc::string_filename_ext(path)
            << u8"\"? Your current Columns UI configuration will be lost.",
            u8"Import configuration",
            MB_YESNO) == IDYES)
        {
            g_import_layout(core_api::get_main_window(), path, is_quiet);
        }
    }
};

static commandline_handler_factory_t<commandline_handler_help> help_cli_handler;
static commandline_handler_factory_t<commandline_handler_columns> import_cli_handler;
