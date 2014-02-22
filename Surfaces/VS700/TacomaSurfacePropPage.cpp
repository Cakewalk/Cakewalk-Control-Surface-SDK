/////////////////////////////////////////////////////////////////////////////
// ACTControllerPropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "ACTController."
//

#include "stdafx.h"

#include "TacomaSurfacePropPage.h"
#include "HtmlHelp.h"

#include "ButtonPropsDlg.h"
#include "RegReminderDlg.h"

#include "..\..\CakeControls\NWControl.h"
#include "..\..\CakeControls\NWBitmapButton.h"
#include "..\..\CakeControls\NWDropDownCtrl.h"
#include "..\..\CakeControls\NWBitmapKnob.h"
#include "..\..\CakeControls\NWBitmapButton.h"
#include "..\..\CakeControls\NWHorizGradientCtrl.h"
#include "..\..\CakeControls\NWSliderKnobCtrl.h"
#include "..\..\CakeControls\OffscreenDC.h"
#include <algorithm>


//#include <itimeconvert.h>
//#include <AudAutoHelpers.h>
//#include "..\..\CakeControls\Properties.h"
//#include "..\..\CakeControls\CellPropEditCtrl.h"



// statics
bool							CTacomaSurfacePropPage::sm_bInitialized = false;
std::vector<CTacomaSurfacePropPage::NamedKey>	CTacomaSurfacePropPage::sm_vKeys;

static DWORD s_dwDMChans[] =
{ 
	0,1,2,3,4,5,6,7,				// mic pre chans
	16,17,18,19,20,21,22,23,		// adat
	14,15,							// digital 1
	10,11,							// synth
	12,13,							// arx
	8,9								// aux (only L channel used )
};

static DWORD s_dwOCDMChans[] =
{ 
	0,1,2,3,4,5,6,7, 8, 9,			//0-9 InputMixer
	10,11,12,13,14,15,16,17,18,19,	//0-9 OutPutMixer
	20,21,							//Master Input
	22,23							//Master OutPut
};

#define SET_ENCODER_PARAM_DW( i )   ( ( (DWORD) ( i + 1 ) ) << 10 )
#define GET_ENCODER_PARAM( i )      ( ( ( i ) >> 10 ) - 1 )
#define IS_ENCODER_PARAM( i )       ( ( ( ( (DWORD) ( i ) ) & 0xFFC00 ) >> 10 ) > 0 )

static const TacomaIOBoxParam s_aEncoderOptionsIO[] = { TIOP_DMixPan,
																		 TIOP_DMixVol,
																		 TIOP_Threshold,
																		 TIOP_Attack,
																		 TIOP_Release,
																		 TIOP_Ratio,
																		 TIOP_MakeupGain };

static const SEncoderParam s_aEncoderOptions[] = { { MIX_PARAM_PAN, 0, _T("Pan")},
                                                  { MIX_PARAM_VOL, 0, _T("Vol")},
                                                  { MIX_PARAM_VOL_TRIM, 0, _T("Trim") },
                                                  { MIX_PARAM_INPUT, 0, _T("Input") },
                                                  { MIX_PARAM_OUTPUT, 0, _T("Output") },
                                                  { MIX_PARAM_SEND_VOL, 0, _T("Send1 Vol") },
                                                  { MIX_PARAM_SEND_VOL, 1, _T("Send2 Vol") },
                                                  { MIX_PARAM_SEND_VOL, 2, _T("Send3 Vol") },
                                                  { MIX_PARAM_SEND_VOL, 3, _T("Send4 Vol") },
                                                  { MIX_PARAM_SEND_PAN, 0, _T("Send1 Pan") },
                                                  { MIX_PARAM_SEND_PAN, 1, _T("Send2 Pan") },
                                                  { MIX_PARAM_SEND_PAN, 2, _T("Send3 Pan") },
                                                  { MIX_PARAM_SEND_PAN, 3, _T("Send4 Pan") },
                                                  { MIX_PARAM_SEND_OUTPUT, 0, _T("Send1 Out") },
                                                  { MIX_PARAM_SEND_OUTPUT, 1, _T("Send2 Out") },
                                                  { MIX_PARAM_SEND_OUTPUT, 2, _T("Send3 Out") },
                                                  { MIX_PARAM_SEND_OUTPUT, 3, _T("Send4 Out") } };

static CString s_aType[] = {_T("OFF"), _T("ECHO"), _T("ROOM"), _T("SMALL HALL"), _T("LARGE HALL")};
static CString s_aPredelay[] = {_T("0.0ms"), _T("0.1ms"), _T("0.2ms"), _T("0.4ms"), _T("0.8ms"), _T("1.6ms"), 
										_T("3.2ms"), _T("6.4ms"), _T("10ms"), _T("20ms"), _T("40ms"), 
										_T("80ms"), _T("160ms") };

/////////////////////////////////////////////////////////////////////////////
//
// CTacomaSurfacePropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constructors

CTacomaSurfacePropPage::CTacomaSurfacePropPage(CWnd* pParent /*=NULL*/)
: CDialog(CTacomaSurfacePropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPatchBay( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
	,m_bCommandsDirty( false )
	,m_eDisplayPage( DP_Preamp )
	,m_pctlIOPresetSave( NULL )
	,m_pctlIOPresetDelete( NULL )
	,m_pbtnPrefTab(NULL)
	,m_pbtnPreampTab( NULL )
	,m_pbtnDMTab(NULL)
   ,m_pbtnACTTab(NULL)
	,m_pbtnOCTabA(NULL)
	,m_pbtnOCTabB(NULL)
	,m_pbtnOCTabC(NULL)
	,m_pbtnOCTabD(NULL)
	,m_pctlDisableFaderMotors (NULL)
	,m_pctlTouchSelects(NULL)
	,m_pctlMBT(NULL)
	,m_pctlHMSF(NULL)
	,m_pctlFlip(NULL)
	,m_pctlTracks(NULL)
	,m_pctlBuses(NULL)
	,m_pctlMains(NULL)
	,m_pctlIO(NULL)
	,m_pctlTrackEncFunc(NULL)
	,m_pctlBusEncFunc(NULL)
	,m_pctlIOEncFunc(NULL)
	,m_pfeetSw1(NULL)
	,m_pfeetSw2(NULL)
	,m_idbTabBkg(0)
	,m_nACTPage(0)
	,m_pctlACTContextLbl(NULL)
	,m_pctlReturn( NULL )
	,m_pctlRevTime( NULL ),
	m_pctlMarkersInd(NULL),
	m_pctlACTInd(NULL),
	m_pctlLayerInd(NULL),
	m_pctlFxInd(NULL),
	m_pTBarACTLbl( NULL ),
	m_pctlDigSync( NULL ),
	m_pctlInSync( NULL),
	m_pctlDamp( NULL),
	m_pBtnMasterLink( NULL )
{
	m_bInitDone = false;

	m_bIsOcta = false;

	m_uiTimerID = 0;

	//{{AFX_DATA_INIT(CTacomaSurfacePropPage)
	//}}AFX_DATA_INIT

	if ( !sm_bInitialized )
	{
		sm_bInitialized = true;

		NamedKey nk;
		for ( short c = _T('A'); c <= _T('Z'); c++ )
		{
			nk.strName = (TCHAR)c;
			nk.vk = c;
			sm_vKeys.push_back( nk );
		}
		for ( int i = 0; i <= 9; i++ )
		{
			nk.strName.Format( _T("Num %d"), i );
			nk.vk = VK_NUMPAD0 + i;
			sm_vKeys.push_back( nk );
		}
		for ( int i = 0; i < 12; i++ )
		{
			nk.strName.Format( _T("F%d"), i + 1 );
			nk.vk = VK_F1 + i;
			sm_vKeys.push_back( nk );
		}

		// more
		nk.vk = VK_TAB;
		nk.strName = _T("Tab");
		sm_vKeys.push_back( nk );
	}

		// Controls colors
	m_rgbCtrlBkgd = RGB(173,178,189); // RGB(0,37,0);
	m_rgbCtrlLEDBright = RGB(192,192,192);
	m_rgbCtrlLEDDark = RGB(47,79,79);


	CString strFont = _T("Arial");
	//strFont.LoadString ( IDS_NEWQUICKSTART_FONT );

	memset( &small_lf, 0, sizeof (small_lf) );
   ::_tcscpy(small_lf.lfFaceName, strFont);
	small_lf.lfHeight = 13;
	small_lf.lfWeight = FW_DEMIBOLD;
	//small_lf.lfQuality = CBaseApp::GetDefaultFontSmoothing();

	m_pBrush = new CBrush(RGB(30 ,30, 30));

}

/////////////////////////////////////////////////////////////////////////////

void CTacomaSurfacePropPage::DoDataExchange(CDataExchange* pDX)
{
   TRACE("CTacomaSurfacePropPage::DoDataExchange()\n");

   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CTacomaSurfacePropPage)
   //}}AFX_DATA_MAP
   DDX_Control(pDX, IDC_PRESET, m_cPresets);
   DDX_Control(pDX, IDC_INTERFACE, m_cInterface);
   DDX_Control(pDX, IDC_REVTYPE, m_cType);
   DDX_Control(pDX, IDC_REVPREDELAY, m_cPredelay);
	DDX_Control(pDX, IDC_FADER_DISABLE, m_sDisable);
	DDX_Control(pDX, IDC_FADER_TOUCH, m_sTouch);


}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTacomaSurfacePropPage, CDialog)
	//{{AFX_MSG_MAP(CTacomaSurfacePropPage)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_PRESET, OnCbnSelchangePreset)
	ON_CBN_SELCHANGE(IDC_INTERFACE, OnCbnSelchangeInterface)
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_REVPREDELAY, &CTacomaSurfacePropPage::OnCbnSelchangeRevpredelay)
	ON_CBN_SELCHANGE(IDC_REVTYPE, &CTacomaSurfacePropPage::OnCbnSelchangeRevtype)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTacomaSurfacePropPage::~CTacomaSurfacePropPage()
{
	cleanUpControlList();
	SAFE_DELETE(m_pBrush);
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CTacomaSurfacePropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CTacomaSurfacePropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	TRACE("CTacomaSurfacePropPage::SetObjects()\n");

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CTacomaSurface* const pSurface = dynamic_cast<CTacomaSurface*>(*ppUnk);
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



static NWCTRLATTRIB s_attrCmdBtn;
static NWCTRLATTRIB s_attrRadBtn;
static NWCTRLATTRIB s_attrParamBtn;
static NWCTRLATTRIB s_attrParamBlackBtn;
static NWCTRLATTRIB s_attrParamBlackOffsetBtn;
static NWCTRLATTRIB s_attrSliders;
static NWCTRLATTRIB s_attrSliders2;

static const CTacomaSurface::ControlId s_aid[] = {
				CTacomaSurface::BID_EZr0c0,CTacomaSurface::BID_EZr0c1,CTacomaSurface::BID_EZr0c2,CTacomaSurface::BID_EZr0c3,
				CTacomaSurface::BID_EZr1c0,CTacomaSurface::BID_EZr1c1,CTacomaSurface::BID_EZr1c2,CTacomaSurface::BID_EZr1c3, 
				CTacomaSurface::BID_EZr2c0,CTacomaSurface::BID_EZr2c1,CTacomaSurface::BID_EZr2c2,CTacomaSurface::BID_EZr2c3,
				CTacomaSurface::BID_EZr3c0,CTacomaSurface::BID_EZr3c1,CTacomaSurface::BID_EZr3c2,CTacomaSurface::BID_EZr3c3 
					};



//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

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

	// create controls
	s_attrCmdBtn.bModifySelf = FALSE;
	s_attrCmdBtn.bShowValue = TRUE;
	s_attrCmdBtn.nTextOffset = 3;
	s_attrCmdBtn.crBkNormal = RGB( 174,187,210 );

	s_attrRadBtn = s_attrCmdBtn;
	s_attrRadBtn.bShowValue = FALSE;

	s_attrParamBtn.bModifySelf = TRUE;
	s_attrParamBtn.bShowValue = FALSE;
	s_attrParamBtn.bShowLabel = TRUE;
	s_attrParamBtn.crBkNormal = RGB( 110,110,140);
	s_attrParamBtn.crTxtDisable = RGB(110,110,110);
	s_attrParamBtn.crBkHighlight = RGB( 220,220, 250 );
	s_attrParamBtn.crBkDisable = RGB( 90,90,90 );

	s_attrParamBlackBtn = s_attrParamBtn;
	s_attrParamBlackBtn.crBkNormal = RGB( 20, 20, 20 );
	s_attrParamBlackBtn.crTxtNormal = RGB( 110, 210, 210 );
	s_attrParamBlackBtn.crTxtDisable = RGB( 110, 110, 110 );
	s_attrParamBlackBtn.crBkHighlight = RGB( 20, 20, 20 );
	s_attrParamBlackBtn.crBkDisable = RGB( 20, 20, 20 );
	s_attrParamBlackBtn.bShowValue = TRUE;
	s_attrParamBlackBtn.bShowLabel = FALSE;
	s_attrParamBlackBtn.nTextOffset = 1;

	s_attrParamBlackOffsetBtn = s_attrParamBlackBtn;
	s_attrParamBlackOffsetBtn.nTextOffset = 5;

	s_attrSliders.bModifySelf = TRUE;
	s_attrSliders.bShowLabel = FALSE;
	s_attrSliders.bShowValue = TRUE;
	s_attrSliders.crBkHighlight = s_attrSliders.crBkNormal = RGB(40,40,50);
	s_attrSliders.crTxtNormal = RGB(200,200,200);
	s_attrSliders.crTxtDisable = RGB(80,80,80);
	s_attrSliders.bDynamicTips = FALSE;

	
	s_attrSliders2.bModifySelf = TRUE;
	s_attrSliders2.bShowLabel = FALSE;
	s_attrSliders2.bShowValue = TRUE;
	s_attrSliders2.crBkHighlight = s_attrSliders2.crBkNormal = RGB(240,240,250);
	s_attrSliders2.crTxtNormal = RGB(40,40,40);
	s_attrSliders2.crTxtDisable = RGB(80,80,80);
	s_attrSliders2.bDynamicTips = FALSE;

	createGlobalControls();
	createPrefControls();
	createACTControls();
	createIOControls();
	createDMControls();

	m_bCommandsDirty = true;

	onPrefTab();

	Initface();

	m_bActivate = true;

	StartTimer();

	return S_OK;
}

void CTacomaSurfacePropPage::createACTControls()
{
   const DWORD adwDisplayBtns[] = { IDB_DISPBTN1, IDB_DISPBTN2, IDB_DISPBTN3, IDB_DISPBTN4 };

	DWORD dwActID = ACTKEY_BASE;

	// Note we're using NWLabelButtons just for display convenience here.
	// There is no action when these are clicked.  They are only to display
	// the ACT parameter binding

	CSize sizActLabel(74,17);
 
	// Rotary ACT Label buttons
   for ( DWORD dwCntr = 0; dwCntr < 12; dwCntr ++ )
   {
      CNWLabelButton* pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( sizActLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtn );
		pCmb->SetParameter( dwActID++ );
      m_vRotaryACTLbl.push_back( pCmb );
   }

	// ACT Switch Labels
	for ( DWORD dw = 0; dw < 4; dw++ )
	{
      CNWLabelButton* pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( sizActLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtn );
		pCmb->SetParameter( dwActID++ );
		m_vSwitchACTLbl.push_back( pCmb );
	}

	// TBar ACT Label button
	m_pTBarACTLbl = new CNWLabelButton( this );
	addControl( m_pTBarACTLbl );
	m_pTBarACTLbl->SetSize( CSize( 72, 17 ) );
	m_pTBarACTLbl->SetSharedAttributes( &s_attrCmdBtn );
	m_pTBarACTLbl->SetParameter( dwActID );

   for ( DWORD dwCntr = 0; dwCntr < 4; dwCntr ++ )
   {
      CNWMultiBtn *pCtrl = new CNWMultiBtn( this );
      addControl( pCtrl );
      pCtrl->SetSharedAttributes( &s_attrCmdBtn );
      pCtrl->SetArt( adwDisplayBtns[ dwCntr ] );
      m_vACTPageButtons.push_back( pCtrl );
   }
}

//---------------------------------------------------------------------------
void CTacomaSurfacePropPage::createPrefControls()
{
	// get the  current command
	const CTacomaSurface::MsgIdToBADMap& mapCmds = m_pSurface->GetButtonCommandMap();

	// create Command buttons
	for ( size_t i = 0; i < _countof(s_aid); i++ )
	{
		CTacomaSurface::ControlId cid = s_aid[i];
		const CTacomaSurface::MsgIdToBADMap::const_iterator it = mapCmds.find( cid );
		if ( it == mapCmds.end() )
		{
			ASSERT(0);
			break;
		}
		CNWDropDownCtrl* pbtn = new CNWDropDownCtrl( this );
		addControl( pbtn );
		pbtn->SetSize( CSize( 75,18 ) );
		pbtn->SetSharedAttributes( &s_attrCmdBtn );
		pbtn->SetParameter( cid );
		m_mapCommandButtons[cid] = pbtn;

      CNWMultiBtn *pCtrl = new CNWMultiBtn( this );
      addControl( pCtrl );
      pCtrl->SetSharedAttributes( &s_attrCmdBtn );
      pCtrl->SetArt( IDB_PREFS_BTN );
      m_mapCommandButtonsBmp[ cid ] = pCtrl;
	}

	// command buttons for feetswitch
	m_pfeetSw1 =  new CNWDropDownCtrl( this );
	addControl( m_pfeetSw1 );
	m_pfeetSw1->SetSize( CSize( 90,18 ) );
	m_pfeetSw1->SetSharedAttributes( &s_attrCmdBtn );
	m_pfeetSw1->SetParameter( CTacomaSurface::BID_Footswitch1 );

	m_pfeetSw2 =  new CNWDropDownCtrl( this );
	addControl( m_pfeetSw2 );
	m_pfeetSw2->SetSize( CSize( 90,18 ) );
	m_pfeetSw2->SetSharedAttributes( &s_attrCmdBtn );
	m_pfeetSw2->SetParameter( CTacomaSurface::BID_Footswitch2 );

	// checkboxes
	m_pctlDisableFaderMotors = new CNWMultiBtn( this );
	addControl( m_pctlDisableFaderMotors );
	m_pctlDisableFaderMotors->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlDisableFaderMotors->SetArt( IDB_CHECKBOX, CNWMultiBtn::NS );
	m_pctlDisableFaderMotors->SetCtrlName( IDS_DISABLE_FADER_CHK );

	m_pctlTouchSelects = new CNWMultiBtn( this );
	addControl( m_pctlTouchSelects );
	m_pctlTouchSelects->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlTouchSelects->SetArt( IDB_CHECKBOX, CNWMultiBtn::NS );
	m_pctlTouchSelects->SetCtrlName( IDS_TOUCHSELECT_CHK );

   // Encoder
   //  btns
	m_pctlTrackEncFunc = new CNWMultiBtn( this );
	addControl( m_pctlTrackEncFunc );
	m_pctlTrackEncFunc->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlTrackEncFunc->SetArt( IDB_PREFS_BTN );

	m_pctlBusEncFunc = new CNWMultiBtn( this );
	addControl( m_pctlBusEncFunc );
	m_pctlBusEncFunc->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlBusEncFunc->SetArt( IDB_PREFS_BTN );

	m_pctlIOEncFunc = new CNWMultiBtn( this );
	addControl( m_pctlIOEncFunc );
	m_pctlIOEncFunc->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlIOEncFunc->SetArt( IDB_PREFS_BTN );

   //  ctrls
   DWORD dwCntr = 0;
	for ( size_t i = 0; i < 12; i++, dwCntr ++ )
	{
      if ( dwCntr >= 4 )
         dwCntr = 0;

		CNWDropDownCtrl* pbtn = new CNWDropDownCtrl( this );
		addControl( pbtn );
		pbtn->SetSize( CSize( 75, 18 ) );
		pbtn->SetSharedAttributes( &s_attrCmdBtn );
		pbtn->SetParameter( SET_ENCODER_PARAM_DW( dwCntr ) );
      m_vEncoderButtons.push_back( pbtn );
	}
}


