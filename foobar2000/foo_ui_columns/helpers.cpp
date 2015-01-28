#include "foo_ui_columns.h"

void g_ui_selection_manager_register_callback_no_now_playing_fallback(ui_selection_callback * p_callback)
{
	if (static_api_test_t<ui_selection_manager_v2>())
		static_api_ptr_t<ui_selection_manager_v2>()->register_callback(p_callback, ui_selection_manager_v2::flag_no_now_playing);
	else
		static_api_ptr_t<ui_selection_manager>()->register_callback(p_callback);
}

bool g_ui_selection_manager_is_now_playing_fallback()
{
	if (static_api_test_t<ui_selection_manager_v2>())
		return false;
	else
		return static_api_ptr_t<ui_selection_manager>()->get_selection_type() == contextmenu_item::caller_now_playing;
}


void g_compare_file_with_bytes(const service_ptr_t<file> & p1, const pfc::array_t<t_uint8> & p2, bool & b_same, abort_callback & p_abort)
{
	try
	{
		b_same = false;
		t_filesize bytes;
		bytes = p1->get_size(p_abort);

		if (bytes == p2.get_size())
		{

			enum { BUFSIZE = 1024 * 1024 };
			unsigned size = (unsigned)(BUFSIZE<bytes ? BUFSIZE : bytes);
			pfc::array_t<t_uint8> temp, temp2;
			temp.set_size(size); temp2.set_size(size);

			unsigned io_bytes_done;
			t_filesize done = 0;
			while (done<bytes)
			{
				if (p_abort.is_aborting()) throw exception_aborted();

				t_int64 delta64 = bytes - done;
				if (delta64>BUFSIZE) delta64 = BUFSIZE;
				unsigned delta = (unsigned)delta64;

				io_bytes_done = p1->read(temp.get_ptr(), delta, p_abort);

				if (io_bytes_done <= 0) break;

				if (io_bytes_done != delta)
					throw exception_io();

				if (memcmp(temp.get_ptr(), (char*)p2.get_ptr() + done, io_bytes_done))
					return;

				done += delta;
			}
			b_same = true;
		}
	}
	catch (const exception_io &)
	{
		return;
	}
}

HRESULT g_get_comctl32_version(DLLVERSIONINFO2 & p_dvi)
{
	static bool have_version = false;
	static HRESULT rv = E_FAIL;

	static DLLVERSIONINFO2 g_dvi;

	if (!have_version)
	{
		HINSTANCE hinstDll = LoadLibrary(_T("comctl32.dll"));

		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");


			if (pDllGetVersion)
			{

				memset(&g_dvi, 0, sizeof(DLLVERSIONINFO2));
				g_dvi.info1.cbSize = sizeof(DLLVERSIONINFO2);

				rv = (*pDllGetVersion)(&g_dvi.info1);

				if (FAILED(rv))
				{
					memset(&g_dvi, 0, sizeof(DLLVERSIONINFO));
					g_dvi.info1.cbSize = sizeof(DLLVERSIONINFO);

					rv = (*pDllGetVersion)(&g_dvi.info1);
				}
			}

			FreeLibrary(hinstDll);
		}
		have_version = true;
	}
	p_dvi = g_dvi;
	return rv;
}
