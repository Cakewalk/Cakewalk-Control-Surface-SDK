/////////////////////////////////////////////////////////////////////////////
// ControlSurfaceProbePropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "ControlSurfaceProbe."
//

#include "stdafx.h"
#include "ControlSurfaceProbe.h"
#include "ControlSurfaceProbePropPage.h"

/////////////////////////////////////////////////////////////////////////////
//
// CControlSurfaceProbePropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CControlSurfaceProbePropPage::CControlSurfaceProbePropPage(CWnd* pParent /*=NULL*/)
: CDialog(CControlSurfaceProbePropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE ),
   m_windowType ( WT_CLIPS )
{
	m_bInitDone = false;
	m_dwCount = 0;

	//{{AFX_DATA_INIT(CControlSurfaceProbePropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CDialog::DoDataExchange(pDX);

   if (pDX->m_bSaveAndValidate == FALSE)
      TransferSettings(false);

   //{{AFX_DATA_MAP(CControlSurfaceProbePropPage)
   DDX_Control(pDX, IDC_MARKER_TICK_SPIN, m_cMarkerTickSpin);
   DDX_Control(pDX, IDC_MARKER_BEAT_SPIN, m_cMarkerBeatSpin);
   DDX_Control(pDX, IDC_MARKER_MEAS_SPIN, m_cMarkerMeasSpin);
   DDX_Control(pDX, IDC_MARKER_THE_NEAREST, m_cMarkerTheNearest);
   DDX_Control(pDX, IDC_MARKER_TICKS, m_cMarkerTicks);
   DDX_Control(pDX, IDC_MARKER_BEATS, m_cMarkerBeats);
   DDX_Control(pDX, IDC_MARKER_MEASURES, m_cMarkerMeasures);
   DDX_Control(pDX, IDC_MARKER_LIST, m_cMarkerList);
   DDX_Control(pDX, IDC_MARKER_COUNT, m_cMarkerCount);
   DDX_Control(pDX, IDC_STRIP_METER_VALUES, m_cStripMeterValues);
   DDX_Control(pDX, IDC_STRIP_NUM_METERS, m_cStripNumMeters);
   DDX_Control(pDX, IDC_TS_AUTOPUNCH, m_cTransportAutoPunch);
   DDX_Control(pDX, IDC_UNIQUE_ID, m_cUniqueID);
   DDX_Control(pDX, IDC_HOST_VERSION, m_cHostVersion);
   DDX_Control(pDX, IDC_HOST_NAME, m_cHostName);
   DDX_Control(pDX, IDC_COMMANDS, m_cCommands);
   DDX_Control(pDX, IDC_PARAM_NUM_LOW, m_cParamNumLow);
   DDX_Control(pDX, IDC_PARAM_NUM_HIGH, m_cParamNumHigh);
   DDX_Control(pDX, IDC_PARAM_NUM_SPIN_LOW, m_cParamNumSpinLow);
   DDX_Control(pDX, IDC_PARAM_NUM_SPIN_HIGH, m_cParamNumSpinHigh);
   DDX_Control(pDX, IDC_MIX_VALUE_ARM, m_cMixValueArm);
   DDX_Control(pDX, IDC_NUM_MASTER, m_cNumMasters);
   DDX_Control(pDX, IDC_NUM_BUSES, m_cNumBuses);
   DDX_Control(pDX, IDC_TS_SCRUB, m_cTransportScrub);
   DDX_Control(pDX, IDC_TS_REC_AUTO, m_cTransportRecAutomation);
   DDX_Control(pDX, IDC_TS_PLAY, m_cTransportPlay);
   DDX_Control(pDX, IDC_TS_LOOP, m_cTransportLoop);
   DDX_Control(pDX, IDC_TS_AUDIO, m_cTransportAudio);
   DDX_Control(pDX, IDC_TS_REC, m_sTransportRec);
   DDX_Control(pDX, IDC_MIX_NEW_VAL, m_cNewVal);
   DDX_Control(pDX, IDC_NUM_AUXS, m_cNumAuxs);
   DDX_Control(pDX, IDC_NUM_MAINS, m_cNumMains);
   DDX_Control(pDX, IDC_NUM_TRACKS, m_cNumTracks);
   DDX_Control(pDX, IDC_STRIP_NAME, m_cStripName);
   DDX_Control(pDX, IDC_UPDATE_COUNT, m_cUpdateCount);
   DDX_Control(pDX, IDC_STRIP_NUM_SPIN, m_cStripNumSpin);
   DDX_Control(pDX, IDC_PARAM_NUM_SPIN, m_cParamNumSpin);
   DDX_Control(pDX, IDC_MIX_VALUE_TEXT, m_cMixValueText);
   DDX_Control(pDX, IDC_MIX_VALUE_LABEL, m_cMixValueLabel);
   DDX_Control(pDX, IDC_STRIP_NUM, m_cStripNum);
   DDX_Control(pDX, IDC_MIX_VALUE, m_cMixValue);
   DDX_Control(pDX, IDC_PARAM_NUM, m_cParamNum);
   DDX_Control(pDX, IDC_MIX_PARAM, m_cMixParam);
   DDX_Control(pDX, IDC_MIX_STRIP, m_cMixStrip);

   DDX_Control(pDX, IDC_MIX_ARM, m_cWrite);
   DDX_Control(pDX, IDC_READ_ENABLE, m_cRead);


   // dynamic mapped params
   DDX_Control(pDX, IDC_SCT_ROTARY, m_cDynRotary1 );
   DDX_Control(pDX, IDC_SCT_ROTARY2, m_cDynRotary2 );
   DDX_Control(pDX, IDC_SCT_ROTARY3, m_cDynRotary3 );
   DDX_Control(pDX, IDC_SCT_ROTARY4, m_cDynRotary4 );
   DDX_Control(pDX, IDC_SCT_SLIDER, m_cDynSlider1 );
   DDX_Control(pDX, IDC_SCT_SLIDER2, m_cDynSlider2 );
   DDX_Control(pDX, IDC_SCT_SLIDER3, m_cDynSlider3 );
   DDX_Control(pDX, IDC_SCT_SLIDER4, m_cDynSlider4 );
   DDX_Control(pDX, IDC_SCT_SWITCH, m_cDynSwitch );
   DDX_Control(pDX, IDC_MAP_NAME, m_cMapName );
   DDX_Control(pDX, IDC_UI_CONTEXT, m_cUIContext );
   DDX_Control(pDX, IDC_SCT_SLIDER_CAPTION, m_cDynSliderParam1 );
   DDX_Control(pDX, IDC_SCT_SLIDER_CAPTION2, m_cDynSliderParam2 );
   DDX_Control(pDX, IDC_SCT_SLIDER_CAPTION3, m_cDynSliderParam3 );
   DDX_Control(pDX, IDC_SCT_SLIDER_CAPTION4, m_cDynSliderParam4 );

   DDX_Control(pDX, IDC_SCT_ROTARY_CAPTION, m_cDynRotaryParam1 );
   DDX_Control(pDX, IDC_SCT_ROTARY_CAPTION2, m_cDynRotaryParam2 );
   DDX_Control(pDX, IDC_SCT_ROTARY_CAPTION3, m_cDynRotaryParam3 );
   DDX_Control(pDX, IDC_SCT_ROTARY_CAPTION4, m_cDynRotaryParam4 );

   DDX_Control(pDX, IDC_SCT_SWITCH_CAPTION, m_cDynSwitchParam );

   DDX_Control(pDX, IDC_LOCK_CONTEXT, m_cLockContext );
   DDX_Control(pDX, IDC_LEARN_ENABLE, m_cEnableLearn );

   DDX_Control(pDX, IDC_WND_TYPE, m_cWindowType);
   DDX_Control(pDX, IDC_WND_STATE, m_cWindowState);

   //}}AFX_DATA_MAP

   if (pDX->m_bSaveAndValidate == TRUE)
      TransferSettings(true);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CControlSurfaceProbePropPage, CDialog)
	//{{AFX_MSG_MAP(CControlSurfaceProbePropPage)
	ON_CBN_SELCHANGE(IDC_MIX_STRIP, OnSelchangeMixStrip)
	ON_CBN_SELCHANGE(IDC_MIX_PARAM, OnSelchangeMixParam)
	ON_EN_CHANGE(IDC_STRIP_NUM, OnChangeStripNum)
	ON_EN_CHANGE(IDC_PARAM_NUM, OnChangeParamNum)
	ON_BN_CLICKED(IDC_MIX_SEND, OnMixSend)
	ON_NOTIFY(UDN_DELTAPOS, IDC_STRIP_NUM_SPIN, OnDeltaposStripNumSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PARAM_NUM_SPIN, OnDeltaposParamNumSpin)
	ON_BN_CLICKED(IDC_MIX_ARM, OnMixArm)
	ON_EN_CHANGE(IDC_PARAM_NUM_HIGH, OnChangeParamNumHigh)
	ON_EN_CHANGE(IDC_PARAM_NUM_LOW, OnChangeParamNumLow)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PARAM_NUM_SPIN_HIGH, OnDeltaposParamNumSpinHigh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PARAM_NUM_SPIN_LOW, OnDeltaposParamNumSpinLow)
	ON_BN_CLICKED(IDC_TS_AUDIO_SET, OnTsAudioSet)
	ON_BN_CLICKED(IDC_TS_AUDIO_CLEAR, OnTsAudioClear)
	ON_BN_CLICKED(IDC_TS_PLAY_SET, OnTsPlaySet)
	ON_BN_CLICKED(IDC_TS_PLAY_CLEAR, OnTsPlayClear)
	ON_BN_CLICKED(IDC_TS_SCRUB_SET, OnTsScrubSet)
	ON_BN_CLICKED(IDC_TS_SCRUB_CLEAR, OnTsScrubClear)
	ON_BN_CLICKED(IDC_TS_REC_SET, OnTsRecSet)
	ON_BN_CLICKED(IDC_TS_REC_CLEAR, OnTsRecClear)
	ON_BN_CLICKED(IDC_TS_REC_AUTO_SET, OnTsRecAutoSet)
	ON_BN_CLICKED(IDC_TS_REC_AUTO_CLEAR, OnTsRecAutoClear)
	ON_BN_CLICKED(IDC_TS_LOOP_SET, OnTsLoopSet)
	ON_BN_CLICKED(IDC_TS_LOOP_CLEAR, OnTsLoopClear)
	ON_BN_CLICKED(IDC_DO_COMMAND, OnDoCommand)
	ON_BN_CLICKED(IDC_CAPABILITIES, OnCapabilities)
	ON_BN_CLICKED(IDC_TS_AUTOPUNCH_SET, OnTsAutoPunchSet)
	ON_BN_CLICKED(IDC_TS_AUTOPUNCH_CLEAR, OnTsAutoPunchClear)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MARKER_GET_NEAREST, OnMarkerGetNearest)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MARKER_MEAS_SPIN, OnDeltaposMarkerMeasSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MARKER_BEAT_SPIN, OnDeltaposMarkerBeatSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MARKER_TICK_SPIN, OnDeltaposMarkerTickSpin)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SCT_SWITCH, &CControlSurfaceProbePropPage::OnBnClickedSctSwitch)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_LOCK_CONTEXT, &CControlSurfaceProbePropPage::OnBnClickedLockContext)
	ON_BN_CLICKED(IDC_READ_ENABLE, &CControlSurfaceProbePropPage::OnBnClickedReadEnable)
	ON_BN_CLICKED(IDC_LEARN_ENABLE, &CControlSurfaceProbePropPage::OnBnClickedLearnEnable)
   ON_BN_CLICKED(IDC_WND_ZOOM_UP, &CControlSurfaceProbePropPage::OnBnClickedWndZoomUp)
   ON_BN_CLICKED(IDC_WND_ZOOM_UP2, &CControlSurfaceProbePropPage::OnBnClickedWndZoomUp2)
   ON_BN_CLICKED(IDC_WND_ZOOM_DOWN, &CControlSurfaceProbePropPage::OnBnClickedWndZoomDown)
   ON_BN_CLICKED(IDC_WND_ZOOM_DOWN2, &CControlSurfaceProbePropPage::OnBnClickedWndZoomDown2)
   ON_BN_CLICKED(IDC_WND_ZOOM_LEFT, &CControlSurfaceProbePropPage::OnBnClickedWndZoomLeft)
   ON_BN_CLICKED(IDC_WND_ZOOM_LEFT2, &CControlSurfaceProbePropPage::OnBnClickedWndZoomLeft2)
   ON_BN_CLICKED(IDC_WND_ZOOM_RIGHT, &CControlSurfaceProbePropPage::OnBnClickedWndZoomRight)
   ON_BN_CLICKED(IDC_WND_ZOOM_RIGHT2, &CControlSurfaceProbePropPage::OnBnClickedWndZoomRight2)
   ON_BN_CLICKED(IDC_WND_SCROLL_UP, &CControlSurfaceProbePropPage::OnBnClickedWndScrollUp)
   ON_BN_CLICKED(IDC_WND_SCROLL_UP2, &CControlSurfaceProbePropPage::OnBnClickedWndScrollUp2)
   ON_BN_CLICKED(IDC_WND_SCROLL_DOWN, &CControlSurfaceProbePropPage::OnBnClickedWndScrollDown)
   ON_BN_CLICKED(IDC_WND_SCROLL_DOWN2, &CControlSurfaceProbePropPage::OnBnClickedWndScrollDown2)
   ON_BN_CLICKED(IDC_WND_SCROLL_LEFT, &CControlSurfaceProbePropPage::OnBnClickedWndScrollLeft)
   ON_BN_CLICKED(IDC_WND_SCROLL_LEFT2, &CControlSurfaceProbePropPage::OnBnClickedWndScrollLeft2)
   ON_BN_CLICKED(IDC_WND_SCROLL_RIGHT, &CControlSurfaceProbePropPage::OnBnClickedWndScrollRight)
   ON_BN_CLICKED(IDC_WND_SCROLL_RIGHT2, &CControlSurfaceProbePropPage::OnBnClickedWndScrollRight2)
   ON_BN_CLICKED(IDC_WND_OPEN, &CControlSurfaceProbePropPage::OnBnClickedWndOpen)
   ON_BN_CLICKED(IDC_WND_CLOSE, &CControlSurfaceProbePropPage::OnBnClickedWndClose)
   ON_BN_CLICKED(IDC_WND_MIN, &CControlSurfaceProbePropPage::OnBnClickedWndMin)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CControlSurfaceProbePropPage::~CControlSurfaceProbePropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CControlSurfaceProbePropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CControlSurfaceProbe* const pSurface = dynamic_cast<CControlSurfaceProbe*>(*ppUnk);
		if (NULL == pSurface)
			return E_POINTER;
		
		// If different than previous object, release the old one
		if (m_pSurface != pSurface && m_pSurface != NULL)
			m_pSurface->Release();

		m_pSurface = pSurface;
		m_pSurface->AddRef();
		m_dwSurfaceID = m_pSurface->GetSurfaceID();

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