//---------------------------------------------------------------------
void	CTacomaSurfacePropPage::createIOControls() 
{
	CSize sizParamBtn(19,13);
	DWORD dwParam = 0;
	for ( DWORD ix = 0; ix < 8; ix++ )
	{
		CNWControl* pc = NULL;

		IOChanControls iocc;
      iocc.pGain = new CNWSliderKnobCtrl( this, IDB_FADER_PRE_BK, IDB_FADER, 15, 15, 6, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iocc.pGain );
		dwParam = MAKELONG( ix, TIOP_Gain );
		iocc.pGain->SetParameter( dwParam );
      iocc.pGain->SetGenericType( TYPE_GENERIC_VOLUME );
		iocc.pGain->SetSharedAttributes( &s_attrSliders );
		//iocc.pGain->SetCtrlName( _T("Sense") );
		//iocc.pGain->SetNumValueSteps( 44 );

      iocc.pGainLabel = new CNWLabelButton( this );
		addControl( iocc.pGainLabel );
		iocc.pGainLabel->SetSharedAttributes( &s_attrParamBlackOffsetBtn );
		iocc.pGainLabel->SetSize( CSize( 38, 17 ) );
		dwParam = MAKELONG( ix, TIOP_Gain );
		iocc.pGainLabel->SetParameter( dwParam );

		// low cut
		CNWMultiBtn* pb = new CNWMultiBtn( this );
		iocc.pLoCut = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_LoCut );
		pb->SetParameter( dwParam );
      pb->SetArt( IDB_LOCUT, CNWMultiBtn::NSD );

		// Stereo link on odd channels
		if ( ix % 2 == 0 )
		{
		   pb = new CNWMultiBtn( this );
			iocc.pLink = pb;
			addControl( pb );
			pb->SetSharedAttributes( &s_attrParamBtn );
			dwParam = MAKELONG( ix, TIOP_StereoLink );
			pb->SetParameter( dwParam );
			pb->SetArt( IDB_IOLINK, CNWMultiBtn::NS );


		}
		else
			iocc.pLink = NULL;

		// Stereo link on odd channels - OCTA-CAPTURE
		if ( ix % 2 == 0 )
		{
		   pb = new CNWMultiBtn( this );
			iocc.pLinkOC = pb;
			addControl( pb );
			pb->SetSharedAttributes( &s_attrParamBtn );
			dwParam = MAKELONG( ix, TIOP_StereoLink );
			pb->SetParameter( dwParam );
			pb->SetArt( IDB_IOLINKOC, CNWMultiBtn::NS );


		}
		else
			iocc.pLink = NULL;

		// +48
		pb = new CNWMultiBtn( this );
		iocc.pPhantom = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_Phantom );
		pb->SetParameter( dwParam );
		pb->SetArt( IDB_PHANTOM_BTN, CNWMultiBtn::NSD );

		// pad
		pb = new CNWMultiBtn( this );
		iocc.pPad = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_Pad );
		pb->SetParameter( dwParam );
		pb->SetArt( IDB_PAD_BTN, CNWMultiBtn::NSD );

			// Hi-Z
		pb = new CNWMultiBtn( this );
		iocc.pHiz = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_Hiz );
		pb->SetParameter( dwParam );
		pb->SetArt( IDB_HIZ_BTN, CNWMultiBtn::NS );


		// Phase
		pb = new CNWMultiBtn( this );
		iocc.pPhase = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_Phase );
		pb->SetParameter( dwParam );
		pb->SetArt( IDB_PHASE_BTN, CNWMultiBtn::NSD );

		// comp switch
		pb = new CNWMultiBtn( this );
		iocc.pCompEnable = pb;
		addControl( pb );
		pb->SetSharedAttributes( &s_attrParamBtn );
		dwParam = MAKELONG( ix, TIOP_CompEnable );
		pb->SetParameter( dwParam );
		pb->SetArt( IDB_COMP, CNWMultiBtn::NSD );

      const DWORD dwX = 50;
		// Comp Attack
		iocc.pAttack = makeSlider( MAKELONG( ix, TIOP_Attack ), dwX );
		iocc.pAttack->SetCtrlName( _T("Attack") );

		// Comp Release
		iocc.pRelease = makeSlider( MAKELONG( ix, TIOP_Release ), dwX );
		iocc.pRelease->SetCtrlName( _T("Release") );

		// Compressor Ratio
		iocc.pRatio = makeSlider( MAKELONG( ix, TIOP_Ratio ), dwX );
		iocc.pRatio->SetCtrlName( _T("Ratio") ); 
		iocc.pRatio->SetNumValueSteps( 14 );

		// Compressor Threshold
		iocc.pThreshold = makeSlider( MAKELONG( ix, TIOP_Threshold ), dwX );
		iocc.pThreshold->SetCtrlName( _T("Threshold") );
		iocc.pThreshold->SetNumValueSteps( 60 );

		// Compressor Gate
		iocc.pGate = makeSlider( MAKELONG( ix, TIOP_Gate ), dwX );
		iocc.pGate->SetCtrlName( _T("Gate") );
		iocc.pGate->SetNumValueSteps( 50 );

		// Comp Gain
		iocc.pCompGain = makeSlider( MAKELONG( ix, TIOP_MakeupGain ), dwX );
		iocc.pCompGain->SetCtrlName( _T("Comp Gain") );
		iocc.pCompGain->SetNumValueSteps( 60 );

		m_aIOControls.push_back( iocc );
	}

	CString strNames[] = { _T("sync_internal"), _T("sync_WordClk"), _T("sync_Digi1"), _T("sync_Digi2"), _T("digi_aes"), _T("digi_coax") };
   for ( DWORD dwCntr = 0; dwCntr < _countof(strNames); dwCntr ++ )
   {
      CNWMultiBtn *pCtrl = new CNWMultiBtn( this );
      addControl( pCtrl );
		pCtrl->SetCtrlName( strNames[dwCntr] );
      pCtrl->SetSharedAttributes( &s_attrCmdBtn );
      pCtrl->SetArt( IDB_RAD, CNWMultiBtn::NS );
      m_vRad.push_back( pCtrl );
   }
}

//------------------------------------------------------------------
void	CTacomaSurfacePropPage::createDMControls()
{
	CSize sizParamBtn(19,13);
	DWORD dwParam = 0;
	UINT idBitmap = 0;
	UINT idBitmapLink = IDB_IOLINK;
	UINT idBitmapOC = 0;
	UINT idBitmapFader = 0;

	//  Direct Mix controls
	for ( size_t ix = 0; ix < TacomaIOBox::NumChannels; ix++ )
	{
		//create different fader backgrounds for each VS-700 section
		if (( ix >= 0 ) &&  (ix <= 7 ))
			idBitmap = IDB_FADER_DM_ANAIN;
		else
			if (( ix >= 8 ) && ( ix <= 15 ))
			idBitmap = IDB_FADER_DM_DIG2;
		else
			if (( ix >= 16 ) && (ix <= 17 ))
			idBitmap = IDB_FADER_DM_DIG1;
		else
			if (( ix >= 18 ) && ( ix <= 19 ))
			idBitmap = IDB_FADER_DM_ARX;
		else
			if (( ix >= 20 ) && ( ix <= 21 ))
			idBitmap = IDB_FADER_DM_DIG2;
		else
			if ( ix == 22 )
			idBitmap = IDB_FADER_DM_DIG2;


		//create different fader backgrounds for each Octa- Capture Section
		if (( ix >= 0 ) &&  (ix <= 9 ))
		{
			idBitmapOC = IDB_FADER_OC_MIXIN;
			idBitmapFader = IDB_FADER;
		}
		if (( ix >= 10 ) &&  (ix <= 19 ))
		{
			idBitmapOC = IDB_FADER_OC_MIXOUT;
			idBitmapFader = IDB_FADER;
		}
	
		if (( ix >= 20 ) &&  (ix <= 23 ))
		{
			idBitmapOC = IDB_FADER_OC_MASTER;
			idBitmapFader = IDB_FADER_OC;
			idBitmapLink = IDB_IOLINKOC;

		}
		
		//VS-700

		// vol
		IODirectMixControls iodmc;
		iodmc.pDMVol = new CNWSliderKnobCtrl( this, idBitmap, IDB_FADER, 15, 15, 4, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iodmc.pDMVol );
		iodmc.pDMVol->SetSnapValue( .787 );
		iodmc.pDMVol->SetSharedAttributes( &s_attrSliders );
		
		// hack:  There is no such thing as channel 10 in the UI (ix 9). We need to skip it with our
		// paramter numbers
		DWORD dwChan = s_dwDMChans[ix];
		dwParam = MAKELONG( dwChan, TIOP_DMixVol );
		iodmc.pDMVol->SetParameter( dwParam );
		iodmc.pDMVol->SetCtrlName( _T("DMVol") );
		iodmc.pDMVol->SetGenericType( TYPE_GENERIC_VOLUME );

        iodmc.pDMVolLabel = new CNWLabelButton( this );
		addControl( iodmc.pDMVolLabel );
		iodmc.pDMVolLabel->SetSharedAttributes( &s_attrParamBlackBtn );
		iodmc.pDMVolLabel->SetSize( CSize( 33, 14 ) );
		dwParam = MAKELONG( dwChan, TIOP_DMixVol );
		iodmc.pDMVolLabel->SetParameter( dwParam );

		//Octa-Capture
		// vol
//----------------------------------------------------------------------------------------------------------------------------------------			
		iodmc.pOCVol = new CNWSliderKnobCtrl( this, idBitmapOC, idBitmapFader, 15, 15, 4, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iodmc.pOCVol );
		iodmc.pOCVol->SetSnapValue( .5 );
		iodmc.pOCVol->SetSharedAttributes( &s_attrSliders );
	
		// hack:  There is no such thing as channel 10 in the UI (ix 9). We need to skip it with our
		// paramter numbers
		DWORD dwChanOC = s_dwOCDMChans[ix];
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVol->SetParameter( dwParam );
		iodmc.pOCVol->SetCtrlName( _T("OCVolA") );
		iodmc.pOCVol->SetGenericType( TYPE_GENERIC_VOLUME );

		iodmc.pOCVolLabel = new CNWLabelButton( this );
		addControl( iodmc.pOCVolLabel );
		iodmc.pOCVolLabel->SetSharedAttributes( &s_attrParamBlackBtn );
		iodmc.pOCVolLabel->SetSize( CSize( 33, 14 ) );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolLabel->SetParameter( dwParam );



      
		iodmc.pOCVolB = new CNWSliderKnobCtrl( this, idBitmapOC, idBitmapFader, 15, 15, 4, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iodmc.pOCVolB );
		iodmc.pOCVolB->SetSnapValue( .5 );
		iodmc.pOCVolB->SetSharedAttributes( &s_attrSliders );
			// hack:  There is no such thing as channel 10 in the UI (ix 9). We need to skip it with our
		// paramter numbers
		//DWORD dwChanOC = s_dwOCDMChans[ix];
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolB->SetParameter( dwParam );
		iodmc.pOCVolB->SetCtrlName( _T("OCVolB") );
		iodmc.pOCVolB->SetGenericType( TYPE_GENERIC_VOLUME );

		iodmc.pOCVolLabelB = new CNWLabelButton( this );
		addControl( iodmc.pOCVolLabelB );
		iodmc.pOCVolLabelB->SetSharedAttributes( &s_attrParamBlackBtn );
		iodmc.pOCVolLabelB->SetSize( CSize( 33, 14 ) );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolLabelB->SetParameter( dwParam );


		iodmc.pOCVolC = new CNWSliderKnobCtrl( this, idBitmapOC, idBitmapFader, 15, 15, 4, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iodmc.pOCVolC );
		iodmc.pOCVolC->SetSnapValue( .5 );
		iodmc.pOCVolC->SetSharedAttributes( &s_attrSliders );
				// hack:  There is no such thing as channel 10 in the UI (ix 9). We need to skip it with our
		// paramter numbers
		//DWORD dwChanOC = s_dwOCDMChans[ix];
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolC->SetParameter( dwParam );
		iodmc.pOCVolC->SetCtrlName( _T("OCVol") );
		iodmc.pOCVolC->SetGenericType( TYPE_GENERIC_VOLUME );

		iodmc.pOCVolLabelC = new CNWLabelButton( this );
		addControl( iodmc.pOCVolLabelC );
		iodmc.pOCVolLabelC->SetSharedAttributes( &s_attrParamBlackBtn );
		iodmc.pOCVolLabelC->SetSize( CSize( 33, 14 ) );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolLabelC->SetParameter( dwParam );

		iodmc.pOCVolD = new CNWSliderKnobCtrl( this, idBitmapOC, idBitmapFader, 15, 15, 4, CNWSliderKnobCtrl::ST_NONE, 2 );
		addControl( iodmc.pOCVolD );
		iodmc.pOCVolD->SetSnapValue( .5 );
		iodmc.pOCVolD->SetSharedAttributes( &s_attrSliders );
		// hack:  There is no such thing as channel 10 in the UI (ix 9). We need to skip it with our
		// paramter numbers
		//DWORD dwChanOC = s_dwOCDMChans[ix];
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolD->SetParameter( dwParam );
		iodmc.pOCVolD->SetCtrlName( _T("OCVol") );
		iodmc.pOCVolD->SetGenericType( TYPE_GENERIC_VOLUME );

		iodmc.pOCVolLabelD = new CNWLabelButton( this );
		addControl( iodmc.pOCVolLabelD );
		iodmc.pOCVolLabelD->SetSharedAttributes( &s_attrParamBlackBtn );
		iodmc.pOCVolLabelD->SetSize( CSize( 33, 14 ) );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixVol );
		iodmc.pOCVolLabelD->SetParameter( dwParam );
//----------------------------------------------------------------------------------------------------------------------------------------


      // send
		iodmc.pDMSend = makeSlider( MAKELONG( dwChanOC, TIOP_DMixSend ), 35 );
		iodmc.pDMSend->SetCtrlName( _T("DMSend") );
				
		// pan
		iodmc.pDMPan = makeSlider( MAKELONG( dwChan, TIOP_DMixPan ), 35 );
		iodmc.pDMPan->SetCtrlName( _T("DMPan") );

		 // pan
		iodmc.pOCPan = makeSlider( MAKELONG( dwChanOC, TIOP_DMixPan ), 35 );
		iodmc.pOCPan->SetCtrlName( _T("OCPanA") );

		// pan
		iodmc.pOCPanB = makeSlider( MAKELONG( dwChanOC, TIOP_DMixPan ), 35 );
		iodmc.pOCPanB->SetCtrlName( _T("OCPanB") );

		// pan
		iodmc.pOCPanC = makeSlider( MAKELONG( dwChanOC, TIOP_DMixPan ), 35 );
		iodmc.pOCPanC->SetCtrlName( _T("OCPanC") );

		// pan
		iodmc.pOCPanD = makeSlider( MAKELONG( dwChanOC, TIOP_DMixPan ), 35 );
		iodmc.pOCPanD->SetCtrlName( _T("OCPanD") );

		CNWMultiBtn *pBtn = NULL;

		// Mono
		if ( ix % 2 == 0 && ix < TacomaIOBox::NumChannels ) // odd channels only, zero based index
		{
			pBtn = new CNWMultiBtn( this );
			iodmc.pDMMono = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( IDB_MONO_BTN, CNWMultiBtn::NSD );
			dwParam = MAKELONG( dwChan, TIOP_DMixMono );
			pBtn->SetParameter( dwParam );

			pBtn = new CNWMultiBtn( this );
			iodmc.pLink = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( IDB_DM_LINK, CNWMultiBtn::NSD );
			dwParam = MAKELONG( dwChan, TIOP_StereoLink );
			pBtn->SetParameter( dwParam );

			pBtn = new CNWMultiBtn( this );
			iodmc.pLinkOC = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( idBitmapLink, CNWMultiBtn::NS );
			dwParam = MAKELONG( dwChanOC, TIOP_StereoLinkOC );
			pBtn->SetParameter( dwParam );

			pBtn = new CNWMultiBtn( this );
			iodmc.pLinkOCB = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( idBitmapLink, CNWMultiBtn::NS );
			dwParam = MAKELONG( dwChanOC, TIOP_StereoLinkOC );
			pBtn->SetParameter( dwParam );


			pBtn = new CNWMultiBtn( this );
			iodmc.pLinkOCC = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( idBitmapLink, CNWMultiBtn::NS );
			dwParam = MAKELONG( dwChanOC, TIOP_StereoLinkOC );
			pBtn->SetParameter( dwParam );



			pBtn = new CNWMultiBtn( this );
			iodmc.pLinkOCD = pBtn;
			addControl( pBtn );
			pBtn->SetSharedAttributes( &s_attrParamBtn );
			pBtn->SetArt( idBitmapLink, CNWMultiBtn::NS );
			dwParam = MAKELONG( dwChanOC, TIOP_StereoLinkOC );
			pBtn->SetParameter( dwParam );


		}
		else 


		{
			iodmc.pDMMono = NULL;
			iodmc.pLink = NULL;
			iodmc.pLinkOC = NULL;
			iodmc.pLinkOCB = NULL;
			iodmc.pLinkOCC = NULL;
			iodmc.pLinkOCD = NULL;
		}

		// mute
		pBtn = new CNWMultiBtn( this );
		iodmc.pDMMute = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChan, TIOP_DMixMute );
		pBtn->SetParameter( dwParam );

		// solo
		pBtn = new CNWMultiBtn( this );
		iodmc.pDMSolo = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChan, TIOP_DMixSolo );
		pBtn->SetParameter( dwParam );

		// mute
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCMute = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixMute );
		pBtn->SetParameter( dwParam );


		// mute
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCMuteB = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixMute );
		pBtn->SetParameter( dwParam );

		// mute
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCMuteC = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixMute );
		pBtn->SetParameter( dwParam );

		// mute
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCMuteD = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixMute );
		pBtn->SetParameter( dwParam );

		// solo
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCSolo = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixSolo );
		pBtn->SetParameter( dwParam );

		// solo
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCSoloB = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixSolo );
		pBtn->SetParameter( dwParam );

		// solo
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCSoloC = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixSolo );
		pBtn->SetParameter( dwParam );


		// solo
		pBtn = new CNWMultiBtn( this );
		iodmc.pOCSoloD = pBtn;
		addControl( pBtn );
		pBtn->SetSharedAttributes( &s_attrParamBtn );
		pBtn->SetArt( IDB_MUTESOLO_BTN, CNWMultiBtn::NSD );
		dwParam = MAKELONG( dwChanOC, TIOP_DMixSolo );
		pBtn->SetParameter( dwParam );


		m_aDMControls.push_back( iodmc );
	}

	const TacomaIOBoxParam params[] = { TIOP_DMOutMain, TIOP_DMOutSub, TIOP_DMOutDig };
   for ( DWORD dwCntr = 0; dwCntr < _countof(params); dwCntr ++ )
   {
      CNWMultiBtn *pCtrl = new CNWMultiBtn( this );
      addControl( pCtrl );
      pCtrl->SetSharedAttributes( &s_attrRadBtn );
      pCtrl->SetArt( IDB_DM_RADIO, CNWMultiBtn::NS );
		pCtrl->SetParameter( (DWORD) params[ dwCntr ] );
      m_vDMRad.push_back( pCtrl );
   }

   // return
	m_pctlReturn = makeSlider( MAKELONG( 20, TIOP_DMixReturn), 35 );
	m_pctlReturn->SetCtrlName( _T("DMReturn") );


	// Reverb Time / Damp Control
	m_pctlRevTime = makeSlider2( MAKELONG( 0, TIOP_RevTime), 42 );
	m_pctlRevTime->SetCtrlName( _T("Reverb Time") );

	
	m_pBtnMasterLink = new CNWMultiBtn( this );
	addControl( m_pBtnMasterLink );
	m_pBtnMasterLink->SetSharedAttributes( &s_attrParamBtn );
	m_pBtnMasterLink->SetArt( IDB_IOLINK, CNWMultiBtn::NS );
	dwParam = MAKELONG( 20, TIOP_MasterLinkOC );
	m_pBtnMasterLink->SetParameter( dwParam );
}

