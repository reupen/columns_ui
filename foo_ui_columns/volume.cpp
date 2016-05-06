#include "stdafx.h"
#include "volume.h"

class volume_panel_class_name
{
public:
	static const TCHAR * const get_class_name()
	{
		return _T("volume_toolbar");
	}
	static bool get_show_caption() {return false;}
	static COLORREF get_background_colour() {return -1;}
};

class volume_control_panel : public volume_control_t<false, false, volume_panel_class_name, uie::container_ui_extension_t<> >
{
	virtual const GUID & get_extension_guid() const
	{
		// {B3259290-CB68-4d37-B0F1-8094862A9524}
		static const GUID ret = 
		{ 0xb3259290, 0xcb68, 0x4d37, { 0xb0, 0xf1, 0x80, 0x94, 0x86, 0x2a, 0x95, 0x24 } };
		return ret;
	};

	virtual void get_name(pfc::string_base & out)const
	{
		out = "Volume";
	}
	virtual void get_category(pfc::string_base & out)const
	{
		out = "Toolbars";
	}

	virtual unsigned get_type  () const
	{
		return uie::type_toolbar;
	}
};

uie::window_factory<volume_control_panel> g_volume_panel;

#if 0
LONG g_exception_filter_delay_load(PEXCEPTION_POINTERS pExcPointers) {
   LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;  
   PDelayLoadInfo pDelayLoadInfo = 
	PDelayLoadInfo(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

   switch (pExcPointers->ExceptionRecord->ExceptionCode) {
   case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
	  break;
   case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
	  break; 
   default:
	  lDisposition = EXCEPTION_CONTINUE_SEARCH; 
	  break;
   }
   return(lDisposition);
}
#endif

static FARPROC WINAPI g_delay_load_hook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	if (dliNotify == dliFailLoadLib)
		throw exception_delayload_module_not_found(pdli);
	else if (dliNotify == dliFailGetProc)
		throw exception_delayload_procedure_not_found(pdli);
	else
		throw exception_delayload(pdli);
}
/*						if (init_gdiplus(&m_Gdiplus_token))
							m_using_gdiplus = true;

bool init_gdiplus(ULONG_PTR *token)
{
	try
	{
		png_structp png_ptr = NULL;
		png_get_libpng_ver(png_ptr);
		return Gdiplus::Ok == Gdiplus::GdiplusStartup(token, &Gdiplus::GdiplusStartupInput(), NULL);
	}
	catch (const exception_delayload & p_err)
	{
		console::formatter() << "foo_ui_columns: failed to load GDI+ (" << p_err.what() << "), falling back to GDI";
		return false;
	}
}
*/

PfnDliHook __pfnDliFailureHook2 = &g_delay_load_hook;