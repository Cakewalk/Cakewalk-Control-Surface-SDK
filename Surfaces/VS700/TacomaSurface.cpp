// TacomaSurface.cpp : Defines the basic functionality of the "ACTController".
//

#include "stdafx.h"

#include "TacomaSurface.h"
#include "TacomaSurfacePropPage.h"
#include "PatchBayDlg.h"
#include "..\..\CakeControls\OffscreenDC.h"
#include "..\..\CakeControls\BitmapCache.h"
#include "MixParam.h"
#include "IOBoxInterface.h"
#include <algorithm>
#include <iso646.h>
#include <stdarg.h>
#include "StringCruncher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Statics
static DWORD s_dwEqIndex[] =
{ 
	4,9,13,17,					// LF Gain,LMF Gain, HMF Gain, HF Gain
	5,10,14,18,					// LF freq, LMF freq, HMf freq, HF freq 
	6,11,15,19,					// LF Q, LMF Q, HMf Q, HF Q
	3,8,12,16,					// LF freq On/off, LMF freq On/off, HMf freq On/off, HF freq On/off
	25,26,23,22,				// HO freq,HP slope, LP Slope, LP freq
	7,1,29,20,					// LF Shelf, Type, Not Used , HF Shelf
	27,28,29,30,				// not used, not used, not used, not used
	24,2,21,0					// HP On/off , Gloss, On/Off, LPF On/off
};

static DWORD s_dwCompIndex[] =
{ 
	2,3,4,6,					// Input,Attack, Release, Output
	10,12,5,7,				// HPF, not used, Ratio, Dry/Wet 
	11,12,13,14,			//not used,not used,not used,not used
	9,12,0,1					// Side Chain, not used, 4k/'76H, On/Off
};

static DWORD s_dwSatIndex[] =
{ 
	3,1,4,12,				// Input, Drive, Output, not used
	9,10,11,12,				//not used,not used,not used,not used
	9,10,11,12,				//not used,not used,not used,not used
	9,10,2,0					// not used, not used, TYPE, On/Off
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//	Main Implementations for the Control Surface Interfaces
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors


CTacomaSurface::CTacomaSurface() :
	CControlSurface()
	,m_eActSectionMode( KSM_FLEXIBLE_PC )
	,m_wACTDisplayRow( 0 )
	,m_wACTPage( 0 )
	,m_e8StripType( MIX_STRIP_TRACK )
	,m_dwMasterOrg( 0 )
	,m_dw8EncoderParam( 0 )
	,m_wModifierKey(0)
	,m_pIOBoxInterface(NULL)
	,m_VUMidiMsg( this, _T("vu sender") )
	,m_ACTRowIndicator( this, _T("Display Row LED") )
	,m_msgSevenSegLed( this, _T("7-seg led") )
	,m_msgMiscLed( this, _T("Misc LED output msg") )
	,m_mfxfPrimary( TF_MBT )
	,m_bShowFaderOnLCD( false )
	,m_tcbParamChange( this, TS_ParamChanged )
	,m_tcbWindowMove( this, TS_WindowMove )
	,m_tctBlink( this, TS_Blink )
	,m_tctKeyRep( this, TS_KeyRep )
	,m_tctRevert( this, TS_Revert )
	,m_tctShuttleRing( this )
	,m_eBlinkState( BS_None )
	,m_bOnMackie(false)
	,m_eTBM( TBM_XRay )
	,m_eJogMode( JM_Standard )
	,m_bSidewaysChanStrip( false )
	,m_cefCurrent( CEF_CROP_START )
	,m_bStopPressed(false)
	,m_bIOMode(false)
	,m_fTbarRcvd(-1.f)
#if DAISY
   ,m_pDisplayEE(NULL)
#endif
	,m_eDisplayMode( SDM_Normal )
	,m_dwBinInsertIndex(0)
	,m_pPluginTree(NULL)
	,m_dwMessageIdSeed(40000)
	,m_bEnableFaderMotors( true )
	,m_bSelectingOrEditing ( FALSE )
	,m_persist(this)
   ,m_bProjectClosed( true )
	,m_nInserting(0)
	,m_fSendDest(-1.f)
	,m_fXRay( 1.f )
	,m_bFlipped( false )
	,m_bSwitchUI( false )
	,m_bFlexiblePC( false )
	,m_bLegacyEq( true )
	,m_bProChanLock( false )
{ 
	::InterlockedIncrement( &g_lComponents );

	memset( m_pEqTypeParams, 0, sizeof ( m_pEqTypeParams ) );
   memset( m_szACTContext, 0, _countof( m_szACTContext ) );
	memset( &m_aaLCDChars[0], 0, _countof( m_aaLCDChars[0] ) );
	memset( &m_aaLCDChars[1], 0, _countof( m_aaLCDChars[1] ) );

	m_ptGrabbed.x = -1;
	m_ptGrabbed.y = -1;
	m_ptCurrent.x = -1;
	m_ptCurrent.y = -1;
	m_ptTarget.x = -1;
	m_ptTarget.y = -1;

	m_vrCCW.dwL = 0x41;
	m_vrCCW.dwH = 0x4f;
	m_vrCW.dwL = 0x1;
	m_vrCW.dwH = 0x0f;

	m_tcbParamChange.SetIsOneShot( TRUE );
	m_tctBlink.SetIsOneShot( FALSE );
	m_tctKeyRep.SetIsOneShot( FALSE );
	m_tctShuttleRing.SetIsOneShot( FALSE );
	m_tctRevert.SetIsOneShot( TRUE );

	m_VUMidiMsg.SetMessageType( CMidiMsg::mtCC );
	m_VUMidiMsg.SetChannel( 1 );
	m_VUMidiMsg.SetIsOutputThin( false );

	m_ACTRowIndicator.SetMessageType( CMidiMsg::mtNote );
	m_ACTRowIndicator.SetChannel( 1 );
	m_ACTRowIndicator.SetIsOutputThin( false );

	m_msgSevenSegLed.SetMessageType( CMidiMsg::mtCC );
	m_msgSevenSegLed.SetChannel( 0 );
	m_msgSevenSegLed.SetIsOutputThin( false );

	m_msgMiscLed.SetMessageType( CMidiMsg::mtNote );
	m_msgMiscLed.SetIsOutputThin( false );

	::memset( (void*)m_aPluginChildIndex, 0, sizeof(m_aPluginChildIndex) );

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CTacomaSurface::~CTacomaSurface() 
{ 
	::InterlockedDecrement( &g_lComponents );
   m_pIOBoxInterface = NULL;
   m_pPluginTree = NULL;
#if DAISY
   if ( m_pDisplayEE )
      delete ( m_pDisplayEE );
#endif
}


/////////////////////////////////////////////////////////////////////////////
// This is the IControlSurface way of obtaining the MIDI mask.  Older versions 
// of SONAR will only call this instead of IControlSurface3::GetNoEchoStatusMessages(). 
// This function has much less granularity because it only returns a bitmask for the MIDI
// channels in use.  However for a dedicated control surface this is probably just fine.
HRESULT CTacomaSurface::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	// Block the whole port:  all channels and all sysex
	*pwMask = 0xffff;
	*pbNoEchoSysx = TRUE;

	return S_OK;
}


///////////////////////////////////////////////////////////////////////
// IControlSurface3
// Return a list of MIDI status words that we listen to.  the host will
// ignore these and only pass them to us.  Implement this for surfaces that
// have a MIDI keyboard AND controller widgets using the same port.  This is
// required so that the host can be told to intercept *musical* messages from
// the controller but ignore all surface messages.
HRESULT CTacomaSurface::GetNoEchoStatusMessages(WORD** ppwStatus, DWORD* pdwCount )
{
	*pdwCount =  0;
	return S_OK;
}


HRESULT CTacomaSurface::GetStripRangeCount(DWORD *pdwCount )
{
	*pdwCount = 2;
	return S_OK;
}

HRESULT CTacomaSurface::GetStripRange(DWORD dwIxRange, DWORD * pdwLow, DWORD * pdwHi, SONAR_MIXER_STRIP * pType )
{
	if ( 0 == dwIxRange )
	{
      *pdwLow = getCurrentBankOffset();
		*pdwHi = *pdwLow + 7;
		*pType = m_e8StripType;
	}
	else if ( 1 == dwIxRange )
	{
		*pdwLow = m_dwMasterOrg;
		*pdwHi = *pdwLow + 0;
		*pType = MIX_STRIP_BUS;
	}
	else
		return E_INVALIDARG;

	return S_OK;
}



//////////////////////////////////////////////////////////////////
// Set the base strip for a given strip type.
// Note: this is exposed to the host via ISurfaceParamMapping
HRESULT CTacomaSurface::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
{
	bool bChange = false;

	if ( m_e8StripType == mixerStrip )
	{
		bChange = (getBankOffset(mixerStrip) != dwLowStrip);
		setBankOffset(mixerStrip, dwLowStrip);
	}
	else if ( MIX_STRIP_BUS == mixerStrip )
	{
		bChange = (m_dwMasterOrg != dwLowStrip);
		m_dwMasterOrg = dwLowStrip;
	}
	
	if ( bChange )
	{
		updateParamBindings();
		SetHostContextSwitch();
		RequestStatusQuery();
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// Return the count of ACT parameters you are maintaining

HRESULT CTacomaSurface::GetDynamicControlCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	CSFKCriticalSectionAuto csa( m_cs );

	*pdwCount = ACT_PAGE_NUM *	TOTAL_DYN_COUNT;

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// return info about the nth ACT parameter you are maintaining
HRESULT CTacomaSurface::GetDynamicControlInfo(DWORD ix, DWORD* pdwKey, SURFACE_CONTROL_TYPE *pType)
{
	if ( pdwKey )
		*pdwKey = ACTKEY_BASE + ix;

	if ( pType )
	{
		DWORD dwInPage = ix % TOTAL_DYN_COUNT;
		if ( dwInPage < ROTARY_DYN_INDEX )
			*pType = SCT_ROTARY;
		else if ( dwInPage < TBAR_DYN_INDEX )
			*pType = SCT_SWITCH;
		else
			*pType = SCT_SLIDER;
	}

	TRACE( _T("GetDynamicControlInfo for %d is 0x%X, type = %d\n"), ix, *pdwKey, (int) *pType );

	return S_OK;
}


//////////////////////////////////////////////////////////////////
// Host is notifying us of a ACT Learn state change.
HRESULT CTacomaSurface::SetLearnState( BOOL bLearning )
{
	// You should tell all CMixParam objects that are for ACT that
	// they are in Learn mode now.  This switches off any capture modes
	// while learning

	// call base class to set the global bit
	return CControlSurface::SetLearnState( bLearning );
}


// Public access for the prop page to get ACT param labels
CString CTacomaSurface::GetACTParamLabel( DWORD id )
{
	char	sz[16];
	sz[0] = '\0';
	DWORD cb = sizeof(sz);
	m_pSonarMixer->GetMixParamLabel( MIX_STRIP_ANY, id, MIX_PARAM_DYN_MAP, m_dwSurfaceID, sz, &cb );

	return CString(sz);
}

CString CTacomaSurface::GetACTContextName()
{
   char szName[32];
   DWORD dwCountof = _countof( szName );
   szName[0] = '\0';
   m_pSonarParamMapping->GetMapName( m_dwSurfaceID, (LPSTR) &szName, &dwCountof );

	return CString(szName);
}



//////////////////////////////////////////////////////////////////////
// Create a mix param on the heap and put it in the every mix param set
CMixParam* CTacomaSurface::createMixParam()
{
	CMixParam* pParam = new CMixParam;
	pParam->SetInterfaces( m_pSonarMixer, m_pSonarTransport, m_dwSurfaceID );
	m_setEveryMixparam.insert( pParam );
	return pParam;
}


//---------------------------------------------------------------
CMidiMsg* CTacomaSurface::createMidiMsg( LPCTSTR sz, //= 0, 
													 DWORD dwId ) //= (DWORD)-1 )
{
	if ( (DWORD)-1 == dwId )
		dwId = m_dwMessageIdSeed++;	// unspecified ID, just increment the serial number
	
	CSFKCriticalSectionAuto csa( m_cs );

	// check for duplicate ID
	if ( m_mapControlMsgs.count( (ControlId)dwId ) > 0 )
	{
		ASSERT(0);
		return NULL;
	}

	CMidiMsg* pmsg = new CMidiMsg( this, sz, dwId );

	// add to the control map
	m_mapControlMsgs[(ControlId)dwId] = pmsg;

	// and to our memory management set
	m_setEveryMidiMsg.insert( pmsg );

	return pmsg;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Build all MIDI and Parameter Binding tables
HRESULT	CTacomaSurface::buildBindings()
{
	buildStripBindings();
	buildMiscBindings();
	buildCommandBindings();
	buildKeyboardBindings();
	buildTransportBindings();
	buildActSectionBindings();
	
	updateParamBindings();

	return CControlSurface::buildBindings();
}


//////////////////////////////////////////////////////////////////////
// Build the bindings for the Act/Eq/Send section
HRESULT CTacomaSurface::buildActSectionBindings()
{
	PMBINDING pmb;

   const float fEqFreqDefaults[4] = { 0.24f, 0.4f, 0.65f, 0.84f };

	// for each control, we create 3 bindings:  ACT, Send, EQ
	   for ( DWORD col = 0; col < 4; col++ )
	   {
		   pmb.pMsgOut = NULL;

		   /////////////////////////////////////////////////////////////
		   // top knob ( Gain, send level, act 0-3 )
		   pmb.nRefreshMod = RM_EVEN;

		   CMidiMsg* pmsg = createMidiMsg(_T("eqG/sndG,Act0-3"), BID_EncoderActR0C0 + col);
		   pmsg->SetMessageType( CMidiMsg::mtCC );
		   pmsg->SetChannel( 0 );
		   pmsg->SetCCNum( 0x18 + col );
		   pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
		   m_pMidiInputRouter->Add( pmsg );

		   // Push Switch input message
		   CMidiMsg* pmsgP = createMidiMsg( _T("eqG/sndG,Act0-3 push"), BID_EncoderPushActR0C0 + col );
		   pmsgP->SetMessageType( CMidiMsg::mtNote );
		   pmsgP->SetChannel( 1 );
		   pmsgP->SetNoteNum( 0x0 + col );
		   pmsgP->SetIsTrigger( true, 0x7f );
		   m_pMidiInputRouter->Add( pmsgP );
				   
		   // eq binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( false );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(0 + col), .5f ); // gain starts at 0
		   m_mapEQControls[pmsg] = pmb;
		   m_mapEQControls[pmsgP] = pmb;
		   m_setSonitusEQ.insert(pmb.pParam);
			
		   // send binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetAlwaysChanging( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SEND_VOL, col, 101.f/127.f );
		   pmb.pParam->SetCrunchSize( 7 );
		   m_mapSendControls[pmsg] = pmb;
		   m_mapSendControls[pmsgP] = pmb;

		   // ACT Binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + col + 0,	// 1st row
			   MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		   m_mapACTControls[pmsg] = pmb;
		   m_mapACTControls[pmsgP] = pmb;

			// ProChannel binding
	      // PC Comp
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetCompParams( MIX_STRIP_TRACK, 0, (WORD)(0 + s_dwCompIndex[col]), .5f ); // Input starts at 0
		   m_mapPCCompControls[pmsg] = pmb;
		   m_mapPCCompControls[pmsgP] = pmb;
		

		   // PC EQ
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(0 + s_dwEqIndex[col]), .5f ); // gain starts at 0
		   m_mapPCEQControls[pmsg] = pmb;
		   m_mapPCEQControls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   // PC EQ P2
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)( s_dwEqIndex[ 16 + col]), .5f ); // High Pass Frequency starts at 16
		   m_mapPCEQP2Controls[pmsg] = pmb;
		   m_mapPCEQP2Controls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   // PC Sat
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetTubeParams( MIX_STRIP_TRACK, 0, (WORD)(0 + s_dwSatIndex[col]), .5f ); //  starts at 0
		   m_mapPCSatControls[pmsg] = pmb;
		   m_mapPCSatControls[pmsgP] = pmb;
	
		   /////////////////////////////////////////////////////////////
		   // 2nd from top knob ( Freq, Send pan, act 4-7 )
		   pmb.nRefreshMod = RM_ODD;

		   pmsg = createMidiMsg( _T("eqF,sndP,Act4-7"), BID_EncoderActR1C0 + col );
		   pmsg->SetMessageType( CMidiMsg::mtCC );
		   pmsg->SetChannel( 0 );
		   pmsg->SetCCNum( 0x1c + col );
		   pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
		   m_pMidiInputRouter->Add( pmsg );

		   // Push Switch input message
		   pmsgP = createMidiMsg( _T("eqF,sndP,Act4-7 push"), BID_EncoderPushActR1C0 + col );
		   pmsgP->SetMessageType( CMidiMsg::mtNote );
		   pmsgP->SetChannel( 1 );
		   pmsgP->SetNoteNum( 0x04 + col );
		   pmsgP->SetIsTrigger( true, 0x7f );
		   m_pMidiInputRouter->Add( pmsgP );

		   // eq binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( false );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(4 + col), fEqFreqDefaults[ col ] ); // freq starts at 4
		   m_mapEQControls[pmsg] = pmb;
		   m_mapEQControls[pmsgP] = pmb;
			m_setSonitusEQ.insert(pmb.pParam);

		   // send binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetAlwaysChanging( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SEND_PAN, col, 0.5f );
		   pmb.pParam->SetCrunchSize( 7 );
		   m_mapSendControls[pmsg] = pmb;
		   m_mapSendControls[pmsgP] = pmb;

		   // ACT Binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + col + 4,	// 2nd row
			   MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		   m_mapACTControls[pmsg] = pmb;
		   m_mapACTControls[pmsgP] = pmb;

			// ProChannel binding
	      // PC Comp
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetCompParams( MIX_STRIP_TRACK, 0, (WORD)( s_dwCompIndex[ 4 + col]), .5f ); 
		   m_mapPCCompControls[pmsg] = pmb;
		   m_mapPCCompControls[pmsgP] = pmb;

		   // PC EQ
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwEqIndex[4 + col]), fEqFreqDefaults[ col ] ); // freq starts at 4
		   m_mapPCEQControls[pmsg] = pmb;
		   m_mapPCEQControls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   // PC EQ P2
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwEqIndex[20 + col])); // LF Shelf starts at 20
		   m_mapPCEQP2Controls[pmsg] = pmb;
		   m_mapPCEQP2Controls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   
		   // PC Sat
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetTubeParams( MIX_STRIP_TRACK, 0, (WORD)(0 + s_dwSatIndex[4 + col]), .5f ); //  starts at 0
		   m_mapPCSatControls[pmsg] = pmb;
		   m_mapPCSatControls[pmsgP] = pmb;
		   
			
		   /////////////////////////////////////////////////////////////
		   // 3rd from top knob ( Q, Pre/Post, act 8-11 )
		   pmb.nRefreshMod = RM_ODD;

		   pmsg = createMidiMsg( _T("eqQ,sndDest/Act8-11"), BID_EncoderActR2C0 + col );
		   pmsg->SetMessageType( CMidiMsg::mtCC );
		   pmsg->SetChannel( 0 );
		   pmsg->SetCCNum( 0x40 + col );
		   pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
		   m_pMidiInputRouter->Add( pmsg );

		   // Push Switch input message
		   pmsgP = createMidiMsg( _T("eqQ,sndDest/Act8-11 push"), BID_EncoderPushActR2C0 + col );
		   pmsgP->SetMessageType( CMidiMsg::mtNote );
		   pmsgP->SetChannel( 1 );
		   pmsgP->SetNoteNum( 0x08 + col );
		   pmsgP->SetIsTrigger( true, 0x7f );
		   m_pMidiInputRouter->Add( pmsgP );


		   // eq binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( false );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(8 + col), 0.2f ); // Q starts at 8
		   m_mapEQControls[pmsg] = pmb;
		   m_mapEQControls[pmsgP] = pmb;
			m_setSonitusEQ.insert(pmb.pParam);

		   // send binding Pre/Post
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetAlwaysChanging( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SEND_PREPOST, col, NO_DEFAULT, TRUE );
		   pmb.pParam->SetCrunchSize( 7 );
		   m_mapSendControls[pmsg] = pmb;
		   m_mapSendControls[pmsgP] = pmb;

		   // ACT Binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + col + 8,	// 3rd row
			   MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		   m_mapACTControls[pmsg] = pmb;
		   m_mapACTControls[pmsgP] = pmb;

			// ProChannel binding
	      // PC Comp
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetCompParams( MIX_STRIP_TRACK, 0, (WORD)( s_dwCompIndex[8 + col]), .5f ); 
		   m_mapPCCompControls[pmsg] = pmb;
		   m_mapPCCompControls[pmsgP] = pmb;
	
		   // PC EQ
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)( s_dwEqIndex[8 + col]), 0.2f ); // Q starts at 8
		   m_mapPCEQControls[pmsg] = pmb;
		   m_mapPCEQControls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

			// PC EQ P2
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwEqIndex[24 + col])); // Blank Row at 24
		   m_mapPCEQP2Controls[pmsg] = pmb;
		   m_mapPCEQP2Controls[pmsgP] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   
		   // PC Sat
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetTubeParams( MIX_STRIP_TRACK, 0, (WORD)(0 + s_dwSatIndex[8 + col]), .5f ); //  starts at 0
		   m_mapPCSatControls[pmsg] = pmb;
		   m_mapPCSatControls[pmsgP] = pmb;
		   
		   /////////////////////////////////////////////////////////////
		   // Switches ( shape, Pre-post, act 12-15 )
		   pmb.nRefreshMod = RM_EVEN;

		   pmsg = createMidiMsg( _T("eqE,sndE,act sw"));
		   pmsg->SetMessageType( CMidiMsg::mtNote );
		   pmsg->SetChannel( 1 );
		   pmsg->SetNoteNum( 0x0c + col );
		   pmsg->SetIsTrigger( true, 0x7f );
		   pmb.pMsgOut = pmsg;	// has an LED so give it an output message (same as input)
		   m_pMidiInputRouter->Add( pmsg );

		   // eq binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( false );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(16 + col) ); // Enable starts at 16 
		   m_mapEQControls[pmsg] = pmb;
			m_setSonitusEQ.insert(pmb.pParam);

		   // ProChannel binding
	      // PC Comp
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetCompParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwCompIndex[12 + col]) ); 
		   m_mapPCCompControls[pmsg] = pmb;
		
		   // PC EQ
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwEqIndex[12 + col]) ); // Enable starts at index 12
		   m_mapPCEQControls[pmsg] = pmb;
			m_setGlossEQ.insert(pmb.pParam);

		   // PC EQ P2
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwEqIndex[28 + col]) ); //High Pass Freq On/Off starts at 28
		   m_mapPCEQP2Controls[pmsg] = pmb;
			m_setGlossEQ.insert(pmb.pParam);
		
		   // PC Sat
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetDisplayName( true );
		   pmb.pParam->SetTubeParams( MIX_STRIP_TRACK, 0, (WORD)(s_dwSatIndex[ 12 + col]) );
		   m_mapPCSatControls[pmsg] = pmb;
		   
		   // send binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetAlwaysChanging( true );
		   pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SEND_ENABLE, col );
		   pmb.pParam->SetCrunchSize( 7 );
		   m_mapSendControls[pmsg] = pmb;

		   // ACT Binding
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + col + 12,	// 4th row
			   MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		   m_mapACTControls[pmsg] = pmb;

		   // type starts at 12
		   pmb.pParam = createMixParam();
		   pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		   pmb.pParam->SetEqParams( MIX_STRIP_TRACK, 0, (WORD)(12 + col), .5f ); // type starts at 12
		   m_pEqTypeParams[ col ] = pmb.pParam;

	   }

	return S_OK;
}


