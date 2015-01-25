#include "stdafx.h"

namespace mmh
{

#ifdef __DSHOW_INCLUDED__

	HRESULT comptr_t<IGraphBuilder>::instantiate()
	{
		return instantiate(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER);
	}

#endif

}
