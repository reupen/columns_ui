#pragma once

namespace mmh
{

	class osversion
	{
	public:
		static bool is_windows_7_or_newer() {return test_osversion(6,1);}
		static bool is_windows_vista_or_newer() {return test_osversion(6,0);}
		static bool test_osversion(DWORD major, DWORD minor)
		{
			_initialise(); 
			return m_osvi.dwMajorVersion > major || ( m_osvi.dwMajorVersion == major && m_osvi.dwMinorVersion >= minor);
		}
	private:
		static void _initialise()
		{
			if (!m_initialised)
			{
				memset(&m_osvi, 0, sizeof (m_osvi));
				m_osvi.dwOSVersionInfoSize = sizeof(m_osvi);
				GetVersionEx((LPOSVERSIONINFO)&m_osvi);
				m_initialised = true;
			}
		}
		static OSVERSIONINFOEX m_osvi;
		static bool m_initialised;
	};

}