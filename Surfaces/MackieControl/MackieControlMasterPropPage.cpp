/////////////////////////////////////////////////////////////////////////////
// MackieControlMasterPropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "MackieControlMaster."
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

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

#include "HtmlHelp.h"

#include "VirtualKeys.h"

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlMasterPropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CMackieControlMasterPropPage::CMackieControlMasterPropPage(CWnd* pParent /*=NULL*/)
: CDialog(CMackieControlMasterPropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
{
	m_bInitDone = false;
	m_eJogResolution = JOG_MEASURES;
	m_eTransportResolution = JOG_MEASURES;
	m_bDisplaySMPTE = false;

	//{{AFX_DATA_INIT(CMackieControlMasterPropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::DoDataExchange(CDataExchange* pDX)
{
//	TRACE("CMackieControlMasterPropPage::DoDataExchange()\n");

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate == FALSE)
		TransferSettings(false);

	//{{AFX_DATA_MAP(CMackieControlMasterPropPage)
	DDX_Control(pDX, IDC_METERS, m_cMeters);
	DDX_Control(pDX, IDC_VIRTUAL_MAIN_TYPE, m_cVirtualMainType);
	DDX_Control(pDX, IDC_SELECT_HIGHLIGHTS, m_cSelectHighlightsTrack);
	DDX_Control(pDX, IDC_SELECT_DOUBLECLICK, m_cSelectDoubleClick);
	DDX_Control(pDX, IDC_FADER_SELECTS, m_cFaderSelectsChannel);
	DDX_Control(pDX, IDC_DISABLE_RELAY_CLICK, m_cDisableRelayClick);
	DDX_Control(pDX, IDC_DISABLE_LCD_UPDATES, m_cDisableLCD);
	DDX_Control(pDX, IDC_CONFIGURE_LAYOUT, m_cConfigureLayout);
	DDX_Control(pDX, IDC_VIRTUAL_MAIN, m_cVirtualMain);
	DDX_Control(pDX, IDC_SOLO_SELECTS, m_cSoloSelectsChannel);
	DDX_Control(pDX, IDC_DISABLE_FADERS, m_cDisableFaders);
	DDX_Control(pDX, IDC_FOOT_SWITCH_B, m_cFootSwitchB);
	DDX_Control(pDX, IDC_FOOT_SWITCH_A, m_cFootSwitchA);
	DDX_Control(pDX, IDC_FUNCTION8, m_cFunction8);
	DDX_Control(pDX, IDC_FUNCTION7, m_cFunction7);
	DDX_Control(pDX, IDC_FUNCTION6, m_cFunction6);
	DDX_Control(pDX, IDC_FUNCTION5, m_cFunction5);
	DDX_Control(pDX, IDC_FUNCTION4, m_cFunction4);
	DDX_Control(pDX, IDC_FUNCTION3, m_cFunction3);
	DDX_Control(pDX, IDC_FUNCTION2, m_cFunction2);
	DDX_Control(pDX, IDC_FUNCTION1, m_cFunction1);
	DDX_Control(pDX, IDC_DISABLE_HANDSHAKE, m_cDisableHandshake);
	DDX_Control(pDX, IDC_EXCLUDE_FILTERS_FROM_PLUGINS, m_cExcludeFiltersFromPlugins);
	DDX_Control(pDX, IDC_SCRUB_BANK_SELECTS_TRACK_BUS, m_cScrubBankSelectsTrackBus);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate == TRUE)
		TransferSettings(true);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMackieControlMasterPropPage, CDialog)
	//{{AFX_MSG_MAP(CMackieControlMasterPropPage)
	ON_CBN_SELCHANGE(IDC_FUNCTION1, OnSelchangeFunction1)
	ON_CBN_SELCHANGE(IDC_FUNCTION2, OnSelchangeFunction2)
	ON_CBN_SELCHANGE(IDC_FUNCTION3, OnSelchangeFunction3)
	ON_CBN_SELCHANGE(IDC_FUNCTION4, OnSelchangeFunction4)
	ON_CBN_SELCHANGE(IDC_FUNCTION5, OnSelchangeFunction5)
	ON_CBN_SELCHANGE(IDC_FUNCTION6, OnSelchangeFunction6)
	ON_CBN_SELCHANGE(IDC_FUNCTION7, OnSelchangeFunction7)
	ON_CBN_SELCHANGE(IDC_FUNCTION8, OnSelchangeFunction8)
	ON_BN_CLICKED(IDC_JOG_MEASURES, OnJogMeasures)
	ON_BN_CLICKED(IDC_JOG_BEATS, OnJogBeats)
	ON_BN_CLICKED(IDC_JOG_TICKS, OnJogTicks)
	ON_BN_CLICKED(IDC_TIME_FORMAT_MBT, OnTimeFormatMBT)
	ON_BN_CLICKED(IDC_TIME_FORMAT_HMSF, OnTimeFormatHMSF)
	ON_CBN_SELCHANGE(IDC_FOOT_SWITCH_A, OnSelchangeFootSwitchA)
	ON_CBN_SELCHANGE(IDC_FOOT_SWITCH_B, OnSelchangeFootSwitchB)
	ON_BN_CLICKED(IDC_DISABLE_FADERS, OnDisableFaders)
	ON_BN_CLICKED(IDC_SOLO_SELECTS, OnSoloSelects)
	ON_CBN_SELCHANGE(IDC_VIRTUAL_MAIN, OnSelchangeVirtualMain)
	ON_BN_CLICKED(IDC_CONFIGURE_LAYOUT, OnConfigureLayout)
	ON_BN_CLICKED(IDC_DISABLE_LCD_UPDATES, OnDisableLCDUpdates)
	ON_BN_CLICKED(IDC_TRANSPORT_MEASURES, OnTransportMeasures)
	ON_BN_CLICKED(IDC_TRANSPORT_BEATS, OnTransportBeats)
	ON_BN_CLICKED(IDC_TRANSPORT_TICKS, OnTransportTicks)
	ON_BN_CLICKED(IDC_DISABLE_RELAY_CLICK, OnDisableRelayClick)
	ON_BN_CLICKED(IDC_FADER_SELECTS, OnFaderSelects)
	ON_BN_CLICKED(IDC_SELECT_HIGHLIGHTS, OnSelectHighlights)
	ON_BN_CLICKED(IDC_SELECT_DOUBLECLICK, OnSelectDoubleClicks)
	ON_CBN_SELCHANGE(IDC_VIRTUAL_MAIN_TYPE, OnSelchangeVirtualMainType)
	ON_CBN_SELCHANGE(IDC_METERS, OnSelchangeMeters)
	ON_BN_CLICKED(IDC_DISABLE_HANDSHAKE, OnDisableHandshake)
	ON_BN_CLICKED(IDC_EXCLUDE_FILTERS_FROM_PLUGINS, OnExcludeFiltersFromPlugins)
	ON_BN_CLICKED(IDC_SCRUB_BANK_SELECTS_TRACK_BUS, OnScrubBankSelectsTrackBus)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CMackieControlMasterPropPage::~CMackieControlMasterPropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CMackieControlMasterPropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
