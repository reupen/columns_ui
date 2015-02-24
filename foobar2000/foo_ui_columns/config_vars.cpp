#include "stdafx.h"

namespace settings
{
	namespace guids
	{
		// {A1333C45-B247-4b84-AFBA-F5DAF50EBF33}
		const GUID show_status_pane = 
		{ 0xa1333c45, 0xb247, 0x4b84, { 0xaf, 0xba, 0xf5, 0xda, 0xf5, 0xe, 0xbf, 0x33 } };
	}

	cfg_bool show_status_pane(guids::show_status_pane, true);
}