void	CTacomaSurfacePropPage::createGlobalControls()
{
	DWORD dwParam = 0;

   // tabs
   m_pbtnPrefTab = new CNWMultiBtn( this );
	addControl( m_pbtnPrefTab );
	m_pbtnPrefTab->SetCtrlName( _T("Prefs Tab" ) );
	m_pbtnPrefTab->SetArt( IDB_PREFS_TAB, CNWMultiBtn::NS );
	m_pbtnPrefTab->SetSharedAttributes( &s_attrCmdBtn );

	m_pbtnPreampTab = new CNWMultiBtn( this );
	addControl( m_pbtnPreampTab );
	m_pbtnPreampTab->SetCtrlName( _T("Preamp Tab") );
	m_pbtnPreampTab->SetArt( IDB_MICPRE_TAB, CNWMultiBtn::NSD );
	m_pbtnPreampTab->SetSharedAttributes( &s_attrCmdBtn );

	m_pbtnDMTab = new CNWMultiBtn( this );
	addControl( m_pbtnDMTab );
	m_pbtnDMTab->SetCtrlName( _T("DM Tab" ) );
	m_pbtnDMTab->SetArt( IDB_DM_TAB, CNWMultiBtn::NSD );
	m_pbtnDMTab->SetSharedAttributes( &s_attrCmdBtn );

	m_pbtnACTTab = new CNWMultiBtn( this );
	addControl( m_pbtnACTTab );
	m_pbtnACTTab->SetCtrlName( _T("ACT Tab" ) );
	m_pbtnACTTab->SetArt( IDB_ACT_TAB, CNWMultiBtn::NS );
	m_pbtnACTTab->SetSharedAttributes( &s_attrCmdBtn );

	
   // buttons
	m_pctlFlip = new CNWMultiBtn( this );
	addControl( m_pctlFlip );
	m_pctlFlip->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlFlip->SetArt( IDB_LARGE_BTN );

	m_pctlTracks = new CNWMultiBtn( this );
	addControl( m_pctlTracks );
	m_pctlTracks->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlTracks->SetArt( IDB_LARGE_BTN );

	m_pctlBuses = new CNWMultiBtn( this );
	addControl( m_pctlBuses );
	m_pctlBuses->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlBuses->SetArt( IDB_LARGE_BTN );

	m_pctlMains = new CNWMultiBtn( this );
	addControl( m_pctlMains );
	m_pctlMains->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlMains->SetArt( IDB_LARGE_BTN );

	m_pctlIO = new CNWMultiBtn( this );
	addControl( m_pctlIO );
	m_pctlIO->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlIO->SetArt( IDB_LARGE_BTN );

	m_pctlIOPresetSave = new CNWMultiBtn( this );
	addControl( m_pctlIOPresetSave );
	m_pctlIOPresetSave->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlIOPresetSave->SetArt( IDB_PRE_SAVE_BTN, CNWMultiBtn::NORM  );

	m_pctlIOPresetDelete = new CNWMultiBtn( this );
	addControl( m_pctlIOPresetDelete );
	m_pctlIOPresetDelete->SetSharedAttributes( &s_attrCmdBtn );
	m_pctlIOPresetDelete->SetArt( IDB_PRE_DELETE_BTN, CNWMultiBtn::NORM );

	m_pbtnOCTabA = new CNWMultiBtn( this );
	addControl( m_pbtnOCTabA );
	m_pbtnOCTabA->SetArt( IDB_OCTAB_A,CNWMultiBtn::NS);
	m_pbtnOCTabA->SetSharedAttributes( &s_attrParamBtn );
	dwParam = TIOP_DirectMix;
	m_pbtnOCTabA->SetParameter( dwParam );

	m_pbtnOCTabB = new CNWMultiBtn( this );
	addControl( m_pbtnOCTabB );
	m_pbtnOCTabB->SetArt( IDB_OCTAB_B, CNWMultiBtn::NS );
	m_pbtnOCTabB->SetSharedAttributes( &s_attrParamBtn );
	dwParam = TIOP_DirectMix;;
	m_pbtnOCTabB->SetParameter( dwParam );

	m_pbtnOCTabC = new CNWMultiBtn( this );
	addControl( m_pbtnOCTabC );
	m_pbtnOCTabC->SetArt( IDB_OCTAB_C, CNWMultiBtn::NS );
	m_pbtnOCTabC->SetSharedAttributes( &s_attrParamBtn );
	dwParam = TIOP_DirectMix;;
	m_pbtnOCTabC->SetParameter( dwParam );

	m_pbtnOCTabD = new CNWMultiBtn( this );
	addControl( m_pbtnOCTabD );
	m_pbtnOCTabD->SetArt( IDB_OCTAB_D, CNWMultiBtn::NS );
	m_pbtnOCTabD->SetSharedAttributes( &s_attrParamBtn );
	dwParam = TIOP_DirectMix;;
	m_pbtnOCTabD->SetParameter( dwParam );

	m_pbtnOCBay = new CNWMultiBtn( this );
	addControl( m_pbtnOCBay );
	m_pbtnOCBay->SetArt( IDB_OCBAY_BTN );
	m_pbtnOCBay->SetSharedAttributes( &s_attrParamBtn );


	// indicators
   m_pctlMarkersInd = new CNWMultiBtn( this );
	addControl( m_pctlMarkersInd );
	m_pctlMarkersInd->SetSharedAttributes( &s_attrParamBtn );
	m_pctlMarkersInd->SetArt( IDB_MARKERS_IND, CNWMultiBtn::NS );

   m_pctlACTInd = new CNWMultiBtn( this );
	addControl( m_pctlACTInd );
	m_pctlACTInd->SetSharedAttributes( &s_attrParamBtn );
	m_pctlACTInd->SetArt( IDB_ACT_IND, CNWMultiBtn::NS );

	m_pctlOUT12Ind = new CNWMultiBtn( this );
	addControl( m_pctlOUT12Ind );
	m_pctlOUT12Ind->SetSharedAttributes( &s_attrParamBtn );
	m_pctlOUT12Ind->SetArt( IDB_OUT12_IND, CNWMultiBtn::NS );

	m_pctlOUT34Ind = new CNWMultiBtn( this );
	addControl( m_pctlOUT34Ind );
	m_pctlOUT34Ind->SetSharedAttributes( &s_attrParamBtn );
	m_pctlOUT34Ind->SetArt( IDB_OUT34_IND, CNWMultiBtn::NS );

	m_pctlOUT56Ind = new CNWMultiBtn( this );
	addControl( m_pctlOUT56Ind );
	m_pctlOUT56Ind->SetSharedAttributes( &s_attrParamBtn );
	m_pctlOUT56Ind->SetArt( IDB_OUT56_IND, CNWMultiBtn::NS );

	m_pctlOUT78Ind = new CNWMultiBtn( this );
	addControl( m_pctlOUT78Ind );
	m_pctlOUT78Ind->SetSharedAttributes( &s_attrParamBtn );
	m_pctlOUT78Ind->SetArt( IDB_OUT78_IND, CNWMultiBtn::NS );

	m_pctlOUT910Ind = new CNWMultiBtn( this );
	addControl( m_pctlOUT910Ind );
	m_pctlOUT910Ind->SetSharedAttributes( &s_attrParamBtn );
	m_pctlOUT910Ind->SetArt( IDB_OUT910_IND, CNWMultiBtn::NS );

	
	// act context label - just a transparent label button with no actions
	m_pctlACTContextLbl = new CNWLabelButton( this );
	addControl( m_pctlACTContextLbl );
	NWCTRLATTRIB attrAct;
	attrAct.dwFlags = NWCTRLATTRIB::FDrawTransparent|NWCTRLATTRIB::FTxtNormal|NWCTRLATTRIB::FShowLabel|NWCTRLATTRIB::FShowValue;
	attrAct.bTransparent = TRUE;
	attrAct.bShowLabel = FALSE;
	attrAct.bShowValue = TRUE;
	attrAct.crTxtNormal = RGB(255,168,29 );
	m_pctlACTContextLbl->SetAttributes( attrAct );

	
	m_pctlLayerInd = new CNWMultiBtn( this );
	addControl( m_pctlLayerInd );
	m_pctlLayerInd->SetSharedAttributes( &s_attrParamBtn );
	m_pctlLayerInd->SetArt( IDB_LAYER_IND, CNWMultiBtn::NS );

	m_pctlFxInd = new CNWMultiBtn( this );
	addControl( m_pctlFxInd );
	m_pctlFxInd->SetSharedAttributes( &s_attrParamBtn );
	m_pctlFxInd->SetArt( IDB_FX_IND, CNWMultiBtn::NS );

	m_pctlDigSync = new CNWMultiBtn( this );
	addControl( m_pctlDigSync );
	m_pctlDigSync->SetSharedAttributes( &s_attrParamBtn );
	m_pctlDigSync->SetArt( IDB_SYNC_DIG, CNWMultiBtn::NS );

	m_pctlInSync = new CNWMultiBtn( this );
	addControl( m_pctlInSync );
	m_pctlInSync->SetSharedAttributes( &s_attrParamBtn );
	m_pctlInSync->SetArt( IDB_SYNC_IN, CNWMultiBtn::NS );


	//----------------------------------------------------
		NWCTRLATTRIB attrClose;
	attrClose.dwFlags = NWCTRLATTRIB::FShowLabel | NWCTRLATTRIB::FHover | NWCTRLATTRIB::FBkNormal | NWCTRLATTRIB::FShowFocus |
						NWCTRLATTRIB::FTxtNormal | NWCTRLATTRIB::FTextAlign | NWCTRLATTRIB::FShowValue | NWCTRLATTRIB::FDynamicTips |
						NWCTRLATTRIB::FDrawTransparent | NWCTRLATTRIB::FBoldTxt | NWCTRLATTRIB::FShortLabel;
	attrClose.bShowLabel = FALSE;
	attrClose.bShowValue = TRUE;
	attrClose.bTransparent = TRUE;
	attrClose.bHover = TRUE;
	attrClose.bShowFocus = TRUE;
	attrClose.crBkNormal = m_rgbCtrlBkgd;
	attrClose.crTxtNormal = m_rgbCtrlLEDDark;
	attrClose.fTextAlign = DT_CENTER;
	attrClose.bDynamicTips = FALSE;
	attrClose.bBoldTxt = TRUE;


	NWCTRLATTRIB attrDrop = attrClose;
	attrDrop.bShowLabel = FALSE;
	attrDrop.crTxtNormal = m_rgbCtrlLEDDark;
	

}


//-------------------------------------------------------------
CNWControl*	CTacomaSurfacePropPage::makeSlider( DWORD dwParam, DWORD dwX )
{
	CSize sizSlider( dwX, 16 );

	CNWHorizGradientCtrl* pS = NULL;
	pS = new CNWHorizGradientCtrl( this, 0, 2, CNWHorizGradientCtrl::ePAN );
	addControl( pS );
	pS->SetMargins( 2, 2, 3, 3 );
	pS->SetSharedAttributes( &s_attrSliders );
	pS->SetSize( sizSlider );
	pS->SetColors( RGB(110,130,140), .2 );

	pS->SetParameter( dwParam );

	return pS;
}

//-------------------------------------------------------------
CNWControl*	CTacomaSurfacePropPage::makeSlider2( DWORD dwParam, DWORD dwX )
{
	CSize sizSlider( dwX, 20 );

	CNWHorizGradientCtrl* pS = NULL;
	pS = new CNWHorizGradientCtrl( this, 0, 2, CNWHorizGradientCtrl::ePAN );
	addControl( pS );
	pS->SetMargins( 2, 2, 3, 3 );
	pS->SetSharedAttributes( &s_attrSliders2 );
	pS->SetSize( sizSlider );
	pS->SetColors( RGB(110,130,140), .2 );

	pS->SetParameter( dwParam );

	return pS;
}


void	CTacomaSurfacePropPage::onPreampTab()
{
	m_eDisplayPage = DP_Preamp;
	m_idbTabBkg = IDB_PREAMP_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 404, 90, 110, 17, SWP_NOZORDER );
	fillPresetCombo();
	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();
}

void	CTacomaSurfacePropPage::onOCPreampTab()
{
	m_eDisplayPage = OC_Preamp;
	m_idbTabBkg = IDB_OC_PREAMP_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 374, 90, 225, 17, SWP_NOZORDER );
	fillPresetCombo();
	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();
}


void	CTacomaSurfacePropPage::onPrefTab()
{
	m_eDisplayPage = DP_Preferences;
	m_idbTabBkg = IDB_PREFS_BKG;
	RecalcLayout();
	Invalidate();

	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();
	m_sDisable.SetWindowPos( NULL, 784, 147, 170, 17, SWP_NOSIZE);
	m_sTouch.SetWindowPos( NULL, 784, 174, 170, 17, SWP_NOSIZE);
}

void CTacomaSurfacePropPage::onACTTab()
{
	m_eDisplayPage = DP_ACT;
	m_idbTabBkg = IDB_ACT_BKG;
	RecalcLayout();
	Invalidate();

	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();
}

void	CTacomaSurfacePropPage::onDMTab()
{
	m_eDisplayPage = DP_DirectMixer;
	m_idbTabBkg = IDB_DM_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 365, 86, 165, 15, SWP_NOZORDER );
	fillPresetCombo();
	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();
}

void	CTacomaSurfacePropPage::onOCDMTabA()
{
	m_eDisplayPage = OC_DirectMixerA;
	m_idbTabBkg = IDB_DMA_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 465, 90, 135, 15, SWP_NOZORDER );
	fillPresetCombo();
	
	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();

	m_cType.SetWindowPos( NULL, 900, 152, 59, 12, SWP_NOZORDER );
	m_cType.SetDroppedWidth(70);
	fillTypeCombo();

	m_cPredelay.SetWindowPos( NULL, 911, 196, 54, 12, SWP_NOZORDER );
	fillPredelayCombo();

	m_pSurface->GetIOBoxInterface()->m_byDirectMix = DA;

}

void	CTacomaSurfacePropPage::onOCDMTabB()
{
	m_eDisplayPage = OC_DirectMixerB;
	m_idbTabBkg = IDB_DMB_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 465, 90, 135, 15, SWP_NOZORDER );
	fillPresetCombo();

	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();

	m_pSurface->GetIOBoxInterface()->m_byDirectMix = DB;
}
void	CTacomaSurfacePropPage::onOCDMTabC()
{
	m_eDisplayPage = OC_DirectMixerC;
	m_idbTabBkg = IDB_DMC_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 465, 90, 135, 15, SWP_NOZORDER );
	fillPresetCombo();

	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();

	m_pSurface->GetIOBoxInterface()->m_byDirectMix = DC;
}

void	CTacomaSurfacePropPage::onOCDMTabD()
{
	m_eDisplayPage = OC_DirectMixerD;
	m_idbTabBkg = IDB_DMD_BKG;
	RecalcLayout();
	Invalidate();

	m_cPresets.SetWindowPos( NULL, 465, 90, 135, 15, SWP_NOZORDER );
	fillPresetCombo();

	m_cInterface.SetWindowPos( NULL, 103, 9, 150, 17, SWP_NOZORDER );
	fillInterfaceCombo();

	m_pSurface->GetIOBoxInterface()->m_byDirectMix = DD;
}

//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Show( UINT nCmdShow )
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	// Ignore wrong show flags
	if (nCmdShow != SW_SHOW && nCmdShow != SW_SHOWNORMAL && nCmdShow != SW_HIDE)
		return E_INVALIDARG;

	ShowWindow( nCmdShow );

	return S_OK;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	StopTimer();

	DestroyMFCDialog();
	return S_OK;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;
	return S_OK;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::Help( LPCWSTR lpszHelpDir )
{
	m_HelpHelper.Launch( HELP_FINDER, 0 );

	return S_OK;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}


//--------------------------------------------------------------------
HRESULT CTacomaSurfacePropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "VS-700 Controller";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );


	// really measure the bitmap
	CBitmap bmp;
	bmp.LoadBitmap( IDB_IOPANEL );
	BITMAP bm;
	bmp.GetBitmap( &bm );

	// Populate the page info structure
	pPageInfo->cb					= sizeof(PROPPAGEINFO);
	pPageInfo->pszDocString		= NULL;
	pPageInfo->pszHelpFile		= NULL;
	pPageInfo->dwHelpContext	= 0;
	pPageInfo->size.cx			= bm.bmWidth;
	pPageInfo->size.cy			= bm.bmHeight;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// End of IPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------
void CTacomaSurfacePropPage::StartTimer()
{
	m_uiTimerID = SetTimer(1, 200, NULL);
}

////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
void CTacomaSurfacePropPage::StopTimer()
{
	KillTimer(m_uiTimerID);
}

////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
void CTacomaSurfacePropPage::LoadAllFields()
{
	//TRACE("CTacomaSurfacePropPage::LoadAllFields()\n");

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
}


////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.

// Registry keys to control Nagging
static LPCTSTR s_szSurfacesHive = _T("Software\\Cakewalk Music Software\\ControlSurfaces");
static LPCTSTR s_szRegReminderKey = _T("vs700RegRemind");
static LPCTSTR s_szRegAppKey = _T("SOFTWARE\\Cakewalk Music Software\\Shared Surfaces");
static LPCTSTR s_szDisableReminder = _T("vs700NoRegRemind");

//--------------------------------------------------------------------
BOOL CTacomaSurfacePropPage::OnInitDialog() 
{
	BOOL bRet = CDialog::OnInitDialog();

	// init the help system				
	CString strPath;
	TCHAR szDllPath[_MAX_PATH];

	MakeSurfacePathName( AfxGetInstanceHandle(), szDllPath, _MAX_PATH );
	strPath = szDllPath;
	CString strHelpFile;
	strHelpFile.Format( _T("%s\\%s"), strPath, SZAPPHELPFILE );
	m_HelpHelper.Init( AfxGetInstanceHandle(), GetSafeHwnd(), strHelpFile );

	// OK see if a disable nagging key has been written to HKLM
	bool bRemindReg = false;
	HKEY hkApp = NULL;
	::RegOpenKeyEx( HKEY_LOCAL_MACHINE, s_szRegAppKey, 0, KEY_QUERY_VALUE, &hkApp );
	if ( hkApp )
	{
		// if we fail querying this key, it does not exist - so nag away
		if ( ERROR_SUCCESS != ::RegQueryValueEx( hkApp, s_szDisableReminder, NULL, NULL, NULL, NULL ) )
			bRemindReg = true;
	}


	if ( bRemindReg )
	{
		// Ok now check in the HKCU area to see if we've already done the nag
		HKEY hk = NULL;
		::RegOpenKeyEx( HKEY_CURRENT_USER, s_szSurfacesHive, 0, KEY_ALL_ACCESS, &hk );
		if ( hk )
		{
			if ( ERROR_SUCCESS != ::RegQueryValueEx( hk, s_szRegReminderKey, NULL, NULL, NULL, NULL ) )
			{
				CRegReminderDlg dlg;
				dlg.DoModal();

				// only once
				DWORD dw1 = 1;
				::RegSetValueEx( hk, s_szRegReminderKey, 0, REG_DWORD, (BYTE*)&dw1, sizeof(dw1) );
			}
		}
	}

	m_bInitDone = true;

	CString str;

	LoadAllFields();  // apparently does nothing !?

	if ( !m_pSurface->GetIOBoxInterface()->HasMidiIO() )
	{
		if (!m_pSurface->GetIOBoxInterface()->HasConsolePorts())
			AfxMessageBox( IDS_WARN_NO_PORTS );
	}

	return bRet;
}

void CTacomaSurfacePropPage::OnDestroy()
{
	m_HelpHelper.Terminate();
}

HBRUSH CTacomaSurfacePropPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
      
   if ((nCtlColor == CTLCOLOR_STATIC) && m_pBrush)
   {
	  hbr = (HBRUSH)m_pBrush->GetSafeHandle();
	  pDC->SetTextColor(RGB(224, 224, 224));
     pDC->SetBkMode( TRANSPARENT );
	  hbr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	 
   }
   return hbr;
}
//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == m_uiTimerID)
	{
		if ( m_pSurface->IsFirstLoaded() )
		{
			LoadAllFields();
		}
	}

	RefreshAllControls();

	if ( m_pSurface->GetIOBoxInterface()->m_bLinkclick == true )
		enableDMStrips();

	CDialog::OnTimer(nIDEvent);
}