//--------------------------------------------------------
// Create the Midi message and Parameter objects for
// all per-strip parameters.  
HRESULT CTacomaSurface::buildStripBindings()
{
	// mackie faders
	for ( DWORD i = 0; i < 8; i++ )
	{
		PMBINDING pmb;
		pmb.nRefreshMod = RM_EVERY;	// faders have motors - update them as often as possible

		// create a mix param for vol. This will have TWO Input messages associated
		// with it (fader, touch switch), and one output message (motor).
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_VOL, 0, 101.0f/127 );
		pmb.pParam->SetStripPhysicalIndex( i );
		pmb.pParam->SetThrottle( false );

		// Fader Input Messages
		CMidiMsg* pmsg = createMidiMsg( _T("fader"), BID_Fader0 + i);
		pmsg->SetMessageType( CMidiMsg::mtWheel );
		pmsg->SetChannel( i );
		m_pMidiInputRouter->Add( pmsg );

		// Fader Motor output messages (use input)
		pmb.pMsgOut = pmsg;

		m_mapStripMsgs[pmsg] = pmb;

		// Fader touch switch
		pmb.pMsgOut = NULL;	// no output
		pmb.nRefreshMod = RM_NEVER;
		CMidiMsg* pmsgT = createMidiMsg( _T("fader touch"), BID_FaderTouch0 + i);
		pmsgT->SetMessageType( CMidiMsg::mtNote );
		pmsgT->SetChannel(0);	// all 0
		pmsgT->SetNoteNum( 0x68 + i );
		m_pMidiInputRouter->Add( pmsgT );

		m_mapStripMsgs[pmsgT] = pmb;

		// the rest of the params can be interlaced
		pmb.nRefreshMod = (i % 2) == 0 ? RM_EVEN : RM_ODD;

		////////////////////////////////////////////
		// Encoder
		// create a mix param for pan.
		pmb.pMsgOut = NULL;
		pmb.pParam = createMixParam();
		pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_PAN, 0, 0.5f );
		pmb.pParam->SetStripPhysicalIndex( i );

		// Encoder Input Messages
		CMidiMsg* pmsgR = createMidiMsg( _T("encoder"), BID_Encoder0 + i );
		pmsgR->SetMessageType( CMidiMsg::mtCC );
		pmsgR->SetChannel( 0 );
		pmsgR->SetCCNum( 0x10 + i );

		pmsgR->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
		m_pMidiInputRouter->Add( pmsgR );

		// Main strip binding
		m_mapStripMsgs[pmsgR] = pmb;

		// Push Switch input message
		CMidiMsg* pmsgP = createMidiMsg( _T("encoder push"), BID_EncoderPush0 + i );
		pmsgP->SetMessageType( CMidiMsg::mtNote );
		pmsgP->SetChannel( 0 );
		pmsgP->SetNoteNum( 0x20 + i );
		pmsgP->SetIsTrigger( true, 0x7f );
		m_pMidiInputRouter->Add( pmsgP );

		m_mapStripMsgs[pmsgP] = pmb;	// connect this also to the pan parameter


		////////////////////////////////////////////
		// MUTES
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_MUTE, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );

		pmsg = createMidiMsg( _T("mute"), BID_Mute0 + i);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x10 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)
		m_mapStripMsgs[pmsg] = pmb;

		// Input Monitoring
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_INPUT_ECHO, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );
		m_mapShiftMsgs[ pmsg ] = pmb;

		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_AUTOMATED_MUTE, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );
		m_mapAltMsgs[ pmsg ] = pmb;

		////////////////////////////////////////////
		// SOLO
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_SOLO, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );

		pmsg = createMidiMsg( _T("solo"), BID_Solo0 + i);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x08 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

		m_mapStripMsgs[pmsg] = pmb;

		// EXCLUSIVE SOLO OVERRIDE (MUTE DEFEAT)
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_MUTE_DEFEAT, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );
		m_mapShiftMsgs[pmsg] = pmb;

		////////////////////////////////////////////
		// ARM
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_RECORD_ARM, 0 );
		pmb.pParam->SetStripPhysicalIndex( i );

		pmsg = createMidiMsg( _T("arm"), BID_Arm0 + i);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x00 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

		m_mapStripMsgs[pmsg] = pmb;
	}

	// and lastly make the master fader
	PMBINDING pmb;
	pmb.nRefreshMod = RM_EVERY;	// faders have motors - update them as often as possible

	// create a mix param for vol. This will have TWO Input messages associated
	// with it (fader, touch switch), and one output message (motor).
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_BUS, m_dwMasterOrg, MIX_PARAM_VOL, 0, 101.0f/127 );
	pmb.pParam->SetStripPhysicalIndex( 8 );	// the "9th" fader for locking purposes
	pmb.pParam->SetThrottle( false );

	// Fader Input Messages
	CMidiMsg* pmsg = createMidiMsg( _T("mstr fader"), BID_FaderMstr );
	pmsg->SetMessageType( CMidiMsg::mtWheel );
	pmsg->SetChannel( 8 );
	m_pMidiInputRouter->Add( pmsg );

	// Fader Motor output messages (use input)
	pmb.pMsgOut = pmsg;

	m_mapMiscMsgs[pmsg] = pmb;

	// Fader touch switch
	pmb.pMsgOut = NULL;	// no output
	CMidiMsg* pmsgT = createMidiMsg( _T("fader touch"), BID_FaderTouchMstr);
	pmsgT->SetMessageType( CMidiMsg::mtNote );
	pmsgT->SetChannel(1);
	pmsgT->SetNoteNum( 0x17 );
	m_pMidiInputRouter->Add( pmsgT );

	m_mapMiscMsgs[pmsgT] = pmb;

	return S_OK;
}

////////////////////////////////////////////////////////////////////
// Set the 8 strip type to Tracks / Bus / Mains
void CTacomaSurface::Set8StripType( SONAR_MIXER_STRIP eStrip )
{
	m_e8StripType = eStrip;
	updateParamBindings();

	onLayersMode( false );
	endInsertPlugin();

	initLEDs();

	RequestStatusQuery();
	SetHostContextSwitch();

	PSEncoderParams pEnc = GetEncoderListParam( 0, MIX_STRIP_ANY, getModeForRotary() );
	if ( pEnc )
		AssignRotaries( pEnc->mixParam, pEnc->dwParam, pEnc->dwIndex );

	initLCDs();
}

/////////////////////////////////////////////////////////////////
void CTacomaSurface::AssignRotaries( SONAR_MIXER_PARAM smp, DWORD dwParam, DWORD dwIndex )
{
	if ( m_bFlipped ) // nop if flipped
		return;

	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;
		const DWORD dwId = pmsg->GetId();

		if ( (dwId >= BID_Encoder0 && dwId <= BID_Encoder7) ||
			  (dwId >= BID_EncoderPush0 && dwId <= BID_EncoderPush7) )
		{
			// set to the new param
			SONAR_MIXER_STRIP const eStrip = pmb.pParam->GetMixerStrip();
			float fDefault = .5f;
			if ( smp == MIX_PARAM_SEND_VOL || smp == MIX_PARAM_VOL )
				fDefault = 101.f / 127.f;
			pmb.pParam->SetParams( eStrip, pmb.pParam->GetStripPhysicalIndex(), smp, dwParam, fDefault, TRUE );
			pmb.pParam->SetCrunchSize( 7 );

			// set the underlying MIDI msg objects' dwCurrent value correctly
			float fVal = -1.f;
			const HRESULT hrChange = pmb.pParam->GetVal( &fVal );
			if ( SUCCEEDED( hrChange ) )
				pmsg->SetVal( fVal );

			pmb.pParam->ResetHistory();
		}
	}

	m_dw8EncoderParam = dwIndex;

	// and refresh all displays
	updateParamStateOnLCD();
}

///////////////////////////////////////////////////////////////
// Flip button has been pressed
void CTacomaSurface::OnFlip()
{
	if ( m_bIOMode )
	{
		m_bFlipped = !m_bFlipped;
		return;
	}

	// if shift key held, we only flip the LCD parameter
	if (m_wModifierKey & SMK_SHIFT)
	{
		m_bShowFaderOnLCD = !m_bShowFaderOnLCD;
	}
	else
	{
		m_bFlipped = !m_bFlipped;

		PSEncoderParams const pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, getModeForRotary() );
		const SONAR_MIXER_PARAM mixerParam = pEnc ? pEnc->mixParam : MIX_PARAM_PAN;
		const DWORD dwParamNum = pEnc ? pEnc->dwParam : 0;

		for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
		{
			PMBINDING& pmb = it->second;
			CMidiMsg* pmsg = it->first;

			SONAR_MIXER_STRIP const eStrip = pmb.pParam->GetMixerStrip();
			const DWORD dwId = pmsg->GetId();
			const SONAR_MIXER_PARAM eParamCur = pmb.pParam->GetMixerParam();

			// if it's a fader, or fader touch switch, bind its param to either VOL or the encoder param
			if ( BID_IN( dwId, BID_Fader0, BID_Fader7 ) )
			{
				SONAR_MIXER_PARAM eParamNew = mixerParam;
				DWORD dwParamNumNew = dwParamNum;
				if ( eParamCur == mixerParam )
				{
					eParamNew = MIX_PARAM_VOL;
					dwParamNumNew = 0;
				}

				const float fDefault = eParamNew == MIX_PARAM_VOL ? 101.f / 127.f : .5f;
				// swap
				pmb.pParam->SetParams( eStrip, pmb.pParam->GetStripNum(), eParamNew, dwParamNumNew, fDefault );
				pmb.pParam->ResetHistory();
			}

			// else if it's an encoder or an encoder push switch, bind its parameter to either VOL or the encoder param
			else if ( BID_IN(dwId, BID_Encoder0, BID_Encoder7) ) //|| BID_IN(dwId, BID_EncoderPush0, BID_EncoderPush7) )
			{
				SONAR_MIXER_PARAM eParamNew = MIX_PARAM_VOL;
				DWORD dwParamNumNew = 0;
				if ( eParamCur == MIX_PARAM_VOL )
				{
					eParamNew = mixerParam;
					dwParamNumNew = dwParamNum;
				}

				const float fDefault = eParamNew == MIX_PARAM_VOL ? 101.f / 127.f : .5f;
				// swap
				pmb.pParam->SetParams( eStrip, pmb.pParam->GetStripNum(), eParamNew, dwParamNumNew, fDefault, TRUE );
				pmb.pParam->ResetHistory();

				// set the underlying MIDI msg objects' dwCurrent value correctly
				float fVal = -1.f;
				const HRESULT hrChange = pmb.pParam->GetVal( &fVal );
				pmsg->SetVal( fVal );
			}
		}

		// force an update of the faders
		onRefreshSurface( 0, 0 );
	}

	updateParamStateOnLCD();
}

/////////////////////////////////////////////////////////////////////////////////////
void	CTacomaSurface::updateParamStateOnLCD()
{
	CMixParam* pParamForName = NULL;

	if ( m_bShowFaderOnLCD )
		pParamForName = paramFromId( BID_Fader0 );
	else
		pParamForName = paramFromId( BID_Encoder0 );

	initLCDs();

	if ( m_bIOMode )
		return;

	// now what to show in the 13th LCD...
	switch( m_eDisplayMode )
	{
	case SDM_ChannelBranch:
		showText( "Channel", 12, 0 );
		showText( "Branch", 12, 1 );
		break;
	case SDM_Markers:
		showText( "Markers", 12, 0 );
		showText( "Push", 12, 1 );
		break;
	case SDM_Layers:
		showText( "Layers", 12, 0 );
		break;

	default:
		{
		   char szName[8];
		   *szName = '\0';
		   DWORD dwLen = _countof(szName);
		   if ( pParamForName->GetMixerParam() == MIX_PARAM_SEND_VOL )
			   sprintf( szName, "SendVol" );
			else if ( pParamForName->GetMixerParam() == MIX_PARAM_SEND_PAN )
			   sprintf( szName, "SendPan" );
		   else if ( pParamForName )
			   pParamForName->GetCrunchedParamLabel( szName, 7 );

		   if ( TBM_ACT != m_eTBM )
		   {
			   // update the LCD to show current Parameter type
			   showText( m_bShowFaderOnLCD ? "Fader" : "Encoder", 12, 0 );
			   showText( szName, 12, 1 );
		   }
		   else
		   {
			   // ACT t-bar mode
			   *szName = '\0';
			   pParamForName = paramFromId( BID_Tbar );
			   if ( pParamForName )
				   pParamForName->GetParamLabel( szName, &dwLen );

			   char sz[64];
			   ::sprintf( sz, "%s", szName );
			   // truncate
			   sz[8] = '\0';
			   showText( sz, 12, 0 );
		   }
		}
		break;
	}
}




///////////////////////////////////////////////////////////////
// Build midi messages for various misc function buttons on the surface
HRESULT CTacomaSurface::buildMiscBindings()
{
	////////////////////////////////////////
	// Track Bank switching
	CMidiMsg* pmsg = createMidiMsg( _T("bank R"), BID_BankR );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2e );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("bank L"), BID_BankL );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2f );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Rude Mute"), BID_RudeMute );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x33 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Rude Solo"), BID_RudeSolo );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x4a );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Rude Arm"), BID_RudeArm );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x4b );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("IO Control"), BID_IOCtrl );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x23 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Flip button
	pmsg = createMidiMsg( _T("Flip"), BID_Flip );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x32 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Assign button (step through rotary assignments)
	pmsg = createMidiMsg( _T("Rot Assign"), BID_RotaryAssign );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2a );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );


	// EQ Button
	pmsg = createMidiMsg( _T("Eq"), BID_Eq );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2c );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Send Button
	pmsg = createMidiMsg( _T("Send"), BID_Send );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2d );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// ACT Button
	pmsg = createMidiMsg( _T("ACT"), BID_Act );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2b );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// ACT/EQ/Send display button
	pmsg = createMidiMsg( _T("Display"), BID_ACTDisplay );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x34 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Act/Eq/Send Page L/R buttons
	pmsg = createMidiMsg( _T("Page L"), BID_PageL );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x28 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Page R"), BID_PageR );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x29 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Command Modifier
	pmsg = createMidiMsg( _T("Shift"), BID_Shift );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x46 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Ctrl"), BID_Control );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x48 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Alt"), BID_Alt );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x49 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Command"), BID_Command );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x47 );
	addControlMsg( pmsg );

	// Strip Sel buttons
	for ( DWORD i = 0; i < 8; i++ )
	{
		CString str;
		str.Format( _T("StripSel %d"), i );
		pmsg = createMidiMsg( str, BID_Sel0 + i );
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);
		pmsg->SetNoteNum( 0x18 + i );
		pmsg->SetIsTrigger( true, 0x7f );
		pmsg->SetIsOutputThin( true ); // don't update all the time
		addControlMsg( pmsg );
	}

	pmsg = createMidiMsg( _T("Marker Ins"), BID_MarkerIns );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5a );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Left Marker"), BID_LeftMarker );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x58 );
	//pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Right Marker"), BID_RightMarker );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x54 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("SetLoopPunch"), BID_SetLoopPunch );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x55 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Punch Enab"), BID_PunchOn );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x57 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Loop Enab"), BID_LoopOn );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x56 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Snap"), BID_Snap );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x59 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );


	pmsg = createMidiMsg( _T("Scroll"), BID_Scroll );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x65 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("DataSelect"), BID_DataSelect );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x64 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("DataEdit"), BID_DataEdit );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x53 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("TimeCodeMode"), BID_TimeCodeMode );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x35 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// jog / shuttle
	pmsg = createMidiMsg( _T("Jog"), BID_Jog );
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x3c );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Shuttle"), BID_Shuttle );
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x44 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Tracks"), BID_Tracks );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 0 );
	pmsg->SetNoteNum( 0x4c);
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Buses"), BID_Buses );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 0 );
	pmsg->SetNoteNum( 0x4d );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Masters"), BID_Masters );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 0 );
	pmsg->SetNoteNum( 0x4e );
	addControlMsg( pmsg );

	// feetswitches
	ButtonActionDefinition bad;
	pmsg = createMidiMsg( _T("Footswitch1"), BID_Footswitch1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x66 );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Transport;
	bad.transportState = TRANSPORT_STATE_PLAY;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("Footswitch2"), BID_Footswitch2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x67 );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Transport;
	bad.transportState = TRANSPORT_STATE_REC;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	// automation 
	pmsg = createMidiMsg( _T("Global Write"), BID_Automation );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x30 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Shapshot"), BID_Snapshot );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x31 );
	addControlMsg( pmsg );

	// the Joystick.
	// here we just create messages and parameters but they don't fit into a binding map
	// Joystick

	// Joystick is special - we are controlled by two midi messages: one for x and one for y.
	// However the host expects us to set Angle and Focus.
	// Therefore, we can't use our automatic binding patterns and just have to handle these
	// specifically

	m_Surround.pmsgJoyX = createMidiMsg( _T("JoyX"), BID_JoyX );
	m_Surround.pmsgJoyX->SetMessageType( CMidiMsg::mtWheel7Bit );
	m_Surround.pmsgJoyX->SetChannel( 0xb );
	m_pMidiInputRouter->Add( m_Surround.pmsgJoyX );

	m_Surround.pmsgJoyY = createMidiMsg( _T("JoyY"), BID_JoyY );
	m_Surround.pmsgJoyY->SetMessageType( CMidiMsg::mtWheel7Bit );
	m_Surround.pmsgJoyY->SetChannel( 0xc );
	m_pMidiInputRouter->Add( m_Surround.pmsgJoyY );

	// surround 
	m_Surround.pParamAngle = createMixParam();
	m_Surround.pParamAngle->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SURROUND_ANGLE, 0, .5f );

	m_Surround.pParamFocus = createMixParam();
	m_Surround.pParamFocus->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SURROUND_FOCUS, 0, .707f );

	// LFE Send
	m_Surround.pmsgLFE = createMidiMsg( _T("LFE Send"), BID_LFESend );
	m_Surround.pmsgLFE->SetMessageType( CMidiMsg::mtWheel7Bit );
	m_Surround.pmsgLFE->SetChannel( 0xd );
	m_pMidiInputRouter->Add( m_Surround.pmsgLFE );

	m_Surround.pParamLFE = createMixParam();
	m_Surround.pParamLFE->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SURROUND_LFE, 0, .707f );

	m_Surround.pParamFRBal = createMixParam();
	m_Surround.pParamFRBal->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SURROUND_FR_BAL, 0, .5f );

	m_Surround.pParamWidth = createMixParam();
	m_Surround.pParamWidth->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SURROUND_WIDTH, 0, .5f );

	// t-bar
	pmsg = createMidiMsg( _T("FnTbAr"), BID_Tbar );
	pmsg->SetMessageType( CMidiMsg::mtWheel7Bit );
	pmsg->SetChannel( 9 );
	m_pMidiInputRouter->Add( pmsg );

	// for t-bar, also add the binding for the full-time ACT parameter 
	// when in that mode
	// full-time ACT plugin param for T-bar
	PMBINDING pmb;
	pmb.nRefreshMod = RM_EVEN;
	pmb.pMsgOut = NULL;	// no output
	pmb.pParam = createMixParam();
	pmb.pParam->SetCaptureType( CMixParam::CT_Match );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + TBAR_DYN_INDEX, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	pmb.pParam->SetThrottle( false );
	m_mapMiscMsgs[pmsg] = pmb;

	pmsg = createMidiMsg( _T("Tbar FR Bal"), BID_TbarFRBal);
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 1 );
	pmsg->SetNoteNum( 0x18 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Tbar ACT"), BID_TbarACT);
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 1 );
	pmsg->SetNoteNum( 0x19 );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Tbar x-ray"), BID_TbarXRay);
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel( 1 );
	pmsg->SetNoteNum( 0x1a );
	addControlMsg( pmsg );

	// Command / Keys
	// surround view button
	pmsg = createMidiMsg( _T("SurroundView"), BID_SurroundView );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetNoteNum( 0x1f );
	pmsg->SetChannel( 1 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_VIEW_SURROUND_PANNER;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("key up"), BID_KeyUp );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x60 );
	addControlMsg( pmsg );

	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_UP;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("key dn"), BID_KeyDn );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x61 );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_DOWN;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("key left"), BID_KeyL );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x62 );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_LEFT;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] =bad;

	pmsg = createMidiMsg( _T("key right"), BID_KeyR );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x63 );
	addControlMsg( pmsg );
	bad.eActionType = CTacomaSurface::ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_RIGHT;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTacomaSurface::buildCommandBindings()
{
	ButtonActionDefinition bad;
	bad.eActionType = ButtonActionDefinition::AT_Command;

	CMidiMsg* pmsg = createMidiMsg( _T("Save"), BID_Save );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x4f );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("UndoRedo"), BID_UndoRedo );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x50 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("BID_EZr0c0"), BID_EZr0c0 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x42 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.dwCommandOrKey = CMD_TRACK_VIEW;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;
	
	pmsg = createMidiMsg( _T("BID_EZr0c1"), BID_EZr0c1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x43 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.dwCommandOrKey = CMD_VIEW_SYNTH_RACK;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;
	
	pmsg = createMidiMsg( _T("BID_EZr0c2"), BID_EZr0c2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x44 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.dwCommandOrKey = CMD_AUDIOSNAP_PALETTE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_EZr0c3"), BID_EZr0c3 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x45 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Key;
	bad.wModKeys = CTacomaSurface::SMK_CTRL;	// next
	bad.dwCommandOrKey = VK_TAB;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_EZr1c0"), BID_EZr1c0 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3e );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.wModKeys = 0;
	bad.dwCommandOrKey = CMD_VIEW_CONSOLE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_EZr1c1"), BID_EZr1c1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3f );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.wModKeys = 0;
	bad.dwCommandOrKey = CMD_VIEW_NEW_PIANO_ROLL;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_EZr1c2"), BID_EZr1c2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x40 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.wModKeys = 0;
	bad.dwCommandOrKey = CMD_VIEW_CONTROLBAR_TRANSPORTMCV;	// transport
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_EZr1c3"), BID_EZr1c3 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x41 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Key;
	bad.wModKeys = CTacomaSurface::SMK_CTRL;	// close
	bad.dwCommandOrKey = VK_F4;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r2c0"), BID_EZr2c0 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3a );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_PROCESS_FADE_SELECTED;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r2c1"), BID_EZr2c1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3b );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_TRACK_FREEZE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r2c2"), BID_EZr2c2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3c );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_SPLIT_CLIPS_NOW;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r2c3"), BID_EZr2c3 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x3d );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_CLIP_MUTE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r3c0"), BID_EZr3c0 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x36 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_EDIT_CUT;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r3c1"), BID_EZr3c1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x37 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_EDIT_COPY;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r3c2"), BID_EZr3c2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x38 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_EDIT_PASTE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("BID_r3c3"), BID_EZr3c3 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x39 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_EDIT_DELETE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
HRESULT CTacomaSurface::buildKeyboardBindings()
{
	ButtonActionDefinition bad;

	CMidiMsg* pmsg = createMidiMsg( _T("OK"), BID_OK );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x51 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_RETURN;
	m_mapButton2BAD[BID_OK] = bad;

	pmsg = createMidiMsg( _T("Cancel"), BID_Cancel );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x52 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	bad.eActionType = ButtonActionDefinition::AT_Key;
	bad.dwCommandOrKey = VK_ESCAPE;
	m_mapButton2BAD[BID_Cancel] = bad;

	return S_OK;
}


/////////////////////////////////////////////////////////////////
HRESULT CTacomaSurface::buildTransportBindings()
{
	CMidiMsg* pmsg = createMidiMsg( _T("Play"), BID_Play );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5e );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("stop"), BID_Stop );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5d );	// no trigger because we need down/up state for Scrub
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("record"), BID_Record );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5f );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("FFwd"), BID_FF );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5c );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("Rewnd"), BID_Rew );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5b );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("RTZ"), BID_RTZ );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x21 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("FFwd"), BID_RTE );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x22 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// Given a CMidiMsg, add it to the input router and to our 
// Message Map
void	CTacomaSurface::addControlMsg( CMidiMsg* pmsg )
{
	CSFKCriticalSectionAuto csa( m_cs );

	m_pMidiInputRouter->Add( pmsg );
}


