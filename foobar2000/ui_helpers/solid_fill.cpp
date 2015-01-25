#include "stdafx.h"

LRESULT window_transparent_fill::on_message(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		SetLayeredWindowAttributes(wnd, 0, 255/2, LWA_ALPHA);
		return 0;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(wnd, &ps);
			FillRect(dc, &ps.rcPaint, GetSysColorBrush(COLOR_HIGHLIGHT));
			/*
			RECT rc_client;
			GetClientRect(wnd, &rc_client);
			//Offscreen rendering to eliminate flicker
			HDC dc_mem = CreateCompatibleDC(dc);

			//Create a rect same size of update rect
			HBITMAP bm_mem = CreateCompatibleBitmap(dc, RECT_CX(rc_client), RECT_CY(rc_client));

			HBITMAP bm_old = (HBITMAP)SelectObject(dc_mem, bm_mem);

			////we should always be erasing first, so shouldn't be needed
			//BitBlt(dc_mem, 0, 0, rc_client.right, rc_client.bottom, dc, 0, 0, SRCCOPY);
			gdi_object_t<HBRUSH>::ptr_t p_brush = CreateSolidBrush(RGB(200,200,200));//m_fill_colour);
			HBRUSH br_old = SelectBrush(dc_mem, p_brush.get());
			PatBlt(dc_mem, 0, 0, RECT_CX(rc_client), RECT_CY(rc_client), PATCOPY);
			SelectBrush(dc_mem, br_old);

			//BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top, RECT_CX(ps.rcPaint), RECT_CY(ps.rcPaint), dc_mem, 0, 0, SRCAND);
			BitBlt(dc, 0, 0, RECT_CX(rc_client), RECT_CY(rc_client), dc_mem, 0, 0, SRCAND);
			SelectObject(dc_mem, bm_old);
			DeleteObject(bm_mem);
			DeleteDC(dc_mem);

			p_brush.release();*/
			EndPaint(wnd, &ps);
			return 0;
		}
	}
	return DefWindowProc(wnd, msg, wp, lp);
};