//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnPaint()
{
	CPaintDC dcScreen(this); // device context for painting
   CWMemDC dc( &dcScreen );

	// face plate
	if ( !m_bmpFaceplate.m_hObject )
		m_bmpFaceplate.LoadBitmap( IDB_IOPANEL );

	// select the faceplate
	CDC dcFace;
	VERIFY( dcFace.CreateCompatibleDC( &dc ) );
	dcFace.SelectObject( &m_bmpFaceplate );
	BITMAP bm;
	m_bmpFaceplate.GetBitmap( &bm );

	// blit the faceplate
	dc.BitBlt( 0, 0, bm.bmWidth, bm.bmHeight, &dcFace, 0, 0, SRCCOPY );

	// Now the tab background
	if ( m_idbTabBkg )
	{
		CBitmap bmpTabBkg;
		bmpTabBkg.LoadBitmap( m_idbTabBkg );
		bmpTabBkg.GetBitmap( &bm );
		dcFace.SelectObject( &bmpTabBkg );

		dc.BitBlt( 10, 71, bm.bmWidth, bm.bmHeight, &dcFace, 0, 0, SRCCOPY );
	}

	PaintAllControls( &dc );
}


//--------------------------------------------------------------------
void CTacomaSurfacePropPage::RecalcLayout( void )
{
	switch( m_eDisplayPage )
	{
	case DP_Preamp:
		layoutPreamp();
		break;
	case OC_Preamp:
		layoutOCPreamp();
		break;
	case DP_DirectMixer:
		layoutDirectMixer();
		break;
	case OC_DirectMixerA:
		layoutDirectMixerA();
		break;
	case OC_DirectMixerB:
		layoutDirectMixerB();
		break;
	case OC_DirectMixerC:
		layoutDirectMixerC();
		break;
	case OC_DirectMixerD:
		layoutDirectMixerD();
		break;
	case DP_Preferences:
		layoutPreferences();
		break;
	case DP_ACT:
		layoutACT();
		break;
	}

	layoutGlobal();


}


static const int cxChanGap = 5;


//-----------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutPreamp()
{
	CPoint ptPreOrg( 58, 134 );
   const int cxComp[8] = { 63, 185, 305, 426, 548, 667, 788, 909 };
   const int cyCtrl[5] = { 245, 302, 328, 354, 380 };
   const int cxCtrl[8] = { 23, 210, 265, 452, 507, 694, 749, 936 };
   const int cxGain[8] = { 71, 155, 313, 397, 555, 639, 797, 881 };
   const int cxLink[4] = { 124, 366, 608, 850 };

   const int cxStrip[7] = { 122, 119, 120, 118, 121, 120, 122 };
	
   enablePage( DP_Preamp );

	CPoint pt;

	VERIFY( m_aIOControls.size() == TacomaIOBox::NumMicInputChannels );

	// IO control
	for ( size_t i = 0; i < m_aIOControls.size(); i++ )
	{
		pt = ptPreOrg;

		IOChanControls& iocc = m_aIOControls[i];

      pt.x = cxComp[ i ];

		//Set New Numvalues for Certain Controls
		 iocc.pThreshold->SetNumValueSteps(60);
		 iocc.pRatio->SetNumValueSteps(14);
		 iocc.pCompGain->SetNumValueSteps(60);

		placeAndOffset( iocc.pAttack, pt,    0, 0, 0, 19 );
		placeAndOffset( iocc.pRelease, pt,   0, 0, 0, 20 );
		placeAndOffset( iocc.pThreshold, pt, 0, 0, 0, 19 );
		placeAndOffset( iocc.pRatio, pt,     0, 0, 0, 20 );
		placeAndOffset( iocc.pCompGain, pt,  0, 0, 0, 0 );

      pt.x = cxCtrl[ i ];
      pt.y = cyCtrl[0];
      iocc.pCompEnable->SetOrigin( pt );
      pt.y = cyCtrl[1];
      iocc.pPhantom->SetOrigin( pt );
      pt.y = cyCtrl[2];
		iocc.pPhase->SetOrigin( pt );
      pt.y = cyCtrl[3];
      iocc.pPad->SetOrigin( pt );
	  //hide HiZ
	  iocc.pHiz->SetIsVisible( FALSE );
	  pt.y = cyCtrl[4];
      iocc.pLoCut->SetOrigin( pt );
	  
	  //Gain label
      pt.y = 405;
      pt.x = cxGain[ i ];
      iocc.pGainLabel->SetOrigin( pt );

	  //Gain control
      pt.y = 275;
      pt.x += 3;
      iocc.pGain->SetOrigin( pt );


		if ( iocc.pLink )
		{
         ASSERT( ( i / 2 ) < 4 );
			pt.x = cxLink[ i / 2 ];
		 pt.x -=8;
         pt.y = 241;
         iocc.pLink->SetOrigin( pt );
		}

      if ( i < 7 )
		   ptPreOrg.x += cxStrip[ i ];
	}

   const int cyRad[2] = { 86, 101 };
   const int cxRad[3] = { 644, 740, 899 };

   for ( DWORD dwX = 0; dwX < 3; dwX ++ )
   {
      for ( DWORD dwY = 0; dwY < 2; dwY ++ )
      {
         pt.x = cxRad[ dwX ];
         pt.y = cyRad[ dwY ];
         m_vRad[ ( dwX * 2 ) + dwY ]->SetOrigin( pt );
      }
   }

	// Preset
   CPoint ptPreset( 521, 92 );
   m_pctlIOPresetSave->SetOrigin( ptPreset );
   ptPreset.x = 556;
   m_pctlIOPresetDelete->SetOrigin( ptPreset );

	enableMicPreStrips();
}

//-----------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutOCPreamp()
{
	
	CPoint ptPreOrg( 58, 128 );
   const int cxComp[8] = { 63, 185, 305, 426, 548, 667, 788, 909 };
   const int cyCtrl[5] = { 255, 302, 328, 354, 380 };
   const int cxCtrl[8] = { 21, 208, 265, 452, 507, 692, 747, 934 };
   const int cxGain[8] = { 71, 155, 313, 397, 555, 639, 797, 881 };
   const int cxLink[4] = { 119, 365, 605, 845 };

   const int cxStrip[7] = { 122, 119, 120, 118, 121, 120, 122 };
	
   enablePage( OC_Preamp );

	CPoint pt;

	VERIFY( m_aIOControls.size() == TacomaIOBox::NumMicInputChannels );

	// IO control
	for ( size_t i = 0; i < m_aIOControls.size(); i++ )
	{
		pt = ptPreOrg;

		IOChanControls& iocc = m_aIOControls[i];

		//Set New Numvalues for Certain Controls
		 iocc.pGain->SetNumValueSteps(100);
		 //iocc.pGain->SetNumSmallSteps(0.5);
		// iocc.pGain->SetSnapValue(.5);
		 iocc.pThreshold->SetNumValueSteps(50);
		 iocc.pRatio->SetNumValueSteps(8);
		 iocc.pCompGain->SetNumValueSteps(74);

      pt.x = cxComp[ i ];
		placeAndOffset( iocc.pGate, pt,    0, 0, 0, 19 );
		placeAndOffset( iocc.pAttack, pt,    0, 0, 0, 19 );
		placeAndOffset( iocc.pRelease, pt,   0, 0, 0, 20 );
		placeAndOffset( iocc.pThreshold, pt, 0, 0, 0, 19 );
		placeAndOffset( iocc.pRatio, pt,     0, 0, 0, 20 );
		placeAndOffset( iocc.pCompGain, pt,  0, 0, 0, 0 );

	  pt.x = cxCtrl[ i ];
      pt.y = cyCtrl[0];
      iocc.pCompEnable->SetOrigin( pt );
      pt.y = cyCtrl[1];
      iocc.pPhantom->SetOrigin( pt );
      pt.y = cyCtrl[2];
		iocc.pPhase->SetOrigin( pt );
      //Hide Pad for OC
      iocc.pPad->SetIsVisible( FALSE );
	 
	  //Draw Hiz Button for Channel 1 & 2 of OC, Hide for other channels
	  if (i < 2)
	  {
		  pt.y = cyCtrl[3];
		  iocc.pHiz->SetOrigin( pt );
	  }
	  else
		  iocc.pHiz->SetIsVisible(FALSE);

	  //locut
      pt.y = cyCtrl[4];
      iocc.pLoCut->SetOrigin( pt );
	  
	  //Gain label
      pt.y = 405;
      pt.x = cxGain[ i ];
      iocc.pGainLabel->SetOrigin( pt );

	  //Gain control
      pt.y = 275;
      pt.x += 3;
      iocc.pGain->SetOrigin( pt );


		if ( iocc.pLinkOC )
		{
         ASSERT( ( i / 2 ) < 4 );
			pt.x = cxLink[ i / 2 ];
		 pt.x -=0;
         pt.y = 164;
         iocc.pLinkOC->SetOrigin( pt );
		}

      if ( i < 7 )
		   ptPreOrg.x += cxStrip[ i ];
	}
			

	// Preset
   CPoint ptPreset( 616, 92 );
   m_pctlIOPresetSave->SetOrigin( ptPreset );
   ptPreset.x = 658;
   m_pctlIOPresetDelete->SetOrigin( ptPreset );

   //SYNC Indicator
   CPoint ptSync( 794, 92 );
    m_pctlInSync->SetOrigin( ptSync );
	ptSync.x += 65;
	m_pctlDigSync->SetOrigin( ptSync );
   
	enableMicPreStrips();
	
}

//-------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutDirectMixer()
{
	enablePage( DP_DirectMixer );

	const int cxStrip = 42;
	CPoint ptDMOrg( 17, 143 );
	CPoint pt( ptDMOrg );

	for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		pt = ptDMOrg;

		if (i  < 23 )
		{
			// mono, mute, solo, gain display
			IODirectMixControls& iodmc = m_aDMControls[ i ];

			if ( iodmc.pDMMono )
				iodmc.pDMMono->SetOrigin( pt );
			pt.y += 33;
			placeAndOffset( iodmc.pDMMute, pt, 0, 0, 0, 32);
			placeAndOffset( iodmc.pDMSolo, pt, 0, 0, -2, 36);
			// pan
			iodmc.pDMPan->SetOrigin( pt );


			if ( iodmc.pLink )
			{
				CPoint ptLnk( pt );
				ptLnk.x += 26;
				ptLnk.y += 18;
				iodmc.pLink->SetOrigin( ptLnk );
			}

			// fader
			pt.x = ptDMOrg.x + 4;
			pt.y = ptDMOrg.y + 133;
			placeAndOffset( iodmc.pDMVol, pt, 0, 1, 0, 2 );
			// fader label
			pt.x -= 7;
			pt.y += 5;
			iodmc.pDMVolLabel->SetOrigin( pt );

			ptDMOrg.x += cxStrip;

			//hide the extra pLink and Mono controls needed for OCTA-CAPTURE
			if ( i > 21 )
			{
				iodmc.pDMMono->SetIsVisible( FALSE );
				iodmc.pLink->SetIsVisible( FALSE );
			}
		}




	}



	const int nxRad[3] = { 753, 827, 888 };
	for ( DWORD dwCntr = 0; dwCntr < m_vDMRad.size(); dwCntr ++ )
	{
		const CPoint pt( nxRad[ dwCntr ], 91 );
		m_vDMRad[ dwCntr ]->SetOrigin( pt );
	}

	// Preset
	CPoint ptPreset( 538, 88 );
	m_pctlIOPresetSave->SetOrigin( ptPreset );
	ptPreset.x = 575;
	m_pctlIOPresetDelete->SetOrigin( ptPreset );

	enableDMStrips();
}

//-------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutDirectMixerA()
{
	enablePage( OC_DirectMixerA );

	const int cxStrip = 42;
	CPoint ptDMOrg( 18, 156 );
	CPoint pt( ptDMOrg );

	for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		pt = ptDMOrg;

		// mono, mute, solo, gain display
		IODirectMixControls& iodmc = m_aDMControls[ i ];


		//hide controls from other tabs

		iodmc.pOCMute->SetIsVisible( TRUE );
		iodmc.pOCSolo->SetIsVisible( TRUE );
		iodmc.pOCPan->SetIsVisible( TRUE );
		iodmc.pOCVol->SetIsVisible( TRUE );
		iodmc.pOCVolLabel->SetIsVisible ( TRUE );

		iodmc.pOCMuteB->SetIsVisible( FALSE );
		iodmc.pOCSoloB->SetIsVisible( FALSE );
		iodmc.pOCPanB->SetIsVisible( FALSE );
		iodmc.pOCVolB->SetIsVisible( FALSE );
		iodmc.pOCVolLabelB->SetIsVisible ( FALSE );


		iodmc.pOCMuteC->SetIsVisible( FALSE );
		iodmc.pOCSoloC->SetIsVisible( FALSE );
		iodmc.pOCPanC->SetIsVisible( FALSE );
		iodmc.pOCVolC->SetIsVisible( FALSE );
		iodmc.pOCVolLabelC->SetIsVisible ( FALSE );

		iodmc.pOCMuteD->SetIsVisible( FALSE );
		iodmc.pOCSoloD->SetIsVisible( FALSE );
		iodmc.pOCPanD->SetIsVisible( FALSE );
		iodmc.pOCVolD->SetIsVisible( FALSE );
		iodmc.pOCVolLabelD->SetIsVisible ( FALSE );



		if ( iodmc.pLinkOC )
		{


			CPoint ptLnk( pt );
			if (i < 20)
			{
				ptLnk.x += 26;
			}
			else
			{
				ptLnk.x += 15;
				ptLnk.y += 149;
			}

			iodmc.pLinkOC->SetOrigin( ptLnk );

			if ( iodmc.pLinkOCB )
				iodmc.pLinkOCB->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCC )
				iodmc.pLinkOCC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCD )
				iodmc.pLinkOCD->SetIsVisible( FALSE );	
		}

		if (i < 20)
		{

			pt.y += 33;
			placeAndOffset( iodmc.pOCMute, pt, 0, 0, 0, 32);
			placeAndOffset( iodmc.pOCSolo, pt, 0, 0, -1, 35);



			//Send
			if (( i < 10 ) && ( m_eDisplayPage == OC_DirectMixerA )) 
			{
				placeAndOffset( iodmc.pDMSend, pt, 0, 0, -1, 26);
				// pan 
				iodmc.pOCPan->SetOrigin( pt );
			}
			else
			{
				pt.y += 26;
				iodmc.pOCPan->SetOrigin( pt );
				iodmc.pDMSend->SetIsVisible ( FALSE );
			}

		}
		else
		{
			iodmc.pOCMute->SetIsVisible( FALSE );
			iodmc.pOCSolo->SetIsVisible( FALSE );
			iodmc.pOCPan->SetIsVisible( FALSE );


		}

		// fader
		pt.x = ptDMOrg.x + 4;
		pt.y = ptDMOrg.y + 158;


		placeAndOffset( iodmc.pOCVol, pt, 0, 1, 0, 2 );


		// fader label
		pt.x -= 5;
		pt.y += 7;

		//if ( i < 20 )
		iodmc.pOCVolLabel->SetOrigin( pt );

		//Master Section Strips
		if ( i < 20 )
			ptDMOrg.x += cxStrip;
		else 
		{
			if (i == 20)
				ptDMOrg.x += cxStrip - 20;
			else if (i == 21) 
				ptDMOrg.x += cxStrip - 10;
			else if (i == 22) 
				ptDMOrg.x += cxStrip - 20;
			//else if (i == 23)
			///	ptDMOrg.x += cxStrip - 10;
		}



	}

	// Preset
	CPoint ptPreset( 608, 92 );
	m_pctlIOPresetSave->SetOrigin( ptPreset );
	ptPreset.x = 645;
	m_pctlIOPresetDelete->SetOrigin( ptPreset );

	//Direct Mix Sub Tabs
	CPoint ptTabs( 108, 92 );
	m_pbtnOCTabA->SetOrigin( ptTabs );
	ptTabs.x += 38;
	placeAndOffset( m_pbtnOCTabB,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabC,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabD,ptTabs, 0, 0, 50, 0 );

	//Patch Bay Button
	placeAndOffset( m_pbtnOCBay,ptTabs, 0, 0, 60, 0 );;


	//Out Port Indicators
	CPoint ptPort( 757, 91 );
	m_pctlOUT12Ind->SetOrigin( ptPort );
	ptPort.x +=42;
	placeAndOffset( m_pctlOUT34Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT56Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT78Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT910Ind,ptPort, 0, 0, 0, 0 );


	//Time Control
	CPoint ptTime( 860, 197 );
	m_pctlRevTime->SetOrigin( ptTime);

	//Return Control
	CPoint ptRet( 867, 282 );
	m_pctlReturn->SetOrigin( ptRet );

	//Master Link Control
	CPoint ptMLnk( 897, 248 );
	m_pBtnMasterLink->SetOrigin (ptMLnk);



	enableDMStrips();
}

//-------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutDirectMixerB()
{
	enablePage( OC_DirectMixerB );

	const int cxStrip = 42;
	CPoint ptDMOrg( 18, 156 );
	CPoint pt( ptDMOrg );

	for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		pt = ptDMOrg;

		// mono, mute, solo, gain display
		IODirectMixControls& iodmc = m_aDMControls[ i ];


		//hide controls from other tabs

		iodmc.pOCMute->SetIsVisible( FALSE );
		iodmc.pOCSolo->SetIsVisible( FALSE );
		iodmc.pOCPan->SetIsVisible( FALSE );
		iodmc.pOCVol->SetIsVisible( FALSE );
		iodmc.pOCVolLabel->SetIsVisible ( FALSE );

		iodmc.pOCMuteB->SetIsVisible( TRUE );
		iodmc.pOCSoloB->SetIsVisible( TRUE );
		iodmc.pOCPanB->SetIsVisible( TRUE );
		iodmc.pOCVolB->SetIsVisible( TRUE );
		iodmc.pOCVolLabelB->SetIsVisible ( TRUE );


		iodmc.pOCMuteC->SetIsVisible( FALSE );
		iodmc.pOCSoloC->SetIsVisible( FALSE );
		iodmc.pOCPanC->SetIsVisible( FALSE );
		iodmc.pOCVolC->SetIsVisible( FALSE );
		iodmc.pOCVolLabelC->SetIsVisible ( FALSE );

		iodmc.pOCMuteD->SetIsVisible( FALSE );
		iodmc.pOCSoloD->SetIsVisible( FALSE );
		iodmc.pOCPanD->SetIsVisible( FALSE );
		iodmc.pOCVolD->SetIsVisible( FALSE );
		iodmc.pOCVolLabelD->SetIsVisible ( FALSE );


		if ( iodmc.pLinkOC )
			iodmc.pLinkOC->SetIsVisible( FALSE );

		if ( iodmc.pLinkOCB )
		{


			CPoint ptLnk( pt );
			if (i < 20)
			{
				ptLnk.x += 26;
			}
			else
			{
				ptLnk.x += 15;
				ptLnk.y += 149;
			}

			iodmc.pLinkOCB->SetOrigin( ptLnk );
			if ( iodmc.pLinkOC )
				iodmc.pLinkOC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCC )
				iodmc.pLinkOCC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCD )
				iodmc.pLinkOCD->SetIsVisible( FALSE );
		}

		if (i < 20)
		{

			pt.y += 33;
			placeAndOffset( iodmc.pOCMuteB, pt, 0, 0, 0, 32);
			placeAndOffset( iodmc.pOCSoloB, pt, 0, 0, -1, 35);


			//Send
			if (( i < 10 ) && ( m_eDisplayPage == OC_DirectMixerA )) 
			{
				placeAndOffset( iodmc.pDMSend, pt, 0, 0, -1, 26);
				// pan 
				iodmc.pOCPanB->SetOrigin( pt );
			}
			else
			{
				pt.y += 26;
				iodmc.pOCPanB->SetOrigin( pt );
				iodmc.pDMSend->SetIsVisible ( FALSE );
			}

		}
		else
		{
			iodmc.pOCMuteB->SetIsVisible( FALSE );
			iodmc.pOCSoloB->SetIsVisible( FALSE );
			iodmc.pOCPanB->SetIsVisible( FALSE );

		}

		// fader
		pt.x = ptDMOrg.x + 4;
		pt.y = ptDMOrg.y + 158;

		placeAndOffset( iodmc.pOCVolB, pt, 0, 1, 0, 2 );

		// fader label
		pt.x -= 5;
		pt.y += 7;


		iodmc.pOCVolLabelB->SetOrigin( pt );


		//Master Section Strips
		if ( i < 20 )
			ptDMOrg.x += cxStrip;
		else 
		{
			if (i == 20)
				ptDMOrg.x += cxStrip - 20;
			else if (i == 21) 
				ptDMOrg.x += cxStrip - 10;
			else if (i == 22) 
				ptDMOrg.x += cxStrip - 20;
			//else if (i == 23)
			//	ptDMOrg.x += cxStrip - 10;
		}
	}

	// Preset
	CPoint ptPreset( 608, 92 );
	m_pctlIOPresetSave->SetOrigin( ptPreset );
	ptPreset.x = 645;
	m_pctlIOPresetDelete->SetOrigin( ptPreset );

	//Direct Mix Sub Tabs
	CPoint ptTabs( 108, 92 );
	m_pbtnOCTabA->SetOrigin( ptTabs );
	ptTabs.x += 38;
	placeAndOffset( m_pbtnOCTabB,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabC,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabD,ptTabs, 0, 0, 50, 0 );

	//Patch Bay Button
	placeAndOffset( m_pbtnOCBay,ptTabs, 0, 0, 60, 0 );;

	//Out Port Indicators
	CPoint ptPort( 757, 91 );
	m_pctlOUT12Ind->SetOrigin( ptPort );
	ptPort.x +=42;
	placeAndOffset( m_pctlOUT34Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT56Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT78Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT910Ind,ptPort, 0, 0, 0, 0 );

	//Return Control
	m_pctlReturn->SetIsVisible(FALSE);
	m_pctlRevTime->SetIsVisible(FALSE);


	//Master Link Control
	CPoint ptMLnk( 897, 248 );
	m_pBtnMasterLink->SetOrigin (ptMLnk);


	enableDMStrips();
}