///////////////////////////////////////////////////////////////////////////////
// For whatever reason, the host parameter bindings have changed.
// This may be because the track bank has shifted, or the ACT context has changed.
HRESULT CTacomaSurface::updateParamBindings()
{
	// For any strip oriented parameters, set the strip offset to match our current bank
	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;
		DWORD cid = pmsg->GetId();
		const bool bIsMute = BID_IN( cid, BID_Mute0, BID_Mute7 );
		const bool bIsSolo = BID_IN( cid, BID_Solo0, BID_Solo7 );
		const bool bIsFader = BID_IN( cid, BID_Fader0, BID_Fader7 );
		const bool bIsArm = BID_IN( cid, BID_Arm0, BID_Arm7 );

      STRIP_INFO const info = getStripInfo( pmb.pParam->GetStripPhysicalIndex() );

		// Mutes and solos have to switch between Layer and track mode
		if ( bIsMute || bIsSolo )
		{
			const DWORD ixSwitch = bIsMute ? cid - BID_Mute0 : cid - BID_Solo0;
			if ( SDM_Layers == m_eDisplayMode )
			{
				// in layer mode, the M/Ss get the strip number of the sel strip and 0 offset for all switches
				pmb.pParam->SetMixParam( bIsMute ? MIX_PARAM_LAYER_MUTE : MIX_PARAM_LAYER_SOLO );
				pmb.pParam->SetParamNum( ixSwitch );
            pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripNumOffset( 0 );
			}
			else if ( SDM_ExistingFX == m_eDisplayMode && bIsMute )
			{
				// switch to fx bypass
				pmb.pParam->SetMixParam( MIX_PARAM_FX );
				pmb.pParam->SetParamNum( ixSwitch );
            pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripNumOffset( 0 );
				pmb.pParam->SetDisplayValue( false );
			}
			else if ( m_eDisplayMode == SDM_ChannelBranch )
			{
				if ( ixSwitch == 0 ) // first channel becomes the selected volume, msr, etc.
					pmb.pParam->SetAllParams( m_selectedStrip.stripType, m_selectedStrip.stripIndex,
													  bIsMute ? MIX_PARAM_MUTE : MIX_PARAM_SOLO, 0, 0 );
				else
					pmb.pParam->SetAllParams( m_selectedStrip.stripType, m_selectedStrip.stripIndex, 
													  bIsMute ? MIX_PARAM_SEND_ENABLE : MIX_PARAM_SEND_SOLO, ixSwitch - 1, 0, 7 );

				pmb.pParam->SetDisplayValue( true );
				pmb.pParam->SetDisplayName( true );
			}
			else
			{
				if ( !( bIsSolo && ( m_e8StripType == MIX_STRIP_MASTER ) ) )
					pmb.pParam->SetAllParams( pmb.pParam->GetMixerStrip(), ixSwitch, bIsMute ? MIX_PARAM_MUTE : MIX_PARAM_SOLO, 0, getCurrentBankOffset() );
				else
					pmb.pParam->SetAllParams( pmb.pParam->GetMixerStrip(), ixSwitch, MIX_PARAM_ANY, 0, 0 );

				pmb.pParam->SetDisplayValue( true );
				pmb.pParam->SetDisplayName( true );
			}
		}
		else if ( m_eDisplayMode == SDM_ChannelBranch )
		{
			if ( bIsArm || bIsFader )
			{
				const SONAR_MIXER_STRIP type = m_selectedStrip.stripType;
				const DWORD dwIndex = m_selectedStrip.stripIndex;

				if ( info.stripIndexPhysical != 0 )
					pmb.pParam->SetAllParams( type, dwIndex, bIsArm ? MIX_PARAM_SEND_PREPOST : MIX_PARAM_SEND_VOL, info.stripIndexPhysical - 1, 0, 7 );
				else
					pmb.pParam->SetAllParams( type, dwIndex, bIsArm ? MIX_PARAM_RECORD_ARM : MIX_PARAM_VOL, 0, 0 );
			}
		}
		else if ( ( SDM_Normal == m_eDisplayMode ) && ( bIsFader || bIsArm ) )
		{
			if ( bIsFader )
			{
				if ( !m_bFlipped )
					pmb.pParam->SetAllParams( pmb.pParam->GetMixerStrip(), ( cid - BID_Fader0 ), MIX_PARAM_VOL, 0, getCurrentBankOffset() );
				else
				{
					PSEncoderParams const pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, getModeForRotary() );
					const SONAR_MIXER_PARAM mixerParam = pEnc ? pEnc->mixParam : MIX_PARAM_PAN;
					const DWORD dwParamNum = pEnc ? pEnc->dwParam : 0;
					pmb.pParam->SetAllParams( pmb.pParam->GetMixerStrip(), ( cid - BID_Fader0 ), mixerParam, dwParamNum, getCurrentBankOffset() );
				}
			}
			else if ( bIsArm )
				pmb.pParam->SetAllParams( pmb.pParam->GetMixerStrip(), ( cid - BID_Arm0 ), MIX_PARAM_RECORD_ARM, 0, getCurrentBankOffset() );
		}
		else
			pmb.pParam->SetStripNumOffset( getCurrentBankOffset() );

      pmb.pParam->SetStripType( info.stripType );
		pmb.pParam->ResetHistory();
	}

	for ( InputBindingIterator it = m_mapShiftMsgs.begin(); it != m_mapShiftMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;
		const DWORD cid = pmsg->GetId();
		const bool bIsSolo = BID_IN( cid, BID_Solo0, BID_Solo7);
		const bool bIsMute = BID_IN( cid, BID_Mute0, BID_Mute7);

      STRIP_INFO const info = getStripInfo( pmb.pParam->GetStripPhysicalIndex() );

		// Mutes and solos have to switch between Layer and track mode
		if ( m_eDisplayMode == SDM_Normal )
		{
			if ( bIsSolo || bIsMute )
			{
				const DWORD ixSwitch = bIsMute ? cid - BID_Mute0 : cid - BID_Solo0;

				pmb.pParam->SetMixParam( bIsSolo ? MIX_PARAM_MUTE_DEFEAT : MIX_PARAM_INPUT_ECHO );
				pmb.pParam->SetParamNum( 0 );
            pmb.pParam->SetStripNum( ixSwitch );
				pmb.pParam->SetStripNumOffset( getCurrentBankOffset() );
				pmb.pParam->SetTriggerAction( CMixParam::TA_TOGGLE );
				pmb.pParam->ResetHistory();
			}
		}
		else if ( ( m_eDisplayMode == SDM_ChannelBranch ) && bIsMute && ( info.stripIndexPhysical != 0 ) )
		{
			const SONAR_MIXER_STRIP type = m_selectedStrip.stripType;
			const DWORD dwIndex = m_selectedStrip.stripIndex;
			pmb.pParam->SetAllParams( type, dwIndex, MIX_PARAM_SEND_INSERT, info.stripIndexPhysical - 1, 0, 7 );
			pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
			pmb.pParam->SetDefaultValue( 0.f );
			pmb.pParam->ResetHistory();
		}

		pmb.pParam->SetStripType( info.stripType );
	}

	// and update the master fader
	for ( InputBindingIterator it = m_mapMiscMsgs.begin(); it != m_mapMiscMsgs.end(); ++it )
	{
		CMidiMsg* pmsg = it->first;
		PMBINDING& pmb = it->second;
		if ( pmsg->GetId() == BID_FaderMstr || pmsg->GetId() == BID_FaderTouchMstr )
			pmb.pParam->SetStripNum( m_dwMasterOrg );
	}

	// and the Surround Panner
	const SONAR_MIXER_STRIP type = m_selectedStrip.stripType;
	const DWORD dwTrackIndex = m_selectedStrip.stripIndex;
	m_Surround.pParamAngle->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_ANGLE, 0 );
	m_Surround.pParamFocus->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_FOCUS, 0 );
	m_Surround.pParamFRBal->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_FR_BAL, 0 );
	m_Surround.pParamWidth->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_WIDTH, 0 );
	m_Surround.pParamLFE->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_LFE, 0 );

	// ACT and other non-strip params should be rebound too
	updateActSectionBindings( false );

	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
//Remap Eq/Send/ACT knobs based on strip sel and mode
HRESULT CTacomaSurface::updateActSectionBindings( bool bUpdateDisplay )
{
	// bump the ACT keys based on which page we're on.
	for ( InputBindingIterator it = m_mapACTControls.begin(); it != m_mapACTControls.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		pmb.pParam->SetStripNumOffset( m_wACTPage * TOTAL_DYN_COUNT );
	}

	// bump the send and eq strip numbers based on the Sel strip and bank.
	for ( InputBindingIterator it = m_mapSendControls.begin(); it != m_mapSendControls.end(); ++it )
	{
		PMBINDING& pmb = it->second;

      // BUG55444: If the strips' send is surround the "pan" should change the surround angle
      BOOL bSurround = IsSendSurround(m_selectedStrip.stripType, m_selectedStrip.stripIndex, pmb.pParam->GetParamNum());

      if ((pmb.pParam->GetMixerParam() == MIX_PARAM_SEND_PAN) && bSurround)
      {
         pmb.pParam->SetMixerParam(MIX_PARAM_SURROUND_SENDANGLE);
      }
      else if ((pmb.pParam->GetMixerParam() == MIX_PARAM_SURROUND_SENDANGLE) && !bSurround)
      {
         pmb.pParam->SetMixerParam(MIX_PARAM_SEND_PAN);
      }
      pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
      pmb.pParam->SetStripType( m_selectedStrip.stripType );
	}

	switch ( m_eActSectionMode )
	{
	case KSM_EQ:
		{
			// EQ
			for ( InputBindingIterator it = m_mapEQControls.begin(); it != m_mapEQControls.end(); ++it )
			{
				PMBINDING& pmb = it->second;
				pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripType( m_selectedStrip.stripType );
			}
			for ( DWORD dwIndex = 0; dwIndex < 4; dwIndex ++ )
			{
				m_pEqTypeParams[dwIndex]->SetStripNum( m_selectedStrip.stripIndex );
				m_pEqTypeParams[dwIndex]->SetStripType( m_selectedStrip.stripType );
			}

		}
		break;
	case KSM_PC_EQ:
		{
			//PC EQ
			for ( InputBindingIterator it = m_mapPCEQControls.begin(); it != m_mapPCEQControls.end(); ++it )
			{
				PMBINDING& pmb = it->second;
				pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripType( m_selectedStrip.stripType );
			}
		}
		break;
	case KSM_PC_EQ_P2:
		{
			//PC EQ P2
			for ( InputBindingIterator it = m_mapPCEQP2Controls.begin(); it != m_mapPCEQP2Controls.end(); ++it )
			{
				PMBINDING& pmb = it->second;
				pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripType( m_selectedStrip.stripType );
			}
		}
		break;
	case KSM_PC_COMP:
		{
			//PC Comp
			for ( InputBindingIterator it = m_mapPCCompControls.begin(); it != m_mapPCCompControls.end(); ++it )
			{
				PMBINDING& pmb = it->second;
				pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripType( m_selectedStrip.stripType );
			}
		}
		break;
	case KSM_PC_SAT:
		{
			//PC Sat
			for ( InputBindingIterator it = m_mapPCSatControls.begin(); it != m_mapPCSatControls.end(); ++it )
			{
				PMBINDING& pmb = it->second;
				pmb.pParam->SetStripNum( m_selectedStrip.stripIndex );
				pmb.pParam->SetStripType( m_selectedStrip.stripType );
			}
		}
		break;
	}

	if ( bUpdateDisplay )
		initLCDs();

	return S_OK;
}



//////////////////////////////////////////////////////////////////////
// Here is where you put any output messages that are not driven by
// refreshes from the host.  Such as ACT Display Row LED, Strip Select LED
// etc etc.

void CTacomaSurface::lightNthSEL( DWORD n, int nOnBlink )
{
	for ( DWORD iSel = 0; iSel < 8; iSel++ )
	{
		ControlId id = (ControlId)(iSel + BID_Sel0);
		ControlMessageMapIterator it = m_mapControlMsgs.find( id );
		if ( it != m_mapControlMsgs.end() )
		{
			CMidiMsg* pmsg = it->second;

			STRIP_INFO const info = getStripInfo( iSel );
			const bool bThisIndex = (iSel == n);

			if ( nOnBlink == 1 )
			{
				if ( bThisIndex )
					pmsg->Send( 1.f );
				else
				{
					stopBlink( id );
					pmsg->Send( 0.f );
				}
			}
			else if ( nOnBlink == 2 && bThisIndex )
				startBlink( id );
			else if ( nOnBlink == 0 && bThisIndex )
			{
				stopBlink( id );
				pmsg->Send( 0.f );
			}
		}
	}
}

void CTacomaSurface::initLEDs()
{
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	//const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;

	// ACT display row LEDs
	for ( WORD i = 0; i < 4; i++ )
	{
		m_ACTRowIndicator.SetNoteNum( 0x30 + i );
		m_ACTRowIndicator.Send( i == m_wACTDisplayRow ? 1.f : 0.f );
	}

	// select buttons
	if ( m_eDisplayMode != SDM_ChannelBranch )
	{
		for ( DWORD iSel = 0; iSel < 8; iSel++ )
		{
			ControlId id = (ControlId)(iSel + BID_Sel0);
			ControlMessageMapIterator it = m_mapControlMsgs.find( id );
			if ( it != m_mapControlMsgs.end() )
			{
				CMidiMsg* pmsg = it->second;

				STRIP_INFO const info = getStripInfo( iSel );
				pmsg->Send( ((info.stripIndex == m_selectedStrip.stripIndex) && (info.stripType == m_selectedStrip.stripType)) ? 1.f : 0.f );
			}
		}
	}

	// ACT/EQ/Send Mode LEDs
	ControlMessageMapIterator it = m_mapControlMsgs.find( BID_Eq );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_FLEXIBLE_PC == m_eActSectionMode ) ? 1.f : 0.f );
	}
#if 0
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_EQ == m_eActSectionMode ) ? 1.f : 0.f );
	}
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_PC_EQ == m_eActSectionMode ) ? 1.f : 0.f );
	}
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_PC_EQ_P2 == m_eActSectionMode ) ? 1.f : 0.f );
	}
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_PC_COMP == m_eActSectionMode ) ? 1.f : 0.f );
	}
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_PC_SAT == m_eActSectionMode ) ? 1.f : 0.f );
	}
#endif

	it = m_mapControlMsgs.find( BID_Send );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_SEND == m_eActSectionMode ) ? 1.f : 0.f );
	}

	it = m_mapControlMsgs.find( BID_Act );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( KSM_ACT == m_eActSectionMode ) ? 1.f : 0.f );
	}
	//Add for 3 New MOdules


	// Track / Bus / Main switches
	it = m_mapControlMsgs.find( BID_Tracks );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( MIX_STRIP_TRACK == m_e8StripType ? 1.f : 0.f );
	}
	it = m_mapControlMsgs.find( BID_Buses );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( MIX_STRIP_BUS == m_e8StripType ? 1.f : 0.f );
	}
	it = m_mapControlMsgs.find( BID_Masters );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( MIX_STRIP_MASTER == m_e8StripType ? 1.f : 0.f );
	}

	// time code mode led
	it = m_mapControlMsgs.find( BID_TimeCodeMode );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( TF_SMPTE == m_mfxfPrimary ? 1.f : 0.f );
	}

	// sync mode indicator
	m_msgMiscLed.SetChannel( 1 );
	m_msgMiscLed.SetNoteNum( 0x36 );	// set smpte

	m_msgMiscLed.SetNoteNum( 0x37 );	// set mtc

	// T-bar modes
	it = m_mapControlMsgs.find( BID_TbarFRBal );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( m_eTBM == TBM_FRBalance? 1.f : 0.f );
	}
	it = m_mapControlMsgs.find( BID_TbarACT );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( m_eTBM == TBM_ACT ? 1.f : 0.f );
	}
	it = m_mapControlMsgs.find( BID_TbarXRay );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( m_eTBM == TBM_XRay ? 1.f : 0.f );
	}

	// IO Control Mode
	it = m_mapControlMsgs.find( BID_IOCtrl );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( m_bIOMode ? 1.f : 0.f );
	}

	// Jog Mode Buttons
	bool bSelecting = true;
	it = m_mapControlMsgs.find( BID_DataSelect );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		stopBlink( BID_DataSelect );

		//JogMode jmEffective = bCommand ? JM_Zoom : m_eJogMode;
		JogMode jmEffective = m_eJogMode;
		if ( bShift && jmEffective == JM_SelByTime )
			jmEffective = JM_SelByClips;
		else if ( bShift && jmEffective == JM_SelByClips )
			jmEffective = JM_SelByTime;

		if ( jmEffective == JM_SelByTime )
			pmsg->Send( 1.f );
		else if ( jmEffective == JM_SelByClips )
			startBlink( BID_DataSelect );
		else
		{
			pmsg->Send( 0.f );
			bSelecting = false;
		}
	}
	it = m_mapControlMsgs.find( BID_DataEdit );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( ( m_eJogMode == JM_Edit ) ? 1.f : 0.f );
	}
	it = m_mapControlMsgs.find( BID_Scroll );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		stopBlink( BID_Scroll );

		JogMode jmEffective = m_eJogMode;
		//if ( ( bShift && m_eJogMode == JM_Zoom ) || ( bCommand && bShift ) )
		if ( bShift && m_eJogMode == JM_Zoom )
			jmEffective = JM_Scroll;
		//else if ( ( bShift && m_eJogMode == JM_Scroll ) || bCommand )
		else if ( bShift && m_eJogMode == JM_Scroll )
			jmEffective = JM_Zoom;

		if ( jmEffective == JM_Scroll )
			pmsg->Send( 1.f );
		else if ( jmEffective == JM_Zoom )
			startBlink( BID_Scroll );
		else
			pmsg->Send( 0.f );
	}

	// Record/Edit tools
	// Left/Right markers, punch, loop, snap
	it = m_mapControlMsgs.find( BID_LeftMarker );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		stopBlink( BID_LeftMarker );
		if ( m_eJogMode == JM_Punch || m_eJogMode == JM_Loop )
		{
			if ( !m_bEditRight )
				startBlink( BID_LeftMarker );
			else
				pmsg->Send( 0.f );
		}
		else if ( bCommand )
			pmsg->Send( 0.f );
		else if ( m_bSelectingOrEditing && bSelecting )
			pmsg->Send( 1.f );
		else
		{
			float f = 0.f;
			if ( JM_Edit == m_eJogMode && CEF_CROP_START == m_cefCurrent )
				f = 1.f;
			pmsg->Send( f );

			if ( JM_Edit == m_eJogMode && ( ( CEF_FADE_START == m_cefCurrent ) || ( ( CEF_CROP_START == m_cefCurrent ) && bShift ) ) )
				startBlink( BID_LeftMarker );
		}
	}

	it = m_mapControlMsgs.find( BID_RightMarker );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		stopBlink( BID_RightMarker );
		if ( m_eJogMode == JM_Punch || m_eJogMode == JM_Loop )
		{
			if ( m_bEditRight )
				startBlink( BID_RightMarker );
			else
				pmsg->Send( 0.f );
		}
		else if ( bCommand )
			pmsg->Send( 0.f );
		else if ( m_bSelectingOrEditing && bSelecting )
			startBlink( BID_RightMarker );
		else
		{
			float f = 0.f;
			if ( JM_Edit == m_eJogMode && CEF_CROP_END == m_cefCurrent )
				f = 1.f;
			pmsg->Send( f );

			if ( JM_Edit == m_eJogMode && ( ( CEF_FADE_END == m_cefCurrent ) || ( ( CEF_CROP_END == m_cefCurrent ) && bShift ) ) )
				startBlink( BID_RightMarker );
		}
	}

	if ( ( m_eJogMode == JM_Punch ) || ( m_eJogMode == JM_Loop ) )
	{
		it = m_mapControlMsgs.find( BID_PunchOn );
		if ( it != m_mapControlMsgs.end() )
		{
			CMidiMsg* pmsg = it->second;
			stopBlink( BID_PunchOn );
			if ( m_eJogMode == JM_Punch )
				startBlink( BID_PunchOn );
			else if ( m_eJogMode == JM_Loop )
				pmsg->Send( 0.f );
		}

		it = m_mapControlMsgs.find( BID_LoopOn );
		if ( it != m_mapControlMsgs.end() )
		{
			CMidiMsg* pmsg = it->second;
			stopBlink( BID_LoopOn );
			if ( m_eJogMode == JM_Loop )
				startBlink( BID_LoopOn );
			else if ( m_eJogMode == JM_Punch )
				pmsg->Send( 0.f );
		}
	}
	else
	{
		stopBlink( BID_PunchOn );
		stopBlink( BID_LoopOn );
	}
}


/////////////////////////////////////////////////////////////////
// Given a control ID, find a mix param if any
CMixParam* CTacomaSurface::paramFromId( ControlId cid )
{
	CMixParam* pParam = NULL;

	// find the MIDI message so we can find the parameter
	ControlMessageMapIterator it = m_mapControlMsgs.find( cid );
	if ( it != m_mapControlMsgs.end() )
	{
		CMidiMsg* pmsg = it->second;
		InputBindingIterator itm = m_mapMiscMsgs.find( pmsg );
		if ( itm != m_mapMiscMsgs.end() )
		{
			PMBINDING& pmb = itm->second;
			pParam = pmb.pParam;
		}
		else
		{
			itm = m_mapStripMsgs.find( pmsg );
			if ( itm != m_mapStripMsgs.end() )
			{
				PMBINDING& pmb = itm->second;
				pParam = pmb.pParam;
			}
		}
	}

	return pParam;
}



