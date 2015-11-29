#ifndef _COLUMNS_UI_PLAYLIST_SWITCHER_H_
#define _COLUMNS_UI_PLAYLIST_SWITCHER_H_

#include "extern.h"

namespace playlist_switcher
{

	namespace colours 
	{
		class config_inactive_selection_text_t : public config_item_t<COLORREF>
		{
		public:
			virtual COLORREF get_default_value ();
			virtual void on_change();
			virtual const GUID & get_guid();
			config_inactive_selection_text_t();
		};

		extern config_inactive_selection_text_t config_inactive_selection_text;
	};
};

#endif