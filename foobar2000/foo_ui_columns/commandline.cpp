#include "stdafx.h"

static const char * g_help_text = "syntax: foobar2000 /columnsui:<command>\n\n"
	"Available commands:\n"
	"help,? - displays this commandline help\n"
	//"export:\"<path>\" - exports an fcs files\n"
	"import:\"<path>\" - imports an fcl or fcs file";

class commandline_handler_columns : public commandline_handler
{
private:
	enum 
	{
		COMMAND_IMPORT = 1,
		COMMAND_EXPORT,
	};
//	metadb_handle_list files;
//	pfc::ptr_list_t<char> commands;

	pfc::string8 path;
	unsigned command;

	void reset()
	{
		path.reset();
		command = 0;
	//	files.delete_all();
	//	commands.free_all();
	}

public:
	virtual result on_token(const char * token)
			{
		reset();
		if (!stricmp_utf8_partial(token,"/columnsui:"))
		{
			const char * ptr = strchr(token,':')+1;
			if (!stricmp_utf8(ptr,"help") || !stricmp_utf8(ptr,"?"))
			{
				HWND parent = core_api::get_main_window();
				if (parent) static_api_ptr_t<ui_control>()->activate();//SetActiveWindow(parent);
				uMessageBox(parent,g_help_text,"Columns UI commandline help",0);
				return RESULT_PROCESSED;
			}
			else if (!stricmp_utf8_partial(ptr,"import:"))
			{
				ptr += 7;
				if (*ptr && *ptr =='\"') ptr++;
				unsigned len = strlen(ptr);
				const char * end = strchr(ptr, '\"');
				if (end) len = end-ptr;

				pfc::string8 path(ptr, len);

				HWND parent = core_api::get_main_window();
				if (parent) static_api_ptr_t<ui_control>()->activate();

				pfc::string_extension ext(path);
				if (!stricmp_utf8("fcl", ext))
					g_import_layout2(core_api::get_main_window(), path);
				else if (!stricmp_utf8("fcs", ext))
					g_import(path);
				static_api_ptr_t<ui_control>()->activate();
				return RESULT_PROCESSED;
			}
			/*else if (!stricmp_utf8_partial(ptr,"export:"))
			{
				ptr += 7;
				if (*ptr && *ptr =='\"') ptr++;
				unsigned len = strlen(ptr);
				const char * end = strchr(ptr, '\"');
				if (end) len = end-ptr;

				pfc::string8 path(ptr, len);
				g_export(path);

				return RESULT_PROCESSED;
			}*/
		}
		return RESULT_NOT_OURS;
	}
	
	commandline_handler_columns() {}

	~commandline_handler_columns() {}
};

static commandline_handler_factory_t<commandline_handler_columns> asdfasdfadsf;