/////////////////////////////////////////////////////////////////
void CTacomaSurface::initOverUnderLeds()
{
	// Over/Under led for Width/FRBal
	m_msgMiscLed.SetChannel( 1 );
	CMixParam* pParamToTest = NULL;
	if ( TBM_ACT == m_eTBM )
	{
		pParamToTest = paramFromId( BID_Tbar );
	}
	else if ( TBM_FRBalance == m_eTBM )
		pParamToTest = m_Surround.pParamFRBal;

	if ( pParamToTest )
	{
		CMixParam::VALUE_HISTORY vh = pParamToTest->GetNullStatusForValue( m_fTbarRcvd );

//		TRACE( _T("NullStatus =%d\n"), vh );

		m_msgMiscLed.SetNoteNum( 0x34 );	// above
		m_msgMiscLed.Send( (vh & CMixParam::VT_WASABOVE) ? 1.f : 0.f );
		m_msgMiscLed.SetNoteNum( 0x35 );	// below
		m_msgMiscLed.Send( (vh & CMixParam::VT_WASBELOW) ? 1.f : 0.f );
	}
	else
	{
		m_msgMiscLed.SetNoteNum( 0x34 );	// above
		m_msgMiscLed.Send(0.f);
		m_msgMiscLed.SetNoteNum( 0x35 );	// below
		m_msgMiscLed.Send(0.f);
	}
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// The Call back from a MIDI Input Message.  Your job here is to figure out what
// to do in the the host based on which param called you back, and what its
// value is.
HRESULT	CTacomaSurface::OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue, CMidiMsg::ValueChange vc )
{
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	
	bool bMatch = false;
	InputBindingIterator it;

	if ( !bMatch )
		bMatch = handlePushMarkerSelect( pObj, fValue );

	if ( !bMatch )
	{
		// Try the EQ / send / Aux section
		InputBindingMap* pMap = NULL;
		switch( m_eActSectionMode )
		{
		case KSM_PC_EQ:
			pMap = &m_mapPCEQControls;
			break;
		case KSM_PC_EQ_P2:
			pMap = &m_mapPCEQP2Controls;
			break;
		case KSM_PC_COMP:
			pMap = &m_mapPCCompControls;
			break;
		case KSM_PC_SAT:
			pMap = &m_mapPCSatControls;
			break;
		case KSM_EQ:
			pMap = &m_mapEQControls;
			break;
		case KSM_SEND:
			pMap = &m_mapSendControls;
			break;

		case KSM_FLEXIBLE_PC:
		case KSM_ACT:
			pMap = &m_mapACTControls;
			break;
		}

		if ( pMap )
		{
			it = pMap->find( pObj );
			if ( pMap->end() != it )
			{
				bMatch = true;
				PMBINDING& pmb = it->second;
				///*
				//if ( bCommand )
				//	requestRevert( pmb.pParam );
				//else
				//*/
				{
					if ( pObj->IsTrigger() )
					{
						pmb.pParam->Trigger();
						// for those commands that reset to default - the actual value of the input MSG must follow
						if ( pmb.pParam->GetTriggerAction() == CMixParam::TA_DEFAULT )
						{
							// find the matching midi message with the same param if this is a push button
							for ( InputBindingIterator itInner = pMap->begin(); itInner != pMap->end(); ++itInner )
							{
								if ( ( itInner->second.pParam == it->second.pParam ) && ( itInner->first != it->first ) )
								{
									CMidiMsg * const pmsgIn = itInner->first;
									float f = 0.f;

									pmb.pParam->ResetHistory();
									if ( SUCCEEDED( pmb.pParam->GetVal( &f ) ) )
										pmsgIn->SetVal( f );
								}
							}
						}
					}
					else
               {
                  pmb.pParam->SetVal( fValue, MIX_TOUCH_TIMEOUT, vc );
               }

					showParam( pmb.pParam, PSM_Touch );
					setTimer( m_tcbParamChange, 1500 );
				}
			}
		}
	}

	bool bEffects = false;
	if ( !bMatch )
	{
#define COUNT_MAPS	3
		InputBindingMap *pMap[COUNT_MAPS] = { &m_mapShiftMsgs, &m_mapAltMsgs, &m_mapStripMsgs };
		const WORD wModifier[COUNT_MAPS] = { SMK_SHIFT, SMK_ALT, 0 };
		PSEncoderParams pEnc = NULL;
		if ( m_bIOMode )
		{
			pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, Enc_IO );
			if ( !pEnc )
				pEnc = GetEncoderListParam( 0, MIX_STRIP_ANY, Enc_IO );
			if ( !pEnc )
				return ( E_UNEXPECTED );
		}

		for ( DWORD dwCntr = 0; dwCntr < COUNT_MAPS; dwCntr ++ )
		{
			it = pMap[ dwCntr ]->find( pObj );
			if ( pMap[ dwCntr ]->end() != it )
			{
				bMatch = true;
				PMBINDING& pmb = it->second;

				if ( bCommand )
				{
					requestRevert( pmb.pParam );
					break;
				}

				// special case assigning a bus to the master fader - SHIFT + Arm
				if ( ( Get8StripType() == MIX_STRIP_BUS ) && ( m_wModifierKey & SMK_SHIFT ) &&  ( BID_IN( pObj->GetId(), BID_Arm0, BID_Arm7 ) ) )
				{
					m_dwMasterOrg = pmb.pParam->GetStripNum();
					updateParamBindings();
					return S_OK;
				}

					// special case toggling Phatom Switch - ALT + Arm
			    if ((( m_wModifierKey & SMK_ALT ) &&  ( BID_IN( pObj->GetId(), BID_Arm0, BID_Arm7 ) ) ))
					{
						const DWORD dwIxChan = pObj->GetId() - BID_Arm0;
						const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_Phantom );
						m_pIOBoxInterface->SetParam( dwIxChan, TIOP_Phantom, fVal >= 0.5f ? 0.f : 1.f );
					}

				
				const BOOL bEncoder = BID_IN( pObj->GetId(), BID_Encoder0, BID_Encoder7 );
				const BOOL bAct = BID_IN( pObj->GetId(), BID_EncoderActR0C0, BID_EncoderActR0C3 ) || 
										BID_IN( pObj->GetId(), BID_EncoderActR1C0, BID_EncoderActR1C3 ) || 
										BID_IN( pObj->GetId(), BID_EncoderActR2C0, BID_EncoderActR2C3 );

				if ( ( m_wModifierKey != wModifier[ dwCntr ] ) && !( bEncoder || bAct ) )
					continue;

				// if we're in IO control mode, do special stuff with the encoders
				if ( m_bIOMode && m_pIOBoxInterface )
				{
					if ( bEncoder )
					{
						const DWORD dwIxChan = pObj->GetId() - BID_Encoder0;
						const TacomaIOBoxParam p = m_bFlipped ? TIOP_Gain : pEnc->ioParam;

						if (  m_wModifierKey & SMK_SHIFT )
						{
							// fine control
							const float fReadValue = m_pIOBoxInterface->GetParam( dwIxChan, p );
							fValue = fReadValue + ( ( fValue - fReadValue ) / 5 );
						}

						m_pIOBoxInterface->SetParam( dwIxChan, p, fValue );
					}
					else if ( BID_IN( pObj->GetId(), BID_EncoderPush0, BID_EncoderPush7 ) )
					{
						// toggle compressor
						const DWORD dwIxChan = pObj->GetId() - BID_EncoderPush0;
						const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_CompEnable );
						m_pIOBoxInterface->SetParam( dwIxChan, TIOP_CompEnable, fVal >= .5 ? 0.f : 1.f );
					}
					else if ( BID_IN( pObj->GetId(), BID_Fader0, BID_Fader7 ) )
					{
						const DWORD dwIxChan = pObj->GetId() - BID_Fader0;
						m_pIOBoxInterface->SetParam( dwIxChan, m_bFlipped ? pEnc->ioParam : TIOP_Gain, fValue );
					}
					else if ( BID_IN( pObj->GetId(), BID_Mute0, BID_Mute7 ) )
					{
						const DWORD dwIxChan = pObj->GetId() - BID_Mute0;
						const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_Phase );
						m_pIOBoxInterface->SetParam( dwIxChan, TIOP_Phase, fVal >= 0.5f ? 0.f : 1.f );
					}
					
					else if ( BID_IN( pObj->GetId(), BID_Solo0, BID_Solo7 ) )
					{
						UINT uActive = GetIOBoxInterface()->GetActiveInterface();
						
						if ( ( GetIOBoxInterface()->m_viofacestr[uActive] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[uActive] == (_T("VS-700R2"))) )
						{
							const DWORD dwIxChan = pObj->GetId() - BID_Solo0;
							const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_Pad );
							m_pIOBoxInterface->SetParam( dwIxChan, TIOP_Pad, fVal >= 0.5f ? 0.f : 1.f );
						}
						else
						{
							const DWORD dwIxChan = pObj->GetId() - BID_Solo0;
							const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_Hiz );
							m_pIOBoxInterface->SetParam( dwIxChan, TIOP_Hiz, fVal >= 0.5f ? 0.f : 1.f );
						}
					}
					else if ( BID_IN( pObj->GetId(), BID_Arm0, BID_Arm7 ) )
					{
						const DWORD dwIxChan = pObj->GetId() - BID_Arm0;
						const float fVal = m_pIOBoxInterface->GetParam( dwIxChan, TIOP_LoCut );
						m_pIOBoxInterface->SetParam( dwIxChan, TIOP_LoCut, fVal >= 0.5f ? 0.f : 1.f );
					}
				

					return ( S_OK );
				}
				else if ( SDM_ExistingFX == m_eDisplayMode )
				{
					bEffects = true;
					if ( BID_IN( pObj->GetId(), BID_Solo0, BID_Solo7 ) )
					{
						// Solo button = delete Fx
						IHostPluginAccess* pHostPlugs = NULL;
						if ( SUCCEEDED( m_pSonarMixer->QueryInterface( IID_IHostPluginAccess, (void**)&pHostPlugs )  ) )
						{
							DWORD dwFx = pObj->GetId() - BID_Solo0;

							float fFx = 0.f;
                     m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FX_COUNT, 0, &fFx );
							if ( dwFx < (DWORD)fFx )
                        pHostPlugs->DeletePlugin( m_selectedStrip.stripType, m_selectedStrip.stripIndex, dwFx );
							pHostPlugs->Release();

							updatePluginLEDs();
							return S_OK;
						}
					}
					else if ( BID_IN( pObj->GetId(), BID_Arm0, BID_Arm7 ) )
					{
						// Arm button = insert Fx
						IHostPluginAccess* pHostPlugs = NULL;
						if ( SUCCEEDED( m_pSonarMixer->QueryInterface( IID_IHostPluginAccess, (void**)&pHostPlugs )  ) )
						{
							setDisplayMode( SDM_FXTree );

							m_dwBinInsertIndex = pObj->GetId() - BID_Arm0;

							// make sure it stays in range
							float fFx = 0.f;
                     m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FX_COUNT, 0, &fFx );
							m_dwBinInsertIndex = min( m_dwBinInsertIndex, (DWORD)fFx );

							updatePluginLEDs();

							ControlMessageMapIterator it = m_mapControlMsgs.find( (ControlId)(BID_Arm0 + m_dwBinInsertIndex) );
							if ( it != m_mapControlMsgs.end() )
							{
								CMidiMsg* pmsg = it->second;
								pmsg->Send( 1.f );
							}

							startBlink( (ControlId)(BID_Mute0 + m_dwBinInsertIndex) );

							// obtain the entire plugin tree structure from the host
                     pHostPlugs->GetAvailablePlugins( m_selectedStrip.stripType, m_selectedStrip.stripIndex, &m_pPluginTree );
							pHostPlugs->Release();

							// zero out our child indexes
							::memset( (void*)m_aPluginChildIndex, 0, sizeof(m_aPluginChildIndex) );

							// display the tree
							showAvailablePlugins();
							return S_OK;
						}
					}
				}
				else if ( SDM_FXTree == m_eDisplayMode )
				{
					bEffects = true;
					// encoders step through the tree or choose a plugin
					if ( BID_IN( pObj->GetId(), BID_Encoder0, BID_Encoder7 ) )
					{
						int ixRotary = pObj->GetId() - BID_Encoder0;
						ixRotary /= (NumPluginChar/8);
						setPluginChild( ixRotary, vc );
						return S_OK;
					}
					else if ( BID_IN( pObj->GetId(), BID_EncoderPush0, BID_EncoderPush7 ) )
					{
						if ( TRUEFLOAT(fValue) )
						{
							int ixRotary = pObj->GetId() - BID_EncoderPush0;
							ixRotary /= (NumPluginChar/8);
							insertPlugin( ixRotary );
						}
						return S_OK;
					}
				}

				if ( BID_IN( pObj->GetId(), BID_FaderTouch0, BID_FaderTouchMstr ) )
				{
					pmb.pParam->Touch( fValue > 0.5f );
					if ( m_eDisplayMode != SDM_ChannelBranch && m_bFaderTouchSelectsChannel && !bEffects )
					{
						DWORD dwIx = pObj->GetId() - BID_FaderTouch0;
						doSelectStrip( dwIx );
					}
				}
				else if ( m_nInserting )
				{
					if ( pObj->IsTrigger() )
					{
						// commit the destination
						// but scale the value because SetVal does a scaleToHost() call
						float fMax = -1.f;
						m_pSonarMixer->GetMixParam( pmb.pParam->GetMixerStrip(), pmb.pParam->GetStripNum(), MIX_PARAM_SEND_OUTPUT_MAX, pmb.pParam->GetParamNum(), &fMax );
						if ( fMax > 0.f )
							m_fSendDest /= (fMax - 1.f);
						m_fSendDest = max( 0.f, m_fSendDest );
						m_fSendDest = min( 1.f, m_fSendDest );

						pmb.pParam->SetVal( m_fSendDest, MIX_TOUCH_TIMEOUT );

						endChannelBranchInserting();
					}
					else
					{
						HRESULT hr = S_OK;
						float fMin = 0.f, fMax = 0.f, fStep = 0.f;
						if ( !SUCCEEDED( hr = pmb.pParam->GetMinMaxStep( &fMin, &fMax, &fStep ) ) )
							return ( hr );

						if ( m_fSendDest == -1.f )
						{
							float fVal = -1.f;
							if ( !SUCCEEDED( hr = pmb.pParam->GetVal( &fVal ) ) )
								return ( hr );

							m_fSendDest = fVal - 1.f;
						}

						if ( vc == CMidiMsg::VC_Increase )
							m_fSendDest = ( min( fMax, m_fSendDest + fStep ) );
						else if ( vc == CMidiMsg::VC_Decrease )
							m_fSendDest = ( max( fMin, m_fSendDest - fStep ) );

						m_fSendDest ++; // because 0 is "--- None ---", but we can't set None

						// update display
						char sz[64];
						memset( sz, 0, sizeof ( sz ) );
						DWORD csz = _countof( sz );
						if ( !SUCCEEDED( m_pSonarMixer->GetMixParamValueText(	m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_OUTPUT,
																								m_nInserting - 1, m_fSendDest--, sz, &csz ) ) )
							return ( true );

						CStringCruncher cCruncher;
						char szBuf[8];
						memset( szBuf, 0, 8 );
						cCruncher.CrunchString( sz, szBuf, 7 );
						showText( szBuf, m_nInserting + 4, 1 );

						return ( S_OK );
					}
				}
				else if ( pObj->IsTrigger() )
					pmb.pParam->Trigger();
				else
					pmb.pParam->SetVal( fValue, MIX_TOUCH_TIMEOUT, vc );

				if ( !bEffects )
				{
					if ( bEncoder )
					{
						float fVal = 0.f;
						SONAR_MIXER_STRIP eStripType;
						DWORD dwStripNum = 0;
						pmb.pParam->GetStripInfo( &eStripType, &dwStripNum );
						if ( m_pSonarMixer->GetMixParam( eStripType, dwStripNum, pmb.pParam->GetMixerParam(), pmb.pParam->GetParamNum(), &fVal ) != E_NOTIMPL )
							showParam( pmb.pParam, PSM_Touch );
					}
					else
						showParam( pmb.pParam, PSM_Touch );
					setTimer( m_tcbParamChange, 1500 );
				}

				return ( S_OK );
			}
		}
	}

	if ( !bMatch )
	{
		// try the misc params
		it = m_mapMiscMsgs.find( pObj );
		if ( m_mapMiscMsgs.end() != it )
		{
			bMatch = true;
			PMBINDING& pmb = it->second;
			const DWORD dwId = pObj->GetId();
			if ( bCommand )
			{
				if ( dwId == BID_Tbar && TBM_FRBalance != m_eTBM )
					bMatch = false;
				else
					requestRevert( pmb.pParam );
			}
			else
			{
				// only allow tbar ACT if in that mode
				if ( dwId == BID_Tbar )
				{
					m_fTbarRcvd = fValue;
					if ( TBM_ACT != m_eTBM )
						bMatch = false;
				}

				if ( bMatch )
				{
					// handle specials...
					if ( dwId == BID_FaderTouchMstr )
						pmb.pParam->Touch( fValue > .1f );
					else if ( pObj->IsTrigger() )
						pmb.pParam->Trigger();
					else
						pmb.pParam->SetVal( fValue, MIX_TOUCH_TIMEOUT, vc );

					showParam( pmb.pParam, PSM_Touch );
					setTimer( m_tcbParamChange, 1500 );
				}
			}

		}
	}

	if ( !bMatch )
		bMatch = handleJoyStick( pObj, fValue );

	if (!bMatch )
		bMatch = handleTbar( pObj, fValue );

	if ( !bMatch )
	{
		// is it a param-less control?
		if ( m_mapControlMsgs.count( (ControlId)pObj->GetId() ) > 0 )
		{
			bMatch = true;
			handleControlMsg( pObj, fValue );
		}
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////
bool	CTacomaSurface::handleTbar( CMidiMsg* pmsg, float fVal )
{
	bool bHandled = true;
	switch ( pmsg->GetId() )
	{
	case BID_Tbar:
		{
			switch( m_eTBM )
			{
			case TBM_FRBalance:
				m_Surround.pParamFRBal->SetVal( fVal );
				break;
			case TBM_ACT:
				// should be handled by main param handler
				break;
			case TBM_XRay:
				if ( m_wModifierKey & SMK_SHIFT )
				{
#if DAISY
					if ( fVal == 0.f )
						showDisplayEE();
#endif
				}
				else
				{
					m_pSonarMixer->SetMixParam( MIX_STRIP_TRACK, 0, MIX_PARAM_XRAY_OPACITY, 0, fVal, MIX_TOUCH_TIMEOUT );
					m_fXRay = fVal;
				}
				break;
			}
		}
		break;
		case BID_TbarFRBal:
			m_eTBM = TBM_FRBalance;
			updateParamStateOnLCD();
		break;
		case BID_TbarACT:
			m_eTBM = TBM_ACT;
			updateParamStateOnLCD();
		break;
		case BID_TbarXRay:
			m_eTBM = TBM_XRay;
			updateParamStateOnLCD();
		break;
		default:
			bHandled = false;
		break;
	}

	if ( bHandled )
		initLEDs();

	return bHandled;
}


//////////////////////////////////////////////////////////////////////

#define _PI				3.1415926535f
#define QUARTER_PI	(_PI / 4)
#define HALF_PI		(_PI / 2)

bool CTacomaSurface::handleJoyStick( CMidiMsg* pmsg, float fVal )
{
	const DWORD dwId = pmsg->GetId();
	bool bHandled = false;

	if ( dwId == BID_JoyX || dwId == BID_JoyY )
	{
		bHandled = true;

		if ( dwId == BID_JoyX )
			m_Surround.fValX = fVal;
		else
			m_Surround.fValY = fVal;

		// now we know the x-y pos, so convert to angle/focus and 
		// set params

		// param 0 = Angle. 0 = North increasing counterclockwise
		// param 1 = Focus. 0 =  center of circle, 1 = on circumference
		const float dx = 2.f * (m_Surround.fValX - .5f);
		const float dy = 2.f * (.5f - m_Surround.fValY);

		// Determine quadrant.  1 is SE, 2 is NE, 3 is NW, 4 is SW
		int nQuad = 1;
		if ( dx < 0 )
		{
			if ( dy < 0 )
				nQuad = 3;
			else
				nQuad = 4;
		}
		else if ( dy < 0 )
			nQuad = 2;

		float fAngle = 0.25f;
		if ( dy != 0 )
			fAngle = ::atan( ::fabs( dx / dy ) ) / (3.1416f * 2);

		// now offset based on quadrant
		switch ( nQuad )
		{
		case 1:
			// nothing
			break;
		case 2:
			fAngle = 0.50f - fAngle;
			break;
		case 3:
			fAngle += 0.50f;
			break;
		case 4:
			fAngle = 1.00f - fAngle;
			break;
		}
		
		// use Pythagorean to find magnitude
		float fFocus = ::sqrt( ::pow( dx, 2 ) + ::pow( dy, 2 ) );
		fFocus = min(1, fFocus);

		if ( ( m_wModifierKey & SMK_COMMAND ) && m_pHostWindow )
		{
			// move window
			const float fAngle2 = ::atan2f( dx, dy );

			RECT rectScreen;
			rectScreen.left = 0; 
			rectScreen.top = 0; 
			rectScreen.right = GetSystemMetrics (SM_CXMAXTRACK); 
			rectScreen.bottom = GetSystemMetrics (SM_CYMAXTRACK);
			int nNewX = 0, nNewY = 0;
			const int nScreenRadX = ( rectScreen.right - rectScreen.left ) / 2;
			const int nScreenRadY = ( rectScreen.bottom - rectScreen.top ) / 2;
			const int nMultX = ( nQuad > 2 ) ? -1 : 1;
			if ( ( abs( fAngle2 ) < QUARTER_PI ) || ( abs( fAngle2 ) > ( QUARTER_PI * 3 ) ) )
			{
				// horizontal boundaries
				const int nMult = ( nQuad > 1 && nQuad < 4 ) ? -1 : 1;
				nNewX = (int) ( nScreenRadX + ( nScreenRadX * fFocus * tan( abs( fAngle2 ) ) * nMult * nMultX ) );
				nNewY = (int) ( nScreenRadY + ( nScreenRadY * fFocus * nMult ) );
			}
			else //if ( ( abs( fAngle2 ) >= QUARTER_PI ) && ( abs( fAngle2 ) <= ( QUARTER_PI * 3 ) ) )
			{
				// vertical boundaries
				nNewX = (int) ( nScreenRadX + ( nScreenRadX * fFocus * nMultX ) );
				nNewY = (int) ( nScreenRadY - ( nScreenRadY * fFocus * tan( abs( fAngle2 ) - HALF_PI ) ) );
			}

			// set new target and kick off timer
			m_ptTarget.x = nNewX;
			m_ptTarget.y = nNewY;
			setTimer( m_tcbWindowMove, 20 );
		}
		else
		{
         // Here we are now setting the value for the angle to be (1 - the angle) since the rotary encoders
         // send a midi value that corresponds with rotation in the other direction.
         // We offset this in scaleValueToHost() and scaleValueFromHost() by subtracting it from 1 also.
			m_Surround.pParamAngle->SetVal( 1.f - fAngle );
			m_Surround.pParamFocus->SetVal( fFocus );
		}
	}

	if ( dwId == BID_LFESend )
	{
		bHandled = true;
		if ( m_wModifierKey & SMK_SHIFT )
			m_Surround.pParamFRBal->SetVal( fVal );
		else if ( m_wModifierKey & SMK_CTRL )
			m_Surround.pParamWidth->SetVal( fVal );
		else
			m_Surround.pParamLFE->SetVal( fVal );
	}

	return bHandled;
}


////////////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handlePushMarkerSelect( CMidiMsg* pmsg, float )
{
	bool bHandled = false;
	if ( SDM_Markers != m_eDisplayMode )
		return bHandled;

	DWORD dwMkr = 0;
	const DWORD dwId = pmsg->GetId();
	if ( BID_IN( dwId, BID_EncoderPushActR0C0, BID_EncoderPushActR0C3 ) )
	{
		bHandled = true;
		dwMkr = dwId - BID_EncoderPushActR0C0;
	}
	else if ( BID_IN( dwId, BID_EncoderPush0, BID_EncoderPush7 ) )
	{
		bHandled = true;
		dwMkr = dwId - BID_EncoderPush0 + 4;
	}

	if ( bHandled )
		GotoMarker( dwMkr, true );

	return bHandled;
}



///////////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleControlMsg( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = handleBankButton( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleTransportControl( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleCommandOrKey( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleModeButton( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleRecEditTools( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleModifierButton( pmsg, fValue );
	if (!bHandled )
		bHandled = handleStripSelButton( pmsg, fValue );

	return bHandled;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CTacomaSurface::doSelectStrip( DWORD ixStrip )
{
	if ( Get8StripType() != MIX_STRIP_MASTER )
	{
		float fVal = 0.f;
		m_pSonarMixer->GetMixParam( Get8StripType(),
											 ixStrip, 
											 (Get8StripType() == MIX_STRIP_TRACK) ? MIX_PARAM_TRACK_EXISTS : MIX_PARAM_BUS_EXISTS,
											 0,
											 &fVal );
		if ( fVal < 1.f )
			return ( false );
	}

	m_prevSelectedStrip = m_selectedStrip;
	m_selectedStrip     = getStripInfo( ixStrip ); // Take into account any locked tracks, etc
	initLEDs();
	updateActSectionBindings( false );

	// also if the 8strip type is Track, update the host's active track
   if ( MIX_STRIP_TRACK == m_selectedStrip.stripType)
	{
      SetSelectedTrack( m_selectedStrip.stripIndex );
		updateParamBindings();
	}
	else if (MIX_STRIP_BUS == m_selectedStrip.stripType)
	{
		SetSelectedBus( m_selectedStrip.stripIndex );
		updateParamBindings();
	}

	RequestStatusQuery();

   // update ACT context to the track selected
   if ( m_pSonarUIContext )
   {
		if (m_eActSectionMode == KSM_ACT)
		{
			m_pSonarUIContext->SetUIContext( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS );
			m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_USER_BIN );
			SetProChanLock( false );
		}
		else if (m_eActSectionMode == KSM_FLEXIBLE_PC)
		{
			m_pSonarUIContext->SetUIContext( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FILTER_PARAM, -1, UIA_FOCUS );
			m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_PROCHANNEL );
			SetProChanLock( true );
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SONAR_MIXER_STRIP CTacomaSurface::GetStripFocus()
{
	if (!m_pSonarMixer)
		return MIX_STRIP_ANY;

	float fValue = 0.f;
	if (S_OK == m_pSonarMixer->GetMixParam( MIX_STRIP_ANY, 0, MIX_PARAM_SELECTED, 0, &fValue ))
	{
		return SONAR_MIXER_STRIP(int(fValue));
	}

	return MIX_STRIP_ANY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTacomaSurface::SetStripFocus( SONAR_MIXER_STRIP type )
{
	if (!m_pSonarMixer)
		return;

	float fValue = float(int(type));
	m_pSonarMixer->SetMixParam( MIX_STRIP_ANY, 0, MIX_PARAM_SELECTED, 0, fValue, MIX_TOUCH_NORMAL );
}

/////////////////////////////////////////////////////////////////////////////
// Convenient helper Get the Active Bus in the host
DWORD CTacomaSurface::GetSelectedBus()
{
	if (!m_pSonarMixer)
		return DWORD(-1);

	float fVal;

	HRESULT hr = m_pSonarMixer->GetMixParam( MIX_STRIP_BUS, 0,MIX_PARAM_SELECTED, 0, &fVal );

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////
// Convenient helper to set the active track in the host
void CTacomaSurface::SetSelectedBus(DWORD dwStripNum)
{
	if (!m_pSonarMixer)
		return;

	m_pSonarMixer->SetMixParam( MIX_STRIP_BUS, 0, MIX_PARAM_SELECTED, 0, (float)dwStripNum, MIX_TOUCH_NORMAL );
}


//////////////////////////////////////////////////////////////////////
// Handle one of the 8 strip sel buttons.  These are used more for our
// internal mapping so they don't have attached mix parameters
bool CTacomaSurface::handleStripSelButton( CMidiMsg* pmsg, float fValue )
{
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;

	bool bHandled = false;
	if ( BID_IN( pmsg->GetId(), BID_Sel0, BID_Sel7 ) )
	{
		bHandled = true;
		const DWORD dwStripIndex = pmsg->GetId() - BID_Sel0;

		if ( m_eDisplayMode != SDM_ChannelBranch )
		{
			if (GetStripFocus() != m_selectedStrip.stripType)
				SetStripFocus(  m_selectedStrip.stripType ); // force tracks vs. bus

			if ( !doSelectStrip( dwStripIndex ) )
				return ( true );

			// lock / unlock strip?
			if ( bShift )
			{
				if ( m_pHostLockStrip  )
				{
					BOOL bLocked = FALSE;
					m_pHostLockStrip->IsLocked( m_dwSurfaceID, dwStripIndex, &bLocked );
					if ( bLocked )
					{
                  // Unlock the strip
						SONAR_MIXER_STRIP eStrip;
						DWORD dwStrip = 0;
						m_pHostLockStrip->GetLockedStripInfo( m_dwSurfaceID, dwStripIndex, &eStrip, &dwStrip );
						m_pHostLockStrip->Lock( m_dwSurfaceID, dwStripIndex, eStrip, dwStrip, FALSE );
                  updateParamBindings(); // Fix BUG55565
					}
					else
               {
                  // Lock the strip
						m_pHostLockStrip->Lock( m_dwSurfaceID, dwStripIndex, m_selectedStrip.stripType, m_selectedStrip.stripIndex, TRUE );
               }
				}			
			}
			else if ( bCommand )
			{
				if ( m_pHostLockStrip  )
				{
					// toggle the global lock bit for this surface
					BOOL bLocked = FALSE;
					m_pHostLockStrip->GetEnableLocking( m_dwSurfaceID, &bLocked );
					m_pHostLockStrip->SetEnableLocking( m_dwSurfaceID, !bLocked );
				}
			}
			else if ( bAlt )
				onChannelBranchMode( true );
			else if ( bCtrl )
			{
				if ( m_selectedStrip.stripType == MIX_STRIP_MASTER )
					return ( S_FALSE );

				// FX Mode?
				IHostPluginAccess* pHostPlugs = NULL;
				if ( SUCCEEDED( m_pSonarMixer->QueryInterface( IID_IHostPluginAccess, (void**)&pHostPlugs )  ) )
				{
					setDisplayMode( SDM_ExistingFX );
					pHostPlugs->Release();
					refreshPlugins( true );
					// clear SOLO indicators
					for ( size_t i = 0; i < 8; i++ )
					{
						const ControlId cId = (ControlId) ( BID_Solo0 + i );

						ControlMessageMapIterator it = m_mapControlMsgs.find( cId );
						if ( it != m_mapControlMsgs.end() )
						{
							stopBlink( cId );
							CMidiMsg* pmsg = it->second;
							pmsg->Send( 0.f );
						}
					}
				}
			}
			else if ( ( SDM_FXTree == m_eDisplayMode ) || ( SDM_ExistingFX == m_eDisplayMode ) )
				endInsertPlugin();
		}
		else if ( bAlt )
			onChannelBranchMode( false );
		else if ( dwStripIndex == 0 )
		{
			// select this channel for the surround panners
			const SONAR_MIXER_STRIP type = m_selectedStrip.stripType;
			const DWORD dwTrackIndex = m_selectedStrip.stripIndex;
			m_Surround.pParamAngle->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_ANGLE, 0 );
			m_Surround.pParamFocus->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_FOCUS, 0 );
			m_Surround.pParamFRBal->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_FR_BAL, 0 );
			m_Surround.pParamWidth->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_WIDTH, 0 );
			m_Surround.pParamLFE->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_LFE, 0 );
			lightNthSEL( dwStripIndex, 1 );
		}
		else // we are in channel branch mode and selecting a send strip
		{
			// 'inserting' actually means that the send's destination is being edited.
			// If we're here, we want to either:
			// 1. insert a send and go into 'inserting' mode - if SEL was pressed on the first empty strip
			// 2. end inserting (changing the destination) - if SEL pressed on strip that is in 'inserting' mode
			// 3. go into 'inserting' mode - if CTRL+SEL pressed on an existing send
			// 4. simply select this send for the surround panner - if SEL is pressed on an existing send

			if ( m_nInserting != 0 )
				endChannelBranchInserting();
			else
			{
				// if 1 higher than the sends count, insert a new send and put that into insert mode
				float fVal = 0.f;
				if ( !SUCCEEDED( m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SENDCOUNT, 0, &fVal ) ) )
					return ( true );

				const DWORD dwSendIndex = dwStripIndex - 1;
				if ( dwSendIndex > ( (DWORD) fVal ) )
					return ( true ); // nothing to do - can only insert the next available

				bool bInsertMode = bCtrl;

				// is this the next available strip?
				if ( dwSendIndex == ( (DWORD) fVal ) )
				{
					// re-select the track we think should be selected - as the insert works on the selected track only
					if ( !doSelectStrip( m_selectedStrip.stripIndex ) )
						return ( true );

					// insert send
					fVal = 1.f;
					if ( !SUCCEEDED( m_pSonarMixer->SetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_INSERT, 0, fVal, MIX_TOUCH_NORMAL ) ) )
						return ( true );

					bInsertMode = true; // always go into insert mode on a send that is just inserted
				}

				if ( bInsertMode )
				{
					// remap rotaries to control send destination
					if ( !SUCCEEDED( m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_OUTPUT, dwSendIndex, &fVal ) ) )
						fVal = -1.f;

					m_fSendDest = fVal;
					m_nInserting = dwStripIndex;
					AssignChannelBranchRotaries();

					// display name
					showText( "SendDst", m_nInserting + 4, 0 );
					char sz[64];
					memset( sz, 0, sizeof ( sz ) );
					DWORD csz = _countof( sz );
					m_fSendDest ++;
					if ( !SUCCEEDED( m_pSonarMixer->GetMixParamValueText(	m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_OUTPUT,
																							dwSendIndex, m_fSendDest --, sz, &csz ) ) )
						return ( true );

					CStringCruncher cCruncher;
					char szBuf[8];
					memset( szBuf, 0, 8 );
					cCruncher.CrunchString( sz, szBuf, 7 );
					showText( szBuf, m_nInserting + 4, 1 );

					lightNthSEL( m_nInserting, 2 ); // start blink
				}
				else
				{
					// just select this send for the surround panner
					const SONAR_MIXER_STRIP type = m_selectedStrip.stripType;
					const DWORD dwTrackIndex = m_selectedStrip.stripIndex;
					m_Surround.pParamAngle->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_SENDANGLE, dwSendIndex );
					m_Surround.pParamFocus->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_SENDFOCUS, dwSendIndex );
					m_Surround.pParamFRBal->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_SENDFR_BAL, dwSendIndex );
					m_Surround.pParamWidth->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_SENDWIDTH, dwSendIndex );
					m_Surround.pParamLFE->SetParams( type, dwTrackIndex, MIX_PARAM_SURROUND_SENDLFE, dwSendIndex );

					lightNthSEL( dwStripIndex, 1 );
				}
			}
		}
	}

	return bHandled;
}


