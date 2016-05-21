#pragma once

#define OEMRESOURCE

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <vector>
#include <memory>
#include <complex>
#include <iostream>
#include <io.h>
#include <share.h>
#include <utility>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <winuser.h>
#include <gdiplus.h>
#include <zmouse.h>
#include <uxtheme.h>
#include <wincodec.h>
#include <strsafe.h>

#include "../foobar2000/SDK/foobar2000.h"
#include "../foobar2000/SDK/core_api.h"
#include "../foobar2000/helpers/helpers.h"

#include "../columns_ui-sdk/ui_extension.h"
#include "../ui_helpers/stdafx.h"
#include "../mmh/stdafx.h"

#include "resource.h"
#include "utf8api.h"
#include "helpers.h"
#include "config_items.h"
#include "gdiplus.h"
#include "config_vars.h"
#include "config_defaults.h"
#include "extern.h"