//	TRACE("CMackieControlMasterPropPage::SetObjects()\n");

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CMackieControlMaster* const pSurface = dynamic_cast<CMackieControlMaster*>(*ppUnk);
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

HRESULT CMackieControlMasterPropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
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

HRESULT CMackieControlMasterPropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::Show( UINT nCmdShow )
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

HRESULT CMackieControlMasterPropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	m_pSurface->SetConfigureLayoutMode(false);

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
{
//	TRACE("CMackieControlMasterPropPage::SetPageSite()\n");

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

HRESULT CMackieControlMasterPropPage::Apply()
{
//	TRACE("CMackieControlMasterPropPage::Apply()\n");

	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::Help( LPCWSTR lpszHelpDir )
{
	// Returning E_NOTIMPL here should be enough to cause the help file
	// specified by GetPageInfo to be used, but it doesn't seem to work

	TCHAR szDLL[_MAX_PATH];

	DWORD dwLen = ::GetModuleFileName(theApp.m_hInstance, szDLL, sizeof(szDLL));

	if (dwLen < 3)
	    return E_FAIL;

	// OK not to use strlcpy here
	_tcscpy(szDLL + dwLen - 3, _T("chm"));

	::HtmlHelp(m_hWnd, szDLL, HH_DISPLAY_TOPIC, NULL);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "MackieControlMaster";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
	pPageInfo->size.cx      = 100;
	pPageInfo->size.cy      = 100;
	pPageInfo->pszDocString = NULL;
	pPageInfo->pszHelpFile  = NULL;
	pPageInfo->dwHelpContext= 0;

	// Create the property page in order to determine its size
	HWND const hWnd = CreateDialogParam( theApp.m_hInstance, MAKEINTRESOURCE( IDD_PROPPAGE_MASTER ), 
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

void CMackieControlMasterPropPage::TransferSettings(bool bSave)
{
	TRACE("CMackieControlMasterPropPage::TransferSettings()...");

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

		// Foot switches
		m_pSurface->SetFootSwitch(0, (DWORD)m_cFootSwitchA.GetItemData(m_cFootSwitchA.GetCurSel()));
		m_pSurface->SetFootSwitch(1, (DWORD)m_cFootSwitchB.GetItemData(m_cFootSwitchB.GetCurSel()));

		// Virtual main type
		m_pSurface->SetMasterFaderType((SONAR_MIXER_STRIP)m_cVirtualMainType.GetItemData(m_cVirtualMainType.GetCurSel()));

		// Virtual main
		m_pSurface->SetMasterFaderOffset(m_cVirtualMain.GetCurSel());

		// Jog resolution
		m_pSurface->SetJogResolution(m_eJogResolution);

		// Transport resolution
		m_pSurface->SetTransportResolution(m_eTransportResolution);

		// Time display format
		m_pSurface->SetDisplaySMPTE(m_bDisplaySMPTE);

		// Disable faders
		m_pSurface->SetDisableFaders(m_cDisableFaders.GetCheck() != 0);

		// Disable relay click
		m_pSurface->SetDisableRelayClick(m_cDisableRelayClick.GetCheck() != 0);

		// Disable LCD updates
		m_pSurface->SetDisableLCDUpdates(m_cDisableLCD.GetCheck() != 0);

		// Solo selects channel
		m_pSurface->SetSoloSelectsChannel(m_cSoloSelectsChannel.GetCheck() != 0);

		// Fader touch selects channel
		m_pSurface->SetFaderTouchSelectsChannel(m_cFaderSelectsChannel.GetCheck() != 0);

		// Select highlights track
		m_pSurface->SetSelectHighlightsTrack(m_cSelectHighlightsTrack.GetCheck() != 0);

		// Double click select
		if (m_pSurface->GetSelectHighlightsTrack())
		{
			m_cSelectDoubleClick.EnableWindow(1);
			m_pSurface->SetSelectDoubleClick(m_cSelectDoubleClick.GetCheck() != 0);
		}
		else
		{
			m_pSurface->SetSelectDoubleClick(false);
			m_cSelectDoubleClick.EnableWindow(0);
		}
		// Meters
		m_pSurface->SetDisplayLevelMeters((LevelMeters)m_cMeters.GetItemData(m_cMeters.GetCurSel()));

		// Disable handshake
		m_pSurface->SetDisableHandshake(m_cDisableHandshake.GetCheck() != 0);

		// Exclude filsters from plugin list
		m_pSurface->SetExcludeFiltersFromPlugins(m_cExcludeFiltersFromPlugins.GetCheck() != 0);

		// Scrub + Bank Down/Up buttons switch between Tracks & Buses
		m_pSurface->SetScrubBankSelectsTrackBus(m_cScrubBankSelectsTrackBus.GetCheck() != 0);
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

		// Foot switches
		SelectItemData(&m_cFootSwitchA, m_pSurface->GetFootSwitch(0));
		SelectItemData(&m_cFootSwitchB, m_pSurface->GetFootSwitch(1));

		// Virtual main type
		SelectItemData(&m_cVirtualMainType, m_pSurface->GetMasterFaderType());

		// Virtual main
		m_cVirtualMain.SetCurSel(m_pSurface->GetMasterFaderOffset());

		// Jog resolution
		m_eJogResolution = m_pSurface->GetJogResolution();
		switch (m_eJogResolution)
		{
			case JOG_MEASURES:
				CheckRadioButton(IDC_JOG_MEASURES, IDC_JOG_TICKS, IDC_JOG_MEASURES);
				break;

			case JOG_BEATS:
				CheckRadioButton(IDC_JOG_MEASURES, IDC_JOG_TICKS, IDC_JOG_BEATS);
				break;

			case JOG_TICKS:
				CheckRadioButton(IDC_JOG_MEASURES, IDC_JOG_TICKS, IDC_JOG_TICKS);
				break;

			default:
				break;
		}

		// Transport resolution
		m_eTransportResolution = m_pSurface->GetTransportResolution();
		switch (m_eTransportResolution)
		{
			case JOG_MEASURES:
				CheckRadioButton(IDC_TRANSPORT_MEASURES, IDC_TRANSPORT_TICKS, IDC_TRANSPORT_MEASURES);
				break;

			case JOG_BEATS:
				CheckRadioButton(IDC_TRANSPORT_MEASURES, IDC_TRANSPORT_TICKS, IDC_TRANSPORT_BEATS);
				break;

			case JOG_TICKS:
				CheckRadioButton(IDC_TRANSPORT_MEASURES, IDC_TRANSPORT_TICKS, IDC_TRANSPORT_TICKS);
				break;

			default:
				break;
		}

		// Time display format
		m_bDisplaySMPTE = m_pSurface->GetDisplaySMPTE();
		if (m_bDisplaySMPTE)
			CheckRadioButton(IDC_TIME_FORMAT_MBT, IDC_TIME_FORMAT_HMSF, IDC_TIME_FORMAT_HMSF);
		else
			CheckRadioButton(IDC_TIME_FORMAT_MBT, IDC_TIME_FORMAT_HMSF, IDC_TIME_FORMAT_MBT);

		// Disable faders
		m_cDisableFaders.SetCheck(m_pSurface->GetDisableFaders() ? 1 : 0);

		// Disable relay click
		m_cDisableRelayClick.SetCheck(m_pSurface->GetDisableRelayClick() ? 1 : 0);

		// Disable LCD updates
		m_cDisableLCD.SetCheck(m_pSurface->GetDisableLCDUpdates() ? 1 : 0);

		// Solo selects channel
		m_cSoloSelectsChannel.SetCheck(m_pSurface->GetSoloSelectsChannel() ? 1 : 0);

		// Fader touch selects channel
		m_cFaderSelectsChannel.SetCheck(m_pSurface->GetFaderTouchSelectsChannel() ? 1 : 0);

		// Select highlights track
		m_cSelectHighlightsTrack.SetCheck(m_pSurface->GetSelectHighlightsTrack() ? 1 : 0);

		// Double click select
		if (m_pSurface->GetSelectHighlightsTrack())
		{
			m_cSelectDoubleClick.EnableWindow(1);
			m_cSelectDoubleClick.SetCheck(m_pSurface->GetSelectDoubleClick() ? 1 : 0);
		}
		else
		{
			m_cSelectDoubleClick.SetCheck(0);
			m_cSelectDoubleClick.EnableWindow(0);
		}

		// Meters
		SelectItemData(&m_cMeters, m_pSurface->GetDisplayLevelMeters());

		// Disable handshake
		m_cDisableHandshake.SetCheck(m_pSurface->GetDisableHandshake() ? 1 : 0);

		m_cExcludeFiltersFromPlugins.SetCheck(m_pSurface->GetExcludeFiltersFromPlugins() ? 1 : 0);

		m_cScrubBankSelectsTrackBus.SetCheck(m_pSurface->GetScrubBankSelectsTrackBus() ? 1 : 0);
	}
}

////////////////////////////////////////////////////////////////////////////////

// Select the entry
//
void CMackieControlMasterPropPage::SelectItemData(CComboBox *pBox, DWORD dwData)
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

void CMackieControlMasterPropPage::FillVirtualMainCombo()
{
	m_cVirtualMain.ResetContent();

	DWORD dwN = m_pSurface->GetStripCount(m_pSurface->GetMasterFaderType());
	for (BYTE n = 0; n < dwN; n++)
	{
		TCHAR buf[8] = {0};

		_sntprintf(buf, sizeof(buf), _T("%d"), n + 1);

		m_cVirtualMain.AddString(buf);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

void CMackieControlMasterPropPage::AddCommandToCombos(LPTSTR szName, DWORD dwCmdId)
{
	int index;

	index = m_cFunction1.AddString(szName);
	m_cFunction1.SetItemData(index, dwCmdId);

	index = m_cFunction2.AddString(szName);
	m_cFunction2.SetItemData(index, dwCmdId);

	index = m_cFunction3.AddString(szName);
	m_cFunction3.SetItemData(index, dwCmdId);

	index = m_cFunction4.AddString(szName);
	m_cFunction4.SetItemData(index, dwCmdId);

	index = m_cFunction5.AddString(szName);
	m_cFunction5.SetItemData(index, dwCmdId);

	index = m_cFunction6.AddString(szName);
	m_cFunction6.SetItemData(index, dwCmdId);

	index = m_cFunction7.AddString(szName);
	m_cFunction7.SetItemData(index, dwCmdId);

	index = m_cFunction8.AddString(szName);
	m_cFunction8.SetItemData(index, dwCmdId);

	index = m_cFootSwitchA.AddString(szName);
	m_cFootSwitchA.SetItemData(index, dwCmdId);

	index = m_cFootSwitchB.AddString(szName);
	m_cFootSwitchB.SetItemData(index, dwCmdId);
}

void CMackieControlMasterPropPage::AddKeyPressesToCombos(bool bShift, bool bCtrl, bool bAlt)
{
	for (int i = 0; i < (sizeof(VALID_VKEYS) / sizeof(int)); i++)
	{
		DWORD dwCmdId;
		CString csName;

		if (VirtualKeys::isValidKey(VALID_VKEYS[i], bShift, bCtrl, bAlt))
		{
			dwCmdId = VirtualKeys::VirtualKeyToCommandId(VALID_VKEYS[i], bShift, bCtrl, bAlt);
			if (VirtualKeys::GetKeyPressName(dwCmdId, csName))
				AddCommandToCombos((LPTSTR)csName.GetString(), dwCmdId); // we're using Unicode, so don't bother trying to convert string
		}
	}
}

BOOL CMackieControlMasterPropPage::OnInitDialog() 
{
//	TRACE("CMackieControlMasterPropPage::OnInitDialog()\n");

	BOOL bRet = CDialog::OnInitDialog();
	
	// Get a pointer to the Sonar Commands interface
	ISonarCommands *pCommands = m_pSurface->GetSonarCommands();

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

			LPSTR pszName = new char[dwSize];

			if (SUCCEEDED(pCommands->GetCommandInfo(n, &dwCmdId, pszName, &dwSize)))
			{
				LPTSTR szName = new TCHAR[dwSize];
				Char2TCHAR( szName, pszName, dwSize );
#if _DEBUG
				TCHAR szTemp[256];
				_sntprintf(szTemp, sizeof(szTemp), _T("%s [%d]"), pszName, dwCmdId);
#endif
				AddCommandToCombos(szName, dwCmdId);
				delete[] szName;
			}

			delete[] pszName;
		}

		AddKeyPressesToCombos(false, false, false); // no modifiers
		AddKeyPressesToCombos(false, true, false); // CTRL +
		AddKeyPressesToCombos(true, false, false); // SHIFT +
		AddKeyPressesToCombos(false, false, true); // ALT +
		AddKeyPressesToCombos(true, true, false); // CTRL + SHIFT
		AddKeyPressesToCombos(false, true, true); // CTRL + ALT
		AddKeyPressesToCombos(true, false, true); // ALT + SHIFT
	}

	// Fill in the master fader combo boxes
	int index = m_cVirtualMainType.AddString(_T("Bus"));
	m_cVirtualMainType.SetItemData(index, MIX_STRIP_BUS);

	index = m_cVirtualMainType.AddString(_T("Master"));
	m_cVirtualMainType.SetItemData(index, MIX_STRIP_MASTER);

	FillVirtualMainCombo();

	// Fill in the meters combo boxes
	index = m_cMeters.AddString(_T("Off"));
	m_cMeters.SetItemData(index, METERS_OFF);

	index = m_cMeters.AddString(_T("Signal LEDs"));
	m_cMeters.SetItemData(index, METERS_LEDS);
	
	index = m_cMeters.AddString(_T("Signal LEDs + Meters"));
	m_cMeters.SetItemData(index, METERS_BOTH);

	// Disable the Meters combo if meters aren't available
	bool bHaveMeters = m_pSurface->HaveLevelMeters();
	m_cMeters.EnableWindow(bHaveMeters);
	
	m_bInitDone = true;

	UpdateData(FALSE);

//	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction1()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction2()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction3()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction4()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction5()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction6()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction7()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFunction8()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnJogMeasures()
{
	m_eJogResolution = JOG_MEASURES;

	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnJogBeats()
{
	m_eJogResolution = JOG_BEATS;
	
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnJogTicks()
{
	m_eJogResolution = JOG_TICKS;
	
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnTransportMeasures() 
{
	m_eTransportResolution = JOG_MEASURES;
	
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnTransportBeats() 
{
	m_eTransportResolution = JOG_BEATS;
	
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnTransportTicks() 
{
	m_eTransportResolution = JOG_TICKS;
	
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnTimeFormatMBT()
{
	m_bDisplaySMPTE = false;

	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnTimeFormatHMSF()
{
	m_bDisplaySMPTE = true;

	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFootSwitchA()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeFootSwitchB()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnDisableFaders()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnDisableRelayClick() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnDisableLCDUpdates() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSoloSelects()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnFaderSelects() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelectHighlights() 
{
	if (m_cSelectHighlightsTrack.GetCheck() == 0)
		m_cSelectDoubleClick.SetCheck(0);

	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeVirtualMain()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeVirtualMainType() 
{
	TRACE("CMackieControlMasterPropPage::OnSelchangeVirtualMainType()\n");

	UpdateData(TRUE);

	// Now rebuild the fader offset drop down
	FillVirtualMainCombo();
	m_cVirtualMain.SetCurSel(m_pSurface->GetMasterFaderOffset());
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnConfigureLayout() 
{
	bool bVal = m_pSurface->GetConfigureLayoutMode();

	if (bVal)
	{
		m_pSurface->SetConfigureLayoutMode(false);
		m_cConfigureLayout.SetWindowText(_T("Configure Layout"));
		UpdateData(TRUE);
	}
	else
	{
		m_pSurface->SetConfigureLayoutMode(true);
		m_cConfigureLayout.SetWindowText(_T("Press Again When Done"));
	}
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnSelchangeMeters() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////


void CMackieControlMasterPropPage::OnSelectDoubleClicks()
{
	UpdateData(TRUE);
}

void CMackieControlMasterPropPage::OnDisableHandshake()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnExcludeFiltersFromPlugins()
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlMasterPropPage::OnScrubBankSelectsTrackBus()
{
	UpdateData(TRUE);
}
