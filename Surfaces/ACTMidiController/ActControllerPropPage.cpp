/////////////////////////////////////////////////////////////////////////////
// ACTControllerPropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "ACTController."
//

#include "stdafx.h"

#include "ACTController.h"
#include "ACTControllerPropPage.h"

#include "HtmlHelp.h"

/////////////////////////////////////////////////////////////////////////////
//
// CACTControllerPropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CACTControllerPropPage::CACTControllerPropPage(CWnd* pParent /*=NULL*/)
: CDialog(CACTControllerPropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
	m_bInitDone = false;

	m_uiTimerID = 0;

	m_bACTEnabled = false;
	m_bACTLocked = false;

	m_eStripType = MIX_STRIP_TRACK;
	m_eRotariesMode = MCS_ASSIGNMENT_MUTLI_CHANNEL;

	//{{AFX_DATA_INIT(CACTControllerPropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::DoDataExchange(CDataExchange* pDX)
{
	TRACE("CACTControllerPropPage::DoDataExchange()\n");

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CACTControllerPropPage)
	DDX_Control(pDX, IDC_MIDI_LEARN_SHIFT, m_cMidiLearnShift);
	DDX_Control(pDX, IDC_TAB_CTRL, m_cTabCtrl);
	DDX_Control(pDX, IDC_ACT_NAME, m_cACTName);
	DDX_Control(pDX, IDC_ACT_LOCK, m_cACTLock);
	DDX_Control(pDX, IDC_ACT_ENABLE, m_cACTEnable);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CACTControllerPropPage, CDialog)
	//{{AFX_MSG_MAP(CACTControllerPropPage)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ACT_ENABLE, OnACTEnable)
	ON_BN_CLICKED(IDC_ACT_LOCK, OnACTLock)
	ON_BN_CLICKED(IDC_GROUP_TRACK, OnGroupTrack)
	ON_BN_CLICKED(IDC_GROUP_BUS, OnGroupBus)
	ON_BN_CLICKED(IDC_GROUP_MAIN, OnGroupMain)
	ON_BN_CLICKED(IDC_MULTI_CHANNEL, OnMultiChannel)
	ON_BN_CLICKED(IDC_CHANNEL_STRIP, OnChannelStrip)
	ON_BN_CLICKED(IDC_MIDI_LEARN_SHIFT, OnMidiLearnShift)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CACTControllerPropPage::~CACTControllerPropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CACTControllerPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	TRACE("CACTControllerPropPage::SetObjects()\n");

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CACTController* const pSurface = dynamic_cast<CACTController*>(*ppUnk);
		if (NULL == pSurface)
			return E_POINTER;
		
		// If different than previous object, release the old one
		if (m_pSurface != pSurface && m_pSurface != NULL)
			m_pSurface->Release();

		m_pSurface = pSurface;
		m_pSurface->AddRef();

		// Tell everything to reload
		LoadAllFields();

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

HRESULT CACTControllerPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
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

HRESULT CACTControllerPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::Show( UINT nCmdShow )
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