//-------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutDirectMixerC()
{
	enablePage( OC_DirectMixerC );

	const int cxStrip = 42;
	CPoint ptDMOrg( 18, 156 );
	CPoint pt( ptDMOrg );

	for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		pt = ptDMOrg;

		// mono, mute, solo, gain display
		IODirectMixControls& iodmc = m_aDMControls[ i ];

		//hide controls from other tabs

		iodmc.pOCMute->SetIsVisible( FALSE );
		iodmc.pOCSolo->SetIsVisible( FALSE );
		iodmc.pOCPan->SetIsVisible( FALSE );
		iodmc.pOCVol->SetIsVisible( FALSE );
		iodmc.pOCVolLabel->SetIsVisible ( FALSE );

		iodmc.pOCMuteB->SetIsVisible( FALSE );
		iodmc.pOCSoloB->SetIsVisible( FALSE );
		iodmc.pOCPanB->SetIsVisible( FALSE );
		iodmc.pOCVolB->SetIsVisible( FALSE );
		iodmc.pOCVolLabelB->SetIsVisible ( FALSE );

		iodmc.pOCMuteC->SetIsVisible( TRUE );
		iodmc.pOCSoloC->SetIsVisible( TRUE );
		iodmc.pOCPanC->SetIsVisible( TRUE );
		iodmc.pOCVolC->SetIsVisible( TRUE );
		iodmc.pOCVolLabelC->SetIsVisible ( TRUE );

		iodmc.pOCMuteD->SetIsVisible( FALSE );
		iodmc.pOCSoloD->SetIsVisible( FALSE );
		iodmc.pOCPanD->SetIsVisible( FALSE );
		iodmc.pOCVolD->SetIsVisible( FALSE );
		iodmc.pOCVolLabelD->SetIsVisible ( FALSE );

		if ( iodmc.pLinkOCC )
		{
			CPoint ptLnk( pt );
			if (i < 20)
			{
				ptLnk.x += 26;
			}
			else
			{
				ptLnk.x += 15;
				ptLnk.y += 149;
			}

			iodmc.pLinkOCC->SetOrigin( ptLnk );

			if ( iodmc.pLinkOCB )
				iodmc.pLinkOCB->SetIsVisible( FALSE );
			if ( iodmc.pLinkOC )
				iodmc.pLinkOC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCD )
				iodmc.pLinkOCD->SetIsVisible( FALSE );

		}

		if (i < 20)
		{

			pt.y += 33;
			placeAndOffset( iodmc.pOCMuteC, pt, 0, 0, 0, 32);
			placeAndOffset( iodmc.pOCSoloC, pt, 0, 0, -1, 35);

			//Send
			if (( i < 10 ) && ( m_eDisplayPage == OC_DirectMixerA )) 
			{
				placeAndOffset( iodmc.pDMSend, pt, 0, 0, -1, 26);
				// pan 
				iodmc.pOCPanC->SetOrigin( pt );
			}
			else
			{
				pt.y += 26;
				iodmc.pOCPanC->SetOrigin( pt );
				iodmc.pDMSend->SetIsVisible ( FALSE );
			}

		}
		else
		{
			iodmc.pOCMuteC->SetIsVisible( FALSE );
			iodmc.pOCSoloC->SetIsVisible( FALSE );
			iodmc.pOCPanC->SetIsVisible( FALSE );

		}

		// fader
		pt.x = ptDMOrg.x + 4;
		pt.y = ptDMOrg.y + 158;

		placeAndOffset( iodmc.pOCVolC, pt, 0, 1, 0, 2 );

		// fader label
		pt.x -= 5;
		pt.y += 7;

		//if ( i < 20 )
		iodmc.pOCVolLabelC->SetOrigin( pt );


		//Master Section Strips
		if ( i < 20 )
			ptDMOrg.x += cxStrip;
		else 
		{
			if (i == 20)
				ptDMOrg.x += cxStrip - 20;
			else if (i == 21) 
				ptDMOrg.x += cxStrip - 10;
			else if (i == 22) 
				ptDMOrg.x += cxStrip - 20;
			//else if (i == 23)
			//	ptDMOrg.x += cxStrip - 10;
		}
	}

	// Preset
	CPoint ptPreset( 608, 92 );
	m_pctlIOPresetSave->SetOrigin( ptPreset );
	ptPreset.x = 645;
	m_pctlIOPresetDelete->SetOrigin( ptPreset );

	//Direct Mix Sub Tabs
	CPoint ptTabs( 108, 92 );
	m_pbtnOCTabA->SetOrigin( ptTabs );
	ptTabs.x += 38;
	placeAndOffset( m_pbtnOCTabB,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabC,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabD,ptTabs, 0, 0, 50, 0 );

	//Patch Bay Button
	placeAndOffset( m_pbtnOCBay,ptTabs, 0, 0, 60, 0 );


	//Out Port Indicators
	CPoint ptPort( 757, 91 );
	m_pctlOUT12Ind->SetOrigin( ptPort );
	ptPort.x +=42;
	placeAndOffset( m_pctlOUT34Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT56Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT78Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT910Ind,ptPort, 0, 0, 0, 0 );

	//Return Control
	m_pctlReturn->SetIsVisible(FALSE);
	m_pctlRevTime->SetIsVisible(FALSE);

	//Master Link Control
	CPoint ptMLnk( 897, 248 );
	m_pBtnMasterLink->SetOrigin (ptMLnk);

	enableDMStrips();
}

//-------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutDirectMixerD()
{
	enablePage( OC_DirectMixerD );

	const int cxStrip = 42;
	CPoint ptDMOrg( 18, 156 );
	CPoint pt( ptDMOrg );

	for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		pt = ptDMOrg;

		// mono, mute, solo, gain display
		IODirectMixControls& iodmc = m_aDMControls[ i ];


		//hide controls from other tabs
		iodmc.pOCMute->SetIsVisible( FALSE );
		iodmc.pOCSolo->SetIsVisible( FALSE );
		iodmc.pOCPan->SetIsVisible( FALSE );
		iodmc.pOCVol->SetIsVisible( FALSE );
		iodmc.pOCVolLabel->SetIsVisible ( FALSE );


		iodmc.pOCMuteB->SetIsVisible( FALSE );
		iodmc.pOCSoloB->SetIsVisible( FALSE );
		iodmc.pOCPanB->SetIsVisible( FALSE );
		iodmc.pOCVolB->SetIsVisible( FALSE );
		iodmc.pOCVolLabelB->SetIsVisible ( FALSE );


		iodmc.pOCMuteC->SetIsVisible( FALSE );
		iodmc.pOCSoloC->SetIsVisible( FALSE );
		iodmc.pOCPanC->SetIsVisible( FALSE );
		iodmc.pOCVolC->SetIsVisible( FALSE );
		iodmc.pOCVolLabelC->SetIsVisible ( FALSE );


		iodmc.pOCMuteD->SetIsVisible( TRUE );
		iodmc.pOCSoloD->SetIsVisible( TRUE );
		iodmc.pOCPanD->SetIsVisible( TRUE );
		iodmc.pOCVolD->SetIsVisible( TRUE );
		iodmc.pOCVolLabelD->SetIsVisible ( TRUE );

		if ( iodmc.pLinkOCD )
		{
			CPoint ptLnk( pt );
			if (i < 20)
			{
				ptLnk.x += 26;
			}
			else
			{
				ptLnk.x += 15;
				ptLnk.y += 149;
			}

			iodmc.pLinkOCD->SetOrigin( ptLnk );

			if ( iodmc.pLinkOCB )
				iodmc.pLinkOCB->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCC )
				iodmc.pLinkOCC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOC )
				iodmc.pLinkOC->SetIsVisible( FALSE );
		}

		if (i < 20)
		{

			pt.y += 33;
			placeAndOffset( iodmc.pOCMuteD, pt, 0, 0, 0, 32);
			placeAndOffset( iodmc.pOCSoloD, pt, 0, 0, -1, 35);

			//Send
			if (( i < 10 ) && ( m_eDisplayPage == OC_DirectMixerA )) 
			{
				placeAndOffset( iodmc.pDMSend, pt, 0, 0, -1, 26);
				// pan 
				iodmc.pOCPanD->SetOrigin( pt );
			}
			else
			{
				pt.y += 26;
				iodmc.pOCPanD->SetOrigin( pt );
				iodmc.pDMSend->SetIsVisible ( FALSE );
			}

		}
		else
		{
			iodmc.pOCMuteD->SetIsVisible( FALSE );
			iodmc.pOCSoloD->SetIsVisible( FALSE );
			iodmc.pOCPanD->SetIsVisible( FALSE );

		}

		// fader
		pt.x = ptDMOrg.x + 4;
		pt.y = ptDMOrg.y + 158;

		placeAndOffset( iodmc.pOCVolD, pt, 0, 1, 0, 2 );

		// fader label
		pt.x -= 5;
		pt.y += 7;

		//if ( i < 20 )
		iodmc.pOCVolLabelD->SetOrigin( pt );
		//else
		//{

		//iodmc.pOCMasVolLabel->SetOrigin( pt );
		//	}

		//Master Section Strips
		if ( i < 20 )
			ptDMOrg.x += cxStrip;
		else 
		{
			if (i == 20)
				ptDMOrg.x += cxStrip - 20;
			else if (i == 21) 
				ptDMOrg.x += cxStrip - 10;
			else if (i == 22) 
				ptDMOrg.x += cxStrip - 20;
			//else if (i == 23)
			//	ptDMOrg.x += cxStrip - 10;
		}



	}

	// Preset
	CPoint ptPreset( 608, 92 );
	m_pctlIOPresetSave->SetOrigin( ptPreset );
	ptPreset.x = 645;
	m_pctlIOPresetDelete->SetOrigin( ptPreset );

	//Direct Mix Sub Tabs
	CPoint ptTabs( 108, 92 );
	m_pbtnOCTabA->SetOrigin( ptTabs );
	ptTabs.x += 38;
	placeAndOffset( m_pbtnOCTabB,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabC,ptTabs, 0, 0, 38, 0 );
	placeAndOffset( m_pbtnOCTabD,ptTabs, 0, 0, 50, 0 );

	//Patch Bay Button
	placeAndOffset( m_pbtnOCBay,ptTabs, 0, 0, 60, 0 );;

	//Out Port Indicators
	CPoint ptPort( 757, 91 );
	m_pctlOUT12Ind->SetOrigin( ptPort );
	ptPort.x +=42;
	placeAndOffset( m_pctlOUT34Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT56Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT78Ind,ptPort, 0, 0, 42, 0 );
	placeAndOffset( m_pctlOUT910Ind,ptPort, 0, 0, 0, 0 );

	//Return Control
	m_pctlReturn->SetIsVisible(FALSE);
	m_pctlRevTime->SetIsVisible(FALSE);

	//Master Link Control
	CPoint ptMLnk( 897, 248 );
	m_pBtnMasterLink->SetOrigin (ptMLnk);

	enableDMStrips();
}

void CTacomaSurfacePropPage::layoutACT()
{
   enablePage( DP_ACT );

   CPoint pt( 428, 300 );
   const CPoint ptChanOrg( 47, 160 );
   pt = ptChanOrg;
   for ( DWORD dwCntr = 0; dwCntr < m_vRotaryACTLbl.size(); dwCntr ++ )
   {
      m_vRotaryACTLbl[ dwCntr ]->SetOrigin( pt );

      if ( dwCntr == 3 )
      {
         pt.x = 88;
         pt.y = 233;
      }
      else if ( dwCntr == 7 )
      {
         pt.x = 47;
         pt.y = 304;
      }
      else
         pt.x += 76;
   }

	// ACT Switch Button Labels
   const CPoint ptChanBtnOrg( 440, 160 );
   pt = ptChanBtnOrg;
	for ( size_t i = 0; i < m_vSwitchACTLbl.size(); i++ )
	{
      m_vSwitchACTLbl[ i ]->SetOrigin( pt );

      if ( i == 3 )
      {
         pt.x = 471;
         pt.y = 298;
      }
      else if ( i == 7 )
      {
         pt.x = 427;
         pt.y = 268;
      }
      else if ( i == 11 )
      {
         pt.x = 445;
         pt.y = 341;
      }
      else if ( i > 11 )
         pt.x += 82;
      else
         pt.x += 76;
	}

   const CPoint ptBtnOrg( 454, 287 );
   pt = ptBtnOrg;
   for ( DWORD dwCntr = 0; dwCntr < m_vACTPageButtons.size(); dwCntr ++ )
   {
      m_vACTPageButtons[ dwCntr ]->SetOrigin( pt );
      pt.x += 83;
   }

   m_pTBarACTLbl->SetOrigin( CPoint( 846, 301 ) );
}

//-----------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutPreferences()
{
   enablePage( DP_Preferences );

   // Utility btns
	const CPoint ptCmdOrg( 375, 172 );
   const CPoint ptBtnOrg( 389, 141 );
   CPoint ptBtn( ptBtnOrg );
	CPoint pt( ptCmdOrg );

	for ( size_t i = 0; i < _countof(s_aid); i++ )
	{
		CTacomaSurface::ControlId cid = s_aid[i];
		MapControlsIterator it = m_mapCommandButtons.find( cid );
		if ( m_mapCommandButtons.end() == it )
			break;
		CNWControl* pC = it->second;
      pC->SetOrigin( pt );

      // buttons
      it = m_mapCommandButtonsBmp.find( cid );
		if ( m_mapCommandButtonsBmp.end() != it )
      {
		   pC = it->second;
         pC->SetOrigin( ptBtn );
      }

      // offset both
      if ( i == 3 || i == 11 )
      {
			pt.x = ptCmdOrg.x;
         ptBtn.x = ptBtnOrg.x;

         pt.y += 61;
         ptBtn.y += 61;
      }
      else if ( i == 7 )
      {
			pt.x = ptCmdOrg.x;
         ptBtn.x = ptBtnOrg.x;

         pt.y += 74;
         ptBtn.y += 74;
      }
      else
      {
         pt.x += 88;
         ptBtn.x += 88;
      }
	}

	// Encoder
   //   controls
   pt.x = 56;
   pt.y = 215;
	m_pctlTrackEncFunc->SetOrigin( pt );
	pt.x = 159;
	m_pctlBusEncFunc->SetOrigin( pt );
	pt.x = 260;
	m_pctlIOEncFunc->SetOrigin( pt );
   //   buttons
   const CPoint ptEncOrg( 42, 267 );
   pt = ptEncOrg;
   for ( DWORD dwCntr = 0; dwCntr < m_vEncoderButtons.size(); dwCntr ++ )
   {
      CNWControl* pC = m_vEncoderButtons[ dwCntr ];
      pC->SetOrigin( pt );
      if ( dwCntr == 3 ||dwCntr == 7 )
      {
         pt.y = ptEncOrg.y;
         pt.x += 100;
      }
      else
         pt.y += 30;
   }

	// Checkboxes
   pt.x = 764;
   pt.y = 145;
   m_pctlDisableFaderMotors->SetOrigin( pt );

   pt.y = 172;
   m_pctlTouchSelects->SetOrigin( pt );

	// feetswitch
	pt.x = 799;
	pt.y = 246;
   m_pfeetSw1->SetOrigin( pt );
   pt.y = 269;
   m_pfeetSw2->SetOrigin( pt );
}


//-----------------------------------------------------------------------
void CTacomaSurfacePropPage::layoutGlobal()
{
	CPoint pt;

   pt.y = 46;

   pt.x = 368;
	m_pctlTracks->SetOrigin( pt );
   pt.x = 418;
	m_pctlBuses->SetOrigin( pt );
   pt.x = 468;
	m_pctlMains->SetOrigin( pt );
   pt.x = 518;
   m_pctlIO->SetOrigin( pt );
	pt.x = 910;
	m_pctlFlip->SetOrigin( pt );

   // indicators
	CPoint ptInd( 708,31 );
   m_pctlMarkersInd->SetOrigin( ptInd );
   m_pctlACTInd->SetIsVisible( FALSE );
   //m_pctlACTInd->SetOrigin( CPoint( ?, ? ) );

	ptInd.x += 8;
	ptInd.y += 14;
   m_pctlLayerInd->SetOrigin( ptInd );

	ptInd.x += 10;
	ptInd.y += 12;
   m_pctlFxInd->SetOrigin( ptInd );

	m_pctlACTContextLbl->SetOrigin( CPoint( 586, 48 ) );
	m_pctlACTContextLbl->SetSize( CSize( 116, 17 ) );
	m_pctlACTContextLbl->SetIsVisible( TRUE );

	// Place the Tab controls
	CPoint ptTab( 20, 44 );
   m_pbtnPrefTab->SetOrigin( ptTab );
   ptTab.x = 100;
   m_pbtnACTTab->SetOrigin( ptTab );
   ptTab.x = 167;
   m_pbtnPreampTab->SetOrigin( ptTab );
   ptTab.x = 246;
   m_pbtnDMTab->SetOrigin( ptTab );

	BOOL bHasIO = m_pSurface->GetIOBoxInterface()->HasMidiIO();
	m_pbtnPreampTab->SetEnable( bHasIO );
	m_pbtnDMTab->SetEnable( bHasIO );
}


