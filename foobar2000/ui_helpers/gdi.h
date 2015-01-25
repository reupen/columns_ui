#pragma once

namespace ui_helpers
{
	class MemoryDC
	{
	public:
		MemoryDC(const PAINTSTRUCT & ps)
		{
			m_rc = ps.rcPaint;
			m_dc = ps.hdc;
			m_dc_mem = CreateCompatibleDC(m_dc);
			m_bm_mem = CreateCompatibleBitmap(m_dc, RECT_CX(m_rc), RECT_CY(m_rc));
			m_bm_old = SelectBitmap(m_dc_mem, m_bm_mem);

			OffsetWindowOrgEx(m_dc_mem, m_rc.left, m_rc.top, NULL);
		}
		~MemoryDC()
		{
			OffsetWindowOrgEx(m_dc_mem, -m_rc.left, -m_rc.top, NULL);
			BitBlt(m_dc,	m_rc.left, m_rc.top, RECT_CX(m_rc), RECT_CY(m_rc),
				m_dc_mem, 0, 0, SRCCOPY);

			SelectObject(m_dc_mem, m_bm_old);
			DeleteObject(m_bm_mem);
			DeleteDC(m_dc_mem);
		}
		operator HDC () const {return m_dc_mem;};
	private:
		RECT m_rc;
		HDC m_dc, m_dc_mem;
		HBITMAP m_bm_mem, m_bm_old;
	};

	class PaintScope
	{
	public:
		PaintScope(HWND wnd) : m_wnd(wnd)
		{
			memset(&m_ps, 0, sizeof(PAINTSTRUCT));
			BeginPaint(m_wnd, &m_ps);
		};
		~PaintScope()
		{
			EndPaint(m_wnd, &m_ps);
		};
		operator const PAINTSTRUCT &  ()const {return m_ps;};
		const PAINTSTRUCT * operator->  ()const {return &m_ps;};
	private:
		PAINTSTRUCT m_ps;
		HWND m_wnd;
	};

	class DisableRedrawScope
	{
	public:
		DisableRedrawScope(HWND wnd)
		{
			m_wnd = wnd;
			m_active = m_wnd && IsWindowVisible(m_wnd);
			if (m_active)
				SendMessage(m_wnd, WM_SETREDRAW, FALSE, NULL);
		}
		~DisableRedrawScope()
		{
			if (m_active)
			{
				SendMessage(m_wnd, WM_SETREDRAW, TRUE, NULL);
				RedrawWindow(m_wnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_FRAME|RDW_ERASE);
			}
		}
	private:
		bool m_active;
		HWND m_wnd;
	};

};