HRESULT CControlSurfaceProbePropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
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

HRESULT CControlSurfaceProbePropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::Show( UINT nCmdShow )
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

HRESULT CControlSurfaceProbePropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	StopTimer();

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CControlSurfaceProbePropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::Help( LPCWSTR lpszHelpDir )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbePropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "Control Surface Probe";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
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

////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

BOOL CControlSurfaceProbePropPage::OnInitDialog() 
{
	BOOL bRet = CDialog::OnInitDialog();

	m_cStripNumSpin.SetBuddy(&m_cStripNum);
	m_cParamNumSpin.SetBuddy(&m_cParamNum);
	m_cParamNumSpinHigh.SetBuddy(&m_cParamNumHigh);
	m_cParamNumSpinLow.SetBuddy(&m_cParamNumLow);

	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_TRACK",					MIX_STRIP_TRACK);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_AUX",					MIX_STRIP_AUX);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_MAIN",					MIX_STRIP_MAIN);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_MIDI_SEND",				MIX_STRIP_MIDI_SEND);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_BUS",					MIX_STRIP_BUS);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_MASTER",					MIX_STRIP_MASTER);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_RACK",				MIX_STRIP_RACK);
	ComboAddEntry(&m_cMixStrip, "MIX_STRIP_ANY",					MIX_STRIP_ANY);

	ComboAddEntry(&m_cMixParam, "MIX_PARAM_VOL",					MIX_PARAM_VOL);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_PAN",					MIX_PARAM_PAN);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_MUTE",					MIX_PARAM_MUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_MUTE_DEFEAT",			MIX_PARAM_MUTE_DEFEAT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_AUTOMATED_MUTE",		MIX_PARAM_AUTOMATED_MUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SOLO",					MIX_PARAM_SOLO);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_ARCHIVE",				MIX_PARAM_ARCHIVE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_RECORD_ARM",				MIX_PARAM_RECORD_ARM);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_VOL_TRIM",				MIX_PARAM_VOL_TRIM);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_PHASE",					MIX_PARAM_PHASE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_INTERLEAVE",				MIX_PARAM_INTERLEAVE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_INPUT_MAX",				MIX_PARAM_INPUT_MAX);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_INPUT",					MIX_PARAM_INPUT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_OUTPUT_MAX",				MIX_PARAM_OUTPUT_MAX);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_OUTPUT",					MIX_PARAM_OUTPUT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_IS_MIDI",				MIX_PARAM_IS_MIDI);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_BANK",					MIX_PARAM_BANK);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_PATCH",					MIX_PARAM_PATCH);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_KEY_OFFSET",				MIX_PARAM_KEY_OFFSET);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_VEL_OFFSET",				MIX_PARAM_VEL_OFFSET);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SELECTED",				MIX_PARAM_SELECTED);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_ENABLE",			MIX_PARAM_SEND_ENABLE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_MUTE",				MIX_PARAM_SEND_MUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_VOL",				MIX_PARAM_SEND_VOL);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_PAN",				MIX_PARAM_SEND_PAN);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_PREPOST",			MIX_PARAM_SEND_PREPOST);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_INSERT",			MIX_PARAM_SEND_INSERT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_SOLO",				MIX_PARAM_SEND_SOLO);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_OUTPUT",			MIX_PARAM_SEND_OUTPUT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_OUTPUT_MAX",		MIX_PARAM_SEND_OUTPUT_MAX);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_AUDIO_METER",			MIX_PARAM_AUDIO_METER);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_MIDI_METER",				MIX_PARAM_MIDI_METER);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FX",						MIX_PARAM_FX);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FX_COUNT",				MIX_PARAM_FX_COUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FX_PARAM_COUNT",			MIX_PARAM_FX_PARAM_COUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FX_PARAM",				MIX_PARAM_FX_PARAM);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_TRACK_EXISTS",			MIX_PARAM_TRACK_EXISTS);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_BUS_TYPE",				MIX_PARAM_BUS_TYPE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_BUS_EXISTS",			MIX_PARAM_BUS_EXISTS);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FILTER",					MIX_PARAM_FILTER);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FILTER_COUNT",			MIX_PARAM_FILTER_COUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FILTER_PARAM_COUNT",		MIX_PARAM_FILTER_PARAM_COUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FILTER_PARAM",			MIX_PARAM_FILTER_PARAM);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_FILTER_INDEX",			MIX_PARAM_FILTER_INDEX);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_INPUT_ECHO",				MIX_PARAM_INPUT_ECHO);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_ANGLE",			MIX_PARAM_SURROUND_ANGLE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDANGLE",		MIX_PARAM_SURROUND_SENDANGLE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_FOCUS",			MIX_PARAM_SURROUND_FOCUS);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDFOCUS",		MIX_PARAM_SURROUND_SENDFOCUS);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_WIDTH",			MIX_PARAM_SURROUND_WIDTH);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDWIDTH",		MIX_PARAM_SURROUND_SENDWIDTH);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_FR_BAL",		MIX_PARAM_SURROUND_FR_BAL);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDFR_BAL",	MIX_PARAM_SURROUND_SENDFR_BAL);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_LFE",			MIX_PARAM_SURROUND_LFE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDLFE",		MIX_PARAM_SURROUND_SENDLFE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SPKRMUTE",		MIX_PARAM_SURROUND_SPKRMUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SURROUND_SENDSPKRMUTE",	MIX_PARAM_SURROUND_SENDSPKRMUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SENDCOUNT",				MIX_PARAM_SENDCOUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SENDNAME",				MIX_PARAM_SENDNAME);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_SEND_ISSURROUND",		MIX_PARAM_SEND_ISSURROUND);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_ISSURROUND",				MIX_PARAM_ISSURROUND);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_LAYER_MUTE",				MIX_PARAM_LAYER_MUTE);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_LAYER_SOLO",				MIX_PARAM_LAYER_SOLO);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_LAYER_COUNT",			MIX_PARAM_LAYER_COUNT);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_LAYER",					MIX_PARAM_LAYER);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_XRAY_OPACITY",			MIX_PARAM_XRAY_OPACITY);
	ComboAddEntry(&m_cMixParam, "MIX_PARAM_ANY",					MIX_PARAM_ANY);

   ComboAddEntry(&m_cWindowType, "WT_CLIPS", WT_CLIPS);
   ComboAddEntry(&m_cWindowType, "WT_MIDI", WT_MIDI);
   ComboAddEntry(&m_cWindowType, "WT_MIXER", WT_MIXER);

	m_cDynSlider1.SetRange( 0, 100 );
	m_cDynSlider2.SetRange( 0, 100 );
	m_cDynSlider3.SetRange( 0, 100 );
	m_cDynSlider4.SetRange( 0, 100 );

	m_cDynRotary1.SetRange( 0, 100 );
	m_cDynRotary2.SetRange( 0, 100 );
	m_cDynRotary3.SetRange( 0, 100 );
	m_cDynRotary4.SetRange( 0, 100 );


	m_cCommands.ResetContent();

	DWORD dwCount = m_pSurface->GetCommandCount();
	for (DWORD n = 0; n < dwCount; n++)
	{
		CString str;

		DWORD dwCmdId = m_pSurface->GetCommandInfo(n, &str);

		if (dwCmdId > 0)
		{
			int index = m_cCommands.AddString(str);
			m_cCommands.SetItemData(index, dwCmdId);
		}
	}
	m_cCommands.SetCurSel(0);

	m_cMarkerMeasSpin.SetBuddy(&m_cMarkerMeasures);
	m_cMarkerBeatSpin.SetBuddy(&m_cMarkerBeats);
	m_cMarkerTickSpin.SetBuddy(&m_cMarkerTicks);

	m_cMarkerMeasures.SetWindowText("1");
	m_cMarkerBeats.SetWindowText("1");
	m_cMarkerTicks.SetWindowText("0");

	m_cMarkerTheNearest.SetWindowText("");

	m_bInitDone = true;

	UpdateData(FALSE);

	StartTimer();

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::StartTimer()
{
	m_uiTimerID = SetTimer(1, 100, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::StopTimer()
{
	KillTimer(m_uiTimerID);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent = m_uiTimerID)
	{
		UpdateDisplay();

		StartTimer();
	}

	CDialog::OnTimer(nIDEvent);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::UpdateDisplay()
{
	if (!m_pSurface)
		return;

	CString old, tmp, strName, strVal;

	m_pSurface->GetUpdateCount(&tmp);
	m_cUpdateCount.SetWindowText(tmp);

	m_dwCount++;

	switch ((m_dwCount % 4))
	{
		case 0:
			m_pSurface->GetStripCount(MIX_STRIP_TRACK, &tmp);
			m_cNumTracks.SetWindowText(tmp);

			m_pSurface->GetStripCount(MIX_STRIP_AUX, &tmp);
			m_cNumAuxs.SetWindowText(tmp);

			m_pSurface->GetStripCount(MIX_STRIP_MAIN, &tmp);
			m_cNumMains.SetWindowText(tmp);

			m_pSurface->GetStripCount(MIX_STRIP_BUS, &tmp);
			m_cNumBuses.SetWindowText(tmp);

			m_pSurface->GetStripCount(MIX_STRIP_MASTER, &tmp);
			m_cNumMasters.SetWindowText(tmp);

			m_pSurface->GetMarkerCount(&tmp);
			m_cMarkerCount.SetWindowText(tmp);

			m_pSurface->GetMarkerList(&tmp);

			m_cMarkerList.GetWindowText(old);
			if (tmp != old)
				m_cMarkerList.SetWindowText(tmp);

			m_cRead.SetCheck( m_pSurface->GetReadEnable() );
			m_cWrite.SetCheck( m_pSurface->GetWriteEnable() );

			break;

		case 1:
			m_pSurface->GetStripName(&tmp);
			m_cStripName.SetWindowText(tmp);

			m_pSurface->GetValueLabel(&tmp);
			m_cMixValueLabel.SetWindowText(tmp);

			m_pSurface->GetValue(&tmp);
			m_cMixValue.SetWindowText(tmp);

			m_pSurface->GetValueText(&tmp);
			m_cMixValueText.SetWindowText(tmp);

			m_pSurface->GetArm(&tmp);
			m_cMixValueArm.SetWindowText(tmp);

			m_pSurface->GetNumMeters(&tmp);
			m_cStripNumMeters.SetWindowText(tmp);

			m_pSurface->GetMetersValues(&tmp);
			m_cStripMeterValues.SetWindowText(tmp);
			break;

		case 2:
			m_pSurface->GetTransportState(TRANSPORT_STATE_AUDIO, &tmp);
			m_cTransportAudio.SetWindowText(tmp);
			
			m_pSurface->GetTransportState(TRANSPORT_STATE_PLAY, &tmp);
			m_cTransportPlay.SetWindowText(tmp);

			m_pSurface->GetTransportState(TRANSPORT_STATE_SCRUB, &tmp);
			m_cTransportScrub.SetWindowText(tmp);

			m_pSurface->GetTransportState(TRANSPORT_STATE_REC, &tmp);
			m_sTransportRec.SetWindowText(tmp);

			m_pSurface->GetTransportState(TRANSPORT_STATE_REC_AUTOMATION, &tmp);
			m_cTransportRecAutomation.SetWindowText(tmp);

			m_pSurface->GetTransportState(TRANSPORT_STATE_LOOP, &tmp);
			m_cTransportLoop.SetWindowText(tmp);

			m_pSurface->GetTransportState(TRANSPORT_STATE_AUTOPUNCH, &tmp);
			m_cTransportAutoPunch.SetWindowText(tmp);

			m_pSurface->GetUniqueID(&tmp);
			m_cUniqueID.SetWindowText(tmp);

			m_pSurface->GetHostVersion(&tmp);
			m_cHostVersion.SetWindowText(tmp);

			m_pSurface->GetHostName(&tmp);
			m_cHostName.SetWindowText(tmp);
			break;

		case 3:
			if ( m_pSurface->HasMapping() )
			{
				// update the dynamic mapped params
				float fVal;

				// rotaries
				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_ROTARY, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_ROTARY, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynRotary1.SetPos( (int)(fVal * 100) );
				m_cDynRotaryParam1.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_ROTARY2, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_ROTARY2, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynRotary2.SetPos( (int)(fVal * 100) );
				m_cDynRotaryParam2.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_ROTARY3, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_ROTARY3, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynRotary3.SetPos( (int)(fVal * 100) );
				m_cDynRotaryParam3.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_ROTARY4, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_ROTARY4, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynRotary4.SetPos( (int)(fVal * 100) );
				m_cDynRotaryParam4.SetWindowText( tmp );


				// sliders
				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_SLIDER, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_SLIDER, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynSlider1.SetPos( 100 - (int)(fVal * 100) );
				m_cDynSliderParam1.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_SLIDER2, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_SLIDER2, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynSlider2.SetPos( 100 - (int)(fVal * 100) );
				m_cDynSliderParam2.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_SLIDER3, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_SLIDER3, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynSlider3.SetPos( 100 - (int)(fVal * 100) );
				m_cDynSliderParam3.SetWindowText( tmp );

				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_SLIDER4, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_SLIDER4, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynSlider4.SetPos( 100 - (int)(fVal * 100) );
				m_cDynSliderParam4.SetWindowText( tmp );

				// switches
				m_pSurface->GetValueLabel( MIX_STRIP_ANY, IDC_SCT_SWITCH, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &strName );
				m_pSurface->GetValue( MIX_STRIP_ANY, IDC_SCT_SWITCH, MIX_PARAM_DYN_MAP, m_dwSurfaceID, &fVal, &strVal );
				tmp.Format( _T("%s=%s"), strName, strVal );
				m_cDynSwitch.SetCheck( fVal >= .5f ? 1 : 0 );
				m_cDynSwitchParam.SetWindowText( tmp );

				m_pSurface->GetMappingName( &tmp );
				m_cMapName.SetWindowText( tmp );

				m_cLockContext.SetCheck( m_pSurface->GetContextLocked() );

				m_cEnableLearn.SetCheck( m_pSurface->GetLearnEnabled() );

				m_pSurface->GetUIContext( &tmp );
				m_cUIContext.SetWindowText( tmp );
			}
		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::TransferSettings(bool bSave)
{
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
		m_pSurface->SetMixerStrip((SONAR_MIXER_STRIP)ComboGetSelectedData(&m_cMixStrip));
		m_pSurface->SetStripNum(GetEditAsDword(&m_cStripNum));
		m_pSurface->SetMixerParam((SONAR_MIXER_PARAM)ComboGetSelectedData(&m_cMixParam));
		m_pSurface->SetParamNum(GetEditAsDword(&m_cParamNum));

      m_windowType = (WindowType)ComboGetSelectedData(&m_cWindowType);
	}
	else
	{
		ComboSelectItemByData(&m_cMixStrip, m_pSurface->GetMixerStrip());
		SetEditToDword(&m_cStripNum, m_pSurface->GetStripNum());
		ComboSelectItemByData(&m_cMixParam, m_pSurface->GetMixerParam());
		DWORD dwPN = m_pSurface->GetParamNum();
		SetEditToDword(&m_cParamNum, dwPN);
		SetEditToDword(&m_cParamNumHigh, HIWORD(dwPN));
		SetEditToDword(&m_cParamNumLow, LOWORD(dwPN));

      WindowState ws;
      m_pSurface->GetWindowState(m_windowType, &ws);
      switch (ws)
      {
      case WST_OPEN:
         m_cWindowState.SetWindowText("Window is open");
         break;

      case WST_CLOSE:
         m_cWindowState.SetWindowText("Window is closed");
         break;

      case WST_MINIMIZE:
         m_cWindowState.SetWindowText("Window is minimized");
         break;

      default:
         m_cWindowState.SetWindowText("Window state unknown");
      }
	}
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::ComboAddEntry(CComboBox *pCBox, const char *str, DWORD dwData)
{
	int index = pCBox->AddString(str);
	pCBox->SetItemData(index, dwData);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::ComboSelectItemByData(CComboBox *pCBox, DWORD dwData)
{
	if (NULL == pCBox || NULL == pCBox->m_hWnd)
		return;

	pCBox->Clear();

	int iNumItems = pCBox->GetCount();

	for (int n = 0; n < iNumItems; n++)
	{
		if (pCBox->GetItemData(n) == dwData)
		{
			pCBox->SetCurSel(n);
			return;
		}
	}

	pCBox->SetCurSel(0);
}

////////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbePropPage::ComboGetSelectedData(CComboBox *pCBox)
{
	return (DWORD)pCBox->GetItemData(pCBox->GetCurSel());
}

////////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbePropPage::GetEditAsDword(CEdit *pCEdit)
{
	char buf[128];

	pCEdit->GetWindowText(buf, sizeof(buf));
	return strtoul(buf, NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::SetEditToDword(CEdit *pCEdit, DWORD dwVal)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%u", dwVal);

	pCEdit->SetWindowText(buf);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnSelchangeMixStrip() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnSelchangeMixParam() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnChangeStripNum() 
{
	UpdateData(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnChangeParamNum() 
{
//	TRACE("OnChangeParamNum()\n");
	
	DWORD dwN = GetEditAsDword(&m_cParamNum);

	if (m_pSurface)
		m_pSurface->SetParamNum(dwN);

	if (HIWORD(dwN) != GetEditAsDword(&m_cParamNumHigh))
		SetEditToDword(&m_cParamNumHigh, HIWORD(dwN));

	if (LOWORD(dwN) != GetEditAsDword(&m_cParamNumLow))
		SetEditToDword(&m_cParamNumLow, LOWORD(dwN));
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnChangeParamNumHigh() 
{
//	TRACE("OnChangeParamNumHigh()\n");

	DWORD dwN = GetEditAsDword(&m_cParamNum);
	WORD wN = (WORD)GetEditAsDword(&m_cParamNumHigh);
	DWORD dwNew = MAKELONG(LOWORD(dwN), wN);

	if (m_pSurface)
		m_pSurface->SetParamNum(dwN);

	if (dwN != dwNew)
		SetEditToDword(&m_cParamNum, dwNew);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnChangeParamNumLow() 
{
//	TRACE("OnChangeParamNumLow()\n");

	DWORD dwN = GetEditAsDword(&m_cParamNum);
	WORD wN = (WORD)GetEditAsDword(&m_cParamNumLow);
	DWORD dwNew = MAKELONG(wN, HIWORD(dwN));

	if (m_pSurface)
		m_pSurface->SetParamNum(dwN);

	if (dwN != dwNew)
		SetEditToDword(&m_cParamNum, dwNew);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnMixSend() 
{
	char buf[128];

	m_cNewVal.GetWindowText(buf, sizeof(buf));

	m_pSurface->SetValue((float)atof(buf));
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposStripNumSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	DWORD dwN = m_pSurface->GetStripNum();

	dwN -= pNMUpDown->iDelta;

	m_pSurface->SetStripNum(dwN);
	SetEditToDword(&m_cStripNum, dwN);
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposParamNumSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	DWORD dwN = m_pSurface->GetParamNum();

	dwN -= pNMUpDown->iDelta;

	m_pSurface->SetParamNum(dwN);
	SetEditToDword(&m_cParamNum, dwN);
	SetEditToDword(&m_cParamNumHigh, HIWORD(dwN));
	SetEditToDword(&m_cParamNumLow, LOWORD(dwN));

	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposParamNumSpinHigh(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	DWORD dwN = m_pSurface->GetParamNum();
	WORD wN = HIWORD(dwN);

	wN -= pNMUpDown->iDelta;

	dwN = MAKELONG(LOWORD(dwN), wN);

	m_pSurface->SetParamNum(dwN);
	SetEditToDword(&m_cParamNum, dwN);
	SetEditToDword(&m_cParamNumHigh, wN);
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposParamNumSpinLow(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	DWORD dwN = m_pSurface->GetParamNum();
	WORD wN = LOWORD(dwN);

	wN -= pNMUpDown->iDelta;

	dwN = MAKELONG(wN, HIWORD(dwN));

	m_pSurface->SetParamNum(dwN);
	SetEditToDword(&m_cParamNum, dwN);
	SetEditToDword(&m_cParamNumLow, wN);
	
	*pResult = 0;
}



////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsAudioSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_AUDIO, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsAudioClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_AUDIO, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsPlaySet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_PLAY, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsPlayClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_PLAY, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsScrubSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_SCRUB, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsScrubClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_SCRUB, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsRecSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_REC, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsRecClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_REC, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsRecAutoSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_REC_AUTOMATION, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsRecAutoClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_REC_AUTOMATION, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsLoopSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_LOOP, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsLoopClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_LOOP, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsAutoPunchSet() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_AUTOPUNCH, true);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnTsAutoPunchClear() 
{
	m_pSurface->SetTransportState(TRANSPORT_STATE_AUTOPUNCH, false);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDoCommand() 
{
	m_pSurface->DoCommand( DWORD(m_cCommands.GetItemData(m_cCommands.GetCurSel())) );
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnCapabilities() 
{
	m_pSurface->PopupCapabilities();
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnMarkerGetNearest() 
{
	MFX_TIME mfxTime;

	mfxTime.timeFormat = TF_MBT;
	mfxTime.mbt.nMeas = GetEditAsDword(&m_cMarkerMeasures);
	mfxTime.mbt.nBeat = (short)GetEditAsDword(&m_cMarkerBeats);
	mfxTime.mbt.nTick = (short)GetEditAsDword(&m_cMarkerTicks);

	CString tmp;

	m_pSurface->GetMarkerIndexForTime(&mfxTime, &tmp);
	m_cMarkerTheNearest.SetWindowText(tmp);
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposMarkerMeasSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int iVal = GetEditAsDword(&m_cMarkerMeasures);

	iVal -= pNMUpDown->iDelta;

	if (iVal < 1)
		iVal = 1;

	SetEditToDword(&m_cMarkerMeasures, iVal);
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposMarkerBeatSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int iVal = GetEditAsDword(&m_cMarkerBeats);

	iVal -= pNMUpDown->iDelta;

	if (iVal < 1)
		iVal = 1;

	SetEditToDword(&m_cMarkerBeats, iVal);
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnDeltaposMarkerTickSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int iVal = GetEditAsDword(&m_cMarkerTicks);

	iVal -= pNMUpDown->iDelta;

	if (iVal < 0)
		iVal = 0;

	SetEditToDword(&m_cMarkerTicks, iVal);
	
	*pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnBnClickedLockContext()
{
	m_pSurface->SetContextLocked( !!m_cLockContext.GetCheck() );
}


void CControlSurfaceProbePropPage::OnBnClickedSctSwitch()
{
	m_pSurface->SetValue( MIX_STRIP_ANY, IDC_SCT_SWITCH, MIX_PARAM_DYN_MAP, m_dwSurfaceID, m_cDynSwitch.GetCheck() ? 1.0f : 0.0f );
}


void CControlSurfaceProbePropPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UINT id = pScrollBar->GetDlgCtrlID();
	switch( id )
	{
	case IDC_SCT_ROTARY:
		nPos = m_cDynRotary1.GetPos();
		break;
	case IDC_SCT_ROTARY2:
		nPos = m_cDynRotary2.GetPos();
		break;
	case IDC_SCT_ROTARY3:
		nPos = m_cDynRotary3.GetPos();
		break;
	case IDC_SCT_ROTARY4:
		nPos = m_cDynRotary4.GetPos();
		break;
	}
	float fVal = nPos / 100.0f;
	m_pSurface->SetValue( MIX_STRIP_ANY, id, MIX_PARAM_DYN_MAP, m_dwSurfaceID, fVal );

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CControlSurfaceProbePropPage::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UINT id = pScrollBar->GetDlgCtrlID();
	switch( id )
	{
	case IDC_SCT_SLIDER:
		nPos = m_cDynSlider1.GetPos();
		break;
	case IDC_SCT_SLIDER2:
		nPos = m_cDynSlider2.GetPos();
		break;
	case IDC_SCT_SLIDER3:
		nPos = m_cDynSlider3.GetPos();
		break;
	case IDC_SCT_SLIDER4:
		nPos = m_cDynSlider4.GetPos();
		break;
	}
	float fVal = (100 - nPos) / 100.0f;
	m_pSurface->SetValue( MIX_STRIP_ANY, id, MIX_PARAM_DYN_MAP, m_dwSurfaceID, fVal );

	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CControlSurfaceProbePropPage::OnBnClickedReadEnable()
{
	m_pSurface->ReadEnable( !!m_cRead.GetCheck() );
}


////////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbePropPage::OnMixArm() 
{
	m_pSurface->WriteEnable( !!m_cWrite.GetCheck() );
}

void CControlSurfaceProbePropPage::OnBnClickedLearnEnable()
{
	m_pSurface->SetLearnEnabled( m_cEnableLearn.GetCheck() ? true : false );
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomUp()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMV, MO_INCREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomUp2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMV, MO_INCREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomDown()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMV, MO_DECREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomDown2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMV, MO_DECREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomLeft()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMH, MO_DECREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomLeft2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMH, MO_DECREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomRight()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMH, MO_INCREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndZoomRight2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_ZOOMH, MO_INCREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollUp()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLV, MO_INCREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollUp2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLV, MO_INCREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollDown()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLV, MO_DECREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollDown2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLV, MO_DECREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollLeft()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLH, MO_DECREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollLeft2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLH, MO_DECREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollRight()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLH, MO_INCREASE_SMALL);
}

void CControlSurfaceProbePropPage::OnBnClickedWndScrollRight2()
{
   m_pSurface->DoWindowAction(m_windowType, WA_SCROLLH, MO_INCREASE_LARGE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndOpen()
{
   m_pSurface->SetWindowState(m_windowType, WST_OPEN);
}

void CControlSurfaceProbePropPage::OnBnClickedWndClose()
{
   m_pSurface->SetWindowState(m_windowType, WST_CLOSE);
}

void CControlSurfaceProbePropPage::OnBnClickedWndMin()
{
   m_pSurface->SetWindowState(m_windowType, WST_MINIMIZE);
}