//---------------------------------------------------------------------
void CTacomaSurface::setDisplayMode( SpecialDisplayModes eMode )
{
	m_eDisplayMode = eMode;
	updateParamStateOnLCD();
	updateParamBindings();
}


void CTacomaSurface::SetIOMode( bool b )
{ 
	m_bIOMode = b; 
	initLEDs(); 
}

///////////////////////////////////////////////////////////////////////////////////////

STRIP_INFO CTacomaSurface::MapStripInfo( DWORD dwCurStrip  )
{
	DWORD dwPhysicalStrip = dwCurStrip - getBankOffset( m_e8StripType );
	
   STRIP_INFO info;

   info.stripIndexPhysical = dwPhysicalStrip;

   BOOL bLockingEnabled = FALSE;
	if ( m_pHostLockStrip )
	{
      if ( S_OK == m_pHostLockStrip->GetEnableLocking( m_dwSurfaceID, &bLockingEnabled ))
      {
         if ( bLockingEnabled )
         {
            // Global locking is on, so look further to see if this particular strip is locked
		      if ( S_OK == m_pHostLockStrip->GetLockedStripInfo( m_dwSurfaceID, dwPhysicalStrip, &info.stripType, &info.stripIndex ) )
		      {
               info.isStripLocked = TRUE;
               info.stripOffset = getBankOffset( info.stripType );

               return info;
		      }
         }
      }
	}

	// else = return the non-locked version (without regard for master)
	info.stripType = m_e8StripType;
	info.stripIndex = getBankOffset(info.stripType) + dwPhysicalStrip;
   info.isStripLocked = FALSE;
   info.stripOffset = getBankOffset( info.stripType );

   return info;
}

//-----------------------------------------------------------------------------------
// Helper to obtain the true Strip type and number based on a physical strip index.
// This takes into account any strip locking on the host side
STRIP_INFO CTacomaSurface::getStripInfo( DWORD dwPhysicalStrip )
{
   STRIP_INFO info;

   info.stripIndexPhysical = dwPhysicalStrip;

   BOOL bLockingEnabled = FALSE;
	if ( m_pHostLockStrip )
	{
      if ( S_OK == m_pHostLockStrip->GetEnableLocking( m_dwSurfaceID, &bLockingEnabled ))
      {
         if ( bLockingEnabled )
         {
            // Global locking is on, so look further to see if this particular strip is locked
		      if ( S_OK == m_pHostLockStrip->GetLockedStripInfo( m_dwSurfaceID, dwPhysicalStrip, &info.stripType, &info.stripIndex ) )
		      {
               info.isStripLocked = TRUE;
               info.stripOffset = getBankOffset( info.stripType );

               return info;
		      }
         }
      }
	}

	// else = return the non-locked version
	const bool bMasterFader = ( dwPhysicalStrip == 8 );
	info.stripType = bMasterFader ? MIX_STRIP_BUS : m_e8StripType;
	info.stripIndex = bMasterFader ? m_dwMasterOrg : getBankOffset(info.stripType) + dwPhysicalStrip;
   info.isStripLocked = FALSE;
   info.stripOffset = getBankOffset( info.stripType );

   return info;
}


////////////////////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleModifierButton( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = true;
	CString str;
	WORD wBit = 0;
	switch( pmsg->GetId() )
	{
	case 	BID_Shift:
		wBit = SMK_SHIFT;
		str = "SMK_SHIFT: ";
		break;
	case 	BID_Control:
		wBit = SMK_CTRL;
		str = "SMK_CTRL: ";
		break;
	case 	BID_Alt:
		wBit = SMK_ALT;
		str = "SMK_ALT: ";
		break;
	case 	BID_Command:
		wBit = SMK_COMMAND;
		str = "SMK_COMMAND: ";
		break;
	default:
		bHandled = false;
	}

	ClipEditFunction oldCef = getEditMode();

	if ( bHandled )
	{
		if ( TRUEFLOAT(fValue) )
		{
			m_wModifierKey |= wBit;
			str += "pressed\n";
		}
		else
		{
			m_wModifierKey &= (~wBit);
			str += "released\n";

			if ( wBit == SMK_ALT && ( ( JM_SelByTime == m_eJogMode ) || ( JM_SelByClips == m_eJogMode ) ) )
				m_bSelectingOrEditing = FALSE;
			else if ( wBit == SMK_COMMAND )
				m_ptGrabbed.x = -1;
		}

//		TRACE( str );
	}

	if ( wBit == SMK_SHIFT )
	{
		toggleStepCapture();

		if ( ( JM_Edit == m_eJogMode ) && ( m_cefCurrent != CEF_NUDGE ) )
		{
			ClipEditFunction cef = getEditMode();
			TRACE( _T("Edit mode = %d, current = %d\n"), cef, oldCef );
			if ( cef != oldCef )
			{
				endEdit();
				m_pHostDataEdit->SetMode( DEM_EDIT, &cef );
				m_pHostDataEdit->BeginEdit( cef );
			}
		}
	}

	initLEDs();

	return bHandled;
}

///////////////////////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleCommandOrKey( CMidiMsg* pmsg, float fValue )
{
	const bool bShift   = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	const bool bAlt     = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCtrl    = (m_wModifierKey & SMK_CTRL) != 0;

	MsgIdToBADMapIterator it = m_mapButton2BAD.find( (ControlId)pmsg->GetId() );
	if ( m_mapButton2BAD.end() != it )
	{
		ButtonActionDefinition& bad = it->second;
		if ( bad.eActionType == ButtonActionDefinition::AT_Key )
		{
			// handle as a Key
			if( !m_pSonarKeyboard )
				return true;

			if ( pmsg->IsTrigger() )
			{
				if ( bad.dwCommandOrKey == VK_ESCAPE && m_bSelectingOrEditing )
					endEdit( FALSE );

				//Toggle the Multi-dock ( Expand and Collapse )
				if ( bCtrl && !bShift && ( bad.dwCommandOrKey == VK_F4 ) )
					m_pSonarCommands->DoCommand( CMD_TOGGLE_MULTCMDOCK );
				//Close the focus Tab in the Multi-dock
				else if ( bShift && !bCtrl && ( bad.dwCommandOrKey == VK_F4 ) )
					m_pSonarCommands->DoCommand( CMD_ID_CLOSE_CURRENT_TAB );
				//Tab over to the next view in the Multi-Dock
				else if ( bShift && !bCtrl && ( bad.dwCommandOrKey == VK_TAB ) )
					m_pSonarCommands->DoCommand( CMD_ID_NEXT_MDITAB );
				//Maximize current Tab
				else if ( bShift && bCtrl && ( bad.dwCommandOrKey == VK_F4 ) )
					m_pSonarCommands->DoCommand( CMD_MAX_TAB );
				else
				{
					// for Triggers, do Down/Up surrounded by modifier keys
					if ( bad.wModKeys & SMK_SHIFT )
						m_pSonarKeyboard->KeyboardEvent( VK_SHIFT, SKE_KEYDOWN );
					if ( bad.wModKeys & SMK_CTRL )
						m_pSonarKeyboard->KeyboardEvent( VK_CONTROL, SKE_KEYDOWN );

					m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYDOWN );
					m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYUP );

					if ( bad.wModKeys & SMK_SHIFT )
						m_pSonarKeyboard->KeyboardEvent( VK_SHIFT, SKE_KEYUP );
					if ( bad.wModKeys & SMK_CTRL )
						m_pSonarKeyboard->KeyboardEvent( VK_CONTROL, SKE_KEYUP );
				}

			}
			else
			{
				// We get separate Down Up gestures
				ControlId cid = (ControlId)pmsg->GetId();
				const bool bShuttleKey = BID_IN( cid, BID_KeyUp, BID_KeyR);
				const bool bRawKey = ( ( JM_Standard == m_eJogMode && !bCommand ) || !bShuttleKey );

				// start or stop the Key Rep timer
				if ( TRUEFLOAT( fValue ) )
				{
					setTimer( m_tctKeyRep, 100 );
					m_tctKeyRep.SetButton( pmsg->GetId() );
				}
				else
				{
					m_tctKeyRep.SetButton( 0 );
					killTimer( m_tctKeyRep );
				}

				if ( bRawKey )
				{
					// regular key
					if ( TRUEFLOAT( fValue ))
					{
						// send the key down
						m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYDOWN );
					}
					else
					{
						// send the key up
						m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYUP );
					}
				}
				else if ( bShuttleKey && TRUEFLOAT( fValue ) )
				{
					// special case for Cursor keys
					doCursorKeyEdit( cid );
				}	
			}
		}
		else if ( bad.eActionType == ButtonActionDefinition::AT_Command )
		{
			if ( m_pSonarCommands )
			{
				if ( bad.dwCommandOrKey == CMD_VIEW_SYNTH_RACK )
				{
					if ( m_pSonarUIContext )
					{
						if ( bCommand )
                     m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, m_selectedStrip.stripIndex, MIX_PARAM_FX_PARAM, -1, UIA_OPEN );
						else
						{
							if ( !bAlt && !bShift )
							{
								m_pSonarCommands->DoCommand( CMD_VIEW_SYNTH_RACK );
							}

							if ( bShift )
							{
								m_pSonarCommands->DoCommand( CMD_VIEW_NEW_EXPLORER );
							}

							if ( !SUCCEEDED( m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, m_selectedStrip.stripIndex, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS ) ) )
								m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, -1, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS );
						}
					}
				}
				else if ( bad.dwCommandOrKey == CMD_TRACK_VIEW )
				{
					if ( bCtrl )
					{
						if ( bAlt )
							m_pSonarCommands->DoCommand( CMD_INSERT_MIDI_TRACK );
						else
							m_pSonarCommands->DoCommand( CMD_INSERT_AUDIO_TRACK );
					}
					else if (bCommand)
					{
						m_pSonarCommands->DoCommand( CMD_APPEND_BUS );
					}
					else
					{
						m_pSonarCommands->DoCommand( CMD_TRACK_VIEW );
					}
				}
				else if ( bad.dwCommandOrKey == CMD_EDIT_DELETE )
				{
					if (bCommand)
					{
						if ( MIX_STRIP_TRACK == m_selectedStrip.stripType  )
						{
							m_pSonarCommands->DoCommand( CMD_DELETE_TRACK );
						}
					}
					else
					{
						m_pSonarCommands->DoCommand( CMD_EDIT_DELETE );
					}
				}
				else if ( bad.dwCommandOrKey == CMD_TRACK_FREEZE )
				{
					if ( MIX_STRIP_TRACK == m_selectedStrip.stripType  )
					{
						DWORD dwSel = GetSelectedTrack();
						bool bMidi = GetIsStripMidiTrack( MIX_STRIP_TRACK, dwSel );

						if ( bShift )
							m_pSonarCommands->DoCommand( bMidi ? CMD_SYNTH_UNFREEZE : CMD_TRACK_UNFREEZE );
						else if ( bCommand )
							m_pSonarCommands->DoCommand( CMD_TRACK_FREEZE_OPTIONS );
						else if ( bAlt )
							m_pSonarCommands->DoCommand( bMidi ? CMD_SYNTH_QUICK_UNFREEZE : CMD_TRACK_QUICK_UNFREEZE );
						else
							m_pSonarCommands->DoCommand( bMidi ? CMD_SYNTH_FREEZE : CMD_TRACK_FREEZE );
					}
				}
				else if ( ( bad.dwCommandOrKey == CMD_VIEW_CONSOLE ) && bShift && m_pHostWindow )
				{
					m_pSonarCommands->DoCommand( CMD_VIEW_CONSOLE );
					m_pSonarCommands->DoCommand( CMD_MAX_TAB );
					//m_pHostWindow->DoWindowAction( WT_MIXER, WA_MAXIMIZE, 0, ZP_NONE );
				}
				else if ( ( bad.dwCommandOrKey == CMD_VIEW_NEW_PIANO_ROLL ) && bShift && m_pHostWindow )
				{
					m_pSonarCommands->DoCommand( CMD_VIEW_NEW_PIANO_ROLL );
					m_pHostWindow->DoWindowAction( WT_MIDI, WA_MAXIMIZE, 0, ZP_NONE );
				}
				// if it's a trigger or the button is being released, do the command
				else if ( pmsg->IsTrigger() || fValue < .5f )
					m_pSonarCommands->DoCommand( bad.dwCommandOrKey );
			}
		}
		else if ( bad.eActionType == ButtonActionDefinition::AT_Transport )
		{
			if ( m_pSonarTransport )
			{
				BOOL bState = 0;
				m_pSonarTransport->GetTransportState( bad.transportState, &bState );
				m_pSonarTransport->SetTransportState( bad.transportState, !bState );
			}
		}

		return true;
	}
	else if ( BID_UndoRedo == pmsg->GetId() )
	{
		endEdit();

		if ( bCommand )
		{
			if ( bShift )
				m_pSonarCommands->DoCommand( CMD_ZOOM_REDO );
			else
				m_pSonarCommands->DoCommand( CMD_ZOOM_UNDO );
		}
		else if ( bShift )
			m_pSonarCommands->DoCommand( CMD_EDIT_REDO );
		else
			m_pSonarCommands->DoCommand( CMD_EDIT_UNDO );
		return true;
	}
	else if ( BID_Save == pmsg->GetId() )
	{
		if (bShift)
			m_pSonarCommands->DoCommand( CMD_FILE_SAVE_AS );
		else
			m_pSonarCommands->DoCommand( CMD_FILE_SAVE );
		return true;
	}
	else if (BID_Send == pmsg->GetId() && bCommand )
	{
		if ( MIX_STRIP_TRACK == m_selectedStrip.stripType  )
		{
			DWORD dwSel = GetSelectedTrack();
			bool bMidi = GetIsStripMidiTrack( MIX_STRIP_TRACK, dwSel );
			if (!bMidi)
			{
				m_pSonarCommands->DoCommand( CMD_INSERT_SEND_ASSISTANT );
			}

			return true;
		}
	}
	else if (BID_RotaryAssign == pmsg->GetId() && bCommand )
	{
		m_pSonarCommands->DoCommand( CMD_OPTIONS_AUDIOMETERSETTINGS );
		return true;	
	} 

	return false;
}


