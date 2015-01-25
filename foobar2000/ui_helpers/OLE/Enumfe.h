/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          EnumFE.h

   Description:   

**************************************************************************/

#ifndef ENUMFE_H
#define ENUMFE_H
 
/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <ole2.h>

/**************************************************************************
   class definitions
**************************************************************************/

class CEnumFormatEtc : public IEnumFORMATETC
{
private:
   DWORD m_cRefCount;
   LPFORMATETC m_pStrFE;
   ULONG m_iCur;
   ULONG m_cItems;

public:
   CEnumFormatEtc(LPFORMATETC pFE,int numberItems);
   ~CEnumFormatEtc(void);

   //BOOL FInit(HWND);

   //IUnknown members
   STDMETHOD(QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   //IEnumFORMATETC members
   STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG*);
   STDMETHOD(Skip)(ULONG);
   STDMETHOD(Reset)(void);
   STDMETHOD(Clone)(IEnumFORMATETC**);
};

#endif   //ENUMFE_H
