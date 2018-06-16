/////////////////////////////////////////////////////////////////////////////
// MackieControlC4PropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "MackieControlC4."
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
#include "MackieControlC4.h"
#include "MackieControlC4PropPage.h"

#include "HtmlHelp.h"

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlC4PropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CMackieControlC4PropPage::CMackieControlC4PropPage(CWnd* pParent /*=NULL*/)
: CDialog(CMackieControlC4PropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
	m_bInitDone = false;
	m_bIgnoreSave = false;

	//{{AFX_DATA_INIT(CMackieControlC4PropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate == FALSE)
		TransferSettings(false);

	//{{AFX_DATA_MAP(CMackieControlC4PropPage)
	DDX_Control(pDX, IDC_DISP8, m_cDisp8);
	DDX_Control(pDX, IDC_DISP7, m_cDisp7);
	DDX_Control(pDX, IDC_DISP6, m_cDisp6);
	DDX_Control(pDX, IDC_DISP5, m_cDisp5);
	DDX_Control(pDX, IDC_DISP4, m_cDisp4);
	DDX_Control(pDX, IDC_DISP3, m_cDisp3);
	DDX_Control(pDX, IDC_DISP2, m_cDisp2);
	DDX_Control(pDX, IDC_DISP1, m_cDisp1);
	DDX_Control(pDX, IDC_FUNCTION8, m_cFunction8);
	DDX_Control(pDX, IDC_FUNCTION7, m_cFunction7);
	DDX_Control(pDX, IDC_FUNCTION6, m_cFunction6);
	DDX_Control(pDX, IDC_FUNCTION5, m_cFunction5);
	DDX_Control(pDX, IDC_FUNCTION4, m_cFunction4);
	DDX_Control(pDX, IDC_FUNCTION3, m_cFunction3);
	DDX_Control(pDX, IDC_FUNCTION2, m_cFunction2);
	DDX_Control(pDX, IDC_FUNCTION1, m_cFunction1);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate == TRUE)
		TransferSettings(true);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMackieControlC4PropPage, CDialog)
	//{{AFX_MSG_MAP(CMackieControlC4PropPage)
	ON_CBN_SELCHANGE(IDC_FUNCTION1, OnSelchangeFunction1)
	ON_CBN_SELCHANGE(IDC_FUNCTION2, OnSelchangeFunction2)
	ON_CBN_SELCHANGE(IDC_FUNCTION3, OnSelchangeFunction3)
	ON_CBN_SELCHANGE(IDC_FUNCTION4, OnSelchangeFunction4)
	ON_CBN_SELCHANGE(IDC_FUNCTION5, OnSelchangeFunction5)
	ON_CBN_SELCHANGE(IDC_FUNCTION6, OnSelchangeFunction6)
	ON_CBN_SELCHANGE(IDC_FUNCTION7, OnSelchangeFunction7)
	ON_CBN_SELCHANGE(IDC_FUNCTION8, OnSelchangeFunction8)
	ON_EN_UPDATE(IDC_DISP1, OnUpdateDisp1)
	ON_EN_UPDATE(IDC_DISP2, OnUpdateDisp2)
	ON_EN_UPDATE(IDC_DISP3, OnUpdateDisp3)
	ON_EN_UPDATE(IDC_DISP4, OnUpdateDisp4)
	ON_EN_UPDATE(IDC_DISP5, OnUpdateDisp5)
	ON_EN_UPDATE(IDC_DISP6, OnUpdateDisp6)
	ON_EN_UPDATE(IDC_DISP7, OnUpdateDisp7)
	ON_EN_UPDATE(IDC_DISP8, OnUpdateDisp8)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CMackieControlC4PropPage::~CMackieControlC4PropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CMackieControlC4PropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
//	TRACE("CMackieControlC4PropPage::SetObjects()\n");

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CMackieControlC4* const pSurface = dynamic_cast<CMackieControlC4*>(*ppUnk);
		if (NULL == pSurface)
			return E_POINTER;
		
		// If different than previous object, release the old one
		if (m_pSurface != pSurface && m_pSurface != NULL)
			m_pSurface->Release();

		m_pSurface = pSurface;
		m_pSurface->AddRef();

		// Don't call UpdateData() here, it blows up badly,
		// instead transfer the data directly
		TransferSettings(false);

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