///////////////////////////////////////////////////////////////////////////
// Mic pre strips can be disabled because of high sample rate
void	CTacomaSurfacePropPage::enableMicPreStrips()
{
	float fSR = m_pSurface->GetIOBoxInterface()->GetParam(0, TIOP_SampleRate );
	

	DWORD dwNumPres = 8;
	if ( fSR >.75f )	// 192k
		dwNumPres = 4;
	float f01 = 0.f;

	for ( size_t ixStrip = 0; ixStrip < TacomaIOBox::NumMicInputChannels; ixStrip++ )
	{
		DWORD ixChan = s_dwDMChans[ixStrip];
		BOOL bEnable = TRUE;

		if ( ixChan >=4 )
		{
			if ( dwNumPres == 4 )
				bEnable = FALSE;
		}

		IOChanControls& ioc = m_aIOControls[ixStrip];
		ioc.Enable( bEnable );


		// if not totally disabled yet, we may still need to disable
		// various controls based on link, etc
		if ( bEnable )
		{
			// disable all comp controls if comp switch is disabled or
			// if previous even index channel is stereo-linked
			bool bComp = true;
			bool bCompEnable = true;
			f01 = m_pSurface->GetIOBoxInterface()->GetParam( ixChan, TIOP_CompEnable );
			if ( f01 < .5f )
				bComp = false;
			if ( ixChan > 0 && m_aIOControls[ixChan-1].pLink )
			{
				f01 = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_StereoLink );
				if ( f01 >= .5f )
				{
					bComp = false;
					bCompEnable = false;
				}
			}


			if (m_bIsOcta)
				ioc.pGate->SetEnable( bComp );

			ioc.pCompEnable->SetEnable( bCompEnable );
			ioc.pAttack->SetEnable( bComp );
			ioc.pRelease->SetEnable( bComp );
			ioc.pThreshold->SetEnable( bComp );
			ioc.pRatio->SetEnable( bComp );
			ioc.pCompGain->SetEnable( bComp );
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Certain channels' controls should be disabled based on several factors:
// Link On:  disable Odd channels
// Mono On:	 disable Odd Channels
// High Sample Rates: Reduce number of digital channels.  They should be grayed out
void	CTacomaSurfacePropPage::enableDMStrips()
{
	// other reasons to disable - Sample rate can affect avaiable digital inputs
	// From Watase-san:
	// Number of active audio channel is changed dependent on the Sampling rate.
	// In 88.2kHz and 96kHz, channel 5 to 8 of Deigital 2 shoould be disabled.
	// In 192kHz, channel 5 to 8 of the analog input and all digital channels should be disabled.
	 UINT u = 0;

	 u = m_pSurface->GetIOBoxInterface()->GetActiveInterface();


	float fSR = m_pSurface->GetIOBoxInterface()->GetParam(0, TIOP_SampleRate );

	DWORD dwNumAdat = 8;
	bool bDigi1 = true;

	if ( fSR >.75f )	// 192k
	{
		dwNumAdat = 0;
		bDigi1 = false;
	}
	else if ( fSR > .25f )	// 88.2 or 96K
		dwNumAdat = 4;
	if (( m_pSurface->GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (m_pSurface->GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
	for ( size_t ixStrip = 0; ixStrip < TacomaIOBox::NumChannels-1; ixStrip++ )
	{
		DWORD ixChan = s_dwDMChans[ixStrip];

		BOOL bEnable = TRUE;

		// disable for sample rate limitations
		if ( ixChan >= 16 && ixChan <= 23 )	// an adat channel
		{
			if ( dwNumAdat == 0 )
				bEnable = FALSE;
			else if ( dwNumAdat == 4 && ixChan >= 20 )
				bEnable = FALSE;
		}
		else if ( ixChan == 14 || ixChan == 15 )	// digital 1
		{
			if ( !bDigi1 )
				bEnable = FALSE;
		}
		
		// disable becuase of link or mono buttons
		if ( bEnable && ixChan % 2 != 0 )
		{
			// Right channel of L/R pair
			
			// is Mono of prev channel on?
			float fVal = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_StereoLink );
			if ( fVal < .5f )
			{
				// is Link of Prev Channel on?
				//fVal = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_StereoLink );
			}
			if ( fVal >= .5f )
			{

				bEnable = FALSE;
	
			}
		}

		IODirectMixControls& dmc = m_aDMControls[ixStrip];
		dmc.Enable( bEnable );


	}
}
	else
	{	
	for ( size_t ixStrip = 0; ixStrip < TacomaIOBox::NumChannels; ixStrip++ )
	{
		DWORD ixChan = s_dwOCDMChans[ixStrip];

		BOOL bEnable = TRUE;

		
		// disable becuase of link or mono buttons
		if ( bEnable && ixChan % 2 != 0 )
		{
			// Right channel of L/R pair
			
			// is Mono of prev channel on?
			float fVal = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_StereoLinkOC );
			if ( fVal < .5f )
			{
				// is Link of Prev Channel on?
				//fVal = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_DMixMono );
				//m_pSurface->GetIOBoxInterface()->m_bLinked = false;
			}
			if ( fVal >= .5f )
			{
				//float fVol = m_pSurface->GetIOBoxInterface()->GetParam( ixChan-1, TIOP_DMixVol );
				//m_pSurface->GetIOBoxInterface()->SetParam( ixChan, TIOP_DMixVol, fVol  );
			//	m_pSurface->GetIOBoxInterface()->m_bLinked = true;
				bEnable = FALSE;
				
			}
		}
	
	
		IODirectMixControls& dmc = m_aDMControls[ixStrip];

		dmc.Enable( bEnable );	


	}



	}

}



///--------------------------------------------------------------------------------
void	CTacomaSurfacePropPage::placeAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor, int nXOffset, int nYOffset )
{
	placeControlAndOffset( pC, pt, nXFactor, nYFactor );
	pt.x += nXOffset;
	pt.y += nYOffset;
}




//--------------------------------------------------------------------
BOOL CTacomaSurfacePropPage::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

//--------------------------------------------------------------------
void	CTacomaSurfacePropPage::DoCtrlActivate( CNWControl* pSource, UINT nFlags )
{
	DWORD dwIndex = 0;

	if ( pSource == m_pbtnPreampTab )
	{
		if ( m_bIsOcta )
			onOCPreampTab();
		else
			onPreampTab();
	}
	else if ( pSource == m_pbtnDMTab )
	{
		if ( m_bIsOcta )
			onOCDMTabA();
		else
			onDMTab();
	}
	else if ( pSource == m_pbtnOCTabA)
		{
			m_pSurface->GetIOBoxInterface()->m_byDirectMix = DA;//m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) pSource->GetParameter(), (float)dVal );
			onOCDMTabA();
		}
	else if ( pSource == m_pbtnOCTabB)
		{
			m_pSurface->GetIOBoxInterface()->m_byDirectMix = DB;//( 0, (TacomaIOBoxParam) pSource->GetParameter(), (float)dVal );
			onOCDMTabB();
		
		}
	else if ( pSource == m_pbtnOCTabC)
	{
			m_pSurface->GetIOBoxInterface()->m_byDirectMix = DC;//m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) pSource->GetParameter(), (float)dVal );
			onOCDMTabC();
	}
	else if ( pSource == m_pbtnOCTabD)
	{
			m_pSurface->GetIOBoxInterface()->m_byDirectMix = DD;//m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) pSource->GetParameter(), (float)dVal );
			onOCDMTabD();
	}
	else if ( pSource == m_pbtnPrefTab )
		onPrefTab();
	else if ( pSource == m_pbtnACTTab )
		onACTTab();
	else if ( pSource == m_pctlFlip )
		m_pSurface->OnFlip();
	else if ( pSource == m_pbtnOCBay )
		onPatchBay();
	else if ( pSource == m_pctlTracks )
		m_pSurface->Set8StripType( MIX_STRIP_TRACK );
	else if ( pSource == m_pctlBuses )
		m_pSurface->Set8StripType( MIX_STRIP_BUS );
	else if ( pSource == m_pctlMains )
		m_pSurface->Set8StripType( MIX_STRIP_MASTER );
	else if ( pSource == m_pctlIO )
		m_pSurface->SetIOMode( !m_pSurface->GetIOMode() );
	else if ( pSource == m_pctlIOPresetSave )
		onPresetSave();
	else if ( pSource == m_pctlIOPresetDelete )
		onPresetDelete();
	else if ( pSource == m_pctlDisableFaderMotors )
		m_pSurface->SetFaderMotorEnable( !m_pSurface->GetFaderMotorEnable() );
	else if ( pSource == m_pctlTouchSelects )
		m_pSurface->SetFaderTouchSelectsChannel( !m_pSurface->GetFaderTouchSelectsChannel() );
	else if ( isCtrlInVector( pSource, &m_vRad, &dwIndex ) )
	{
		if ( !m_bIsOcta )
		handleDigitalRadio( pSource, dwIndex );
	}
   else if ( isCtrlInVector( pSource, &m_vDMRad, &dwIndex ) )
	{
		float fVal = m_pSurface->GetIOBoxInterface()->GetParam( 0, (TacomaIOBoxParam) pSource->GetParameter() );
		fVal = fVal >= .5f ? 0.f : 1.f;
		m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) pSource->GetParameter(), fVal );
	}
	else if ( isCtrlInVector( pSource, &this->m_vACTPageButtons, &dwIndex ) )
		m_nACTPage = (int)dwIndex;

	RefreshAllControls();
	RecalcLayout();
}

// Given a radio index, set the proper parameters for digital sync source and digital 1 input source
void CTacomaSurfacePropPage::handleDigitalRadio( CNWControl* pSource,  DWORD dwRadio )
{
	TacomaIOBoxParam param;
	float fVal = 0.f;
	switch( dwRadio )
	{
	case 0:	// internal
		param = TIOP_SyncSource;
		fVal = 0.f;
		break;
	case 1:	// Word Clock
		param = TIOP_SyncSource;
		fVal = 3.f/3.f;
		break;
	case 2:	// Digital 1
		param = TIOP_SyncSource;
		fVal = 1.f/3.f;
		break;
	case 3:	// Digital 2
		param = TIOP_SyncSource;
		fVal = 2.f/3.f;
		break;
	case 4:	// AES
		param = TIOP_DigitalInput;
		fVal = 1.f;
		break;
	case 5:	// Coax
		param = TIOP_DigitalInput;
		fVal = 0.f;
		break;
	default:
		ASSERT(0);
	}
	m_pSurface->GetIOBoxInterface()->SetParam( 0, param, fVal );
}


void CTacomaSurfacePropPage::OnCbnSelchangePreset()
{
	CTacomaSurface::PresetType pt = getCurrentPresetType();
	CString str;
	m_cPresets.GetLBText( m_cPresets.GetCurSel(), str );
	if ( !str.IsEmpty() )
		m_pSurface->LoadIOPreset( pt, str );

	RecalcLayout();
}

void CTacomaSurfacePropPage::OnCbnSelchangeInterface()
{
	UINT u = (UINT)m_cInterface.GetCurSel();
	
	m_pSurface->GetIOBoxInterface()->SetActiveInterface( u );


	if (( m_pSurface->GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (m_pSurface->GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
		m_bIsOcta = false;
		if ( m_pbtnPreampTab->GetCheck()) 
			onPreampTab();
		else
			onDMTab();
	}
	else
	{
		m_bIsOcta = true;

		if ( m_bActivate )
			m_pSurface->GetIOBoxInterface()->GetInitialValues();

		if ( m_pbtnPreampTab->GetCheck())
			onOCPreampTab();
		else
			onOCDMTabA();
	}
	
	RecalcLayout();

	m_bActivate = false;

}

void  CTacomaSurfacePropPage::Initface()
{
	if ( !m_pSurface->GetIOBoxInterface()->m_viofacestr.empty() )
	{
		const UINT uActive = m_pSurface->GetIOBoxInterface()->GetActiveInterface();
		CString strface = m_pSurface->GetIOBoxInterface()->m_viofacestr[uActive];

		if (( strface == (_T("VS-700R"))) || (strface == (_T("VS-700R2"))))
			m_bIsOcta = false;
		else if (( strface == (_T("OCTA-CAPTURE"))) || (strface == (_T("OCTA-CAPTURE EXP"))))
			m_bIsOcta = true;
	}
}

void CTacomaSurfacePropPage::fillInterfaceCombo()
{

	m_cInterface.ResetContent();
	const UINT cIfs = m_pSurface->GetIOBoxInterface()->GetInterfaceCount();
	const UINT uActive = m_pSurface->GetIOBoxInterface()->GetActiveInterface();
	

	for ( UINT i = 0; i < cIfs; i++ )
	{
		m_cInterface.AddString( m_pSurface->GetIOBoxInterface()->m_viofacestr[i] );
	}
	
	if ( cIfs > 0 )
		m_cInterface.SetCurSel( uActive );
}

void CTacomaSurfacePropPage::fillPresetCombo()
{
	m_cPresets.ResetContent();

	CTacomaSurface::PresetType pt = getCurrentPresetType();
	std::vector<CString> vStr;
	m_pSurface->GetPresetsOnDisk( pt, &vStr );
	CString strCur = m_pSurface->GetCurrentPresetName( pt );

	int iSel = -1;
	for ( size_t i = 0; i < vStr.size(); i++ )
	{
		CString& str = vStr[i];
		if ( strCur.CompareNoCase( str ) == 0 )
			iSel = (int)i;
		m_cPresets.AddString( str );
	}

	if ( -1 != iSel )
		m_cPresets.SetCurSel( iSel );

	
}

//-------------------------------------------------------------------------
void CTacomaSurfacePropPage::fillTypeCombo()
{
	m_cType.ResetContent();
	for ( size_t i = 0; i <= 4; i++ )
	{
		m_cType.AddString( s_aType[i] );
	}

	int iSel = 0;
	iSel = (int)m_pSurface->GetIOBoxInterface()->m_byRevType;
	m_cType.SetCurSel( iSel );

	OnCbnSelchangeRevtype();

}
//--------------------------------------------------------------------

void CTacomaSurfacePropPage::fillPredelayCombo()
{
	m_cPredelay.ResetContent();

	for ( size_t i = 0; i <= 12; i++ )
	{
		m_cPredelay.AddString( s_aPredelay[i] );
	}

	int iSel = 1;
	iSel =  m_pSurface->GetIOBoxInterface()->m_byRevType;

	switch ( iSel )
	{
	case OFF:
		m_cPredelay.SetCurSel( m_pSurface->GetIOBoxInterface()->m_iOFF_D );
		break;
	case ECHO:
		m_cPredelay.SetCurSel( m_pSurface->GetIOBoxInterface()->m_iECHO_D );
		break;
	case ROOM:
		m_cPredelay.SetCurSel( m_pSurface->GetIOBoxInterface()->m_iROOM_D );
		break;
	case SMALLH:
		m_cPredelay.SetCurSel( m_pSurface->GetIOBoxInterface()->m_iSHALL_D );
		break;
	case LARGEH:
		m_cPredelay.SetCurSel( m_pSurface->GetIOBoxInterface()->m_iLHALL_D );
		break;
	}

}

//-------------------------------------------------------------------

void  CTacomaSurfacePropPage::onPatchBay()
{

	CPatchBayDlg dlg(this);

	dlg.m_pSurfaceBay = m_pSurface;
	m_pPatchBay = dlg.m_pBay;

	if (IDOK != dlg.DoModal())
		return;
	

	m_pbtnOCBay->SetCheck(FALSE, FALSE);
}
//-------------------------------------------------------------------
void CTacomaSurfacePropPage::DoCtrlContextMenu( CNWControl* pSource, UINT nFlags, CMenu& rMenu )
{
   DWORD dwIndex = -1;
   if ( isCtrlInVector( pSource, &m_vEncoderButtons, &dwIndex ) )
   {
		const DWORD dwCtrlParam = GET_ENCODER_PARAM( pSource->GetParameter() );
		if ( dwIndex >= TacomaIOBox::NumMicInputChannels )
		{
			// IO box
			const TacomaIOBoxParam *pParam = doEncoderIOMenu( rMenu, pSource->GetControlRect().BottomRight() );
			if ( pParam )
			{
				CTacomaSurface::PSEncoderParams pEncParam = m_pSurface->GetEncoderListParam( dwCtrlParam, MIX_STRIP_ANY, CTacomaSurface::Enc_IO );
				if ( pEncParam )
					pEncParam->ioParam = *pParam;
			}
		}
		else
		{
			const SONAR_MIXER_STRIP strip = dwIndex < 4 ? MIX_STRIP_TRACK : MIX_STRIP_BUS;
			const SEncoderParam *pParam = doEncoderMenu( rMenu, strip, pSource->GetControlRect().BottomRight() );
			if ( pParam )
			{
				CTacomaSurface::PSEncoderParams pEncParam = m_pSurface->GetEncoderListParam( dwCtrlParam, MIX_STRIP_ANY, strip );
				if ( pEncParam )
				{
					pEncParam->mixParam = pParam->mixParam;
					pEncParam->dwParam = pParam->dwParam;
				}
			}
		}

      return;
   }

   if ( pSource->GetParameter() == -1 )
      return;

	if ( m_pfeetSw1 == pSource || m_pfeetSw2 == pSource || m_mapCommandButtons.count( pSource->GetParameter() ) > 0 )
		setCmdForButton( rMenu, pSource );
}


void  CTacomaSurfacePropPage::UpdateControls()
{
	if ( m_pSurface == NULL )
		return;

	if  (m_pSurface->m_bSwitchUI)
	{
		const UINT cIfs = m_pSurface->GetIOBoxInterface()->GetInterfaceCount();
		const UINT uActive = m_pSurface->GetIOBoxInterface()->GetActiveInterface();

		if (( uActive == 0) && ( cIfs == 2 ))
		{
			m_cInterface.SetCurSel( 1 );
			m_pSurface->GetIOBoxInterface()->SetActiveInterface( 1 );
			m_pbtnPreampTab->SetCheck( TRUE, FALSE );
			OnCbnSelchangeInterface();
		}
		else if (( uActive == 1) && ( cIfs == 2 ))
		{
			m_cInterface.SetCurSel( 0 );
			m_pSurface->GetIOBoxInterface()->SetActiveInterface( 0 );
			m_pbtnPreampTab->SetCheck( TRUE, FALSE );
			OnCbnSelchangeInterface();
		}
	
		m_pSurface->m_bSwitchUI = false;
	}

	if ( m_eDisplayPage == OC_DirectMixerA )
	{
		m_pctlOUT12Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT12 == DA ? TRUE : FALSE , FALSE);
		m_pctlOUT34Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT34 == DA ? TRUE : FALSE , FALSE);
		m_pctlOUT56Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT56 == DA ? TRUE : FALSE , FALSE);
		m_pctlOUT78Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT78 == DA ? TRUE : FALSE , FALSE);
		m_pctlOUT910Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT910 == DA ? TRUE : FALSE , FALSE);

		if( m_pSurface->GetIOBoxInterface()->m_bMidiVerb == true )
		{
			m_cType.SetCurSel((int) m_pSurface->GetIOBoxInterface()->m_byRevType);
			OnCbnSelchangeRevtype();
		}

		if( m_pSurface->GetIOBoxInterface()->m_bMidiDelay == true )
			m_cPredelay.SetCurSel((int) m_pSurface->GetIOBoxInterface()->m_byDelay);

		m_pSurface->GetIOBoxInterface()->m_bMidiVerb = false;
		m_pSurface->GetIOBoxInterface()->m_bMidiDelay = false;

	}

	if ( m_eDisplayPage == OC_DirectMixerB )
	{
		m_pctlOUT12Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT12 == DB ? TRUE : FALSE , FALSE);
		m_pctlOUT34Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT34 == DB ? TRUE : FALSE , FALSE);
		m_pctlOUT56Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT56 == DB ? TRUE : FALSE , FALSE);
		m_pctlOUT78Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT78 == DB ? TRUE : FALSE , FALSE);
		m_pctlOUT910Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT910 == DB ? TRUE : FALSE , FALSE);
	}

	if ( m_eDisplayPage == OC_DirectMixerC )
	{
		m_pctlOUT12Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT12 == DC ? TRUE : FALSE , FALSE);
		m_pctlOUT34Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT34 == DC ? TRUE : FALSE , FALSE);
		m_pctlOUT56Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT56 == DC ? TRUE : FALSE , FALSE);
		m_pctlOUT78Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT78 == DC ? TRUE : FALSE , FALSE);
		m_pctlOUT910Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT910 == DC ? TRUE : FALSE , FALSE);
	}

	if ( m_eDisplayPage == OC_DirectMixerD )
	{
		m_pctlOUT12Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT12 == DD ? TRUE : FALSE , FALSE);
		m_pctlOUT34Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT34 == DD ? TRUE : FALSE , FALSE);
		m_pctlOUT56Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT56 == DD ? TRUE : FALSE , FALSE);
		m_pctlOUT78Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT78 == DD ? TRUE : FALSE , FALSE);
		m_pctlOUT910Ind->SetCheck( m_pSurface->GetIOBoxInterface()->m_iOUT910 == DD ? TRUE : FALSE , FALSE);
	}
}

//--------------------------------------------------------------------

void CTacomaSurfacePropPage::RefreshAllControls()
{
	__super::RefreshAllControls();

	// light the status lights
	const CTacomaSurface::SpecialDisplayModes	edm = m_pSurface->GetSpecialDisplayMode();
	switch ( edm )
	{
	case CTacomaSurface::SDM_Normal:
		break;
	case CTacomaSurface::SDM_ExistingFX:
	case CTacomaSurface::SDM_FXTree:
		break;
	case CTacomaSurface::SDM_Markers:
		break;
	case CTacomaSurface::SDM_Layers:
		break;
	}
	m_pctlFlip->SetCheck( m_pSurface->IsFlipped(), FALSE );

	const SONAR_MIXER_STRIP eStrip = m_pSurface->Get8StripType();
	m_pctlTracks->SetCheck( eStrip == MIX_STRIP_TRACK, FALSE );
	m_pctlBuses->SetCheck( eStrip == MIX_STRIP_BUS, FALSE );
	m_pctlMains->SetCheck( eStrip == MIX_STRIP_MASTER, FALSE );

	m_pctlIO->SetCheck( m_pSurface->GetIOMode(), FALSE );

	//OC DM Sub Tabs
	
	m_pbtnOCTabA->SetCheck( m_eDisplayPage == OC_DirectMixerA, FALSE );
	m_pbtnOCTabB->SetCheck( m_eDisplayPage == OC_DirectMixerB, FALSE );
	m_pbtnOCTabC->SetCheck( m_eDisplayPage == OC_DirectMixerC, FALSE );
	m_pbtnOCTabD->SetCheck( m_eDisplayPage == OC_DirectMixerD, FALSE );
		
	UpdateControls();

	m_bCommandsDirty = false;

	m_pSurface->GetIOBoxInterface()->m_bMidiVerb = false;
	m_pSurface->GetIOBoxInterface()->m_bMidiDelay = false;
}