/////////////////////////////////////////////////////////////////////
void CTacomaSurface::doCursorKeyEdit( ControlId cid )
{
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;

	JogMode jmEffective = m_eJogMode;
	if ( bCommand )
		jmEffective = bAlt ? JM_Scroll : JM_Zoom;

	if ( JM_SelByTime == jmEffective || JM_SelByClips == jmEffective )
	{
		EditCursorDirection dir;
		switch( cid )
		{
			case BID_KeyUp:	dir = ECD_UP;		break;
			case BID_KeyDn:	dir = ECD_DOWN;	break;
			case BID_KeyL:		dir = ECD_LEFT;	break;
			case BID_KeyR:		dir = ECD_RIGHT;	break;
			default:	return;
		}
		if ( m_pHostDataEdit )
		{
			const bool bByClips = ( JM_SelByClips == jmEffective ) xor bShift;
			const bool bKeepOldSel = m_bSelectingOrEditing || !( bByClips xor bAlt );
			if ( bAlt )
				m_bSelectingOrEditing = TRUE;
			const DWORD dwEcf = ( m_bSelectingOrEditing ? ECF_SELECT : ECF_CLEAR ) | 
									  ( bByClips ? ECF_SNAPTOCLIPS | ECF_SELECT : ECF_CLEAR ) |
									  ( bKeepOldSel ? ECF_KEEP_SELECTION : ECF_CLEAR );
			m_pHostDataEdit->MoveDataCursor( dir, dwEcf, -2 );
		}
	}
	else if ( JM_Scroll == jmEffective || JM_Zoom == jmEffective )
	{
		EMoveOperator wo;
		WindowAction wa;
		bool bScroll = ( JM_Scroll == jmEffective ) != bShift;
		switch( cid )
		{
		case BID_KeyUp:
			wa = bScroll ? WA_SCROLLV : WA_ZOOMV;
			wo = bScroll ? MO_DECREASE_SMALL : MO_INCREASE_SMALL;
			break;
		case BID_KeyDn:
			wa = bScroll ? WA_SCROLLV : WA_ZOOMV;
			wo = bScroll ? MO_INCREASE_SMALL : MO_DECREASE_SMALL;
			break;
		case BID_KeyL:
			wa = bScroll ? WA_SCROLLH : WA_ZOOMH;
			wo = MO_DECREASE_SMALL;
			break;
		case BID_KeyR:
			wa = bScroll ? WA_SCROLLH : WA_ZOOMH;
			wo = MO_INCREASE_SMALL;
			break;
		default:
			return;
		}
		if ( m_pHostWindow )
		{
			if ( !bCommand )
				m_pHostWindow->DoWindowAction( WT_CLIPS, wa, wo, bScroll ? SP_CENTER_DATACURSOR : ZP_DATACURSOR );
			else
				m_pHostWindow->DoWindowAction( WT_CLIPS, wa, wo, bScroll ? SP_NONE : ZP_SCREENCENTER );

		}
	}
	if ( jmEffective == JM_Loop || jmEffective == JM_Punch )
	{
		if ( m_pSonarTransport2 )
		{
			UTransportMoveParams params;
			switch( cid )
			{
				case BID_KeyL:
					params.eOp = MO_DECREASE_SMALL;
				break;
				case BID_KeyR:
					params.eOp = MO_INCREASE_SMALL;
				break;
				default:
				return;
			}
			SONAR_TRANSPORT_TIME time = m_bEditRight ? TRANSPORT_TIME_LOOP_OUT : TRANSPORT_TIME_LOOP_IN;
			if ( jmEffective == JM_Punch )
				time = m_bEditRight ? TRANSPORT_TIME_PUNCH_OUT : TRANSPORT_TIME_PUNCH_IN;

			m_pSonarTransport2->MoveTransportTime( time, TMF_NONE, params );
		}
	}
	else if ( JM_Edit == jmEffective )
	{
		if ( m_cefCurrent != CEF_NUDGE )
		{
			// when command is pressed, do the current edit action
			ClipEditOperator ceo;
			switch( cid )
			{
			case BID_KeyL:
				ceo = CEO_LEFT_SLOW;
				break;
			case BID_KeyR:
				ceo = CEO_RIGHT_SLOW;
				break;
			default:
				return;
			}
			if ( m_pHostDataEdit )
			{
				ClipEditFunction func = getEditMode();
				if ( SUCCEEDED( m_pHostDataEdit->DoEdit( func, ceo, bCtrl ? EP_TIME_STRETCH : EP_NONE ) ) )
					m_bSelectingOrEditing = true;
			}
		}
		else
		{
			// nudge functions
			DWORD dwCmd = 0, dwParams = 0;
			ClipEditOperator ceo;
			switch( cid )
			{
				case BID_KeyL:
					ceo = CEO_LEFT_SLOW;

					dwCmd = CMD_NUDGE_LEFT1;
					if ( bCtrl )
						dwCmd = CMD_NUDGE_LEFT2;
					else if ( bAlt )
						dwCmd = CMD_NUDGE_LEFT3;
				break;
				case BID_KeyR:
					ceo = CEO_RIGHT_SLOW;

					dwCmd = CMD_NUDGE_RIGHT1;
					if ( bCtrl )
						dwCmd = CMD_NUDGE_RIGHT2;
					else if ( bAlt )
						dwCmd = CMD_NUDGE_RIGHT3;
				break;
				case BID_KeyUp:
					ceo = CEO_LEFT_SLOW;
					dwParams = EP_VERTICAL;

					dwCmd = CMD_NUDGE_UP;
				break;
				case BID_KeyDn:
					ceo = CEO_RIGHT_SLOW;
					dwParams = EP_VERTICAL;

					dwCmd = CMD_NUDGE_DOWN;
				break;
				default:
					return;
			}
			if ( !m_bSelectingOrEditing )
			{
				if ( m_pSonarCommands )
					m_pSonarCommands->DoCommand( dwCmd );
			}
			else
				m_pHostDataEdit->DoEdit( CEF_NUDGE, ceo, dwParams );
		}
	}
}

///////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleRecEditTools( CMidiMsg* pmsg, float fVal )
{
	bool bHandled = true;

	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;

	switch( pmsg->GetId() )
	{
		case BID_LeftMarker:
			onMarker( true, fVal );
		break;
		case BID_RightMarker:
			onMarker( false, fVal );
		break;
		case BID_SetLoopPunch:
			if ( m_eJogMode == JM_Loop || m_eJogMode == JM_Punch )
				onJogMode( JM_Standard );
			else if ( m_bSelectingOrEditing && ( m_eJogMode == JM_Edit ) )
				endEdit();
			else if ( bAlt && m_pHostWindow )
				m_pHostWindow->DoWindowAction( WT_CLIPS, WA_ZOOMVH, 0, ZP_FITSELECTION );
			else if ( bCommand && m_pSonarCommands )
				m_pSonarCommands->DoCommand( CMD_FIT_PROJECT );
			else if ( m_pHostDataEdit && ( m_eJogMode != JM_Standard ) )
				m_pHostDataEdit->MoveDataCursor( ECD_STILL, ECF_SELECT | ECF_SNAPTOCLIPS | ( bCtrl ? ECF_KEEP_SELECTION : ECF_CLEAR ), 0 );
		break;
		case BID_Snap:
			if ( m_pSonarCommands )
			{
				if ( bCommand )
					m_pSonarCommands->DoCommand(  CMD_NUDGE_SETTINGS );
				else
					m_pSonarCommands->DoCommand( CMD_SNAP_TIME );
			}
		break;
		case BID_MarkerIns:
			if ( ( SDM_Markers != m_eDisplayMode ) && bShift )
				onMarkersDisplay( true );
			else if ( bCommand )
				m_pSonarCommands->DoCommand( CMD_VIEW_MARKERS );
			else
			{
				if ( SDM_Markers == m_eDisplayMode )
					onMarkersDisplay( false );
				else
					m_pSonarCommands->DoCommand( CMD_INSERT_MARKER );
			}
		break;
		case BID_LoopOn:
			if ( m_eJogMode == JM_Loop )
				onJogMode( JM_Standard );
			else if ( m_eJogMode == JM_Punch )
				onJogMode( JM_Loop );
			else if ( bAlt )
				m_pSonarCommands->DoCommand( CMD_SET_LOOP_FROM_SELECTION );
			else if ( bShift )
				onJogMode( JM_Loop );
			else
				m_pSonarCommands->DoCommand( CMD_LOOP_TOGGLE );
		break;
		case BID_PunchOn:
			if ( m_eJogMode == JM_Punch )
				onJogMode( JM_Standard );
			else if ( m_eJogMode == JM_Loop )
				onJogMode( JM_Punch );
			else if ( bCommand )
				m_pSonarCommands->DoCommand( CMD_REALTIME_RECORD_MODE );
			else if ( bAlt )
				m_pSonarCommands->DoCommand( CMD_SET_PUNCH_FROM_SELECTION );
			else if ( bShift )
				onJogMode( JM_Punch );
			else
				 m_pSonarCommands->DoCommand( CMD_AUTOPUNCH_TOGGLE );
		break;
		default:
			bHandled = false;
		break;
	}

	if ( bHandled )
		initLEDs();

	return bHandled;
}

///////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleModeButton( CMidiMsg* pmsg, float fVal )
{
	bool bHandled = true;

	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;

	int i = 0;

	switch( pmsg->GetId() )
	{
	case BID_Act:
		m_eActSectionMode = KSM_ACT;
		SetProChanLock( false );
		if ( bCommand )
		{
			if (m_pSonarUIContext)
			{
				SONAR_UI_CONTAINER uiContainer = CNR_PROCHANNEL;
				if (SUCCEEDED( m_pSonarUIContext->GetUIContextEx( m_dwSurfaceID, &uiContainer ) ))
				{
					if (uiContainer != CNR_USER_BIN)
					{
						m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_USER_BIN );
					}
				}

				// get any act mapping to get the parameter identification
				if ( !m_mapACTControls.empty() )
				{
					SONAR_UI_ACTION uia = bShift ? UIA_CLOSE : UIA_OPEN;
					ActivateCurrentFx( uia );
				}
			}
		}
		else if ( bShift )
         SetLockDynamicMappings( !GetLockDynamicMappings() );
		else if ( bCtrl )
			m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, 0, MIX_PARAM_FX_PARAM, 0, UIA_FOCUS );
      else
		{
			if (m_pSonarUIContext)
			{
				SONAR_UI_CONTAINER uiContainer = CNR_PROCHANNEL;
				if (SUCCEEDED( m_pSonarUIContext->GetUIContextEx( m_dwSurfaceID, &uiContainer ) ))
				{
					if (uiContainer != CNR_USER_BIN)
					{
						m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_USER_BIN );
					}
#if 0 // User is used to pressing Command + Page> or Page<
					else
					{
						m_pSonarUIContext->SetNextUIContext( m_dwSurfaceID, UIA_FOCUS );
					}
#endif
				}					
			}
		}

		resetChannelStrip();
		initLEDs();
		break;

	case BID_Send:
		m_eActSectionMode = KSM_SEND;
		resetChannelStrip();
		initLEDs();
		initLCDs();
		break;

	case BID_Eq:
		SetProChanLock();
		if ( bCommand )
		{
			if (IsFlexibleProChan()) // X1d (and Expanded and future)
			{
				m_eActSectionMode = KSM_FLEXIBLE_PC;
			}

			if ( m_pSonarUIContext )
			{
				m_pSonarUIContext->SetUIContext( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FILTER, 0, UIA_OPEN );
			}
		}
		else
		{
			// legacy
			if (IsLegacyEQ())
			{
				m_eActSectionMode = KSM_EQ;
			}
			else if (IsFlexibleProChan()) // X1d (and Expanded and future)
			{
				if (m_pSonarUIContext)
				{
					m_eActSectionMode = KSM_FLEXIBLE_PC;

					SONAR_UI_CONTAINER uiContainer = CNR_PROCHANNEL;
					if (SUCCEEDED( m_pSonarUIContext->GetUIContextEx( m_dwSurfaceID, &uiContainer ) ))
					{
						if (uiContainer != CNR_PROCHANNEL)
						{
							m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_PROCHANNEL );
						}
						else
							m_pSonarUIContext->SetNextUIContext( m_dwSurfaceID, UIA_FOCUS );
					}					
				}

			}
			else // it's pre X1d
			{
				//Navigate through modules
				if ( ( m_eActSectionMode == KSM_EQ ) || ( m_eActSectionMode == KSM_ACT ) || ( m_eActSectionMode == KSM_SEND ))
					m_eActSectionMode = KSM_PC_COMP;
				else if ( m_eActSectionMode == KSM_PC_COMP )
					m_eActSectionMode = KSM_PC_EQ;
				else if ( m_eActSectionMode == KSM_PC_EQ )
					m_eActSectionMode = KSM_PC_SAT;
				else if ( m_eActSectionMode == KSM_PC_EQ_P2 )
					m_eActSectionMode = KSM_PC_SAT;
				else if ( m_eActSectionMode == KSM_PC_SAT )
					m_eActSectionMode = KSM_PC_COMP;
			}
		}

		resetChannelStrip();
		initLEDs();
		initLCDs();

		if (!IsFlexibleProChan())
			updatePCContextName( true );
		break;

	case BID_Flip:
		OnFlip();
		break;

	case BID_ACTDisplay:
		i = (int)m_wACTDisplayRow;
		i = bShift ? i - 1 : i + 1;
		if ( i < 0 ) i = 3;
		if ( i > 3 ) i = 0;
		m_wACTDisplayRow = (WORD)i;
		updateActSectionBindings( true );

		// handle the Display Row LED
		initLEDs();
		break;

	case BID_PageL:
		if ( bCommand )
			ActivatePrevFx( UIA_FOCUS );
		else if ( bCtrl )
			m_pSonarCommands->DoCommand( CMD_PLUGIN_PRESET_DEC );
		else if ( m_eActSectionMode == KSM_PC_EQ_P2 )
		{
			m_eActSectionMode = KSM_PC_EQ;
			resetChannelStrip();
			initLEDs();
			initLCDs();
			updatePCContextName( true );
		}
		else
		{
			i = ( (int) m_wACTPage ) - 1;
			if ( i < 0 )
				i = ACT_PAGE_NUM - 1;
			m_wACTPage = (WORD) max( i, 0 );
			updateActSectionBindings( true );
			resetChannelStrip();
		}
		break;

	case BID_PageR:
		if ( bCommand )
			ActivateNextFx( UIA_FOCUS );
		else if ( bCtrl )
			m_pSonarCommands->DoCommand( CMD_PLUGIN_PRESET_INC );
		else if ( m_eActSectionMode == KSM_PC_EQ )
		{
			m_eActSectionMode = KSM_PC_EQ_P2;
			resetChannelStrip();
			initLEDs();
			initLCDs();
			updatePCContextName( true );
		}
		else
		{
			i = ( (int) m_wACTPage ) + 1;
			if ( i >= ACT_PAGE_NUM )
				i = 0;
			m_wACTPage = (WORD) min( ACT_PAGE_NUM - 1, i );
			updateActSectionBindings( true );
			resetChannelStrip();
		}
		break;

	case BID_RotaryAssign:
		{
			if ( SDM_ChannelBranch == m_eDisplayMode )
				break;

			SONAR_MIXER_PARAM smp = MIX_PARAM_PAN;

			PSEncoderParams pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, getModeForRotary() );
			if ( !pEnc )
				pEnc = GetEncoderListParam( 0, MIX_STRIP_ANY, getModeForRotary() );

			if ( !pEnc )
				break; // no encoder params for this strip type

         DWORD dwNextIndex = pEnc->dwIndex + 1;
			if ( dwNextIndex >= getCountByStripType( getModeForRotary() ) )
            dwNextIndex = 0;

         pEnc = GetEncoderListParam( dwNextIndex, MIX_STRIP_ANY, getModeForRotary() );
			if ( pEnc )
			{
				if ( !m_bIOMode )
					AssignRotaries( pEnc->mixParam, pEnc->dwParam, pEnc->dwIndex );
				else
				{
					m_dw8EncoderParam = pEnc->dwIndex;
					showIoCtrlText();

					for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
					{
						PMBINDING& pmb = it->second;
						CMidiMsg* pmsg = it->first;

						const TacomaIOBoxParam p = m_bFlipped ? TIOP_Gain : pEnc->ioParam;
						const float fVal = m_pIOBoxInterface->GetParam( pmb.pParam->GetStripPhysicalIndex(), p );

						// set the underlying MIDI msg objects' dwCurrent value correctly
						pmsg->SetVal( fVal );
					}
				}
			}
		}
		break;

	case BID_RudeMute:
		ClearMute();
		break;

	case BID_RudeSolo:
		if ( bShift )
			m_pSonarCommands->DoCommand( CMD_DIM_SOLO );
		else if ( bCtrl )
			m_pSonarCommands->DoCommand( CMD_EXC_SOLO );
		else
			ClearSolo();
		break;

	case BID_RudeArm:
		ClearArm();
		break;
	case BID_Tracks:
		{
			Set8StripType( MIX_STRIP_TRACK );
			DWORD dwTrack = GetSelectedTrack();
			if (dwTrack == DWORD( -1 ))
				dwTrack = 0; // use first track if none

			SONAR_MIXER_STRIP type = GetStripFocus();
			if (type != m_selectedStrip.stripType || m_selectedStrip.stripType != MIX_STRIP_TRACK)
			{
				SetStripFocus( MIX_STRIP_TRACK );
				doSelectStrip( dwTrack - getBankOffset( MIX_STRIP_TRACK ) );
			}
			else if (dwTrack != m_selectedStrip.stripIndex )
			{		
				doSelectStrip( dwTrack - getBankOffset( MIX_STRIP_TRACK ) );
			}

			if ( bShift )
			{
				onLayersMode( true );
			}

			break;
		}

	case BID_Buses:
		{
			Set8StripType( MIX_STRIP_BUS );
			DWORD dwBus  = GetSelectedBus();
			if (dwBus == DWORD( -1 ))
				dwBus = 0; // use first track if none

			SONAR_MIXER_STRIP type = GetStripFocus();
			if (type != m_selectedStrip.stripType || m_selectedStrip.stripType != MIX_STRIP_BUS)
			{
				SetStripFocus( MIX_STRIP_BUS );
				doSelectStrip( dwBus - getBankOffset( MIX_STRIP_BUS ) );
			}
			else if (dwBus != m_selectedStrip.stripIndex )
			{		
				doSelectStrip( dwBus - getBankOffset( MIX_STRIP_BUS ) );
			}

			break;
		}

	case BID_Masters:
		Set8StripType( MIX_STRIP_MASTER );
		break;

	case BID_Automation:
		if ( bShift )
		{
			if ( bCommand )
			{
            ISonarMixer2* pm2 = NULL;
            if ( FAILED( m_pSonarMixer->QueryInterface( IID_ISonarMixer2, (void**)&pm2 ) ) )
               break;

            const DWORD dwCount = GetStripCount( m_e8StripType );
            BOOL bRead = FALSE;
            pm2->GetReadMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_ANY, 0, &bRead );
				bRead = !bRead;
				for ( DWORD dwCntr = 0; dwCntr < dwCount; dwCntr ++ )
            {
               pm2->SetReadMixParam( m_e8StripType, dwCntr, MIX_PARAM_ANY, 0, bRead );
            	pm2->SetReadMixParam( MIX_STRIP_RACK, dwCntr, MIX_PARAM_FX_PARAM, -1, bRead );
            }

            pm2->Release();
			}
			else
				AllRead( m_selectedStrip.stripType, m_selectedStrip.stripIndex, true );
		}
		else if ( bCommand )
		{
			const DWORD dwCount = GetStripCount( m_e8StripType );
         BOOL bWrite = FALSE;
         m_pSonarMixer->GetArmMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_ANY, 0, &bWrite );
			bWrite = !bWrite;
			for ( DWORD dwCntr = 0; dwCntr < dwCount; dwCntr ++ )
         {
            m_pSonarMixer->SetArmMixParam( m_e8StripType, dwCntr, MIX_PARAM_ANY, 0, bWrite );
            m_pSonarMixer->SetArmMixParam( MIX_STRIP_RACK, dwCntr, MIX_PARAM_FX_PARAM, -1, bWrite );
         }
		}
		else
		{
			AllWrite( m_selectedStrip.stripType, m_selectedStrip.stripIndex, true );
		}
	break;

	case BID_Snapshot:
		if ( bShift )
		{
			m_pSonarCommands->DoCommand( CMD_AUTOMATION_SNAPSHOT );
		}
		else if ( bCtrl )
		{
		}
		else if ( bAlt )
		{
		}
		else
		{
			m_pSonarCommands->DoCommand( CMD_AUTOMATION_STATIC_MODE );
		}
		break;

	case BID_TimeCodeMode:
		if ( TF_MBT == m_mfxfPrimary )
			m_mfxfPrimary = TF_SMPTE;
		else
			m_mfxfPrimary = TF_MBT;

		initLEDs();
		break;

	case BID_DataSelect:
      if ( bAlt )
      {
         // move the FEP to the now time
         m_pHostDataEdit->MoveDataCursor( ECD_NOWTIME, ECF_DONTFOLLOWSNAP | ECF_KEEP_SELECTION, 0 );
      }
      else if ( bCtrl )
      {
         // move the FEP to center of screen
         m_pHostDataEdit->MoveDataCursor( ECD_CENTER, ECF_KEEP_SELECTION, 0 );
      }
      else
		   onJogMode( bShift ? JM_SelByClips : JM_SelByTime );
		break;

	case BID_DataEdit:
		onJogMode( JM_Edit );
		break;

	case BID_Scroll:
		onJogMode( bShift ? JM_Zoom : JM_Scroll );
		break;

	case BID_IOCtrl:
	{
		if ( bCommand )
			ToggleProps();

		else if ( bAlt )
		{
			m_bSwitchUI = true;
		}
		else
		{
			m_bIOMode = !m_bIOMode;
			m_dw8EncoderParam = 0;

			if ( !m_bIOMode )
			{
				initLCDs();
				updateParamStateOnLCD();
				for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
				{
					PMBINDING& pmb = it->second;
					pmb.pParam->ResetHistory();

					CMidiMsg* pmsgIn = it->first;
					if ( BID_IN( pmsg->GetId(), BID_Encoder0, BID_Encoder7 ) )
						pmsgIn->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
				}
			}
			else
			{
				PSEncoderParams pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, Enc_IO );
				if ( !pEnc )
					pEnc = GetEncoderListParam( 0, MIX_STRIP_ANY, Enc_IO );
				if ( !pEnc )
					break;
				const TacomaIOBoxParam p = m_bFlipped ? TIOP_Gain : pEnc->ioParam;

				for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
				{
					PMBINDING& pmb = it->second;
					pmb.pParam->ResetHistory();

					CMidiMsg* pmsgIn = it->first;
					if ( BID_IN( pmsgIn->GetId(), BID_Encoder0, BID_Encoder7 ) )
					{
						const DWORD dwIxChan = pmsgIn->GetId() - BID_Encoder0;
						const float fReadValue = m_pIOBoxInterface->GetParam( dwIxChan, p );
						// set the underlying MIDI message
						pmsgIn->SetVal( fReadValue );
					}
				}

				showIoCtrlText();
			}

			initLEDs();
		}
		break;
	}

	default:
		bHandled = false;
	}

	return bHandled;
}

//----------------------------------------------------------
// One of the marker buttons was hit
void CTacomaSurface::onMarker( bool bLeft, float fVal )
{
	if ( bLeft )
	{
		if ( fVal == 0.f )
		{
			m_wModifierKey &= ~SMK_LEFTMARKER;
			return;
		}
		else
			m_wModifierKey |= SMK_LEFTMARKER;
	}

	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;

	if ( ( ( JM_SelByTime == m_eJogMode ) || ( JM_SelByClips == m_eJogMode ) ) && m_pHostDataEdit )
	{
		if ( bLeft )
		{
			// clear any previous selection and set selection flag
			m_pHostDataEdit->MoveDataCursor( ECD_STILL, ECF_SELECT, 0 );
			m_bSelectingOrEditing = TRUE;
		}
		else
		{
			if ( m_wModifierKey & SMK_LEFTMARKER )
				m_pSonarCommands->DoCommand( CMD_SELECT_NONE );
			else if ( !m_bSelectingOrEditing  )
				m_pHostDataEdit->MoveDataCursor( ECD_STILL, ECF_SELECT | ECF_KEEP_SELECTION, 0 );

			m_bSelectingOrEditing = FALSE;
		}
	}
	else if ( JM_Edit == m_eJogMode && m_pHostDataEdit )
	{
	   ClipEditFunction func = m_cefCurrent;
	   if ( bShift )
		   func = bLeft ? CEF_FADE_START : CEF_FADE_END;
	   else
		   func = bLeft ? CEF_CROP_START : CEF_CROP_END;

		endEdit();
		m_bSelectingOrEditing = FALSE;

	   if ( m_cefCurrent != func )
		{
			m_cefCurrent = func;
			m_pHostDataEdit->SetMode( DEM_EDIT, &m_cefCurrent );
			if ( func != CEF_NUDGE )
			{
				m_pHostDataEdit->BeginEdit( m_cefCurrent );
				m_bSelectingOrEditing = TRUE;
			}
		}
		else
		{
			m_pHostDataEdit->SetMode( DEM_NONE, NULL );
			m_eJogMode = JM_Standard;
			initLEDs();
		}
	}
	else if ( m_eJogMode == JM_Loop || m_eJogMode == JM_Punch )
	{
		m_bEditRight = bLeft ? FALSE : TRUE;
	}
	else
	{
		m_pSonarCommands->DoCommand( bLeft ? CMD_MARKER_PREVIOUS : CMD_MARKER_NEXT );
	}

	initLEDs();
}

