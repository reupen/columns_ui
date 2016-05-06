#pragma once

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <vector>
#include <memory>
#include <complex>
#include <iostream>

#define OEMRESOURCE

#include "../foobar2000/SDK/foobar2000.h"
#include "../foobar2000/SDK/core_api.h"

#include "uxtheme.h"
#include "Wincodec.h"
#include "../columns_ui-sdk/ui_extension.h"
#include "../ui_helpers/stdafx.h"
#include "../mmh/stdafx.h"
#include "../foobar2000/helpers/helpers.h"
#include "resource.h"
#include "utf8api.h"
#include "helpers.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <SHLWAPI.H>
#include "zmouse.h"

/*#include <tmschema.h>*/
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <io.h>
#include <share.h>

#include "main_window.h"

#include "config_items.h"

#include "gdiplus.h"
#include "utf8api.h"
#include "common.h"
#include "menu_helpers.h"
#include "config_vars.h"
#include "config_defaults.h"
#include "prefs_utils.h"

#include "list_view_panel.h"
#include "callback.h"
#include "status_bar.h"
#include "columns_v2.h"
#include "cache.h"
#include "rebar.h"
#include "font_notify.h"
#include "sort.h"
#include "config.h"
#include "splitter.h"
#include "layout.h"

#include "playlist_search.h"
#include "playlist_view.h"
#include "fcs.h"
#include "playlist_manager_utils.h"
#include "playlist_switcher.h"
#include "playlist_switcher_v2.h"
#include "playlist_tabs.h"
#include "seekbar.h"
#include "vis_gen_host.h"
#include "volume.h"
#include "splitter_tabs.h"
#include "filter.h"
#include "get_msg_hook.h"
#include "setup_dialog.h"
#include "buttons.h"
