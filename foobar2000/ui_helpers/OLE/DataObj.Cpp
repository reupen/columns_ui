#include "stdafx.h"

/**************************************************************************
THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

File:          DataObj.cpp

Description:   

**************************************************************************/

/**************************************************************************
#include statements
**************************************************************************/

#include "DataObj.h"
#include "EnumFE.h"
//#include "Render.h"
#include <shlobj.h>

/**************************************************************************
global variables and definitions
**************************************************************************/

extern USHORT  g_cfFileContents;
extern USHORT  g_cfFileGroupDescriptor;

/**************************************************************************

CDataObject::CDataObject()

**************************************************************************/

IUnknown *GetCanonicalIUnknown(IUnknown *punk)
{
    IUnknown *punkCanonical;
    if (punk && SUCCEEDED(punk->QueryInterface(IID_IUnknown,
                                               (LPVOID*)&punkCanonical))) {
        punkCanonical->Release();
    } else {
        punkCanonical = punk;
    }
    return punkCanonical;
}

CDataObject::CDataObject()
{
	m_cRefCount = 0;
}

/**************************************************************************

CDataObject::~CDataObject()

**************************************************************************/

CDataObject::~CDataObject()
{
	{
		t_size i, count = m_data_entries.get_count();

		for(i = 0; i < count; i++)
			ReleaseStgMedium(&m_data_entries[i].sm);

	}
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

CDataObject::QueryInterface()

**************************************************************************/

STDMETHODIMP CDataObject::QueryInterface(REFIID riid, LPVOID *ppvOut)
{
	*ppvOut = NULL;

	//IUnknown
	if(IsEqualIID(riid, IID_IUnknown))
	{
		*ppvOut = this;
	}

	//IDataObject
	else if(IsEqualIID(riid, IID_IDataObject))
	{
		*ppvOut = (IDataObject*)this;
	}

	if(*ppvOut)
	{
		(*(LPUNKNOWN*)ppvOut)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

/**************************************************************************

CDataObject::AddRef()

**************************************************************************/

STDMETHODIMP_(ULONG) CDataObject::AddRef(void)
{
	return ++m_cRefCount;
}

/**************************************************************************

CDataObject::Release()

**************************************************************************/

STDMETHODIMP_(ULONG) CDataObject::Release(void)
{
	if(--m_cRefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_cRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDataObject Implementation
//

/**************************************************************************

CDataObject::GetData()

**************************************************************************/

HRESULT CDataObject::_GetStgMediumAddRef(t_size index, STGMEDIUM *pstgmOut)
{
	HRESULT hres = S_OK;
	STGMEDIUM * pstgmIn = &m_data_entries[index].sm;
	STGMEDIUM stgmOut = *pstgmIn;

	if (pstgmIn->pUnkForRelease == NULL && !(pstgmIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE))) 
		stgmOut.pUnkForRelease = static_cast<IDataObject*>(this);

	if (SUCCEEDED(hres)) 
	{
		switch (stgmOut.tymed)
		{
		case TYMED_ISTREAM:
			stgmOut.pstm->AddRef();
			break;
		case TYMED_ISTORAGE:
			stgmOut.pstg->AddRef();
			break;
		}
		if (stgmOut.pUnkForRelease)
			stgmOut.pUnkForRelease->AddRef();

		*pstgmOut = stgmOut;
	}
	return hres;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
	if(pFE == NULL || pSM == NULL)
		return E_INVALIDARG;

	ZeroMemory(pSM, sizeof(STGMEDIUM));

	t_size index;
	HRESULT hr = _FindFormatEtc(pFE, index, true);
	if (SUCCEEDED(hr))
	{
		hr = _GetStgMediumAddRef(index, pSM);
		//hr = CopyStgMedium(&m_data_entries[index].sm, pSM);

		//if(pSM->tymed == TYMED_HGLOBAL)
		//{
			//this tell's the caller not to free the global memory
			//QueryInterface(IID_IUnknown, (LPVOID*)&pSM->pUnkForRelease);
		//}
	}
	return hr;
}

/**************************************************************************

CDataObject::GetDataHere()

**************************************************************************/

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
	return E_NOTIMPL;
}

/**************************************************************************

CDataObject::QueryGetData()

**************************************************************************/

HRESULT CDataObject::_FindFormatEtc(LPFORMATETC pFE, t_size & index, bool b_checkTymed)
{
	if(!pFE)
		return DV_E_FORMATETC;

	if (pFE->ptd != NULL) return DV_E_DVTARGETDEVICE;


	index = 0;
	bool b_found;
	if (b_found = m_data_entries.bsearch_t(t_data_entry::g_compare_formatetc_value, pFE, index))
	{
		if (!b_checkTymed || (m_data_entries[index].fe.tymed & pFE->tymed)) //why & ?
		{
			return S_OK;
		}
		return DV_E_TYMED;
	}
	//console::formatter() << "_FindFormatEtc" << ": " << (int)pFE->cfFormat << " " << (int)pFE->dwAspect << " " << (int)pFE->lindex << " " << b_found << " " << index;
	return DV_E_FORMATETC;
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pFE)
{ 
	t_size index;
	return _FindFormatEtc(pFE, index, true);
}

/**************************************************************************

CDataObject::GetCanonicalFormatEtc()

**************************************************************************/

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC pFE1, LPFORMATETC pFE2)
{
	return DATA_S_SAMEFORMATETC;
}

/**************************************************************************

CDataObject::SetData()

**************************************************************************/

STDMETHODIMP CDataObject::SetData(LPFORMATETC pFE , LPSTGMEDIUM pSM, BOOL fRelease)
{
	if (pFE->ptd != NULL) return DV_E_DVTARGETDEVICE;

	HRESULT hr;
	t_size index = 0;
	if (FAILED(_FindFormatEtc(pFE, index, false)))
	{
		//index = m_data_entries.get_count();
		m_data_entries.insert_item(t_data_entry(), index);
		//console::formatter() << "Added Data: " << (unsigned)pFE->tymed;
	}
	else
		ReleaseStgMedium(&m_data_entries[index].sm);

	m_data_entries[index].fe = *pFE;
	memset(&m_data_entries[index].sm, 0, sizeof(STGMEDIUM));

	if(fRelease)
	{
		m_data_entries[index].sm = *pSM;
	}
	else
	{
		CopyStgMedium(pSM, &m_data_entries[index].sm);
	}

	/* Subtlety!  Break circular reference loop */
	if (GetCanonicalIUnknown(m_data_entries[index].sm.pUnkForRelease) ==
		GetCanonicalIUnknown(static_cast<IDataObject*>(this))) 
	{
		m_data_entries[index].sm.pUnkForRelease->Release();
		m_data_entries[index].sm.pUnkForRelease = NULL;
	}

	//for (t_size i = 0, count = m_data_entries.get_count(); i<count; i++)
	//	console::formatter() << i << ": " << (int)m_data_entries[i].fe.cfFormat << " " << (int)m_data_entries[i].fe.dwAspect << " " << (int)m_data_entries[i].fe.lindex;
	//m_data_entries.sort_t(t_data_entry::g_compare_formatetc);

	return S_OK;

}

/**************************************************************************

CDataObject::EnumFormatEtc()

**************************************************************************/

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
	//console::formatter() << "EnumFormatEtc";

	*ppEnum = NULL;

	switch(dwDir)
	{
	case DATADIR_GET:
		{
			t_size count = m_data_entries.get_count();
			pfc::array_staticsize_t<FORMATETC> data(count);
			for (t_size i=0; i<count; i++)
				data[i] = m_data_entries[i].fe;
			return SHCreateStdEnumFmtEtc(count, data.get_ptr(), ppEnum);
			//*ppEnum = new CEnumFormatEtc(data.get_ptr(), count);

			if(*ppEnum)
				return S_OK;

			break;
		}

	default:
		return OLE_S_USEREG;
	}

	return E_OUTOFMEMORY;
}

/**************************************************************************

CDataObject::DAdvise()

**************************************************************************/

STDMETHODIMP CDataObject::DAdvise(  LPFORMATETC pFE, 
								  DWORD advf, 
								  LPADVISESINK pAdvSink, 
								  LPDWORD pdwConnection)
{
	return E_NOTIMPL;
}

/**************************************************************************

CDataObject::DUnadvise()

**************************************************************************/

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}

/**************************************************************************

CDataObject::EnumDAdvise()

**************************************************************************/

STDMETHODIMP CDataObject::EnumDAdvise(LPENUMSTATDATA *ppenumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