//--------------------------------------------------------------------
// request to be refreshed from the App
void	CTacomaSurfacePropPage::RefreshControl( CNWControl* pSource )
{
	CTacomaSurface::PresetType pt = getCurrentPresetType();

   DWORD dwIndex = 0;
	const DWORD dwParam = pSource->GetParameter();
	std::map<DWORD,CNWControl*>::iterator it = m_mapCommandButtons.find( dwParam );
	if (m_pfeetSw1 == pSource || m_pfeetSw2 == pSource || (m_mapCommandButtons.end() != it && it->second == pSource) )
	{
		if ( m_bCommandsDirty )
		{
			// get the map of button ID to Command ID
			CTacomaSurface::MsgIdToBADMap& mapCmds = m_pSurface->GetButtonCommandMap();

			CTacomaSurface::MsgIdToBADMapIterator it = mapCmds.find( (CTacomaSurface::ControlId)dwParam );
			if ( it == mapCmds.end() )
			{
				ASSERT(0);
				return;
			}

			CTacomaSurface::ButtonActionDefinition& bad = it->second;
			if ( bad.eActionType == CTacomaSurface::ButtonActionDefinition::AT_Key )
			{
				pSource->SetCtrlValueString( getFriendlyKeystrokeName( bad ) );
			}
			else if ( bad.eActionType == CTacomaSurface::ButtonActionDefinition::AT_Command )
			{
				DWORD dwIx = m_pSurface->IndexForCommandID( bad.dwCommandOrKey );

				char sz[32] = { 0 };
				DWORD dwLen = sizeof(sz);
				DWORD dwID = 0;
				m_pSurface->GetCommandInterface()->GetCommandInfo( dwIx, &dwID, sz, &dwLen );

				char* psz = ::strrchr( sz, '|');
				if ( !psz )
					psz = sz;
				else
					psz += 2;	// get past the pipe
				pSource->SetCtrlValueString( psz );
			}
			else if ( bad.eActionType == CTacomaSurface::ButtonActionDefinition::AT_Transport )
				pSource->SetCtrlValueString( getFriendlyTransportName( bad ) );
		}
	}
   else if ( isCtrlInVector( pSource, &m_vEncoderButtons, &dwIndex ) )
   {
		const DWORD dwCtrlParam = GET_ENCODER_PARAM( pSource->GetParameter() );
		if ( dwIndex >= TacomaIOBox::NumMicInputChannels )
		{
			// IO box
			CTacomaSurface::PSEncoderParams pParam = m_pSurface->GetEncoderListParam( dwCtrlParam, MIX_STRIP_ANY, CTacomaSurface::Enc_IO );
			if ( !pParam )
				return;

			pSource->SetCtrlValueString( m_pSurface->GetIOBoxInterface()->GetParamName( pParam->ioParam ) );
		}
		else
		{
			const SONAR_MIXER_STRIP strip = dwIndex < 4 ? MIX_STRIP_TRACK : MIX_STRIP_BUS;
			CTacomaSurface::PSEncoderParams pParam = m_pSurface->GetEncoderListParam( dwCtrlParam, MIX_STRIP_ANY, strip );
			if ( !pParam )
				return;

			pSource->SetCtrlValueString( GetStringForMixParam( pParam->mixParam, pParam->dwParam ) );
		}
   }
   else if ( isCtrlInVector( pSource, &m_vRad, &dwIndex ) )
		refreshDigitalRadio( pSource, dwIndex );
   else if ( isCtrlInVector( pSource, &m_vDMRad, &dwIndex ) )
   {
		const BOOL bOn = (BOOL) m_pSurface->GetIOBoxInterface()->GetParam( 0, (TacomaIOBoxParam) pSource->GetParameter() );
      ((CNWMultiBtn *) pSource)->SetCheck( bOn, FALSE );
   }
	else if( isCtrlInVector ( pSource, &m_vRotaryACTLbl, NULL ) || isCtrlInVector( pSource, &m_vSwitchACTLbl, NULL ) || pSource == m_pTBarACTLbl )
	{
		// update with act parameter name
		DWORD dwID = (DWORD)-1;
		if ( pSource == m_pTBarACTLbl )
			dwID = ACTKEY_BASE + TBAR_DYN_INDEX;
		else
			dwID = pSource->GetParameter() + m_nACTPage * TOTAL_DYN_COUNT;
		pSource->SetCtrlValueString( m_pSurface->GetACTParamLabel( dwID ) );
	}
	else if ( isCtrlInVector( pSource, &this->m_vACTPageButtons, &dwIndex ) )
		pSource->SetCtrlValue( ((int)dwIndex == m_nACTPage), FALSE );

	else if ( (DWORD)-1 != dwParam )
	{
		const DWORD dwIxChan = LOWORD(dwParam);
		TacomaIOBoxParam const iop = (TacomaIOBoxParam)HIWORD(dwParam);

		const float f01 = m_pSurface->GetIOBoxInterface()->GetParam( dwIxChan, iop );
		if ( iop != TIOP_CompEnable )
		{
#if 0	// risky so close to RTM
			// hack - The string representation of the gain param can change based on 
			// the state of another parameter (Pad). So we need to force it to re-acquire
			// a value string even if the internal value is not changing
			if ( TIOP_Gain == iop )
				pSource->SetCtrlValueString( _T("") );
#endif
			pSource->SetCtrlValue( f01, FALSE );

		}
		else if (iop == TIOP_StereoLinkOC)
			enableDMStrips();
		else
		{
			const float fValOld = (float) pSource->GetCtrlValue();
			pSource->SetCtrlValue( f01, FALSE );
			if ( fValOld != f01 )
				RecalcLayout();
		}
	}
	else if ( pSource == m_pbtnPrefTab )
	{
		if ( m_bIsOcta )
		m_pbtnPrefTab->SetCheck( DP_Preferences == m_eDisplayPage, FALSE );
		else
		m_pbtnPrefTab->SetCheck( DP_Preferences == m_eDisplayPage, FALSE );
	}
	else if ( pSource == m_pbtnPreampTab )
	{
		if ( m_bIsOcta )
			m_pbtnPreampTab->SetCheck( OC_Preamp == m_eDisplayPage, FALSE );
		else
			m_pbtnPreampTab->SetCheck( DP_Preamp == m_eDisplayPage, FALSE );
	}
	else if ( pSource == m_pbtnDMTab )
	{
		if ( m_bIsOcta )
			m_pbtnDMTab->SetCheck( OC_DirectMixerA == m_eDisplayPage, FALSE );
		else
			m_pbtnDMTab->SetCheck( DP_DirectMixer == m_eDisplayPage, FALSE );
	}
	else if ( pSource == m_pbtnACTTab )
	{
	if ( m_bIsOcta )
			m_pbtnACTTab->SetCheck( DP_ACT == m_eDisplayPage, FALSE );
		else
			m_pbtnACTTab->SetCheck( DP_ACT == m_eDisplayPage, FALSE );
	}
	else if ( pSource == m_pctlACTContextLbl )
		pSource->SetCtrlValueString( m_pSurface->GetACTContextName() );
	else if ( pSource == m_pctlDisableFaderMotors )
		m_pctlDisableFaderMotors->SetCheck( !m_pSurface->GetFaderMotorEnable(), FALSE );
	else if ( pSource == m_pctlTouchSelects )
		m_pctlTouchSelects->SetCheck( m_pSurface->GetFaderTouchSelectsChannel(), FALSE );
	else if ( pSource == m_pctlIO )
		m_pctlIO->SetCheck( m_pSurface->GetIOMode(), FALSE );
	else if ( pSource == m_pctlLayerInd )
		m_pctlLayerInd->SetCheck( m_pSurface->GetSpecialDisplayMode() == CTacomaSurface::SDM_Layers, FALSE );
	else if ( pSource == m_pctlFxInd )
		m_pctlFxInd->SetCheck( m_pSurface->GetSpecialDisplayMode() == CTacomaSurface::SDM_ExistingFX || 
										m_pSurface->GetSpecialDisplayMode() == CTacomaSurface::SDM_FXTree, FALSE );
	else if ( pSource == m_pctlInSync )
		m_pctlInSync->SetCheck( m_pSurface->GetIOBoxInterface()->m_fSyncVal == 0 ? TRUE : FALSE, FALSE );
	else if ( pSource == m_pctlDigSync )
		m_pctlDigSync->SetCheck(m_pSurface->GetIOBoxInterface()->m_fSyncVal == 1 ? TRUE : FALSE, FALSE );
	
}


void CTacomaSurfacePropPage::refreshDigitalRadio( CNWControl* pSource, DWORD dwRadio )
{
	// get the raw float value
	TacomaIOBoxParam param = dwRadio < 4 ? TIOP_SyncSource : TIOP_DigitalInput;
	float fVal = m_pSurface->GetIOBoxInterface()->GetParam( 0, param );

	// and decide which button to check
	DWORD dwIx = 0;
	if ( param == TIOP_SyncSource )
	{
		if ( fVal < .25f )
			dwIx = 0;
		else if ( fVal < .5f )
			dwIx = 2;
		else if ( fVal < .75 )
			dwIx = 3;
		else
			dwIx = 1;
	}
	else
	{
		if ( fVal < .5f )
			dwIx = 5;
		else
			dwIx = 4;
	}

	pSource->SetCtrlValue( dwIx == dwRadio ? 1 : 0, FALSE );
}




void CTacomaSurfacePropPage::OnRequestValueString( CNWControl* pSource, CString* pstr, double* pdVal )//= NULL )
{
	DWORD dwParam = pSource->GetParameter();
	if ( (DWORD)-1 != dwParam )
	{
		DWORD dwIxChan = LOWORD(dwParam);
		TacomaIOBoxParam iop = (TacomaIOBoxParam)HIWORD(dwParam);

		*pstr = m_pSurface->GetIOBoxInterface()->GetParamValueText( iop, dwIxChan, (float)pSource->GetCtrlValue() );
	}
}



CTacomaSurface::PresetType CTacomaSurfacePropPage::getCurrentPresetType()
{
	CTacomaSurface::PresetType pt = CTacomaSurface::PT_NONE;
	switch( m_eDisplayPage )
	{
	case DP_DirectMixer:
		pt = CTacomaSurface::PT_DM;
		break;
	case DP_Preamp:
		pt = CTacomaSurface::PT_Preamp;
		break;
	case OC_DirectMixerA:
		pt = CTacomaSurface::PT_DM;
		break;
	case OC_DirectMixerB:
		pt = CTacomaSurface::PT_DM;
		break;
	case OC_DirectMixerC:
		pt = CTacomaSurface::PT_DM;
		break;
	case OC_DirectMixerD:
		pt = CTacomaSurface::PT_DM;
		break;
	case OC_Preamp:
		pt = CTacomaSurface::PT_Preamp;
		break;
		
	}
	return pt;
}



//-----------------------------------------------------------------------------
void CTacomaSurfacePropPage::onPresetDelete()
{
	CTacomaSurface::PresetType pt = getCurrentPresetType();
	CString strName;
	m_cPresets.GetLBText( m_cPresets.GetCurSel(), strName );
	if ( m_pSurface->PresetExists( pt, strName ) )
	{
		// prompt for overwrite
		CString str;
		str.Format( IDS_PROMPT_PRESET_DELETE, strName );
		if ( IDYES != ::AfxMessageBox( str, MB_YESNO|MB_ICONQUESTION ) )
			return;
	}
	m_pSurface->DeletePreset( pt, strName );

	fillPresetCombo();
}


//-----------------------------------------------------------------------------
void CTacomaSurfacePropPage::onPresetSave()
{
	CTacomaSurface::PresetType pt = getCurrentPresetType();
	CString strName;
	m_cPresets.GetWindowText( strName );

	if ( m_pSurface->PresetExists( pt, strName ) )
	{
		// prompt for overwrite
		CString str;
		str.Format( IDS_PROMPT_PRESET_OVERWRITE, strName );
		if ( IDYES != ::AfxMessageBox( str, MB_YESNO|MB_ICONQUESTION ) )
			return;
	}

	m_pSurface->SaveIOPreset( pt, strName );

	fillPresetCombo();
}



//-------------------------------------------------------------------
CString CTacomaSurfacePropPage::getFriendlyTransportName( CTacomaSurface::ButtonActionDefinition& bad )
{
	CString str(_T("err") );;
	if( bad.eActionType != CTacomaSurface::ButtonActionDefinition::AT_Transport )
	{
		ASSERT(0);
		return str;
	}
	switch( bad.transportState )
	{
	case TRANSPORT_STATE_PLAY:
		str = _T("Play/Stop");
		break;
	case TRANSPORT_STATE_REC:
		str = _T("Record");
		break;
	case TRANSPORT_STATE_REWIND:
		str = _T("Rewind");
		break;
	case TRANSPORT_STATE_FFWD:
		str = _T("Fast Fwd");
		break;
	}

	return str;
}

//-------------------------------------------------------------------
CString CTacomaSurfacePropPage::getFriendlyKeystrokeName( CTacomaSurface::ButtonActionDefinition& bad )
{
	if( bad.eActionType != CTacomaSurface::ButtonActionDefinition::AT_Key )
	{
		ASSERT(0);
		return _T("err");
	}

	CString strNK;
	CString strMod;
	bool bMatch = false;
	for ( size_t i = 0; i < sm_vKeys.size(); i++ )
	{
		NamedKey& nk = sm_vKeys[i];
		if ( bad.dwCommandOrKey == nk.vk )
		{
			strNK = nk.strName;
			if ( bad.wModKeys & CTacomaSurface::SMK_SHIFT )
				strMod = _T("Shift+");
			if ( bad.wModKeys & CTacomaSurface::SMK_CTRL )
				strMod = _T("Ctrl+");
			if ( bad.wModKeys & CTacomaSurface::SMK_ALT )
				strMod = _T("Alt+");

			bMatch = true;
			break;
		}
	}

	CString strRet = strMod + strNK;
	if ( bMatch )
	{
		// special case names
		// Ctrl-f4
		if ( VK_F4 == bad.dwCommandOrKey && CTacomaSurface::SMK_CTRL == bad.wModKeys )
			strRet = _T("Close");

		// F4
		if ( VK_F4 == bad.dwCommandOrKey && 0 == bad.wModKeys )
			strRet = _T("Transport");

		// F1
		else if ( VK_F1 == bad.dwCommandOrKey && 0 == bad.wModKeys )
			strRet = _T("Help");

		// Ctrl-tab
		else if ( VK_TAB == bad.dwCommandOrKey && CTacomaSurface::SMK_CTRL == bad.wModKeys )
			strRet = _T("Next");

		// Ctrl-Shift-tab
		else if ( VK_TAB == bad.dwCommandOrKey && (CTacomaSurface::SMK_CTRL|CTacomaSurface::SMK_SHIFT) == bad.wModKeys )
			strRet = _T("Prev");
	}

	return strRet;
}


//--------------------------------------------------------------------
void	CTacomaSurfacePropPage::OnValueChange( CNWControl* pSource )
{
	// find the control in the array
	DWORD dwParam = pSource->GetParameter();
   DWORD dwIndex = 0;
	if ( (DWORD)-1 != dwParam )
	{
		const DWORD dwIxChan = LOWORD(dwParam);
		const TacomaIOBoxParam iop = (TacomaIOBoxParam)HIWORD(dwParam);

		// For Volume sliders, make sure we quickly refresh the buddy text display control
		m_pSurface->GetIOBoxInterface()->SetParam( dwIxChan, iop, (float)pSource->GetCtrlValue() );


		if ( TIOP_Gain == iop )
		{
			for ( std::vector<IOChanControls>::iterator it = m_aIOControls.begin(); it != m_aIOControls.end(); ++it )
			{
				IOChanControls& ioc = *it;
				if ( ioc.pGain == pSource )
					RefreshControl( ioc.pGainLabel );
			}
		}
		else if ( TIOP_DMixVol == iop  )
		{
			for ( std::vector<IODirectMixControls>::iterator it = m_aDMControls.begin(); it != m_aDMControls.end(); ++it )
			{
				IODirectMixControls& iodm = *it;
				if ( iodm.pDMVol == pSource )
					RefreshControl( iodm.pDMVolLabel );
				if ( iodm.pOCVol == pSource )
					RefreshControl( iodm.pOCVolLabel );
				if ( iodm.pOCVolB == pSource )
					RefreshControl( iodm.pOCVolLabelB );
				if ( iodm.pOCVolC == pSource )
					RefreshControl( iodm.pOCVolLabelC );
				if ( iodm.pOCVolD == pSource )
					RefreshControl( iodm.pOCVolLabelD );
			}
		}
		/*
		else if ( TIOP_StereoLinkOC == iop  )
		{
			for ( std::vector<IODirectMixControls>::iterator it = m_aDMControls.begin(); it != m_aDMControls.end(); ++it )
			{
			IODirectMixControls& iodm = *it;

				if (( iodm.pLinkOC== pSource ) || ( iodm.pLinkOCB== pSource ) || ( iodm.pLinkOCC== pSource ) || ( iodm.pLinkOCD== pSource ) )
					enableDMStrips();

			}
		}
		*/
		
	  }
	
}

//--------------------------------------------------------------------
void	CTacomaSurfacePropPage::OnFirstPunchIn( CNWControl* pSource )
{
}

//--------------------------------------------------------------------
void	CTacomaSurfacePropPage::OnFinalPunchOut( CNWControl* pSource )
{
}


//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnLButtonDown(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetCapture();
	if ( findControlAndSetActive( point ) )
		m_pActiveControl->HandleLMouseDN( nFlags, point );

	__super::OnLButtonDown(nFlags, point);
}


void CTacomaSurfacePropPage::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetCapture();
	if ( findControlAndSetActive( point ) )
		m_pActiveControl->HandleLMouseDClk( nFlags, point );

	__super::OnLButtonDblClk(nFlags, point);
}


//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ReleaseCapture();

	if ( m_pActiveControl )
		m_pActiveControl->HandleLMouseUP( nFlags, point );

	__super::OnLButtonUp(nFlags, point);
}


//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnRButtonDown(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( findControlAndSetActive( point ) )
		m_pActiveControl->HandleRMouseDN( nFlags, point );

	__super::OnRButtonDown(nFlags, point);
}

//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnRButtonUp(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( m_pActiveControl )
		m_pActiveControl->HandleRMouseUP( nFlags, point );
	else
	{
		// secret back-door to refetch params
		if ( nFlags & MK_SHIFT )
			m_pSurface->GetIOBoxInterface()->GetInitialValues();
	}


	__super::OnRButtonUp(nFlags, point);
}

//--------------------------------------------------------------------
void CTacomaSurfacePropPage::OnMouseMove(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( m_pActiveControl )
		m_pActiveControl->HandleMouseMove( nFlags, point );

	__super::OnMouseMove(nFlags, point);
}


//////////////////////////////////////////////////////////////////////////////
// Given a command picker, display the list of commands and allow the user
// to re-assign the button to a different command
void CTacomaSurfacePropPage::setCmdForButton( CMenu& menu, CNWControl* pSource )
{
	// Determine the current command for this button
	CTacomaSurface::MsgIdToBADMap& mapCmds = m_pSurface->GetButtonCommandMap();

	CTacomaSurface::MsgIdToBADMapIterator it = mapCmds.find( (CTacomaSurface::ControlId)pSource->GetParameter() );
	if ( it == mapCmds.end() )
	{
		ASSERT(0);
		return;
	}
	CTacomaSurface::ButtonActionDefinition& bad = it->second;

	CButtonPropsDlg dlg( m_pSurface, bad );
	if ( IDOK == dlg.DoModal() )
		bad = dlg.m_bad;

	m_bCommandsDirty = true;
}

