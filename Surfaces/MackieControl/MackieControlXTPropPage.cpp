/////////////////////////////////////////////////////////////////////////////
// MackieControlXTPropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "MackieControlXT."
//

#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

#include "MackieControlLCDDisplay.h"
#include "MackieControlVPotDisplay.h"
#include "MackieControlFader.h"
#include "MackieControlXT.h"
#include "MackieControlXTPropPage.h"

#include "HtmlHelp.h"

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlXTPropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CMackieControlXTPropPage::CMackieControlXTPropPage(CWnd* pParent /*=NULL*/)
: CDialog(CMackieControlXTPropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
	//{{AFX_DATA_INIT(CMackieControlXTPropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXTPropPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMackieControlXTPropPage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMackieControlXTPropPage, CDialog)
	//{{AFX_MSG_MAP(CMackieControlXTPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CMackieControlXTPropPage::~CMackieControlXTPropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CMackieControlXTPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CMackieControlXT* const pSurface = dynamic_cast<CMackieControlXT*>(*ppUnk);
		if (NULL == pSurface)
			return E_POINTER;
		
		// If different than previous object, release the old one
		if (m_pSurface != pSurface && m_pSurface != NULL)
			m_pSurface->Release();

		m_pSurface = pSurface;
		m_pSurface->AddRef();

		return S_OK;
	}
	else if (cObjects == 0)
	{
		if (m_pSurface == NULL)
			return E_UNEXPECTED;

		m_pSurface->Release();
		m_pSurface = NULL;

		return S_OK;
	}
	else
		return E_UNEXPECTED;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	if (!pRect)
		return E_POINTER;
	if (NULL != m_hWnd)
		return E_UNEXPECTED;	// already active!

	if (!CreateMFCDialog( hwndParent ))
		return E_OUTOFMEMORY;

	// Parent should control us so the user can tab out of property page
	DWORD dwStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	dwStyle = dwStyle | WS_EX_CONTROLPARENT;
	SetWindowLong( m_hWnd, GWL_EXSTYLE, dwStyle );

	// Move page into position and show it.
	Move( pRect );
	Show( SW_SHOWNORMAL );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( nCmdShow );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
{
	if (pPageSite)
	{
		if (m_pPageSite)
			return E_UNEXPECTED;
		m_pPageSite = pPageSite;
		m_pPageSite->AddRef();
	}
	else
	{
		if (m_pPageSite == NULL)
			return E_UNEXPECTED;
		m_pPageSite->Release();
		m_pPageSite = NULL;
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Returning E_NOTIMPL here should be enough to cause the help file
	// specified by GetPageInfo to be used, but it doesn't seem to work

	TCHAR szDLL[_MAX_PATH] = {0};
	DWORD dwLen = ::GetModuleFileName(theApp.m_hInstance, szDLL, _MAX_PATH);
	if (dwLen < 3)
	    return E_FAIL;

	// OK not to use strlcpy here
	_tcscpy(szDLL + dwLen - 3, _T("chm"));

	::HtmlHelp(m_hWnd, szDLL, HH_DISPLAY_TOPIC, NULL);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "MackieControlXT";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = CreateDialogParam( theApp.m_hInstance, MAKEINTRESOURCE( IDD_PROPPAGE_XT ), 
													 ::GetDesktopWindow(), DialogProc, 0 );
	if (hWnd)
	{
		// Get the dialog size and destroy the window
		RECT rc;
		::GetWindowRect( hWnd, &rc );
		pPageInfo->size.cx = rc.right - rc.left;
		pPageInfo->size.cy = rc.bottom - rc.top;
		::DestroyWindow( hWnd );
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// End of IPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

