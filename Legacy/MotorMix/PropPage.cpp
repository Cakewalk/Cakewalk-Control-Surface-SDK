/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// PropPage.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropPage.h"


/////////////////////////////////////////////////////////////////////////
CControlSurfacePropPageWrapper::CControlSurfacePropPageWrapper() :
	m_pPropPage( NULL )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_pPropPage = new CControlSurfacePropPage();
}

/////////////////////////////////////////////////////////////////////////
CControlSurfacePropPageWrapper::~CControlSurfacePropPageWrapper()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pPropPage)
		delete m_pPropPage;
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::SetPageSite(IPropertyPageSite* pPageSite)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.SetPageSite( pPageSite );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Activate(HWND hwndParent,LPCRECT prect,BOOL fModal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Activate( hwndParent, prect, fModal );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Deactivate(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Deactivate();
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.GetPageInfo( pPageInfo );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.SetObjects( cObjects, ppUnk );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Show(UINT nCmdShow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Show( nCmdShow );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Move(LPCRECT prect)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Move( prect );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::IsPageDirty(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.IsPageDirty();
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Apply(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Apply();
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::Help(LPCWSTR lpszHelpDir)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.Help( lpszHelpDir );
}

/////////////////////////////////////////////////////////////////////////
STDMETHODIMP CControlSurfacePropPageWrapper::TranslateAccelerator(LPMSG lpMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_pPropPage->m_xPropertyPage.TranslateAccelerator( lpMsg );
}


