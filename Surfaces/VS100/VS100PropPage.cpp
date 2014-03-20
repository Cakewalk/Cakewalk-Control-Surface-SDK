/////////////////////////////////////////////////////////////////////////////
// ACTControllerPropPage.cpp : implementation file
//
// This file provides the implemention of the property page (user interface)
// for "ACTController."
//

#include "stdafx.h"

#include "VS100PropPage.h"
#include "VS100.h"
#include "ButtonPropsDlg.h"
#include <algorithm>

#include "HtmlHelp.h"
#include "..\..\CakeControls\OffscreenDC.h"
#include "..\..\CakeControls\NWControl.h"
#include "..\..\CakeControls\NWBitmapButton.h"
#include "..\..\CakeControls\NWDropDownCtrl.h"
#include "..\..\CakeControls\NWBitmapKnob.h"
#include "..\..\CakeControls\NWBitmapButton.h"

/////////////////////////////////////////////////////////////////////////////
//
// CVS100PropPage dialog
//
/////////////////////////////////////////////////////////////////////////////

std::vector<CVS100PropPage::NamedKey>	CVS100PropPage::sm_vKeys;


/////////////////////////////////////////////////////////////////////////////
// Constructors


static CSize s_sizLabel( 73, 15 );


CVS100PropPage::CVS100PropPage(CWnd* pParent /*=NULL*/)
: CDialog(CVS100PropPage::IDD, pParent),
	m_cRef( 1 ),
	m_pSurface( NULL ),
	m_pPageSite( NULL ),
	m_bDirty( FALSE )
	,m_bCommandsDirty( false )
	,m_pfeetSw1(NULL)
	,m_pfeetSw2(NULL)
	,m_pctlACTContextLbl(NULL)
	,m_pctlACTModeLbl(NULL)
	,m_pctlDisableFaderMotors(NULL)
	,m_pctlValueEncoderACTLbl(NULL)
	,m_pctlCompACTLbl(NULL)
{
	m_bInitDone = false;

	if ( sm_vKeys.empty() )
	{
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



	m_uiTimerID = 0;

	//{{AFX_DATA_INIT(CVS100PropPage)
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CVS100PropPage::DoDataExchange(CDataExchange* pDX)
{
	TRACE("CVS100PropPage::DoDataExchange()\n");

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVS100PropPage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CVS100PropPage, CDialog)
	//{{AFX_MSG_MAP(CVS100PropPage)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CVS100PropPage::~CVS100PropPage()
{
}

////////////////////////////////////////////////////////////////////////////////
// IPropertyPage Implementation
//
// You may add to the following IPropertyPage methods as you wish.  However, it may
// be useful to realize that each of these methods has been completed to a necessary level 
// of functionality, and that you are therefore not required to make any modifications 
// to the existing code.

INT_PTR CALLBACK CVS100PropPage::DialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// This proc is only a dummy. It's necessary to to get the size of the dialog
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::SetObjects( ULONG cObjects, LPUNKNOWN* ppUnk )
{
	TRACE("CVS100PropPage::SetObjects()\n");

	if (cObjects == 1)
	{
		// Validate arguments
		if (ppUnk == NULL || *ppUnk == NULL)
			return E_POINTER;

		// Cast this to be our type.  Technically, we should be doing a QI
		// on the ppUnk, and talk via our interface.  But this is a private
		// interface for SONAR, that is guaranteed to be running in process.
		CVS100* const pSurface = dynamic_cast<CVS100*>(*ppUnk);
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

HRESULT CVS100PropPage::Activate( HWND hwndParent, LPCRECT pRect, BOOL fModal )
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

	m_bCommandsDirty = true;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::Move( LPCRECT pRect )
{
	if (!pRect)
		return E_POINTER;
	if (NULL == m_hWnd)
		E_UNEXPECTED;

	MoveWindow( pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::Show( UINT nCmdShow )
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

HRESULT CVS100PropPage::Deactivate()
{
	if (NULL == m_hWnd)
		return E_UNEXPECTED;

	StopTimer();

	DestroyMFCDialog();
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::SetPageSite( LPPROPERTYPAGESITE pPageSite )
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

HRESULT CVS100PropPage::Apply()
{
	// Take no action except clearing the dirty flag.
	// So that the property page may be used in realtime, all user interface
	// changes are immediately passed to the DLL.
	m_bDirty = FALSE;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::IsPageDirty()
{
	return m_bDirty ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::Help( LPCWSTR lpszHelpDir )
{
	// Returning E_NOTIMPL here should be enough to cause the help file
	// specified by GetPageInfo to be used, but it doesn't seem to work

	TCHAR szDLL[_MAX_PATH];

	DWORD dwLen = ::GetModuleFileName(theApp.m_hInstance, szDLL, sizeof(szDLL));

	if (dwLen < 3)
	    return E_FAIL;

	// OK not to use strlcpy here
	::_tcscpy(szDLL + dwLen - 3, _T("chm") );

	::HtmlHelp(m_hWnd, szDLL, HH_DISPLAY_TOPIC, NULL);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::TranslateAccelerator( LPMSG lpMsg )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100PropPage::GetPageInfo( LPPROPPAGEINFO pPageInfo )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	IMalloc* pIMalloc;
	if (FAILED( CoGetMalloc( MEMCTX_TASK, &pIMalloc ) ))
		return E_FAIL;

	pPageInfo->pszTitle = (LPOLESTR)pIMalloc->Alloc( 256 );

	pIMalloc->Release();

	if (!pPageInfo->pszTitle)
		return E_OUTOFMEMORY;

	static const char szTitle[] = "VS100 Controller";
	mbstowcs( pPageInfo->pszTitle, szTitle, strlen( szTitle ) );

	// really measure the bitmap
	CBitmap bmp;
	if ( !bmp.LoadBitmap( IDB_BACKGROUND ) )
		return E_FAIL;

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

void CVS100PropPage::StartTimer()
{
	m_uiTimerID = SetTimer(1, 100, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void CVS100PropPage::StopTimer()
{
	KillTimer(m_uiTimerID);
}

////////////////////////////////////////////////////////////////////////////////

void CVS100PropPage::LoadAllFields()
{
	//TRACE("CVS100PropPage::LoadAllFields()\n");

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

	// Here is where you can load any state variables for the various
	// MFC widgets you may have on your prop page such as Combo Boxes,
	// Radio Buttons, etc.
}


////////////////////////////////////////////////////////////////////////////////
// Class Wizard will add additional methods here.
//

BOOL CVS100PropPage::OnInitDialog() 
{
	BOOL bRet = CDialog::OnInitDialog();

	m_bInitDone = true;

	CString str;

	LoadAllFields();

	StartTimer();

	createControls();
	RecalcLayout();

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

void CVS100PropPage::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == m_uiTimerID)
	{
		if ( m_pSurface->IsFirstLoaded() )
		{
			LoadAllFields();
		}
	}
	RefreshAllControls();

	CDialog::OnTimer(nIDEvent);
}

//-------------------------------------------------------------------------------
void CVS100PropPage::OnPaint()
{
	CPaintDC dcScreen(this); // device context for painting

   CWMemDC dc( &dcScreen );

	// face plate
	if ( !m_bmpFaceplate.m_hObject )
		m_bmpFaceplate.LoadBitmap( IDB_BACKGROUND );

	// select the faceplate
	CDC dcFace;
	VERIFY( dcFace.CreateCompatibleDC( &dc ) );
	dcFace.SelectObject( &m_bmpFaceplate );
	BITMAP bm;
	m_bmpFaceplate.GetBitmap( &bm );

	// blit the faceplate
	dc.BitBlt( 0, 0, bm.bmWidth, bm.bmHeight, &dcFace, 0, 0, SRCCOPY );

	PaintAllControls( &dc );
}


static NWCTRLATTRIB s_attrCmdBtn;
static NWCTRLATTRIB s_attrCmdBtnDark;


void CVS100PropPage::createControls()
{
	s_attrCmdBtn.bModifySelf = FALSE;
	s_attrCmdBtn.bShowValue = TRUE;
   s_attrCmdBtn.nTextOffset = 3;
	s_attrCmdBtn.crBkNormal = RGB( 174,187,210 );
	s_attrCmdBtn.crBkDisable = RGB( 20,20,20 );
	s_attrCmdBtn.crTxtDisable = RGB( 60, 60, 60 );

	s_attrCmdBtnDark = s_attrCmdBtn;
	s_attrCmdBtnDark.crBkNormal = RGB( 20,20,20 );
	s_attrCmdBtnDark.crTxtNormal = RGB( 180,200,200 );
	s_attrCmdBtnDark.crBkDisable = s_attrCmdBtnDark.crBkNormal;

	// command buttons for feetswitch
	m_pfeetSw1 =  new CNWDropDownCtrl( this );
	addControl( m_pfeetSw1 );
	m_pfeetSw1->SetSize( s_sizLabel );
	m_pfeetSw1->SetSharedAttributes( &s_attrCmdBtn );
	m_pfeetSw1->SetParameter( CVS100::CID_Feetswitch1 );

	m_pfeetSw2 =  new CNWDropDownCtrl( this );
	addControl( m_pfeetSw2 );
	m_pfeetSw2->SetSize( s_sizLabel );
	m_pfeetSw2->SetSharedAttributes( &s_attrCmdBtn );
	m_pfeetSw2->SetParameter( CVS100::CID_Feetswitch2 );



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

	// act mode label
	m_pctlACTModeLbl = new CNWLabelButton( this );
	addControl( m_pctlACTModeLbl );
	m_pctlACTModeLbl->SetAttributes( attrAct );

	// checkboxes
	CNWMultiBtn* pbtn = new CNWMultiBtn( this );
	m_pctlDisableFaderMotors = pbtn;
	addControl( m_pctlDisableFaderMotors );
	m_pctlDisableFaderMotors->SetSharedAttributes( &s_attrCmdBtn );
	pbtn->SetArt( IDB_CHECKBOX, CNWMultiBtn::NS );

	createACTControls();
}

void CVS100PropPage::createACTControls()
{
	DWORD dwCnt = 0;;
	m_pSurface->GetDynamicControlCount( &dwCnt );

	DWORD dwActID = ACTKEY_BASE;

	CNWLabelButton* pCmb = NULL;

	// 3 main act knobs
	for ( int ix = 0; ix < 3; ix++ )
	{
      pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( s_sizLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtn );
		pCmb->SetParameter( dwActID++ );
		m_vRotaryACTLabels.push_back(pCmb);
		m_setACTLabel.insert( pCmb );
	}

	// Comp switch
	pCmb = new CNWLabelButton( this );
	m_pctlCompACTLbl = pCmb;
	addControl( pCmb );
	pCmb->SetSize( s_sizLabel );
	pCmb->SetSharedAttributes( &s_attrCmdBtn );
	pCmb->SetParameter( dwActID++ );
	m_setACTLabel.insert( pCmb );;

	// Rotary (R4) push switch
	pCmb = new CNWLabelButton( this );
	m_pctlValueEncoderACTLbl = pCmb;
	addControl( pCmb );
	pCmb->SetSize( s_sizLabel );
	pCmb->SetSharedAttributes( &s_attrCmdBtn );
	pCmb->SetParameter( dwActID++ );
	m_setACTLabel.insert( pCmb );

	// and the Push part of R4
	pCmb = new CNWLabelButton( this );
	m_pctlValueEncoderPushACTLbl = pCmb;
	addControl( pCmb );
	pCmb->SetSize( s_sizLabel );
	pCmb->SetSharedAttributes( &s_attrCmdBtn );
	pCmb->SetParameter( dwActID++ );
	m_setACTLabel.insert( pCmb );

	// Deep act controls (digital mixer controls)

	// pans
	for ( int ixc = 0; ixc < 2; ixc++ )
	{
      CNWLabelButton* pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( s_sizLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtnDark );
		pCmb->SetParameter( dwActID++ );
		m_vDMPanACTLabels.push_back(pCmb);
		m_setACTLabel.insert( pCmb );
	}

	// knobs
	for ( int ixc = 0; ixc < 6; ixc++ )
	{
      CNWLabelButton* pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( s_sizLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtnDark );
		pCmb->SetParameter( dwActID++ );
		m_vDMVolACTLabels.push_back(pCmb);
		m_setACTLabel.insert( pCmb );
	}

	// switches
	for ( int ixc = 0; ixc < 4; ixc++ )
	{
      CNWLabelButton* pCmb = new CNWLabelButton( this );
		addControl( pCmb );
		pCmb->SetSize( s_sizLabel );
		pCmb->SetSharedAttributes( &s_attrCmdBtnDark );
		pCmb->SetParameter( dwActID++ );
		m_vDMSwitchACTLabels.push_back(pCmb);
		m_setACTLabel.insert( pCmb );
	}
}


//////////////////////////////////////////////////////////////////////////////
// Given a command picker, display the list of commands and allow the user
// to re-assign the button to a different command
void CVS100PropPage::setCmdForButton( CNWControl* pSource )
{
	// Determine the current command for this button
	CVS100::MsgIdToBADMap& mapCmds = m_pSurface->GetButtonCommandMap();

	CVS100::MsgIdToBADMapIterator it = mapCmds.find( (CVS100::ControlId)pSource->GetParameter() );
	if ( it == mapCmds.end() )
	{
		ASSERT(0);
		return;
	}
	CVS100::ButtonActionDefinition& bad = it->second;

	CButtonPropsDlg dlg( m_pSurface, bad );
	if ( IDOK == dlg.DoModal() )
		bad = dlg.m_bad;

	m_bCommandsDirty = true;
}



void CVS100PropPage::RecalcLayout(void)
{
	m_pctlACTContextLbl->SetOrigin( CPoint( 216, 60 ) );
	m_pctlACTContextLbl->SetSize( CSize( 163, 17 ) );

	m_pctlACTModeLbl->SetOrigin( CPoint( 378, 60 ) );
	m_pctlACTModeLbl->SetSize( CSize( 40, 17 ) );

	int const xL = 33;
	int const yT = 70;

	CPoint pt;

	// options
	pt.x = 34;
	pt.y = 109;
	placeAndOffset( m_pctlDisableFaderMotors, pt, 0, 0 );

	// feetswitch
	pt.y = 58;
	pt.x = 112;
	placeAndOffset( m_pfeetSw1, pt, 0, 1, 0, 2);
	placeAndOffset( m_pfeetSw2, pt, 1, 0 );

	layoutACT();
}


static int s_lmarg = 28;
static int s_cxStrip = 82;


void CVS100PropPage::layoutACT()
{
	CPoint pt( 110, 166 );

	// comp switch
	m_pctlCompACTLbl->SetOrigin( pt );
	pt.x += s_cxStrip;

	// main rotaries
	for ( size_t i = 0; i < m_vRotaryACTLabels.size(); i++ )
	{
		m_vRotaryACTLabels[i]->SetOrigin( pt );
		pt.x += s_cxStrip;
	}

	// value encoder
	pt.x = 441;
	pt.y = 44;
	placeAndOffset( m_pctlValueEncoderACTLbl, pt, 0, 0 );
	pt.y = 113;
	placeAndOffset( m_pctlValueEncoderPushACTLbl, pt, 0, 0 );

	// pans
	pt.x = s_lmarg;
	pt.y = 246;
	for ( size_t i = 0; i < m_vDMPanACTLabels.size(); i++ )
	{
		m_vDMPanACTLabels[i]->SetOrigin( pt );
		pt.x += s_cxStrip;
	}

	// switches
	pt.x = s_lmarg;
	pt.y = 302;
	for ( size_t i = 0; i < m_vDMSwitchACTLabels.size(); i++ )
	{
		m_vDMSwitchACTLabels[i]->SetOrigin( pt );
		pt.x += s_cxStrip;
	}

	// vols
	pt.x = s_lmarg;
	pt.y = 376;
	for ( size_t i = 0; i < m_vDMVolACTLabels.size(); i++ )
	{
		m_vDMVolACTLabels[i]->SetOrigin( pt );
		pt.x += s_cxStrip;
	}
}

///--------------------------------------------------------------------------------
void	CVS100PropPage::placeAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor, int nXOffset, int nYOffset )
{
	placeControlAndOffset( pC, pt, nXFactor, nYFactor );
	pt.x += nXOffset;
	pt.y += nYOffset;
}



void CVS100PropPage::DoCtrlContextMenu( CNWControl* pSource, UINT nFlags, CMenu& rMenu )
{
	if ( pSource == m_pfeetSw1 || pSource == m_pfeetSw2 )
		setCmdForButton( pSource );
}

void CVS100PropPage::DoCtrlActivate( CNWControl* pSource, UINT nFlags )
{
	if ( pSource == m_pctlDisableFaderMotors )
		m_pSurface->SetMotorEnabled( !m_pSurface->GetMotorEnabled() );
}

//--------------------------------------------------------------------
// request to be refreshed from the App
void CVS100PropPage::RefreshControl( CNWControl* pSource )	// request to be refreshed from the App
{
   DWORD dwIndex = 0;
	const DWORD dwParam = pSource->GetParameter();
	if ( pSource == m_pfeetSw1 || pSource == m_pfeetSw2 )
	{
		if ( m_bCommandsDirty )
		{
			// get the map of button ID to Command ID
			CVS100::MsgIdToBADMap& mapCmds = m_pSurface->GetButtonCommandMap();

			CVS100::MsgIdToBADMapIterator it = mapCmds.find( (CVS100::ControlId)dwParam );
			if ( it == mapCmds.end() )
			{
				ASSERT(0);
				return;
			}

			CVS100::ButtonActionDefinition& bad = it->second;
			if ( bad.eActionType == CVS100::ButtonActionDefinition::AT_Key )
			{
				pSource->SetCtrlValueString( getFriendlyKeystrokeName( bad ) );
			}
			else if ( bad.eActionType == CVS100::ButtonActionDefinition::AT_Command )
			{
				DWORD dwIx = m_pSurface->IndexForCommandID( bad.dwCommandOrKey );

				char sz[32];
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
			else if ( bad.eActionType == CVS100::ButtonActionDefinition::AT_Transport )
				pSource->SetCtrlValueString( getFriendlyTransportName( bad ) );
		}
	}
	else if ( pSource == m_pctlACTContextLbl )
		pSource->SetCtrlValueString( m_pSurface->GetACTContextName() );
	else if ( pSource == m_pctlACTModeLbl )
	{
		CString str;
		if ( !m_pSurface->GetACTMode() )
			str = _T("Off");
		else if ( m_pSurface->GetFullAssignMode() )
			str = _T("Full");
		else
			str = _T("On");
		pSource->SetCtrlValueString( str );
	}
	else if ( pSource == m_pctlDisableFaderMotors )
		m_pctlDisableFaderMotors->SetCtrlValue( m_pSurface->GetMotorEnabled() ? 0 : 1, FALSE );
	else if ( m_setACTLabel.count( pSource ) > 0 )
	{
		DWORD dwID = pSource->GetParameter();
		pSource->SetCtrlValueString( m_pSurface->GetACTParamLabel( dwID ) );
		DWORD dwIx = dwID - ACTKEY_BASE;
		bool bBasicAct = dwIx < 3 || 4 == dwIx || 5 == dwIx;

		// overall act mode on?
		bool bEnable = m_pSurface->GetACTMode();

		// if so and this is a deep act control, also test for full assign mode
		if ( bEnable && !bBasicAct )
			bEnable = m_pSurface->GetFullAssignMode();
		pSource->SetEnable( bEnable );
	}
}



void CVS100PropPage::OnValueChange( CNWControl* pSource )
{
}

void CVS100PropPage::OnFirstPunchIn( CNWControl* pSource )
{
}

void CVS100PropPage::OnFinalPunchOut( CNWControl* pSource )
{
}

void CVS100PropPage::RefreshAllControls()			// cause controls to obtain a new state from the seq.
{
	__super::RefreshAllControls();

	m_bCommandsDirty = false;
}

void CVS100PropPage::OnRequestValueString( CNWControl* pSource, CString* pstr, double* pdVal ) //= NULL )
{
}


bool CVS100PropPage::isCtrlInVector( CNWControl *pCtrl, std::vector< CNWControl * > *pVector, DWORD *pdwIndex )
{
   if ( !pVector || !pCtrl )
      return ( false );

	std::vector<CNWControl*>::iterator it = std::find( pVector->begin(), pVector->end(), pCtrl );
	if ( it != pVector->end() )
	{
		if ( pdwIndex )
			*pdwIndex = (DWORD)( it - pVector->begin() );
		return true;
	}
	
	return false;
}


//-------------------------------------------------------------------
CString CVS100PropPage::getFriendlyTransportName( CVS100::ButtonActionDefinition& bad )
{
	CString str(_T("err") );;
	if( bad.eActionType != CVS100::ButtonActionDefinition::AT_Transport )
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
CString CVS100PropPage::getFriendlyKeystrokeName( CVS100::ButtonActionDefinition& bad )
{
	if( bad.eActionType != CVS100::ButtonActionDefinition::AT_Key )
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
			if ( bad.wModKeys & CVS100::SMK_SHIFT )
				strMod = _T("Shift+");
			if ( bad.wModKeys & CVS100::SMK_CTRL )
				strMod = _T("Ctrl+");
			if ( bad.wModKeys & CVS100::SMK_ALT )
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
		if ( VK_F4 == bad.dwCommandOrKey && CVS100::SMK_CTRL == bad.wModKeys )
			strRet = _T("Close");

		// F4
		if ( VK_F4 == bad.dwCommandOrKey && 0 == bad.wModKeys )
			strRet = _T("Transport");

		// F1
		else if ( VK_F1 == bad.dwCommandOrKey && 0 == bad.wModKeys )
			strRet = _T("Help");

		// Ctrl-tab
		else if ( VK_TAB == bad.dwCommandOrKey && CVS100::SMK_CTRL == bad.wModKeys )
			strRet = _T("Next");

		// Ctrl-Shift-tab
		else if ( VK_TAB == bad.dwCommandOrKey && (CVS100::SMK_CTRL|CVS100::SMK_SHIFT) == bad.wModKeys )
			strRet = _T("Prev");
	}

	return strRet;
}

void CVS100PropPage::OnLButtonDown(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetCapture();
	if ( findControlAndSetActive( point ) )
		m_pActiveControl->HandleLMouseDN( nFlags, point );

	__super::OnLButtonDown(nFlags, point);
}

void CVS100PropPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ReleaseCapture();

	if ( m_pActiveControl )
		m_pActiveControl->HandleLMouseUP( nFlags, point );

	__super::OnLButtonUp(nFlags, point);
}

BOOL CVS100PropPage::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}
