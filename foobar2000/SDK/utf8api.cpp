#include "foobar2000.h"

HWND uCreateDialog(UINT id,HWND parent,DLGPROC proc,LPARAM param)
{
	return CreateDialogParam(core_api::get_my_instance(),MAKEINTRESOURCE(id),parent,proc,param);
}

int uDialogBox(UINT id,HWND parent,DLGPROC proc,LPARAM param)
{
	return (int)DialogBoxParam(core_api::get_my_instance(),MAKEINTRESOURCE(id),parent,proc,param);
}