HRESULT CMackieControlC4PropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
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

HRESULT CMackieControlC4PropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::Show( UINT nCmdShow )
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

HRESULT CMackieControlC4PropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CMackieControlC4PropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::Help( LPCWSTR lpszHelpDir )
{
	// Returning E_NOTIMPL here should be enough to cause the help file
	// specified by GetPageInfo to be used, but it doesn't seem to work

	TCHAR szDLL[_MAX_PATH];

	DWORD dwLen = ::GetModuleFileName(theApp.m_hInstance, szDLL, _countof(szDLL));
	if (dwLen < 3)
	    return E_FAIL;

	// OK not to use strlcpy here
	_tcscpy(szDLL + dwLen - 3, _T("chm"));

	::HtmlHelp(m_hWnd, szDLL, HH_DISPLAY_TOPIC, NULL);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "MackieControlC4";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = CreateDialogParam( theApp.m_hInstance, MAKEINTRESOURCE( IDD_PROPPAGE_C4 ), 
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

void CMackieControlC4PropPage::TransferSettings(bool bSave)
{
	TRACE("CMackieControlC4PropPage::TransferSettings()...");

	if (!m_bInitDone)
	{
		TRACE("Init not done yet\n");
		return;
	}

	if (NULL == m_pSurface)
	{
		TRACE("m_pSurface is NULL\n");
		return;
	}

	if (m_bIgnoreSave)
		return;

	if (bSave)
	{
		TRACE("Saving\n");

		// Function keys
		m_pSurface->SetFunctionKey(0, (DWORD)m_cFunction1.GetItemData(m_cFunction1.GetCurSel()));
		m_pSurface->SetFunctionKey(1, (DWORD)m_cFunction2.GetItemData(m_cFunction2.GetCurSel()));
		m_pSurface->SetFunctionKey(2, (DWORD)m_cFunction3.GetItemData(m_cFunction3.GetCurSel()));
		m_pSurface->SetFunctionKey(3, (DWORD)m_cFunction4.GetItemData(m_cFunction4.GetCurSel()));
		m_pSurface->SetFunctionKey(4, (DWORD)m_cFunction5.GetItemData(m_cFunction5.GetCurSel()));
		m_pSurface->SetFunctionKey(5, (DWORD)m_cFunction6.GetItemData(m_cFunction6.GetCurSel()));
		m_pSurface->SetFunctionKey(6, (DWORD)m_cFunction7.GetItemData(m_cFunction7.GetCurSel()));
		m_pSurface->SetFunctionKey(7, (DWORD)m_cFunction8.GetItemData(m_cFunction8.GetCurSel()));

		CString str;
		m_cDisp1.GetWindowText(str); m_pSurface->SetFunctionKeyName(0, str);
		m_cDisp2.GetWindowText(str); m_pSurface->SetFunctionKeyName(1, str);
		m_cDisp3.GetWindowText(str); m_pSurface->SetFunctionKeyName(2, str);
		m_cDisp4.GetWindowText(str); m_pSurface->SetFunctionKeyName(3, str);
		m_cDisp5.GetWindowText(str); m_pSurface->SetFunctionKeyName(4, str);
		m_cDisp6.GetWindowText(str); m_pSurface->SetFunctionKeyName(5, str);
		m_cDisp7.GetWindowText(str); m_pSurface->SetFunctionKeyName(6, str);
		m_cDisp8.GetWindowText(str); m_pSurface->SetFunctionKeyName(7, str);
	}
	else
	{
		TRACE("Loading\n");

		// Function keys
		SelectItemData(&m_cFunction1, m_pSurface->GetFunctionKey(0));
		SelectItemData(&m_cFunction2, m_pSurface->GetFunctionKey(1));
		SelectItemData(&m_cFunction3, m_pSurface->GetFunctionKey(2));
		SelectItemData(&m_cFunction4, m_pSurface->GetFunctionKey(3));
		SelectItemData(&m_cFunction5, m_pSurface->GetFunctionKey(4));
		SelectItemData(&m_cFunction6, m_pSurface->GetFunctionKey(5));
		SelectItemData(&m_cFunction7, m_pSurface->GetFunctionKey(6));
		SelectItemData(&m_cFunction8, m_pSurface->GetFunctionKey(7));

		// The display text boxes will call UpdateData(TRUE)
		// when we try to set the text, so disable saving data
		// until we're all done
		m_bIgnoreSave = true;

		m_cDisp1.SetWindowText(m_pSurface->GetFunctionKeyName(0));
		m_cDisp2.SetWindowText(m_pSurface->GetFunctionKeyName(1));
		m_cDisp3.SetWindowText(m_pSurface->GetFunctionKeyName(2));
		m_cDisp4.SetWindowText(m_pSurface->GetFunctionKeyName(3));
		m_cDisp5.SetWindowText(m_pSurface->GetFunctionKeyName(4));
		m_cDisp6.SetWindowText(m_pSurface->GetFunctionKeyName(5));
		m_cDisp7.SetWindowText(m_pSurface->GetFunctionKeyName(6));
		m_cDisp8.SetWindowText(m_pSurface->GetFunctionKeyName(7));

		m_bIgnoreSave = false;
	}
}

////////////////////////////////////////////////////////////////////////////////

// Select the entry
//
void CMackieControlC4PropPage::SelectItemData(CComboBox *pBox, DWORD dwData)
{
	if (NULL == pBox || NULL == pBox->m_hWnd)
		return;

	pBox->Clear();

	int iNumItems = pBox->GetCount();

	for (int n = 0; n < iNumItems; n++)
	{
		if (pBox->GetItemData(n) == dwData)
		{
			pBox->SetCurSel(n);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

BOOL CMackieControlC4PropPage::OnInitDialog() 
{
//	TRACE("CMackieControlC4PropPage::OnInitDialog()\n");

	BOOL bRet = CDialog::OnInitDialog();
	
	// Get a pointer to the Sonar Commands interface
	ISonarCommands *pCommands = m_pSurface->GetSonarCommands();

	m_cDisp1.SetLimitText(6);
	m_cDisp2.SetLimitText(6);
	m_cDisp3.SetLimitText(6);
	m_cDisp4.SetLimitText(6);
	m_cDisp5.SetLimitText(6);
	m_cDisp6.SetLimitText(6);
	m_cDisp7.SetLimitText(6);
	m_cDisp8.SetLimitText(6);

	// Fill in the function keys combo box
	DWORD dwCount;
	if (SUCCEEDED(pCommands->GetCommandCount(&dwCount)))
	{
		for (DWORD n = 0; n < dwCount; n++)
		{
			DWORD dwCmdId;
			DWORD dwSize;

			if (FAILED(pCommands->GetCommandInfo(n, &dwCmdId, NULL, &dwSize)))
				continue;

			LPSTR  szNm = new char[dwSize];
			if (SUCCEEDED(pCommands->GetCommandInfo(n, &dwCmdId, szNm, &dwSize)))
			{
				LPTSTR pszName = new TCHAR[dwSize];
				Char2TCHAR( pszName, szNm, dwSize );
				int index;

				index = m_cFunction1.AddString(pszName);
				m_cFunction1.SetItemData(index, dwCmdId);

				index = m_cFunction2.AddString(pszName);
				m_cFunction2.SetItemData(index, dwCmdId);

				index = m_cFunction3.AddString(pszName);
				m_cFunction3.SetItemData(index, dwCmdId);

				index = m_cFunction4.AddString(pszName);
				m_cFunction4.SetItemData(index, dwCmdId);

				index = m_cFunction5.AddString(pszName);
				m_cFunction5.SetItemData(index, dwCmdId);

				index = m_cFunction6.AddString(pszName);
				m_cFunction6.SetItemData(index, dwCmdId);

				index = m_cFunction7.AddString(pszName);
				m_cFunction7.SetItemData(index, dwCmdId);

				index = m_cFunction8.AddString(pszName);
				m_cFunction8.SetItemData(index, dwCmdId);

				delete[] pszName;
			}

			delete[] szNm;
		}
	}

	
	m_bInitDone = true;

	UpdateData(FALSE);

//	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction1() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction2() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction3() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction4() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction5() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction6() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction7() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnSelchangeFunction8() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp1() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp2() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp3() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp4() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp5() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp6() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp7() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlC4PropPage::OnUpdateDisp8() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