//----------------------------------------------------------
// Jog mode button hit.
void CTacomaSurface::onJogMode( JogMode e )
{
	if ( !m_pHostDataEdit )
		return;

	JogMode eOld = m_eJogMode;
	// turn off any previous modes
	switch ( m_eJogMode )
	{
	case JM_SelByTime:
	case JM_SelByClips:
		m_eJogMode = e == m_eJogMode ? JM_Standard : e;
		break;
	case JM_Edit:
		endEdit();
		m_eJogMode = e == m_eJogMode ? JM_Standard : e;
		break;
	case JM_Scroll:
	case JM_Zoom:
		m_eJogMode = (e == m_eJogMode || ( JM_Scroll == e && JM_Zoom == m_eJogMode )) ? JM_Standard : e;
		break;
	case JM_Standard:
	case JM_Loop:
	case JM_Punch:
		m_eJogMode = e;
	break;
	}

	// now turn on the new mode
	switch ( m_eJogMode )
	{
		case JM_SelByTime:
		case JM_SelByClips:
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_SELECT, NULL ) ) )
				m_eJogMode = eOld;
		break;
		case JM_Edit:
			m_cefCurrent = CEF_NUDGE;
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_EDIT, &m_cefCurrent ) ) )
				m_eJogMode = eOld;
		break;
		case JM_Scroll:
		case JM_Zoom:
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_SCROLL_ZOOM, NULL ) ) )
				m_eJogMode = eOld;
		break;
		case JM_Loop:
			if ( m_pSonarTransport )
			{
				BOOL bValue = FALSE;
				if ( SUCCEEDED( m_pSonarTransport->GetTransportState( TRANSPORT_STATE_LOOP, &bValue ) ) && !bValue )
					if ( m_pSonarCommands )
						m_pSonarCommands->DoCommand( CMD_LOOP_TOGGLE );
			}

			m_bEditRight = FALSE;
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_NONE, NULL ) ) )
				m_eJogMode = eOld;
		break;
		case JM_Punch:
			if ( m_pSonarTransport )
			{
				BOOL bValue = FALSE;
				if ( SUCCEEDED( m_pSonarTransport->GetTransportState( TRANSPORT_STATE_AUTOPUNCH, &bValue ) ) && !bValue )
					if ( m_pSonarCommands )
						m_pSonarCommands->DoCommand( CMD_AUTOPUNCH_TOGGLE );
			}

			m_bEditRight = FALSE;
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_NONE, NULL ) ) )
				m_eJogMode = eOld;
		break;
		case JM_Standard:
			if ( !SUCCEEDED( m_pHostDataEdit->SetMode( DEM_NONE, NULL ) ) )
				m_eJogMode = eOld;
		break;
	}

	initLEDs();
}


///////////////////////////////////////////////////////////////////////
bool CTacomaSurface::handleBankButton( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = true;

	if ( m_eDisplayMode == SDM_ChannelBranch )
	{
		int nTrack = m_selectedStrip.stripIndex;
		switch( pmsg->GetId() )
		{
			case BID_BankL:	nTrack = max( 0, nTrack - 1 );	break;
			case BID_BankR:	nTrack ++;								break;
			default:				bHandled = false;						break;
		}

		if ( bHandled )
			doSelectStrip( nTrack );
	}
	else
	{
		int nTrackOrg = (int)getCurrentBankOffset();

		switch( pmsg->GetId() )
		{
		case 	BID_BankL:
			nTrackOrg -= (m_wModifierKey & SMK_SHIFT) ? 1 : 8;
			break;
		case 	BID_BankR:
			nTrackOrg += (m_wModifierKey & SMK_SHIFT) ? 1 : 8;
			break;
		default:
			bHandled = false;
		}

		if ( bHandled )
		{
			nTrackOrg = min( (int)(GetStripCount( m_e8StripType )), nTrackOrg );
			nTrackOrg = max( 0, nTrackOrg );
			SetStripRange( (DWORD)nTrackOrg, m_e8StripType );

			updateParamBindings();
			SetHostContextSwitch();
			initLEDs();
		}
	}

	return bHandled;
}

///////////////////////////////////////////////////////////////
bool CTacomaSurface::handleTransportControl( CMidiMsg* pmsg, float fValue )
{
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
   const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;

	MFX_TIME mfxt;
	bool bHandled = true;
	switch( pmsg->GetId() )
	{
	case BID_Play:
		if ( !m_bStopPressed )
		{
			if ( IsScrub() )
				Scrub( false );
			if ( bShift )
				Audition( );
         else if ( bAlt )
         {
            MFX_TIME time;
            if ( S_OK == m_pHostDataEdit->GetDataCursor( NULL, NULL, &time ) ) // it may return S_FALSE if the FEP is not enabled
            {
               SetNowTime( time, true, false );
               Play();
            }
         }
			else if ( IsPlaying() )
				Pause( true );
			else
				Play( );
		}
		else
			Scrub( true );
		break;
	case BID_Stop:
		m_bStopPressed = TRUEFLOAT( fValue );
		if ( m_bStopPressed )
		{
			if ( IsScrub() )
				Scrub( false );
			else
				Stop();
		}
		break;
	case BID_Record:
		Record( true );
		break;
	case BID_FF:
		FastForward( TRUEFLOAT( fValue ) ? 4 : 0 );
		break;
	case BID_Rew:
		Rewind( TRUEFLOAT( fValue) ? 4 : 0 );
		break;
	case BID_RTZ:
		if ( bShift )
			m_pSonarCommands->DoCommand( CMD_MARKER_PREVIOUS );
		else
		{
			mfxt.timeFormat = TF_SAMPLES;
			mfxt.llSamples = 0;
			SetNowTime( mfxt, true, false );
		}
		break;
	case BID_RTE:
		if ( bShift )
			m_pSonarCommands->DoCommand( CMD_MARKER_NEXT );
		else
			GotoEnd( true );
		break;
	case BID_Jog:
		onJogWheel( fValue );
		break;
	case BID_Shuttle:
		onShuttleRing( fValue );
		break;
	default:
		bHandled = false;
	}
	return bHandled;
}

//--------------------------------------------------------
// Shuttle ring was turned
void CTacomaSurface::onShuttleRing( float fValue )
{
	const bool bCCW = fValue >= .501f;
	const bool bCW = fValue >= .001f && fValue < .5f;
	//const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	TRACE( "shuttle: v=%.8f\n", fValue );

	float fMagnitude = 0.f;
	if ( bCCW )
		fMagnitude = (fValue - .5f) * 0x40 / 0xf;
	else if ( bCW )
		fMagnitude = fValue * 0x40 / 0xf;

	fMagnitude *= 2.f;

	TRACE( "shuttle magnitude = %.1f\n", fMagnitude );

	JogMode jmEffective = m_eJogMode;
	if ( bCommand )
		jmEffective = JM_Zoom;

	// should we use a timer or handle in place?
	bool bUseTimer = (jmEffective == JM_Standard && IsScrub()) || 
							JM_Edit == jmEffective || 
							JM_Scroll == jmEffective || 
							JM_SelByClips == jmEffective || 
							JM_SelByTime == jmEffective || 
							JM_Loop == jmEffective || 
							JM_Punch == jmEffective || 
							JM_Zoom == jmEffective;

	if ( !bCCW && !bCW )
		killTimer( m_tctShuttleRing );

	if ( bUseTimer )
	{
		if ( bCCW || bCW )
		{
			JogDirection jd = bCCW ? JD_CCW : JD_CW;
			m_tctShuttleRing.Set( fMagnitude, jd );

			// adjust frequency based on what we're doing
			const WORD wTime = ( JM_Edit == jmEffective || jmEffective == JM_Loop || jmEffective == JM_Punch ) ? 125 : (WORD) (30 + fMagnitude * 25);
			setTimer( m_tctShuttleRing, wTime );

			// and do one "right now"
			doShuttleRing( fMagnitude, jd );
		}
	}
	else
	{
		// just handle this in-place
		const bool bIsRewind = IsRewind();
		const bool bIsFastFwd = IsFastForward();

		if ( !bCCW )
		{
			if ( bIsRewind )
				Rewind( 0 );
		}
		else
			Rewind( (int)(fMagnitude * 16 + .5f) );

		if ( !bCW )
		{
			if ( bIsFastFwd )
				FastForward( 0 );
		}
		else
			FastForward( (int)(fMagnitude * 16 + .5f) );
	}
}


//------------------------------------------------------------
// Callback from timer
void CTacomaSurface::doShuttleRing( float f01Magnitude, JogDirection jd )
{
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;

	JogMode jmEffective = m_eJogMode;
	if ( bCommand )
		jmEffective = bAlt ? JM_Scroll : JM_Zoom;

	switch ( jmEffective )
	{
		case JM_Standard:
		{
			if ( IsScrub() )
			{
				int nSpeed = (int)(f01Magnitude * JA_4);
				JogAmount ja = (JogAmount)nSpeed;
				Jog( jd, ja );
			}
		}
		break;
		case JM_SelByTime:
		case JM_SelByClips:
		{
			if ( m_pHostDataEdit )
			{
				EditCursorDirection dir;
				if ( JD_CCW == jd )
					dir = bCtrl ? ECD_UP : ECD_LEFT;
				else
					dir = bCtrl ? ECD_DOWN : ECD_RIGHT;

				const bool bByClips = ( JM_SelByClips == jmEffective ) xor bShift;
				const bool bKeepOldSel = m_bSelectingOrEditing || !( bByClips xor bAlt );
				if ( bAlt )
					m_bSelectingOrEditing = TRUE;
				const DWORD dwEcf = ( m_bSelectingOrEditing ? ECF_SELECT : ECF_CLEAR ) | 
										  ( bByClips ? ECF_SNAPTOCLIPS | ECF_SELECT : ECF_CLEAR ) |
										  ( bKeepOldSel ? ECF_KEEP_SELECTION : ECF_CLEAR ) | 
										  ECF_DONTFOLLOWSNAP;
				m_pHostDataEdit->MoveDataCursor( dir, dwEcf, f01Magnitude );
			}
		}
		break;
		case JM_Edit:
		{
			if ( m_pHostDataEdit )
			{
				ClipEditFunction const func = getEditMode();
				if ( ( func != CEF_FADE_START ) && ( func != CEF_FADE_END ) )
				{
					int nOp = (int) ( f01Magnitude * 1000 );
					if ( JD_CCW == jd ) 
						nOp = - nOp;
					if ( SUCCEEDED( m_pHostDataEdit->DoEdit( func, nOp, EP_NONFIXEDOPERATOR | ( bCtrl ? EP_VERTICAL : 0 ) ) ) )
						m_bSelectingOrEditing = true;
				}
				else if ( !bCtrl )
				{
					// fade doesn't support _NONFIXEDOPERATOR yet - otherwise there would be just one block for all edit modes
					ClipEditOperator ceo;
					if ( JD_CCW == jd )
						ceo = f01Magnitude < .5f ? CEO_LEFT_SLOW : CEO_LEFT_FAST;
					else
						ceo = f01Magnitude < .5f ? CEO_RIGHT_SLOW : CEO_RIGHT_FAST;

					if ( SUCCEEDED( m_pHostDataEdit->DoEdit( func, ceo, 0 ) ) )
						m_bSelectingOrEditing = true;
				}
			}
		}
		break;
		case JM_Scroll:
		case JM_Zoom:
		{
			const bool bScroll = ( JM_Scroll == jmEffective ) != bShift;
			WindowAction wa;
			if ( bScroll )
				wa = bCtrl ? WA_SCROLLV : WA_SCROLLH;
			else
				wa = bCtrl ? WA_ZOOMV : WA_ZOOMH;

			int nOp = (int) ( f01Magnitude * 100 );
			if ( JD_CCW == jd )
				nOp = - nOp;

			if ( m_pHostWindow )
			{
				if ( !bCommand )
				{
					m_pHostWindow->DoWindowAction( WT_CLIPS, wa, nOp, bScroll ? ( SP_CENTER_DATACURSOR | SP_NONFIXEDOPERATOR ) :
																				  /* zoom ? */ ( ZP_DATACURSOR | ZP_NONFIXEDOPERATOR ) );
				}
				else
				{
					m_pHostWindow->DoWindowAction( WT_CLIPS, wa, nOp, bScroll ? SP_NONFIXEDOPERATOR :
																				  /* zoom ? */ ( ZP_SCREENCENTER | ZP_NONFIXEDOPERATOR ) );
				}
			}
		}
		break;
		case JM_Loop:
		case JM_Punch:
			if ( m_pSonarTransport2 )
			{
				SONAR_TRANSPORT_TIME time = m_bEditRight ? TRANSPORT_TIME_LOOP_OUT : TRANSPORT_TIME_LOOP_IN;
				if ( jmEffective == JM_Punch )
					time = m_bEditRight ? TRANSPORT_TIME_PUNCH_OUT : TRANSPORT_TIME_PUNCH_IN;

				UTransportMoveParams params;
				params.nRatio = (int) ( f01Magnitude * ( JD_CCW == jd ? -100 : 100 ) );
				m_pSonarTransport2->MoveTransportTime( time, TMF_NONFIXEDOPERATOR | TMF_DONTFOLLOWSNAP, params );
			}
		break;
	}
}



//--------------------------------------------------------------
// Jog wheel was turned
void CTacomaSurface::onJogWheel( float fValue )
{
	const bool bCtrl = (m_wModifierKey & SMK_CTRL) != 0;
	const bool bAlt = (m_wModifierKey & SMK_ALT) != 0;
	const bool bShift = (m_wModifierKey & SMK_SHIFT) != 0;
	const bool bCommand = (m_wModifierKey & SMK_COMMAND) != 0;

	JogMode jmEffective = m_eJogMode;
	if ( bCommand )
		jmEffective = bAlt ? JM_Scroll : JM_Zoom;

	//TRACE( "jog: v=%.2f\n", fValue );
	switch( jmEffective )
	{
	case JM_SelByTime:
	case JM_SelByClips:
		if ( m_pHostDataEdit )
		{
			EditCursorDirection dir;
			if ( bCtrl )
				dir = TRUEFLOAT( fValue ) ? ECD_UP : ECD_DOWN;
			else
				dir = TRUEFLOAT(fValue) ? ECD_LEFT : ECD_RIGHT;

			const bool bByClips = ( JM_SelByClips == jmEffective ) xor bShift;
			const bool bKeepOldSel = m_bSelectingOrEditing || !( bByClips xor bAlt );
			if ( bAlt )
				m_bSelectingOrEditing = TRUE;
			const DWORD dwEcf = ( m_bSelectingOrEditing ? ECF_SELECT : ECF_CLEAR ) | 
									  ( bByClips ? ECF_SNAPTOCLIPS | ECF_SELECT : ECF_CLEAR ) |
									  ( bKeepOldSel ? ECF_KEEP_SELECTION : ECF_CLEAR );
			m_pHostDataEdit->MoveDataCursor( dir, dwEcf, 0 );
		}
		break;

	case JM_Edit:
	{
		if ( m_pHostDataEdit )
		{
			ClipEditOperator const ceo = fValue >= .5 ? CEO_LEFT_FAST : CEO_RIGHT_FAST;
			ClipEditFunction const func = getEditMode();
			if ( SUCCEEDED( m_pHostDataEdit->DoEdit( func, ceo, bCtrl ? EP_VERTICAL : 0 ) ) ) // EP_VERTICAL == EP_TIME_STRETCH
				m_bSelectingOrEditing = true;
		}
	}
	break;
	case JM_Scroll:
	case JM_Zoom:
		{
			const bool bScroll = ( JM_Scroll == jmEffective ) != bShift;
			WindowAction wa;
			if ( bScroll )
				wa = bCtrl ? WA_SCROLLV : WA_SCROLLH;
			else
				wa = bCtrl ? WA_ZOOMV : WA_ZOOMH;
			EMoveOperator wo = TRUEFLOAT(fValue) ? MO_DECREASE_LARGE : MO_INCREASE_LARGE;

			if ( m_pHostWindow )
			{
				if ( !bCommand )
					m_pHostWindow->DoWindowAction( WT_CLIPS, wa, wo, bScroll ? SP_CENTER_DATACURSOR : ZP_DATACURSOR );
				else
					m_pHostWindow->DoWindowAction( WT_CLIPS, wa, wo, bScroll ? SP_NONE : ZP_SCREENCENTER );
			}
		}
		break;
	case JM_Loop:
	case JM_Punch:
		if ( m_pSonarTransport2 )
		{
			SONAR_TRANSPORT_TIME time = m_bEditRight ? TRANSPORT_TIME_LOOP_OUT : TRANSPORT_TIME_LOOP_IN;
			if ( jmEffective == JM_Punch )
				time = m_bEditRight ? TRANSPORT_TIME_PUNCH_OUT : TRANSPORT_TIME_PUNCH_IN;

			UTransportMoveParams params;
			params.eOp = ( TRUEFLOAT(fValue) ) ? MO_DECREASE_LARGE : MO_INCREASE_LARGE;
			m_pSonarTransport2->MoveTransportTime( time, TMF_NONE, params );
		}
	break;
	case JM_Standard:
		Jog( fValue < .5f ? JD_CW : JD_CCW, (m_wModifierKey & SMK_SHIFT) ? JA_1 : JA_3 );

		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Called after the base class connects
void CTacomaSurface::onConnect()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_bFlexiblePC = false;
	m_bLegacyEq   = true;
	
	if (m_pSonarIdentity)
	{
		char  szHostName[ _MAX_PATH ] = { 0 };
		DWORD dwLen = _countof( szHostName );
 		if (SUCCEEDED( m_pSonarIdentity->GetHostName( szHostName, &dwLen ) ))
		{
			ULONG	nMajor = 0;
			ULONG	nMinor = 0;
			ULONG nRevision = 0;
			ULONG nBuild = 0;

			m_pSonarIdentity->GetHostVersion( &nMajor, &nMinor, &nRevision, &nBuild );
			if (0 == _strnicmp( szHostName, "SONAR X1 Producer", 17 ))	// Producer or Producer Expanded
			{
				if (nMajor == 18 && nRevision >= 4)	
				{
					m_bFlexiblePC = true; // prochannel can be reordered
					m_bLegacyEq   = false;
				}
			}
			else if (0 == _strnicmp( szHostName, "SONAR", 5 )) // future version...
			{
				if (nMajor > 18)	
				{
					m_bFlexiblePC = true; // prochannel maybe can be reordered
					m_bLegacyEq   = false;
				}
			}
		}
	}

	emptyEncoderParamList();

	// tracks
   addEncoderParam( 0, Enc_Track, MIX_PARAM_PAN );
	addEncoderParam( 1, Enc_Track, MIX_PARAM_SEND_VOL );
	addEncoderParam( 2, Enc_Track, MIX_PARAM_INPUT );
	addEncoderParam( 3, Enc_Track, MIX_PARAM_OUTPUT );

   // busses
   addEncoderParam( 0, Enc_Bus, MIX_PARAM_PAN );
	addEncoderParam( 1, Enc_Bus, MIX_PARAM_SEND_VOL );
	addEncoderParam( 2, Enc_Bus, MIX_PARAM_SEND_PAN );
   addEncoderParam( 3, Enc_Bus, MIX_PARAM_OUTPUT );

	// IO
   addEncoderParam( 0, Enc_IO, TIOP_MakeupGain );
	addEncoderParam( 1, Enc_IO, TIOP_Threshold );
	addEncoderParam( 2, Enc_IO, TIOP_Attack );
   addEncoderParam( 3, Enc_IO, TIOP_Release );

   m_pIOBoxInterface = new CIOBoxInterface( this );

	// Figure out where the .ini file is
	// and sticky this in the INI file
	CString strPath;
	getDataPath( &strPath );

	CString strIni;
	strIni.Format( _T("%s\\VS700.INI"), strPath );

	buildBindings();

	initLEDs();
	updateParamStateOnLCD();

	endEdit();
}

////////////////////////////////////////////////////////////

#if 0 // no longer a valid test

bool CTacomaSurface::IsLegacyEQ()
{
	static bool sbChecked = false;
	static bool sbLegacy  = false;

	if (!sbChecked)
	{
		// figure out Sonitus or Prochannel Is Present, Set Mapping accordingly
		int nTotal = 0;
		for (int n = 0;;n++)
		{
			float fVal = 0.0;
			if (SUCCEEDED( m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FILTER_PARAM_COUNT, n, &fVal )))
			{
				sbChecked = true;
				nTotal += int(fVal);
			}
			else
			{
				break; // prevent endless loop: either all done or none yet
			}
		}

		sbLegacy	 = (nTotal < 27) ? true : false;
	}

	return sbLegacy;
}

#endif

/////////////////////////////////////////////////////////////////////////////
// Called before the base class disconnects
void CTacomaSurface::onDisconnect()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	endInsertPlugin();
	stopBlink();
   onProjectClose();

	CSFKCriticalSectionAuto csa( m_cs );

	// clean up the every midi message collection
	for ( MidiMsgSetIterator it = m_setEveryMidiMsg.begin(); it != m_setEveryMidiMsg.end(); ++it )
	{
		CMidiMsg* pmsg = *it;
		destroyMidiMsg( pmsg );
	}

	// now clean up the every mix param collection
	for ( MixParamSetIterator it = m_setEveryMixparam.begin(); it != m_setEveryMixparam.end(); ++it )
		delete *it;
	m_setEveryMixparam.clear();
	
	m_setSonitusEQ.clear();
	m_setGlossEQ.clear();
	
   for ( std::vector<PSEncoderParams>::iterator it = m_vEncoderParamList.begin(); it != m_vEncoderParamList.end(); it ++ )
      delete ( *it );

	endEdit();

	delete m_pIOBoxInterface;
}


///////////////////////////////////////////////////////////
// Helper to delete a CMidiMsg object and remove it from the input router
void	CTacomaSurface::destroyMidiMsg( CMidiMsg*& pmsg )
{
	pmsg->Send( 0.f );		// lights out

	m_pMidiInputRouter->Remove( pmsg );
	delete pmsg;
	pmsg = NULL;
}


HRESULT CTacomaSurface::SetHostContextSwitch( )
{
	// More important for ACT controls in Match Mode.  Reset the
	// value history on a context switch so that Nulling gets re-discovered

	return CControlSurface::SetHostContextSwitch();
}


using namespace SurfaceTransportTimeUtils;

///////////////////////////////////////////////////////////////////////////////////////

void	CTacomaSurface::Jog( JogDirection jd, JogAmount ja )
{
	BOOL b;
	m_pSonarTransport->GetTransportState( TRANSPORT_STATE_PLAY, &b );
	if ( b )
		return;

	MFX_TIME time;

	// for scrub always use seconds
	const bool bScrub = IsScrub();
	MFX_TIME_FORMAT fmtToUse = bScrub ? TF_SECONDS : m_mfxfPrimary;

	// Note: SurfaceTransportTimeUtils assumes either Musical or Absolute time
	// bases and will modify only the seconds or the mbt fields of the time.
	// therefore use TF_SECONDS for any absolute format rather than TF_SMPTE
	time.timeFormat = ( TF_MBT == fmtToUse )? TF_MBT : TF_SECONDS;
	m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &time );

	JogResolution jr = JOG_MEASURES;
	int iDir = jd == JD_CCW ? -1 : 1;

	if ( TF_MBT == fmtToUse)
	{
		switch( ja )
		{
		case JA_1:
			jr = JOG_TICKS;
			iDir *= 24;
			break;
		case JA_2:
			jr = JOG_BEATS;
			break;
		case JA_3:
			jr = JOG_MEASURES;
			iDir *= 1;
			break;
		case JA_4:
			jr = JOG_MEASURES;
			iDir *= 2;
			break;
		}
	}
	else
	{
		switch( ja )
		{
		case JA_1:
			iDir *= bScrub ? 1 : 1;
			jr = JOG_FRAMES;
			break;
		case JA_2:
			iDir *= bScrub ? 2 : 8;
			jr = JOG_FRAMES;
			break;
		case JA_3:
			iDir *= bScrub ? 4 : 1;
			jr = bScrub ? JOG_FRAMES : JOG_SECONDS;
			break;
		case JA_4:
			iDir *= bScrub ? 4 : 2;
			jr = bScrub ? JOG_FRAMES : JOG_SECONDS;
			break;
		}
	}


	NudgeTimeCursor( time, jr, iDir );

	SetNowTime( time, false, true );
}

//////////////////////////////////////////////////////////////////////////////
void CTacomaSurface::VZoom( int n )
{
	if ( !m_pSonarKeyboard )
		return;

	// ctrl dn
	m_pSonarKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYDOWN);

	// arrow
	int nVirtKey = n > 0 ? VK_UP : VK_DOWN;
	m_pSonarKeyboard->KeyboardEvent(nVirtKey, SKE_KEYDOWNUP);

	// ctrl up
	m_pSonarKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYUP);
}


