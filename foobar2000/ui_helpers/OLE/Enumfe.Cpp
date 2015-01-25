#include "stdafx.h"

/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          EnumFE.cpp

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <shlobj.h>
#include "EnumFE.h"

/**************************************************************************

   CEnumFormatEtc::CEnumFormatEtc()

**************************************************************************/

CEnumFormatEtc::CEnumFormatEtc(LPFORMATETC pFE, int nItems)
{
m_cRefCount = 1;

m_iCur = 0;

m_pStrFE = new FORMATETC[nItems];

if (m_pStrFE)
    {
    CopyMemory((LPBYTE)m_pStrFE, (LPBYTE)pFE, sizeof(FORMATETC) * nItems);
    m_cItems = nItems;
    }
else
    {
    m_cItems = 0;
    }
}

/**************************************************************************

   CEnumFormatEtc::~CEnumFormatEtc()

**************************************************************************/

CEnumFormatEtc::~CEnumFormatEtc()
{
if(m_pStrFE)
   delete [] m_pStrFE;
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

   CEnumFormatEtc::QueryInterface()

**************************************************************************/

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, LPVOID *ppvOut)
{
*ppvOut = NULL;

//IUnknown
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppvOut = this;
   }

//IEnumFORMATETC
else if(IsEqualIID(riid, IID_IEnumFORMATETC))
   {
   *ppvOut = (IEnumFORMATETC*)this;
   }

if(*ppvOut)
   {
   (*(LPUNKNOWN*)ppvOut)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}

/**************************************************************************

   CEnumFormatEtc::AddRef()

**************************************************************************/

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
{
return ++m_cRefCount;
}

/**************************************************************************

   CEnumFormatEtc::Release()

**************************************************************************/

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
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
// IEnumFORMATETC Implementation
//

/**************************************************************************

   CEnumFormatEtc::Next()

**************************************************************************/

STDMETHODIMP CEnumFormatEtc::Next(ULONG celt, LPFORMATETC pFE, ULONG *puFetched)
{
ULONG cReturn = 0L;

if(celt <= 0 || pFE == NULL || m_iCur >= m_cItems)
   return S_FALSE;

if(puFetched == NULL && celt != 1)
   return S_FALSE;

if(puFetched != NULL)
   *puFetched = 0;

while(m_iCur < m_cItems && celt > 0)
   {
   *pFE++ = m_pStrFE[m_iCur++];
   cReturn++;
   celt--;
   }

if(NULL != puFetched)
   *puFetched = (cReturn - celt);

return S_OK;
}
   
/**************************************************************************

   CEnumFormatEtc::Skip()

**************************************************************************/

STDMETHODIMP CEnumFormatEtc::Skip(ULONG celt)
{
if((m_iCur + celt) >= m_cItems)
   return S_FALSE;

m_iCur += celt;

return S_OK;
}

/**************************************************************************

   CEnumFormatEtc::Reset()

**************************************************************************/

STDMETHODIMP CEnumFormatEtc::Reset(void)
{
m_iCur = 0;

return S_OK;
}
               
/**************************************************************************

   CEnumFormatEtc::Clone()

**************************************************************************/

STDMETHODIMP CEnumFormatEtc::Clone(IEnumFORMATETC **ppCloneEnumFormatEtc)
{
CEnumFormatEtc *newEnum;

if(NULL == ppCloneEnumFormatEtc)
   return S_FALSE;
      
newEnum = new CEnumFormatEtc(m_pStrFE, m_cItems);

if(newEnum ==NULL)
   return E_OUTOFMEMORY;

newEnum->AddRef();
newEnum->m_iCur = m_iCur;

*ppCloneEnumFormatEtc = newEnum;

return S_OK;
}