void CTacomaSurfacePropPage::setCmdForIOEncoder( CMenu& menu, CNWControl* pSource )
{
}

void CTacomaSurfacePropPage::enablePage( const DisplayPage ePage )
{
   //
   // Surface
   // 
   BOOL bSet = ( ePage == DP_Preferences );

	CNWControl* pc = NULL;

   //  commands
	for ( std::map<DWORD,CNWControl*>::iterator it = m_mapCommandButtons.begin(); it != m_mapCommandButtons.end(); ++it )
	{
		pc = it->second;
		pc->SetIsVisible( bSet );
	}
	for ( std::map<DWORD,CNWControl*>::iterator it = m_mapCommandButtonsBmp.begin(); it != m_mapCommandButtonsBmp.end(); ++it )
	{
		pc = it->second;
		pc->SetIsVisible( bSet );
	}
   //  encoder section
	for ( std::vector<CNWControl*>::iterator it = m_vEncoderButtons.begin(); it != m_vEncoderButtons.end(); ++it )
	{
		pc = *it;
		pc->SetIsVisible( bSet );
	}

   //  misc
	m_pctlTrackEncFunc->SetIsVisible( bSet );
	m_pctlBusEncFunc->SetIsVisible( bSet );
	m_pctlIOEncFunc->SetIsVisible( bSet );
	m_pctlDisableFaderMotors->SetIsVisible( bSet );
	m_pctlTouchSelects->SetIsVisible( bSet );
	m_sDisable.ShowWindow( bSet ? SW_SHOW:SW_HIDE );
	m_sTouch.ShowWindow( bSet ? SW_SHOW:SW_HIDE );
	m_pfeetSw1->SetIsVisible( bSet );
	m_pfeetSw2->SetIsVisible( bSet );



    //OC Mic Pre
   //
if ( m_bIsOcta )
{
   bSet = ( ePage == OC_Preamp );

	for ( size_t i = 0; i < m_aIOControls.size(); i++ )
	{
		IOChanControls& iocc = m_aIOControls[i];
		iocc.Show( bSet );

		if ( iocc.pLink )
			iocc.pLink->SetIsVisible( FALSE );
	}
	
   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

}
else
{
	
	//
   // Mic Pre

   bSet = ( ePage == DP_Preamp );

	for ( size_t i = 0; i < m_aIOControls.size(); i++ )
	{
		IOChanControls& iocc = m_aIOControls[i];
		iocc.Show( bSet );

		//hide Gate
		iocc.pGate->SetIsVisible (FALSE);

		if ( iocc.pLinkOC )
			iocc.pLinkOC->SetIsVisible( FALSE );

		if ( iocc.pLinkOC )
			iocc.pLinkOC->SetIsVisible( FALSE );
	}
   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( bSet );
	}

}

if ( m_bIsOcta )
{
   // OC Direct Mix
   //

   bSet = ( ePage == OC_DirectMixerA || ePage == OC_DirectMixerB || ePage == OC_DirectMixerC || ePage == OC_DirectMixerD );


   for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		IODirectMixControls& iodmc = m_aDMControls[i];
		iodmc.Show( bSet );
		if ( i < 20 )
		{
			//iodmc.pOCMasVol->SetIsVisible( FALSE );
			//iodmc.pOCMasVolLabel->SetIsVisible( FALSE );

		}
		if ( i > 9 )
			iodmc.pDMSend->SetIsVisible( FALSE );
		
		iodmc.pDMSolo->SetIsVisible ( FALSE );
		iodmc.pDMMute->SetIsVisible ( FALSE );
		iodmc.pDMPan->SetIsVisible ( FALSE );
		iodmc.pDMVol->SetIsVisible ( FALSE );
		iodmc.pDMVolLabel->SetIsVisible ( FALSE );
		if ( iodmc.pDMMono )
			iodmc.pDMMono->SetIsVisible( FALSE );
		if ( iodmc.pLink )
			iodmc.pLink->SetIsVisible( FALSE );

	

	}
   for ( std::vector< CNWControl* >::iterator it = m_vDMRad.begin(); it != m_vDMRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

 /*  
   //--------------------------------------------------------------------------------------------------------------------------------
      bSet = (  ePage == OC_DirectMixerB  );

   for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		IODirectMixControls& iodmc = m_aDMControls[i];
		iodmc.Show( bSet );
		if ( i < 20 )
		{
			//iodmc.pOCMasVol->SetIsVisible( FALSE );
			//iodmc.pOCMasVolLabel->SetIsVisible( FALSE );

		}
		if ( i > 9 )
			iodmc.pDMSend->SetIsVisible( FALSE );
		
		iodmc.pDMSolo->SetIsVisible ( FALSE );
		iodmc.pDMMute->SetIsVisible ( FALSE );
		iodmc.pDMPan->SetIsVisible ( FALSE );
		iodmc.pDMVol->SetIsVisible ( FALSE );
		iodmc.pDMVolLabel->SetIsVisible ( FALSE );
		if ( iodmc.pDMMono )
			iodmc.pDMMono->SetIsVisible( FALSE );
		if ( iodmc.pLink )
			iodmc.pLink->SetIsVisible( FALSE );
		if ( iodmc.pLinkOC )
			iodmc.pLinkOC->SetIsVisible( FALSE );

		//IODirectMixControls& iodmcA = m_aDMControlsA[i];
		//iodmcA.Show(FALSE);
	}
   for ( std::vector< CNWControl* >::iterator it = m_vDMRad.begin(); it != m_vDMRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

      
   //--------------------------------------------------------------------------------------------------------------------------------
      bSet = (  ePage == OC_DirectMixerC  );

   for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		IODirectMixControls& iodmc = m_aDMControls[i];
		iodmc.Show( bSet );
		if ( i < 20 )
		{
			//iodmc.pOCMasVol->SetIsVisible( FALSE );
			//iodmc.pOCMasVolLabel->SetIsVisible( FALSE );

		}
		if ( i > 9 )
			iodmc.pDMSend->SetIsVisible( FALSE );
		
		iodmc.pDMSolo->SetIsVisible ( FALSE );
		iodmc.pDMMute->SetIsVisible ( FALSE );
		iodmc.pDMPan->SetIsVisible ( FALSE );
		iodmc.pDMVol->SetIsVisible ( FALSE );
		iodmc.pDMVolLabel->SetIsVisible ( FALSE );
		if ( iodmc.pDMMono )
			iodmc.pDMMono->SetIsVisible( FALSE );
		if ( iodmc.pLink )
			iodmc.pLink->SetIsVisible( FALSE );
		if ( iodmc.pLinkOCB )
			iodmc.pLinkOCB->SetIsVisible( FALSE );

		//IODirectMixControls& iodmcA = m_aDMControlsA[i];
		//iodmcA.Show(FALSE);
	}
   for ( std::vector< CNWControl* >::iterator it = m_vDMRad.begin(); it != m_vDMRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}


      
   //--------------------------------------------------------------------------------------------------------------------------------
      bSet = (  ePage == OC_DirectMixerD );

   for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		IODirectMixControls& iodmc = m_aDMControls[i];
		iodmc.Show( bSet );
		if ( i < 20 )
		{
			//iodmc.pOCMasVol->SetIsVisible( FALSE );
			//iodmc.pOCMasVolLabel->SetIsVisible( FALSE );

		}
		if ( i > 9 )
			iodmc.pDMSend->SetIsVisible( FALSE );
		
		iodmc.pDMSolo->SetIsVisible ( FALSE );
		iodmc.pDMMute->SetIsVisible ( FALSE );
		iodmc.pDMPan->SetIsVisible ( FALSE );
		iodmc.pDMVol->SetIsVisible ( FALSE );
		iodmc.pDMVolLabel->SetIsVisible ( FALSE );
		if ( iodmc.pDMMono )
			iodmc.pDMMono->SetIsVisible( FALSE );
		if ( iodmc.pLink )
			iodmc.pLink->SetIsVisible( FALSE );
		if ( iodmc.pLinkOCB )
			iodmc.pLinkOCB->SetIsVisible( FALSE );

		//IODirectMixControls& iodmcA = m_aDMControlsA[i];
		//iodmcA.Show(FALSE);
	}
   for ( std::vector< CNWControl* >::iterator it = m_vDMRad.begin(); it != m_vDMRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}

   for ( std::vector< CNWControl* >::iterator it = m_vRad.begin(); it != m_vRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( FALSE );
	}
	
	*/

}
else 

{   // Direct Mix
   //
   bSet = ( ePage == DP_DirectMixer );

   for ( size_t i = 0; i < m_aDMControls.size(); i++ )
	{
		IODirectMixControls& iodmc = m_aDMControls[i];
		if ( i < 23 )
		{
			iodmc.Show( bSet );
			//iodmc.pOCMasVol->SetIsVisible( FALSE );
			//iodmc.pOCMasVolLabel->SetIsVisible( FALSE );
			iodmc.pOCVol->SetIsVisible( FALSE );
			iodmc.pOCVolLabel->SetIsVisible( FALSE );
			iodmc.pOCMute->SetIsVisible( FALSE );
			iodmc.pOCSolo->SetIsVisible( FALSE );
			iodmc.pOCPan->SetIsVisible( FALSE );
			
			iodmc.pOCVolB->SetIsVisible( FALSE );
			iodmc.pOCVolLabelB->SetIsVisible( FALSE );
			iodmc.pOCMuteB->SetIsVisible( FALSE );
			iodmc.pOCSoloB->SetIsVisible( FALSE );
			iodmc.pOCPanB->SetIsVisible( FALSE );
			
			iodmc.pOCVolC->SetIsVisible( FALSE );
			iodmc.pOCVolLabelC->SetIsVisible( FALSE );
			iodmc.pOCMuteC->SetIsVisible( FALSE );
			iodmc.pOCSoloC->SetIsVisible( FALSE );
			iodmc.pOCPanC->SetIsVisible( FALSE );

			iodmc.pOCVolD->SetIsVisible( FALSE );
			iodmc.pOCVolLabelD->SetIsVisible( FALSE );
			iodmc.pOCMuteD->SetIsVisible( FALSE );
			iodmc.pOCSoloD->SetIsVisible( FALSE );
			iodmc.pOCPanD->SetIsVisible( FALSE );
			
			if ( iodmc.pLinkOC )
				iodmc.pLinkOC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCB )
				iodmc.pLinkOCB->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCC )
				iodmc.pLinkOCC->SetIsVisible( FALSE );
			if ( iodmc.pLinkOCD )
				iodmc.pLinkOCD->SetIsVisible( FALSE );
		}
		else
			iodmc.Show( FALSE );

		iodmc.pDMSend->SetIsVisible( FALSE );

	}
   for ( std::vector< CNWControl* >::iterator it = m_vDMRad.begin(); it != m_vDMRad.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( bSet );
	}

}
   //
   // ACT
   //
   bSet = ( ePage == DP_ACT );

   for ( std::vector< CNWControl* >::iterator it = m_vRotaryACTLbl.begin(); it != m_vRotaryACTLbl.end(); ++ it )
	{
		pc = *it;
      pc->SetIsVisible( bSet );
	}
	for ( size_t i = 0; i < m_vSwitchACTLbl.size(); i++ )
	{
		pc = m_vSwitchACTLbl[i];
		pc->SetIsVisible( bSet );
	}
	for ( size_t i = 0; i < m_vACTPageButtons.size(); i++ )
	{
		pc = m_vACTPageButtons[i];
		pc->SetIsVisible( bSet );
	}

	m_pTBarACTLbl->SetIsVisible( bSet );

   //
   // Misc
   //
   const BOOL bPreset = ( ePage == DP_Preamp ) || ( ePage == DP_DirectMixer ) || ( ePage == OC_Preamp ) || ( ePage == OC_DirectMixerA )
	   || ( ePage == OC_DirectMixerB ) || ( ePage == OC_DirectMixerC ) || ( ePage == OC_DirectMixerD );
   const BOOL bOCTabs = ( ePage ==OC_DirectMixerA ) || ( ePage == OC_DirectMixerB ) || ( ePage == OC_DirectMixerC ) || ( ePage == OC_DirectMixerD );
   const BOOL bSync =  ePage == OC_Preamp;
   const BOOL bDmixA =  ePage == OC_DirectMixerA;
   m_pctlIOPresetSave->SetIsVisible( bPreset );
   m_pctlIOPresetDelete->SetIsVisible( bPreset );

   m_pbtnOCTabA->SetIsVisible( bOCTabs );
   m_pbtnOCTabB->SetIsVisible( bOCTabs );
   m_pbtnOCTabC->SetIsVisible( bOCTabs );
   m_pbtnOCTabD->SetIsVisible( bOCTabs );

   m_pctlOUT12Ind->SetIsVisible( bOCTabs );
   m_pctlOUT34Ind->SetIsVisible( bOCTabs );
   m_pctlOUT56Ind->SetIsVisible( bOCTabs );
   m_pctlOUT78Ind->SetIsVisible( bOCTabs );
   m_pctlOUT910Ind->SetIsVisible( bOCTabs );

   m_pbtnOCBay->SetIsVisible( bOCTabs );
   m_pctlDigSync->SetIsVisible( bSync );
   m_pctlInSync->SetIsVisible( bSync );
   m_pctlReturn->SetIsVisible( bDmixA );
   m_pctlRevTime->SetIsVisible( bDmixA );
   m_pBtnMasterLink->SetIsVisible( bOCTabs );
   
   const DWORD dwShow = bPreset ? SW_SHOW : SW_HIDE;
   const DWORD dwShowRev = bDmixA ? SW_SHOW : SW_HIDE;
   m_cPresets.ShowWindow( dwShow );
  // m_cInterface.ShowWindow( dwShow );
   m_cType.ShowWindow( dwShowRev );
   m_cPredelay.ShowWindow( dwShowRev );
   
}

BOOL CTacomaSurfacePropPage::isCtrlInVector( CNWControl *pCtrl, std::vector< CNWControl * > *pVector, DWORD *pdwIndex )
{
   if ( !pVector || !pCtrl )
      return ( FALSE );

	std::vector<CNWControl*>::iterator it = std::find( pVector->begin(), pVector->end(), pCtrl );
	if ( it != pVector->end() )
	{
		if ( pdwIndex )
			*pdwIndex = (DWORD)( it - pVector->begin() );
		return TRUE;
	}
	
	return FALSE;
}

const TacomaIOBoxParam * CTacomaSurfacePropPage::doEncoderIOMenu( CMenu &rMenu, CPoint pt )
{
	pt.x -= 2;
	pt.y -= 5;
	ClientToScreen( &pt );

   for ( DWORD dwIndex = 0; dwIndex < _countof( s_aEncoderOptionsIO ); dwIndex ++ )
	{
		rMenu.AppendMenu( MF_STRING | MF_ENABLED,
								dwIndex + 1,
								m_pSurface->GetIOBoxInterface()->GetParamName( s_aEncoderOptionsIO[ dwIndex ] ) );
	}

	const TacomaIOBoxParam *pRet = NULL;

   DWORD dwCmd = rMenu.TrackPopupMenu( TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this );
   if ( dwCmd != 0 )
   {
      dwCmd --;
      pRet = &s_aEncoderOptionsIO[ dwCmd ];
   }

   return ( pRet );
}

const SEncoderParam * CTacomaSurfacePropPage::doEncoderMenu( CMenu &rMenu, SONAR_MIXER_STRIP strip, CPoint pt )
{
	pt.x -= 2;
	pt.y -= 5;
	ClientToScreen( &pt );

   for ( DWORD dwIndex = 0; dwIndex < _countof( s_aEncoderOptions ); dwIndex ++ )
   {
      const SONAR_MIXER_PARAM mixParam = s_aEncoderOptions[ dwIndex ].mixParam;
      if ( strip == MIX_STRIP_BUS && mixParam == MIX_PARAM_INPUT )
         continue;

      rMenu.AppendMenu( MF_STRING | MF_ENABLED,
								dwIndex + 1,
								s_aEncoderOptions[dwIndex].strLabel );
   }

   const SEncoderParam *pRet = NULL;

   DWORD dwCmd = rMenu.TrackPopupMenu( TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this );
   if ( dwCmd != 0 )
   {
      dwCmd --;
      pRet = &s_aEncoderOptions[ dwCmd ];
   }

   return ( pRet );
}


CString CTacomaSurfacePropPage::GetStringForMixParam( SONAR_MIXER_PARAM mixParam, DWORD dwParam )
{
	for ( size_t i = 0; i < _countof( s_aEncoderOptions); i++ )
	{
		if ( mixParam == s_aEncoderOptions[i].mixParam && dwParam == s_aEncoderOptions[i].dwParam )
			return s_aEncoderOptions[i].strLabel;
	}

	return _T("");
}


void CTacomaSurfacePropPage::OnCbnSelchangeRevpredelay()
{
	// TODO: Add your control notification handler code here
	UINT u = (UINT)m_cPredelay.GetCurSel();


	switch ( u )
	{
	case 0:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)0);	
		}
		break;
	case 1:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)1);
		}
		break;
	case 2:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)2);
		}
		break;
	case 3:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)3);
		}
		break;
	case 4:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)4);
		}
		break;
	case 5:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)5);
		}
		break;
	case 6:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)6);
		}
		break;
	case 7:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)7);
		}
		break;
	case 8:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)8);
		}
		break;
	case 9:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)9);
		}
		break;
	case 10:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)10);
		}
		break;
	case 11:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)11);
		}
		break;
	case 12:
		{
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevDelay, (float)12);
		}
		break;

	default:
		break;
	}


}


void CTacomaSurfacePropPage::OnCbnSelchangeRevtype()
{
	// TODO: Add your control notification handler code here
	UINT u = (UINT)m_cType.GetCurSel();

	switch ( u )
	{
	case OFF:
		{
			m_pSurface->GetIOBoxInterface()->m_byRevType = OFF;
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevType, (float)OFF);
			m_cPredelay.SetCurSel(m_pSurface->GetIOBoxInterface()->m_iOFF_D);
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevTime, (float)(m_pSurface->GetIOBoxInterface()->m_iOFF_T/49.0));
			m_pctlRevTime->SetCtrlValue( (double)m_pSurface->GetIOBoxInterface()->m_iOFF_T/49, FALSE );
		}
		break;
	case ECHO:
		{
			m_pSurface->GetIOBoxInterface()->m_byRevType = ECHO;
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevType, (float)ECHO);
			m_cPredelay.SetCurSel(m_pSurface->GetIOBoxInterface()->m_iECHO_D);
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevTime, (float)(m_pSurface->GetIOBoxInterface()->m_iECHO_T/49.0));
			m_pctlRevTime->SetCtrlValue( (double)m_pSurface->GetIOBoxInterface()->m_iECHO_T/49, FALSE );
			
		}
		break;
	case ROOM:
		{
			m_pSurface->GetIOBoxInterface()->m_byRevType = ROOM;
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevType, (float)ROOM);
			m_cPredelay.SetCurSel(m_pSurface->GetIOBoxInterface()->m_iROOM_D);
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevTime, (float)(m_pSurface->GetIOBoxInterface()->m_iROOM_T/49.0));
			m_pctlRevTime->SetCtrlValue( (double)m_pSurface->GetIOBoxInterface()->m_iROOM_T/49, FALSE );
			
		}
		break;
	case SMALLH:
		{
			m_pSurface->GetIOBoxInterface()->m_byRevType = SMALLH;
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevType, (float)SMALLH);
			m_cPredelay.SetCurSel(m_pSurface->GetIOBoxInterface()->m_iSHALL_D);
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevTime, (float)(m_pSurface->GetIOBoxInterface()->m_iSHALL_T/49.0));
			//m_pctlRevTime->SetCtrlValue( (double)m_pSurface->GetIOBoxInterface()->m_iSHALL_T/49, FALSE );
			
		}
		break;
	case LARGEH:
		{
			m_pSurface->GetIOBoxInterface()->m_byRevType = LARGEH;
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevType, (float)LARGEH);
			m_cPredelay.SetCurSel(m_pSurface->GetIOBoxInterface()->m_iLHALL_D);
			m_pSurface->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_RevTime, (float)(m_pSurface->GetIOBoxInterface()->m_iLHALL_T/49.0));
			//m_pctlRevTime->SetCtrlValue( (double)m_pSurface->GetIOBoxInterface()->m_iLHALL_T/49, FALSE );
			
		}
		break;
	default:
		break;
	}


}
