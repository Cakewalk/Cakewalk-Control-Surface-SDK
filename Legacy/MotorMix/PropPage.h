/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// PropPage.h
/////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols
#include "ControlSurfacePropPage.h"

class CControlSurfacePropPageWrapper :
	public IPropertyPage,
	public CComObjectRoot,
	public CComCoClass<CControlSurfacePropPageWrapper,&CLSID_ControlSurfacePropPage>
{
public:
	CControlSurfacePropPageWrapper();
	~CControlSurfacePropPageWrapper();
BEGIN_COM_MAP(CControlSurfacePropPageWrapper)
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

	// *** IPropertyPage ***
	STDMETHODIMP SetPageSite(IPropertyPageSite* pPageSite);
	STDMETHODIMP Activate(HWND hwndParent,LPCRECT prect,BOOL fModal);
	STDMETHODIMP Deactivate(void);
	STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
	STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
	STDMETHODIMP Show(UINT nCmdShow);
	STDMETHODIMP Move(LPCRECT prect);
	STDMETHODIMP IsPageDirty(void);
	STDMETHODIMP Apply(void);
	STDMETHODIMP Help(LPCWSTR lpszHelpDir);
	STDMETHODIMP TranslateAccelerator(LPMSG lpMsg);


DECLARE_REGISTRY_RESOURCEID(IDR_ControlSurfacePropPage)

private:
	CControlSurfacePropPage* m_pPropPage;
};