////////////////////////////////////////////////////////////////////////////
void CTacomaSurface::HZoom( int n )
{
	if ( !m_pSonarKeyboard )
		return;

	// ctrl dn
	m_pSonarKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYDOWN);

	// arrow
	int nVirtKey = n > 0 ? VK_LEFT : VK_RIGHT;
	m_pSonarKeyboard->KeyboardEvent(nVirtKey, SKE_KEYDOWNUP);

	// ctrl up
	m_pSonarKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYUP);
}


/////////////////////////////////////////////////////////////////////////////
HRESULT CTacomaSurface::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (pdwLen == NULL)
		return E_POINTER;

	char szStatus[64] = "\0";
	switch( m_e8StripType )
	{
	case MIX_STRIP_TRACK:
		if ( SDM_Layers == m_eDisplayMode )
			::sprintf( szStatus, "Layers Track %d", m_selectedStrip.stripIndex + 1 );
		else
			::sprintf( szStatus, "%s %d-%d", "Tracks", getBankOffset(MIX_STRIP_TRACK) + 1, getBankOffset(MIX_STRIP_TRACK) + 8 );
		break;

	case MIX_STRIP_BUS:
		::sprintf( szStatus, "%s %d-%d", "Buses", getBankOffset(MIX_STRIP_BUS) + 1, getBankOffset(MIX_STRIP_BUS) + 8 );
		break;

	case MIX_STRIP_MASTER:
		::sprintf( szStatus, "%s %d-%d", "Mains", 0,0 );
		break;
	}

	if ( m_bOnMackie )
		::strcat( szStatus, " (mackie)" );

	if ( pszStatus )
	{
		DWORD dwLen = pdwLen ? *pdwLen : (DWORD)::strlen( szStatus );
		::strncpy( pszStatus, szStatus, dwLen );
	}
	else
		*pdwLen = (DWORD)::strlen( szStatus );

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
void	CTacomaSurface::onMarkersDisplay( bool bState )
{
	if ( !m_pSonarProject2 )
		return;

	setDisplayMode( bState ? SDM_Markers : SDM_Normal );
	updateParamStateOnLCD();
	if ( !bState )
		initLCDs();
}

///////////////////////////////////////////////////////////////////////////////
void CTacomaSurface::onLayersMode( bool bState )
{
	setDisplayMode( bState ? SDM_Layers : SDM_Normal );

	if ( bState )
	{
		// turn layers mode ON for the selected track
		m_pSonarMixer->SetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_LAYER, 0, 1.f, MIX_TOUCH_NORMAL );

		startBlink( BID_RudeMute );
		startBlink( BID_RudeSolo );
	}
	else
	{
		stopBlink( BID_RudeMute );
		stopBlink( BID_RudeSolo );
	}

	initLCDs();
}

//---------------------------------------------------------------
// Given a rotary index, and a value change direction enum,
// set the child index into the plugin tree
void CTacomaSurface::setPluginChild( int ixRotary, CMidiMsg::ValueChange vc )
{
	if ( vc == CMidiMsg::VC_None )
		return;

	if ( !m_pPluginTree )
		return;

	// follow the descendents to find the actual node for this rotary
	PLUGINS* pNode = m_pPluginTree;
	PLUGINS* pNodeFound = NULL;

	size_t iDesc = 0;
	for ( iDesc = 0; iDesc < NumPluginDesc; iDesc++ )
	{
		if ( iDesc == ixRotary )
		{
			pNodeFound = pNode;
			break;
		}
		if ( 0 == pNode->cChildren )
			break;
		pNode = pNode->apChildren[ m_aPluginChildIndex[iDesc] ];
	}
	if ( pNodeFound )
	{
		if ( pNodeFound->cChildren )
		{
			// increment / decrement the child index
			int nChild = (int)m_aPluginChildIndex[ixRotary];
			if ( vc == CMidiMsg::VC_Increase )
				nChild++;
			else
				nChild--;
			nChild = max( 0, nChild );
			nChild = min( (int)(pNodeFound->cChildren - 1), nChild );

			DWORD dwChild = (DWORD)nChild;
			if ( m_aPluginChildIndex[ixRotary] != dwChild )
			{
				m_aPluginChildIndex[ixRotary] = dwChild;

				// now set every child after this to 0
				for ( size_t i = iDesc+1; i < NumPluginDesc; i++ )
					m_aPluginChildIndex[i] = 0;

				showAvailablePlugins();
			}
		}
	}
}


//-------------------------------------------------------------------
// Given a Rotary knob index, Determine the current Plugin ID from
// the m_aPluginChildIndex array and ask the host to instantiate it 
// at the current Bin Index on the current strip
void CTacomaSurface::insertPlugin( int ixRotary )
{
	IHostPluginAccess* pHostPlugs = NULL;
	if ( m_pPluginTree && SUCCEEDED( m_pSonarMixer->QueryInterface( IID_IHostPluginAccess, (void**)&pHostPlugs )  ) )
	{
		// follow the descendents to find the actual node for this rotary
		PLUGINS* pNode = m_pPluginTree;
		PLUGINS* pNodeFound = NULL;

		size_t iDesc = 0;
		for ( iDesc = 0; iDesc < NumPluginDesc; iDesc++ )
		{
			if ( iDesc == ixRotary )
			{
				pNodeFound = pNode;
				break;
			}
			if ( 0 == pNode->cChildren )
				break;
			pNode = pNode->apChildren[ m_aPluginChildIndex[iDesc] ];
		}
		if ( pNodeFound )
		{
			pNode = pNodeFound->apChildren[m_aPluginChildIndex[iDesc]];
			if ( pNode )
				pHostPlugs->InsertPlugin( m_selectedStrip.stripType, m_selectedStrip.stripIndex, m_dwBinInsertIndex, pNode->dwId );
		}

		pHostPlugs->Release();
	}

	endInsertPlugin();
	initLCDs();
}

//------------------------------------------------------------------
// Release the Plug-in tree we acquired from the host and reset
// our state variables
void CTacomaSurface::endInsertPlugin()
{
	// normal display mode
	setDisplayMode( SDM_Normal );

	// free the tree
	IHostPluginAccess* pHostPlugs = NULL;
	if ( m_pPluginTree && SUCCEEDED( m_pSonarMixer->QueryInterface( IID_IHostPluginAccess, (void**)&pHostPlugs )  ) )
	{
		pHostPlugs->FreePluginsTree( m_pPluginTree );
		pHostPlugs->Release();
	}	

	// clear out the child indexes
	::memset( (void*)m_aPluginChildIndex, 0, sizeof(m_aPluginChildIndex) );

	// turn off all blinkings
	stopBlink();

	m_pPluginTree = NULL;
}


ClipEditFunction CTacomaSurface::getEditMode( void )
{
	const bool bShift = ( m_wModifierKey & SMK_SHIFT ) != 0;

	ClipEditFunction func = m_cefCurrent;
	if ( bShift )
	{
		if ( m_cefCurrent == CEF_CROP_START )
			func = CEF_FADE_START;
		else if ( m_cefCurrent == CEF_CROP_END )
			func = CEF_FADE_END;
		else if ( m_cefCurrent == CEF_FADE_START )
			func = CEF_CROP_START;
		else if ( m_cefCurrent == CEF_FADE_END )
			func = CEF_CROP_END;
	}

	return ( func );
}

inline void CTacomaSurface::endEdit( BOOL bEnd /* = TRUE */ )
{
	if ( m_pHostDataEdit )
	{
		if ( bEnd )
			m_pHostDataEdit->EndEdit();
		else
			m_pHostDataEdit->CancelEdit();
	}

	m_bSelectingOrEditing = false;
}

void CTacomaSurface::addEncoderParam( DWORD dwIndex, EncoderOptions strip, DWORD dwCmd, DWORD dwParam /* = 0 */ )
{
   PSEncoderParams pEnc = new ( SEncoderParams );
   pEnc->stripType = strip;
   pEnc->dwCmd = dwCmd;
   pEnc->dwParam = dwParam;
   pEnc->dwIndex = dwIndex;

   // try to find the dwIndex'th entry of type type
   for ( std::vector<PSEncoderParams>::iterator it = m_vEncoderParamList.begin(); it != m_vEncoderParamList.end(); it ++ )
   {
      PSEncoderParams pEncTemp = *it;
      if ( pEncTemp->stripType == strip && pEncTemp->dwIndex == dwIndex )
      {
			delete ( *it );
         m_vEncoderParamList.erase( it );
         break;
      }
   }

   m_vEncoderParamList.push_back( pEnc );
}

CTacomaSurface::PSEncoderParams CTacomaSurface::GetEncoderListParam( DWORD dwIndex, DWORD dwCmd, SONAR_MIXER_STRIP mixerStrip )
{
	const EncoderOptions strip = ( mixerStrip == MIX_STRIP_BUS ) ? Enc_Bus : Enc_Track;
	return ( GetEncoderListParam( dwIndex, dwCmd, strip ) );
}


CTacomaSurface::PSEncoderParams CTacomaSurface::GetEncoderListParam( DWORD dwIndex, DWORD dwCmd, EncoderOptions strip )
{
   const BOOL bByCmd = ( dwCmd != MIX_STRIP_ANY );
   const BOOL bByIndex = ( dwIndex != MIX_STRIP_ANY );

   for ( std::vector<PSEncoderParams>::iterator it = m_vEncoderParamList.begin(); it != m_vEncoderParamList.end(); it ++ )
   {
      PSEncoderParams pEncTemp = *it;
      if ( pEncTemp->stripType == strip && 
           ( bByCmd ? pEncTemp->dwCmd == dwCmd : 1 ) && 
           ( bByIndex ? pEncTemp->dwIndex == dwIndex : 1 ) )
         return ( *it );
   }

   return ( NULL );
}

DWORD CTacomaSurface::getCountByStripType( EncoderOptions strip )
{
   DWORD dwCount = 0;
   for ( std::vector<PSEncoderParams>::iterator it = m_vEncoderParamList.begin(); it != m_vEncoderParamList.end(); it ++ )
   {
      PSEncoderParams pEncTemp = *it;
      if ( pEncTemp->stripType == strip )
         dwCount ++;
   }

   return ( dwCount );
}

DWORD CTacomaSurface::getCountByStripType( SONAR_MIXER_STRIP strip )
{
	return ( getCountByStripType( strip == MIX_STRIP_BUS ? Enc_Bus : Enc_Track ) );
}
#if DAISY
void CTacomaSurface::showDisplayEE( void )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if ( m_pDisplayEE )
	{
		delete ( m_pDisplayEE );
		m_pDisplayEE = NULL;
	}

	srand( timeGetTime() );	
	if ( rand() % 5 != 0 )
		return;

	m_pDisplayEE = new CDisplayEE;
   if ( m_pDisplayEE )
      m_pDisplayEE->CreateWnd( m_fXRay );
}

static const int BKG_WIDTH = 176;
static const int BKG_HEIGHT = 184;
static const int BKG_MIDHEIGHT = 113;


IMPLEMENT_DYNAMIC(CDisplayEE, CWnd)

CDisplayEE::CDisplayEE()
{
   CBitmapCache::GetBitmapCache().LoadBitmap( IDB_DISPLAY_EE );
}

CDisplayEE::~CDisplayEE()
{
   CBitmapCache::GetBitmapCache().FreeBitmap( IDB_DISPLAY_EE );
}

BEGIN_MESSAGE_MAP(CDisplayEE, CWnd)
   ON_WM_PAINT()
   ON_WM_KILLFOCUS()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CDisplayEE::OnPaint()
{
   CPaintDC dc(this);
   CDC dcMem;
   dcMem.CreateCompatibleDC( &dc );

   CBitmapCache& bmc = CBitmapCache::GetBitmapCache();
   CBitmap* pBitmap = bmc.GetBitmap( IDB_DISPLAY_EE );
   if ( pBitmap )
   {
      dcMem.SelectObject( pBitmap );
      bmc.DrawBitmap( IDB_DISPLAY_EE, &dc, 0, 0, BKG_WIDTH, BKG_HEIGHT, 0, 0, &dcMem );
   }

/*
   CDC dcOffscreen;
	dcOffscreen.CreateCompatibleDC( &dc );

   CBitmap* pBmBkgd = CBitmapCache::GetBitmapCache().GetBitmap( IDB_DISPLAY_EE );
	CBitmap* pOldBmp = dcOffscreen.SelectObject( pBmBkgd );
   dcMem.BitBlt( 0, 0, BKG_WIDTH, BKG_HEIGHT, &dcOffscreen, 0, 0, SRCCOPY );
	dcOffscreen.SelectObject( pOldBmp );
*/
}

void CDisplayEE::CreateWnd( float fVal )
{
   CRect rect;
   const HWND hWnd = ::GetForegroundWindow();
	if ( hWnd == NULL )
		return;
   if ( !::GetWindowRect( hWnd, &rect ) )
      return;
	if ( GetWindowLong( hWnd, GWL_EXSTYLE ) & WS_EX_LAYERED && fVal < 0.75f )
		return;
   CRect rectScreen;
   if ( !::GetWindowRect( ::GetDesktopWindow(), &rectScreen ) )
      return;
   if ( !( rect.right < rectScreen.right && rect.top > rectScreen.top + BKG_HEIGHT ) )
      return;

   srand( timeGetTime() );
   const int nX = rand() % ( rect.Width() - BKG_WIDTH );
   CRect rcSize( rect.left + nX, rect.top - BKG_MIDHEIGHT, BKG_WIDTH, BKG_HEIGHT );
   rcSize.right += rcSize.left;
   rcSize.bottom += rcSize.top;

   if ( CreateEx( WS_EX_TOPMOST | WS_EX_TOOLWINDOW, ::AfxRegisterWndClass( NULL ), NULL, WS_POPUP, rcSize, NULL, 0 ) )
   {
      SetWindowLong( GetSafeHwnd(), GWL_EXSTYLE, GetExStyle() | WS_EX_LAYERED );
		if ( !SetLayeredWindowAttributes( RGB( 127, 127, 127 ), 0, LWA_COLORKEY ) )
         DestroyWindow();
      else
		{
         ShowWindow( SW_SHOW );
			SetTimer( 0xDA15EE, 4000, NULL );
		}
   }
}

void CDisplayEE::OnTimer( UINT_PTR nTimer )
{
	if ( nTimer == 0xDA15EE )
		DestroyWindow();
}

void CDisplayEE::OnKillFocus( CWnd *pWnd )
{
   DestroyWindow();
}
#endif

BOOL CTacomaSurface::moveActiveWindow( int nX, int nY )
{
	RECT rect;
	if ( SUCCEEDED( m_pHostWindow->GetWindowPos( WT_ACTIVE, &rect ) ) )
	{
		RECT rectScreen;
		rectScreen.left = 0; 
		rectScreen.top = 0; 
		rectScreen.right = GetSystemMetrics (SM_CXMAXTRACK); 
		rectScreen.bottom = GetSystemMetrics (SM_CYMAXTRACK);

		if ( m_ptGrabbed.x != -1 )
		{
			const int nWidth = rect.right - rect.left, nHeight = rect.bottom - rect.top;	// dimensions

			int nNewX = nX - m_ptGrabbed.x, nNewY = nY - m_ptGrabbed.y;		// new left/top

			if ( nNewX + nWidth > rectScreen.right )
				nNewX = rectScreen.right - nWidth;									// don't go beyond screen right
			if ( nNewY + nHeight > rectScreen.bottom )
				nNewY = rectScreen.bottom - nHeight;								// don't go beyond screen bottom
			nNewX = max( 0, nNewX ); nNewY = max( 0, nNewY );					// don't go beyond screen left/top

			RECT rectNew;
			rectNew.left = nNewX; rectNew.right = nNewX + nWidth;
			rectNew.top = nNewY;  rectNew.bottom = nNewY + nHeight;
			if ( memcmp( &rectNew, &rect, sizeof ( RECT ) ) == 0 )
				return ( FALSE );

			m_pHostWindow->SetWindowPos( WT_ACTIVE, rectNew );

			//TRACE( _T("Grabbed: %d, %d. "), m_ptGrabbed.x, m_ptGrabbed.y );
			m_ptGrabbed.x = nX - rectNew.left;
			m_ptGrabbed.y = nY - rectNew.top;
			//TRACE( _T("New: %d, %d\n"), m_ptGrabbed.x, m_ptGrabbed.y );
		}
		else if ( rect.left < nX && rect.right > nX && rect.top < nY && rect.bottom > nY )
		{
			m_ptGrabbed.x = nX - rect.left;
			m_ptGrabbed.y = nY - rect.top;
		}
	}

	return ( TRUE );
}


void CTacomaSurface::resetChannelStrip()
{
	InputBindingMap *pMap = NULL;

	switch ( m_eActSectionMode )
	{
	case KSM_FLEXIBLE_PC:
	case KSM_ACT:
		pMap = &m_mapACTControls;
		break;

	case KSM_EQ:
		pMap = &m_mapEQControls;
		break;

	case KSM_SEND:
		pMap = &m_mapSendControls;
		break;

	case KSM_PC_COMP:
		pMap = &m_mapPCCompControls;
		break;

	case KSM_PC_EQ:
		pMap = &m_mapPCEQControls;
		break;

	case KSM_PC_EQ_P2:
		pMap = &m_mapPCEQP2Controls;
		break;

	case KSM_PC_SAT:
		pMap = &m_mapPCSatControls;
		break;

	default:
		ASSERT( false ); // unmatched
		break;
	}

	for ( InputBindingIterator it = pMap->begin(); it != pMap->end(); ++it )
	{
		CMidiMsg* pmsgIn = it->first;
		PMBINDING& pmb = it->second;

		pmb.pParam->ResetHistory();

		float fVal = 0.f;
		const HRESULT hrChange = pmb.pParam->GetVal( &fVal );
	   if ( FAILED( hrChange ) )
			fVal = 0.f;

		if ( pmb.pMsgOut )
			pmb.pMsgOut->Send( fVal );

	   pmsgIn->SetVal( fVal );	
	}
}


HRESULT CTacomaSurface::updatePluginLEDs( void )
{
	float fFx = 0.f;
	m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_FX_COUNT, 0, &fFx );

	// stop blinking the ARM LEDs
	for ( size_t i = 0; i < 8; i++ )
	{
		const ControlId cId = (ControlId) ( BID_Arm0 + i );
		ControlMessageMapIterator it = m_mapControlMsgs.find( cId );
		if ( it != m_mapControlMsgs.end() )
		{
			stopBlink( cId );
			if ( i > (DWORD)fFx )
			{
				CMidiMsg* const pmsg = it->second;
				pmsg->Send( 0.f );
			}
		}
	}

	return ( S_OK );
}

void CTacomaSurface::setBankOffset(SONAR_MIXER_STRIP type, DWORD offset)
{
	//BankOffsetIterator it = m_mapBankOffsets.find(type);
	//if (it != m_mapBankOffsets.end())
	//{
	//	CMidiMsg* const pmsg = it->second;
	//}
   m_mapBankOffsets[type] = offset;
}

DWORD CTacomaSurface::getBankOffset(SONAR_MIXER_STRIP type)
{
	BankOffsetIterator it = m_mapBankOffsets.find(type);
	if (it != m_mapBankOffsets.end())
	{
	   return (DWORD)it->second;
	}
   return 0;
}

DWORD CTacomaSurface::getCurrentBankOffset()
{
   return getBankOffset(m_e8StripType);
}

CTacomaSurface::EncoderOptions CTacomaSurface::getModeForRotary( void )
{
	EncoderOptions enc = Enc_IO;

	if ( !m_bIOMode )
	{
		if ( Get8StripType() == MIX_STRIP_BUS )
			enc = Enc_Bus;
		else if ( Get8StripType() == MIX_STRIP_MASTER )
			enc = Enc_Main;
		else
			enc = Enc_Track;
	}

	return ( enc );
}


void CTacomaSurface::emptyEncoderParamList( void )
{
	const size_t dwMax = m_vEncoderParamList.size(); 
	for ( size_t dwCntr = 0; dwCntr < dwMax; dwCntr ++ )
		delete ( m_vEncoderParamList[ dwCntr ] );

	m_vEncoderParamList.clear();
}


HRESULT CTacomaSurface::onChannelBranchMode( bool bOn )
{
	if ( m_selectedStrip.stripType != MIX_STRIP_TRACK )
		return ( S_FALSE );

	float fMidi = 0.f;
	if ( !( SUCCEEDED( m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_IS_MIDI, 0, &fMidi ) ) && fMidi < 0.5f ) )
		return ( S_FALSE );

	m_nInserting = 0;
	m_fSendDest = -1.f;

	// and refresh all displays
	setDisplayMode( bOn ? SDM_ChannelBranch : SDM_Normal );

	if ( bOn )
	{
		AssignChannelBranchRotaries();
		lightNthSEL( 0, 1 );
	}
	else
	{
		PSEncoderParams const pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, getModeForRotary() );
		if ( pEnc )
			AssignRotaries( pEnc->mixParam, pEnc->dwParam, pEnc->dwIndex );

		initLEDs();
	}

	return ( S_OK );
}

void CTacomaSurface::AssignChannelBranchRotaries()
{
	// update rotaries
	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;
		const DWORD dwId = pmsg->GetId();

		if ( (dwId >= BID_Encoder0 && dwId <= BID_Encoder7) || (dwId >= BID_EncoderPush0 && dwId <= BID_EncoderPush7) )
		{
			// set to the new param
			if ( pmb.pParam->GetStripPhysicalIndex() != 0 )
			{
				if ( pmb.pParam->GetStripPhysicalIndex() != m_nInserting )
					pmb.pParam->SetParams( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_PAN, 
												  pmb.pParam->GetStripPhysicalIndex() - 1, .5f, TRUE );
				else
					pmb.pParam->SetParams( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_OUTPUT, 
												  pmb.pParam->GetStripPhysicalIndex() - 1, .5f, TRUE );
			}
			else
				pmb.pParam->SetParams( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_PAN, 0, .5f, TRUE );

			pmb.pParam->SetCrunchSize( 7 );
			pmb.pParam->ResetHistory();

			// set the underlying MIDI msg objects' dwCurrent value correctly
			float fVal = -1.f;
			const HRESULT hrChange = pmb.pParam->GetVal( &fVal );
			pmsg->SetVal( fVal );
		}
	}

	updateParamStateOnLCD();
}

// This switches selective params into step mode for Fine Resolution control
BOOL CTacomaSurface::toggleStepCapture( void )
{
	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;
		const DWORD dwId = pmsg->GetId();

		if ( dwId >= BID_Encoder0 && dwId <= BID_Encoder7 )
			toggleCapture( pmb.pParam );
	}
	InputBindingMap * const pMap[] = { &m_mapEQControls, &m_mapACTControls, &m_mapSendControls, &m_mapPCEQControls, &m_mapPCEQP2Controls,  &m_mapPCCompControls,  &m_mapPCSatControls };
	for ( int n = 0; n < _countof( pMap ); ++ n )
	{
		for ( InputBindingIterator it = pMap[n]->begin(); it != pMap[n]->end(); ++it )
		{
			PMBINDING& pmb = it->second;
			CMidiMsg* pmsg = it->first;

			if ( pmsg->GetChannel() == 0 ) // the switches are on 1, rotaries on 0
				toggleCapture( pmb.pParam );
		}
	}

	return ( TRUE );
}

void CTacomaSurface::toggleCapture( CMixParam *pParam )
{
	CMixParam::CaptureType typeDefault = pParam->GetDefaultCaptureType();
	if ( typeDefault != CMixParam::CT_Step )
	{
		// toggle it
		if ( typeDefault != pParam->GetCaptureType() )
			pParam->SetCaptureType( typeDefault );
		else
			pParam->SetCaptureType( CMixParam::CT_Step );
	}
}

void CTacomaSurface::endChannelBranchInserting( void )
{
	lightNthSEL( m_nInserting, 0 ); // stop blink
	m_nInserting = 0;
	m_fSendDest = -1.f;
	AssignChannelBranchRotaries();

	// light the currently 'selected' send
	if ( m_Surround.pParamAngle->GetMixerParam() == MIX_PARAM_SURROUND_SENDANGLE )
	{
		// a send was selected, light it
		lightNthSEL( m_Surround.pParamAngle->GetParamNum(), 1 );
	}
}