HRESULT CACTControllerPropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	StopTimer();

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CACTControllerPropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Returning E_NOTIMPL here should be enough to cause the help file
	// specified by GetPageInfo to be used, but it doesn't seem to work

	TCHAR szDLL[_MAX_PATH];

	DWORD dwLen = ::GetModuleFileName(theApp.m_hInstance, szDLL, sizeof(szDLL));

	if (dwLen < 3)
	    return E_FAIL;

	// OK not to use strlcpy here
	::_tcscpy(szDLL + dwLen - 3, _T("chm"));

	::HtmlHelp(m_hWnd, szDLL, HH_DISPLAY_TOPIC, NULL);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const TCHAR szTitle[] = _T("ACT MIDI Controller");
	_tcsncpy( pPageInfo->pszTitle, szTitle, _countof(szTitle) );

	// Populate the page info structure
	pPageInfo->cb				= sizeof(PROPPAGEINFO);
	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = CreateDialogParam( theApp.m_hInstance, MAKEINTRESOURCE( IDD_PROPPAGE ), 
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

void CACTControllerPropPage::StartTimer()
{
	m_uiTimerID = SetTimer(1, 100, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::StopTimer()
{
	KillTimer(m_uiTimerID);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::LoadAllFields()
{
	TRACE("CACTControllerPropPage::LoadAllFields()\n");

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

	m_cTabCtrl.LoadAllFields();

	UpdateACTStatus(true);
	GreyACTFields();
	UpdateGroupStatus(true);
	UpdateRotariesMode(true);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::UpdateACTStatus(bool bForce)
{
	if (m_pSurface == NULL)
	{
		TRACE("CACTControllerPropPage::UpdateACTStatus(%d) m_pSurface is NULL\n", bForce);
		return;
	}

	bool bACTEnabled = m_pSurface->GetUseDynamicMappings();
	if (bForce || m_bACTEnabled != bACTEnabled)
	{
		m_cACTEnable.SetCheck(bACTEnabled ? 1 : 0);
		m_bACTEnabled = bACTEnabled;
		GreyACTFields();
	}

	bool bACTLocked = m_pSurface->GetLockDynamicMappings();
	if (bForce || m_bACTLocked != bACTLocked)
	{
		m_cACTLock.SetCheck(bACTLocked ? 1 : 0);
		m_bACTLocked = bACTLocked;
	}

	CString strName;

	m_pSurface->GetDynamicMappingName(&strName);
	if (bForce || m_strACTName != strName)
	{
		m_cACTName.SetText(strName);
		m_strACTName = strName;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::GreyACTFields()
{
	BOOL bEnable = (m_pSurface->SupportsDynamicMappings()) ? TRUE : FALSE;

	m_cACTEnable.EnableWindow(bEnable);
	m_cACTLock.EnableWindow(bEnable);

	if (bEnable)
		bEnable = (m_pSurface->GetUseDynamicMappings()) ? TRUE : FALSE;

	COLORREF crText = (bEnable) ? RGB(0, 0, 0) : RGB(150, 150, 150);
	m_cACTName.SetTextColor(crText);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::UpdateGroupStatus(bool bForce)
{
	SONAR_MIXER_STRIP eStripType = m_pSurface->GetStripType();

	if (bForce || m_eStripType != eStripType)
	{
		m_eStripType = eStripType;

		int iButton;

		switch (eStripType)
		{
			default:
				return;

			case MIX_STRIP_TRACK:	iButton = IDC_GROUP_TRACK;	break;
			case MIX_STRIP_AUX:
			case MIX_STRIP_BUS:		iButton = IDC_GROUP_BUS;	break;
			case MIX_STRIP_MAIN:
			case MIX_STRIP_MASTER:	iButton = IDC_GROUP_MAIN;	break;
		}

		CheckRadioButton(IDC_GROUP_TRACK, IDC_GROUP_MAIN, iButton);
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::UpdateRotariesMode(bool bForce)
{
	AssignmentMode eRotariesMode = m_pSurface->GetRotariesMode();

	if (bForce || m_eRotariesMode != eRotariesMode)
	{
		m_eRotariesMode = eRotariesMode;

		int iButton;

		switch (eRotariesMode)
		{
			default:
				return;

			case MCS_ASSIGNMENT_MUTLI_CHANNEL:	iButton = IDC_MULTI_CHANNEL;	break;
			case MCS_ASSIGNMENT_CHANNEL_STRIP:	iButton = IDC_CHANNEL_STRIP;	break;
		}

		CheckRadioButton(IDC_MULTI_CHANNEL, IDC_CHANNEL_STRIP, iButton);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

BOOL CACTControllerPropPage::OnInitDialog() 
{
	BOOL bRet = CDialog::OnInitDialog();

	m_cTabCtrl.InitDialogs(m_pSurface);
	m_cACTName.SetBkColor(ACTBgColour());

	m_bInitDone = true;

	LoadAllFields();

	StartTimer();

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == m_uiTimerID)
	{
		m_cTabCtrl.Refresh();

		UpdateACTStatus();
		UpdateGroupStatus();
		UpdateRotariesMode();
	}

	CDialog::OnTimer(nIDEvent);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnACTEnable() 
{
	m_bACTEnabled = (m_cACTEnable.GetCheck() != 0);

	m_pSurface->SetUseDynamicMappings(m_bACTEnabled);

	GreyACTFields();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnACTLock() 
{
	m_bACTLocked = (m_cACTLock.GetCheck() != 0);

	m_pSurface->SetLockDynamicMappings(m_bACTLocked);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnGroupTrack() 
{
	m_pSurface->SetStripType(MIX_STRIP_TRACK);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnGroupBus() 
{
	m_pSurface->SetStripType(MIX_STRIP_BUS);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnGroupMain() 
{
	m_pSurface->SetStripType(MIX_STRIP_MASTER);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnMultiChannel() 
{
	m_pSurface->SetRotariesMode(MCS_ASSIGNMENT_MUTLI_CHANNEL);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnChannelStrip() 
{
	m_pSurface->SetRotariesMode(MCS_ASSIGNMENT_CHANNEL_STRIP);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPage::OnMidiLearnShift() 
{
	m_pSurface->MidiLearnShift();
}

////////////////////////////////////////////////////////////////////////////////
