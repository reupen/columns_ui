#include "stdafx.h"
#include "fcl.h"

static const char * g_help_text = "syntax: foobar2000 /columnsui:<command>\n\n"
    "Available commands:\n"
    "help, ? - displays this command-line help\n"
    "import:\"<path>\" - imports an fcl or fcs file\n"
    "import-quiet:\"<path>\" - imports an fcl file without confirmation dialog boxes";

class commandline_handler_columns : public commandline_handler
{
private:
    enum 
    {
        command_help = 1,
        command_import,
        command_import_quiet
    };

public:
    // This could probably be improved...
    result on_token(const char * token) override
    {
        if (!stricmp_utf8_partial(token,"/columnsui:"))
        {
            unsigned command = NULL;
            const char * ptr = strchr(token,':')+1;
            if (!stricmp_utf8(ptr, "help") || !stricmp_utf8(ptr, "?"))
                command = command_help;
            else if (!stricmp_utf8_partial(ptr, "import:")) {
                command = command_import;
                ptr += 7;
            }
            else if (!stricmp_utf8_partial(ptr, "import-quiet:")) {
                command = command_import_quiet;
                ptr += 13;
            }

            if (command == command_help)
            {
                HWND parent = core_api::get_main_window();
                if (parent) static_api_ptr_t<ui_control>()->activate();//SetActiveWindow(parent);
                uMessageBox(parent,g_help_text,"Columns UI command-line help",0);
                return RESULT_PROCESSED;
            }
            else if (command == command_import || command == command_import_quiet)
            {
                if (*ptr && *ptr =='\"') ptr++;
                unsigned len = strlen(ptr);
                const char * end = strchr(ptr, '\"');
                if (end) len = end-ptr;

                pfc::string8 path(ptr, len);

                HWND parent = core_api::get_main_window();
                if (parent) static_api_ptr_t<ui_control>()->activate();

                if (!stricmp_utf8("fcl", pfc::string_extension(path))) {
                    pfc::string_formatter formatter;
                    if (command == command_import_quiet || uMessageBox(parent, formatter
                        << "Are you sure you want to import the "
                        << pfc::string_filename(path)
                        << " configuration? Your current configuration will be lost.", "Import configuration?", MB_YESNO) == IDYES)
                    {
                        g_import_layout(core_api::get_main_window(), path, command == command_import_quiet);
                    }

                }

                return RESULT_PROCESSED;
            }
        }
        return RESULT_NOT_OURS;
    }
    
    commandline_handler_columns() {}

    ~commandline_handler_columns() = default;
};

static commandline_handler_factory_t<commandline_handler_columns> asdfasdfadsf;
