// VS100.cpp : Defines the basic functionality of the "CVS100".
//

#include "stdafx.h"

#include "VS100.h"
#include "VS100PropPage.h"
#include "sfkTransportTimeUtils.h"
#include "MixParam.h"
#include "LCDTextWriter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//	Main Implementations for the Control Surface Interfaces
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CVS100::CVS100() :
	CControlSurface()
	,m_bACTMode( true )
	,m_eStripType( MIX_STRIP_TRACK )
	,m_dwMessageIdSeed(40000)
	,m_bShiftPressed(false)
	,m_tcbParamChange( this, TS_ParamChanged )
	,m_tctBlink( this, TS_Blink )
	,m_tctKeyRep( this, TS_KeyRep )
	,m_tctRevert( this, TS_Revert )
	,m_eBlinkState( BS_None )
	,m_pLCD(NULL)
	,m_bEnableMotors(true)
	,m_eRotaryMode(SRM_Normal)
	,m_bFullAssign(true)
	,m_FullAssignReqMidiMsg( this, _T("Full assign request") )
{ 
	::InterlockedIncrement( &g_lComponents );

	// encoder value mappings
	m_vrCCW.dwL = 0x41;
	m_vrCCW.dwH = 0x48;
	m_vrCW.dwL = 0x1;
	m_vrCW.dwH = 0x08;

	EncoderParam ec;
	ec.param = MIX_PARAM_PAN;
	ec.dwParam = 0;
	m_vEncoderParam.push_back(ec);

	ec.param = MIX_PARAM_VOL_TRIM;
	m_vEncoderParam.push_back(ec);
	
	ec.param = MIX_PARAM_SEND_VOL;
	m_vEncoderParam.push_back(ec);
	
	ec.dwParam = 1;
	m_vEncoderParam.push_back(ec);
	m_pLCD = new CLCDTextWriter( this );

	// set up the Full assign mode request message
	m_FullAssignReqMidiMsg.SetMessageType( CMidiMsg::mtNote );
	m_FullAssignReqMidiMsg.SetChannel(0);
	m_FullAssignReqMidiMsg.SetNoteNum( 0x2d );

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CVS100::~CVS100() 
{ 
	delete m_pLCD;
	::InterlockedDecrement( &g_lComponents );
}



/////////////////////////////////////////////////////////////////////////////
// This is the IControlSurface way of obtaining the MIDI mask.  Older versions 
// of SONAR will only call this instead of IControlSurface3::GetNoEchoStatusMessages(). 
// This function has much less granularity because it only returns a bitmask for the MIDI
// channels in use.  However for a dedicated control surface this is probably just fine.
HRESULT CVS100::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
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
HRESULT CVS100::GetNoEchoStatusMessages(WORD** ppwStatus, DWORD* pdwCount )
{
	*pdwCount =  0;
	return S_OK;
}






/////////////////////////////////////////////////////////////////////////////
// ISurfaceParamMapping
// Return a count of strip ranges (ie: if you do tracks and buses, you have 2 ranges).
HRESULT CVS100::GetStripRangeCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = 1;
	
	return S_OK;
}


/////////////////////////////////////////////////////////////////////
// Given an index, return strip range information
HRESULT CVS100::GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip )
{
	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	if ( dwIndex >= 1 )
		return E_INVALIDARG;

	StripRangeMapIterator it = m_mapStripRanges.find( m_eStripType );
	if ( it != m_mapStripRanges.end() )
	{
		STRIPRANGE& sr = it->second;
		*pdwLowStrip = sr.dwL;
		*pdwHighStrip = sr.dwH;
		*pmixerStrip = it->first;
	}
	return S_OK;
}






//////////////////////////////////////////////////////////////////
// Set the base strip for a given strip type.
// Note: this is exposed to the host via ISurfaceParamMapping
HRESULT CVS100::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
{
	bool bChange = false;

	// find the strip range for this type of strip
	StripRangeMapIterator it = m_mapStripRanges.find( mixerStrip );
	if ( it != m_mapStripRanges.end() )
	{
		STRIPRANGE& sr = it->second;
		// change the start and and of this strip range
		DWORD dwCount = sr.dwH - sr.dwL;
		if ( sr.dwL != dwLowStrip )
			bChange = true;
		sr.dwL = dwLowStrip;
		sr.dwH = sr.dwL + dwCount;
	}

	m_eStripType = mixerStrip;
	updateParamBindings();
	SetHostContextSwitch();

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// Return the count of ACT parameters you are maintaining
HRESULT CVS100::GetDynamicControlCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = (DWORD)m_mapACT.size();

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// return info about the nth ACT parameter you are maintaining
HRESULT CVS100::GetDynamicControlInfo(DWORD dwIndex, DWORD* pdwID, SURFACE_CONTROL_TYPE* pType )
{
	if ( !pdwID || !pType )
		return E_POINTER;

	if ( dwIndex >= (DWORD)m_mapACT.size() )
		return E_INVALIDARG;
	
	// assign some arbitrary ID. This ID should never change for any given control
	*pdwID = ACTKEY_BASE + dwIndex;
	
	// return the type.
	if ( 3 == dwIndex ||	// comp switch
		5 == dwIndex ||	// R4 push switch
		12 < dwIndex )
		*pType = SCT_SWITCH;
	else
		*pType = SCT_ROTARY;

	return S_OK;
}


//////////////////////////////////////////////////////////////////
// Host is notifying us of a ACT Learn state change.
HRESULT CVS100::SetLearnState( BOOL bLearning )
{
	// You should tell all CMixParam objects that are for ACT that
	// they are in Learn mode now.  This switches off any capture modes
	// while learning

	// call base class to set the global bit
	return CControlSurface::SetLearnState( bLearning );
}



// ACT Text Query
// Public access for the prop page to get ACT param labels
CString CVS100::GetACTParamLabel( DWORD id )
{
	char	sz[16];
	sz[0] = '\0';
	DWORD cb = sizeof(sz);
	m_pSonarMixer->GetMixParamLabel( MIX_STRIP_ANY, id, MIX_PARAM_DYN_MAP, m_dwSurfaceID, sz, &cb );

	return CString(sz);
}

CString CVS100::GetACTContextName()
{
   char szName[32];
   DWORD dwCountof = _countof( szName );
   szName[0] = '\0';
   m_pSonarParamMapping->GetMapName( m_dwSurfaceID, (LPSTR) &szName, &dwCountof );

	return CString(szName);
}




//////////////////////////////////////////////////////////////////////
// Create a mix param on the heap and put it in the every mix param set
CMixParam* CVS100::createMixParam()
{
	CMixParam* pParam = new CMixParam;
	pParam->SetInterfaces( m_pSonarMixer, m_pSonarTransport, m_dwSurfaceID );
	m_setEveryMixparam.insert( pParam );
	return pParam;
}

//---------------------------------------------------------------
CMidiMsg* CVS100::createMidiMsg( LPCTSTR sz, //= 0, 
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
HRESULT	CVS100::buildBindings()
{
	// Build the strip ranges
	m_mapStripRanges.clear();
	STRIPRANGE sr;
	sr.dwL = 0;
	sr.dwH = sr.dwL; 
	m_mapStripRanges[MIX_STRIP_TRACK] = sr;

	m_mapStripRanges[MIX_STRIP_BUS] = sr;

	buildStripBindings();
	buildMiscBindings();
	buildTransportBindings();

	updateParamBindings();

	return CControlSurface::buildBindings();
}


//--------------------------------------------------------
// Create the Midi message and Parameter objects for
// all per-strip parameters.  
HRESULT CVS100::buildStripBindings()
{
	// mackie faders
	PMBINDING pmb;
	pmb.nRefreshMod = RM_EVERY;	// faders have motors - update them as often as possible

	// create a mix param for vol. This will have TWO Input messages associated
	// with it (fader, touch switch), and one output message (motor).
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_VOL, 0, 101.0f/127 );

	// Fader Input Messages
	CMidiMsg* pmsg = createMidiMsg( _T("fader"), CID_Fader );
	pmsg->SetMessageType( CMidiMsg::mtWheel );
	pmsg->SetChannel( 0 );
	m_pMidiInputRouter->Add( pmsg );

	// Fader Motor output messages (use input)
	pmb.pMsgOut = pmsg;

	m_mapStripMsgs[pmsg] = pmb;

	// Fader touch switch
	pmb.pMsgOut = NULL;	// no output
	CMidiMsg* pmsgT = createMidiMsg(_T("touch") );
	pmsgT->SetMessageType( CMidiMsg::mtNote );
	pmsgT->SetChannel(0);	// all 0
	pmsgT->SetNoteNum( 0x68 );
	m_pMidiInputRouter->Add( pmsgT );

	m_mapFaderTouchMsgs[pmsgT] = pmb;

	////////////////////////////////////////////
	// Encoder
	// create a mix param for pan.
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_PAN, 0, 0.5f );
	m_setFineControls.insert( pmb.pParam );

	// Encoder Input Messages
	pmsg = createMidiMsg(_T("pan"), CID_StripRotary);
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x10 );
	pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
	m_pMidiInputRouter->Add( pmsg );

	// Main strip binding
	m_mapStripMsgs[pmsg] = pmb;


	////////////////////////////////////////////
	// MUTES
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_MUTE, 0 );

	pmsg = createMidiMsg( _T("mute"), CID_Mute );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);	// all 0
	pmsg->SetNoteNum( 0x10 );
	pmsg->SetIsTrigger( TRUE, 0x7f );
	m_pMidiInputRouter->Add( pmsg );
	pmb.pMsgOut = pmsg;	// light for the switch (use input msg)
	m_mapStripMsgs[pmsg] = pmb;

	// Input Monitoring
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_INPUT_ECHO, 0 );
	m_mapShiftMsgs[ pmsg ] = pmb;


	////////////////////////////////////////////
	// SOLO
	pmb.pParam = createMixParam(  );
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_SOLO, 0 );

	pmsg = createMidiMsg(_T("solo"), CID_Solo);
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);	// all 0
	pmsg->SetNoteNum( 0x08 );
	pmsg->SetIsTrigger( TRUE, 0x7f );
	m_pMidiInputRouter->Add( pmsg );
	pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

	m_mapStripMsgs[pmsg] = pmb;

	////////////////////////////////////////////
	// ARM
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_TRACK, 0, MIX_PARAM_RECORD_ARM, 0 );

	pmsg = createMidiMsg( _T("arm"), CID_Arm);
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);	// all 0
	pmsg->SetNoteNum( 0x00 );
	pmsg->SetIsTrigger( TRUE, 0x7f );
	m_pMidiInputRouter->Add( pmsg );
	pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

	m_mapStripMsgs[pmsg] = pmb;

	return S_OK;
}


///////////////////////////////////////////////////////////////////
// Determine if encoders / faders are flipped or not
bool	CVS100::IsFlipped()
{
	bool bFlipped = false;
	// Find a VOL.  If it's Midi Message type is wheel, we're not flipped
	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		CMidiMsg* pmsg = it->first;

		if ( pmb.pParam->GetMixerParam() == MIX_PARAM_VOL && pmb.pParam->GetMixerStrip() == m_eStripType )
		{
			// this is a vol controller 
			bFlipped = (pmsg->GetMessageType() != CMidiMsg::mtWheel);
			break;
		}
	}

	return bFlipped;
}


/////////////////////////////////////////////////////////////////
void CVS100::AssignRotaries( size_t ixEncoderParam )
{
	m_ixEncoderParam = min( ixEncoderParam, m_vEncoderParam.size()-1 );
	EncoderParam ec = m_vEncoderParam[m_ixEncoderParam];

	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		CMidiMsg* pMsg = it->first;
		PMBINDING& pmb = it->second;
		if ( pMsg->GetId() == CID_StripRotary )
		{
			// set to the new param
			pmb.pParam->SetParams( pmb.pParam->GetMixerStrip(), 
											pmb.pParam->GetStripNum(), 
											ec.param, 
											ec.dwParam, 
											.5f );

			showEncoderParam( pmb.pParam );
		}
	}
}



///////////////////////////////////////////////////////////////
// Flip button has been pressed
void CVS100::OnFlip()
{
	if ( m_ixEncoderParam >= m_vEncoderParam.size() )
		return;

	EncoderParam ec = m_vEncoderParam[m_ixEncoderParam];

	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;

		SONAR_MIXER_STRIP const eStrip = pmb.pParam->GetMixerStrip();
		// it's on the group of 8 main faders
		if ( pmb.pParam->GetMixerParam() == MIX_PARAM_VOL )
		{
			// swap to m_eEncoderParam
			pmb.pParam->SetParams( eStrip, pmb.pParam->GetStripNum(), ec.param, ec.dwParam, .5f );
		}
		else if ( pmb.pParam->GetMixerParam() == ec.dwParam && pmb.pParam->GetParamNum() == ec.dwParam )
		{
			// swap to vol
			pmb.pParam->SetParams( eStrip, pmb.pParam->GetStripNum(), MIX_PARAM_VOL, 0, 101.f/127.f );
		}
	}
}


///////////////////////////////////////////////////////////////
// Build midi messages for various misc function buttons on the surface
HRESULT CVS100::buildMiscBindings()
{
	////////////////////////////////////////
	// Track Bank switching
	CMidiMsg* pmsg = createMidiMsg( _T("Prev"), CID_Prev );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("bank R"), CID_Next );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2e );
	addControlMsg( pmsg );

	// Assign button (step through rotary assignments)
	pmsg = createMidiMsg( _T("Rot Assign"), CID_RotaryAssign );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2a );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Dyn Button (use to switch Encoders into ACT mode)
	pmsg = createMidiMsg( _T("ACT"), CID_ACT );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2b );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	// Command Modifier
	pmsg = createMidiMsg( _T("Shift"), CID_Shift );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x46 );
	addControlMsg( pmsg );

	ButtonActionDefinition bad;

	// Marker view
	pmsg = createMidiMsg( _T("Marker"), CID_Marker );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x30 );
	addControlMsg( pmsg );
	bad.eActionType = CVS100::ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_INSERT_MARKER;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	// next view
	pmsg = createMidiMsg( _T("CID_View"), CID_View );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x45 );
	addControlMsg( pmsg );
	bad.eActionType = CVS100::ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_VIEW_TRACKVIEW;
	bad.wModKeys = 0;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	// Loop
	pmsg = createMidiMsg( _T("CID_Loop"), CID_Loop );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x56 );
	addControlMsg( pmsg );
	bad.eActionType = CVS100::ButtonActionDefinition::AT_Command;
	bad.dwCommandOrKey = CMD_LOOP_TOGGLE;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	// feetswitches
	pmsg = createMidiMsg( _T("Footswitch1"), CID_Feetswitch1 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x66 );
	addControlMsg( pmsg );
	bad.eActionType = CVS100::ButtonActionDefinition::AT_Transport;
	bad.transportState = TRANSPORT_STATE_PLAY;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;

	pmsg = createMidiMsg( _T("Footswitch2"), CID_Feetswitch2 );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetIsTrigger( true, 0x7f );
	pmsg->SetChannel(  0 );
	pmsg->SetNoteNum( 0x67 );
	addControlMsg( pmsg );
	bad.eActionType = CVS100::ButtonActionDefinition::AT_Transport;
	bad.transportState = TRANSPORT_STATE_REC;
	m_mapButton2BAD[(ControlId)pmsg->GetId()] = bad;


	// Full assign (deep act mode)
	// this is not a button but this message will be sent to us
	// when the user switches to full assign mode in the hardware menus
	pmsg = createMidiMsg( _T("Full Assign"), CID_FullAssign );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x2d );
	addControlMsg( pmsg );

	// ACT knobs
	PMBINDING pmb;
	pmb.nRefreshMod = RM_ODD;

	DWORD dwAct = 0;
	pmsg = createMidiMsg(_T("ACT0"), CID_Rotary0);
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x18);
	pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	m_setFineControls.insert( pmb.pParam );
	pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	m_mapACT[pmsg] = pmb;

	pmsg = createMidiMsg(_T("ACT1"), CID_Rotary1);
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x19);
	pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	m_setFineControls.insert( pmb.pParam );
	pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	m_mapACT[pmsg] = pmb;

	pmsg = createMidiMsg(_T("ACT2"), CID_Rotary2);
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x1b);
	pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	m_setFineControls.insert( pmb.pParam );
	pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	m_mapACT[pmsg] = pmb;

	// Comp button
	pmsg = createMidiMsg( _T("ACT-Comp switch"), CID_Comp );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x2e );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	pmb.pMsgOut = pmsg;	// light for the switch (use input msg)
	m_mapACT[pmsg] = pmb;

	// value encoder
	pmsg = createMidiMsg(_T("ACT Value Enc"), CID_ValueEnc );
	pmsg->SetMessageType( CMidiMsg::mtCC );
	pmsg->SetChannel( 0 );
	pmsg->SetCCNum( 0x1d);
	pmsg->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &m_vrCCW, &m_vrCW );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	pmb.pParam->SetTriggerAction( CMixParam::TA_DEFAULT );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
	m_mapACT[pmsg] = pmb;

	// push switch for encoder
	pmsg = createMidiMsg(_T("ACT Value Enc Push"), CID_ValueEncPush );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel( 0 );
	pmsg->SetNoteNum( 0x2c);
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );
	pmb.pParam = createMixParam();
	pmb.pParam->SetTriggerAction( CMixParam::TA_TOGGLE );
	pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID );
	m_mapACT[pmsg] = pmb;

	// Deep act controls (digital mixer controls)

	// pans
	CString str;
	for ( int ixc = 0; ixc < 2; ixc++ )
	{
		str.Format(_T("CID_MixerPan:%d"), ixc );
		pmsg = createMidiMsg(str, CID_MixerPan0 + ixc);
		pmsg->SetMessageType( CMidiMsg::mtCC );
		pmsg->SetChannel( ixc );
		pmsg->SetCCNum( 0x12);
		addControlMsg( pmsg );
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		pmb.pParam->SetCaptureType( CMixParam::CT_Match );
		m_mapACT[pmsg] = pmb;
	}
	
	// knobs
	for ( int ixc = 0; ixc < 6; ixc++ )
	{
		str.Format(_T("CID_MixerVol:%d"), ixc );
		pmsg = createMidiMsg(str, CID_MixerVol0 + ixc);
		pmsg->SetMessageType( CMidiMsg::mtCC );
		pmsg->SetChannel( ixc );
		pmsg->SetCCNum( 0x11);
		addControlMsg( pmsg );
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		pmb.pParam->SetCaptureType( CMixParam::CT_Match );
		m_mapACT[pmsg] = pmb;
	}

	// switches
	for ( int ixc = 0; ixc < 4; ixc++ )
	{
		str.Format(_T("CID_MixerSw:%d"), ixc );
		pmsg = createMidiMsg(str, CID_MixerSw0 + ixc);
		pmsg->SetMessageType( CMidiMsg::mtCC );
		pmsg->SetChannel( ixc );
		pmsg->SetCCNum( 0x50);
		pmsg->SetIsTrigger( true, 0x7f );
		addControlMsg( pmsg );
		pmb.pParam = createMixParam();
		pmb.pParam->SetTriggerAction( CMixParam::TA_TOGGLE );
		pmb.pParam->SetParams( MIX_STRIP_ANY, ACTKEY_BASE + dwAct++, MIX_PARAM_DYN_MAP, m_dwSurfaceID, .5f );
		pmb.pMsgOut = pmsg;	// echo state out to led
		m_mapACT[pmsg] = pmb;
	}


	return S_OK;
}


/////////////////////////////////////////////////////////////////
HRESULT CVS100::buildTransportBindings()
{
	CMidiMsg* pmsg = createMidiMsg( _T("Play"), CID_Play );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5e );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("stop"), CID_Stop );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5d );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("record"), CID_Record );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5f );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("rtz"), CID_RTZ );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x21 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("gte"), CID_GTE );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(1);
	pmsg->SetNoteNum( 0x22 );
	pmsg->SetIsTrigger( true, 0x7f );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("rew"), CID_Rew );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5b );
	addControlMsg( pmsg );

	pmsg = createMidiMsg( _T("ffw"), CID_FF );
	pmsg->SetMessageType( CMidiMsg::mtNote );
	pmsg->SetChannel(0);
	pmsg->SetNoteNum( 0x5c );
	addControlMsg( pmsg );

	return S_OK;
}

/////////////////////////////////////////////////////////////////
void	CVS100::addControlMsg( CMidiMsg* pmsg )
{
	m_pMidiInputRouter->Add( pmsg );
	m_mapControlMsgs[(ControlId)pmsg->GetId()] = pmsg;
}



////////////////////////////////////////////////////////////////////////
// For whatever reason, the host parameter bindings have changed.
// This may be because the track bank has shifted, or the ACT context has changed.
HRESULT CVS100::updateParamBindings()
{
	// get Track strip range
	StripRangeMapIterator it = m_mapStripRanges.find( m_eStripType );
	if ( it == m_mapStripRanges.end() )
		return E_UNEXPECTED;		// ranges not set up yet

	STRIPRANGE& sr = it->second;

	// For any strip oriented parameters, set the strip offset to match our current bank
	for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		pmb.pParam->SetStripNumOffset( sr.dwL );
		pmb.pParam->SetStripType( m_eStripType );
	}

	for ( InputBindingIterator it = m_mapShiftMsgs.begin(); it != m_mapShiftMsgs.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		pmb.pParam->SetStripNumOffset( sr.dwL );
		pmb.pParam->SetStripType( m_eStripType );
	}

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// The Call back from a MIDI Input Message.  Your job here is to figure out what
// to do in the the host based on which param called you back, and what its
// value is.
HRESULT	CVS100::OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue, CMidiMsg::ValueChange vtRet )
{
	TRACE("OnMIDIMessageDelivered: %s fValue=%.2f vtRet=%d\n", pObj->m_strName, fValue, vtRet);

	bool bMatch = false;
	InputBindingIterator it;

	if ( !bMatch )
	{
		// if ACT mode, try that message binding first
		if ( m_bACTMode )
		{
			it = m_mapACT.find( pObj );
			if ( m_mapACT.end() != it )
			{
				bMatch = true;

				// Value (push) encoder
				if ( pObj->GetId() == CID_ValueEnc && m_bShiftPressed )
				{
					// switch act context
					if ( fValue > .5f )
						ActivateNextFx( UIA_FOCUS );
					else
						ActivatePrevFx( UIA_FOCUS );
					pObj->SetVal( .5f );	// center it
				}
				else
				{
					PMBINDING& pmb = it->second;
					if ( pObj->IsTrigger() )
						pmb.pParam->Trigger();
					else
						pmb.pParam->SetVal( fValue, MIX_TOUCH_TIMEOUT, vtRet );
					// put our name in the act param display
					showACTParamLabel( pmb.pParam );
				}
			}
		}
	}

	if ( !bMatch )
	{
		PMBINDING* pbnd = NULL;

		if ( m_bShiftPressed )
		{
			bMatch = handleShiftStripFunctions( pObj, fValue );
		}
		
		if ( !bMatch )
		{
			it = m_mapStripMsgs.find( pObj );
			if ( m_mapStripMsgs.end() != it )
			{
				if ( m_eRotaryMode == SRM_Normal )
				{
					PMBINDING& pbm = it->second;
					bMatch = true;
					if ( pObj->IsTrigger() )
						pbm.pParam->Trigger();
					else
						pbm.pParam->SetVal( fValue, MIX_TOUCH_TIMEOUT, vtRet );
				}
				else if ( pObj->GetId() == CID_StripRotary )
				{
					if ( SRM_Jog == m_eRotaryMode )
						onJog( pObj, fValue );
					else if ( SRM_TrackSel == m_eRotaryMode )
						onRotaryTrackSel( pObj, fValue );
				}
			}
		}
	}


	if ( !bMatch )
	{
		// No match, try the Fader Touch switches
		it = m_mapFaderTouchMsgs.find( pObj );
		if ( m_mapFaderTouchMsgs.end() != it )
		{
			bMatch = true;
			PMBINDING& pmb = it->second;
			pmb.pParam->Touch( fValue > .1f );
		}
	}


	if ( !bMatch )
	{
		// is it a param-less switch?
		if ( m_mapControlMsgs.count( (ControlId)pObj->GetId() ) > 0 )
		{
			bMatch = true;
			handleButtonMsg( pObj, fValue );
		}
	}

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////
bool CVS100::handleButtonMsg( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = handleBankButton( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleTransportButton( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleCommandOrKey( pmsg, fValue );
	if ( !bHandled )
		bHandled = handleModeButton( pmsg, fValue );

	return bHandled;
}


///////////////////////////////////////////////////////////////////////
bool CVS100::handleModeButton( CMidiMsg* pmsg, float fVal )
{
	bool bHandled = true;

	switch( pmsg->GetId() )
	{
	case CID_ACT:
		m_bACTMode = !m_bACTMode;
		onActToggle();
		break;
	case CID_FullAssign:
		m_bFullAssign = TRUEFLOAT( fVal );
		if ( m_bFullAssign )
		{
			m_bACTMode = true;
			onActToggle();
		}
		break;
	case CID_RotaryAssign:
		{
			if ( m_bShiftPressed )
			{
				onJogMode( true );
			}
			else
			{
				bool bWasInSpecialMode = m_eRotaryMode != SRM_Normal;
				onJogMode( false );

				if ( !bWasInSpecialMode )
				{
					size_t ix = m_ixEncoderParam;
					ix++;
					if ( ix >= m_vEncoderParam.size() )
						ix = 0;
					AssignRotaries( ix );
				}
			}
		}
		break;
	default:
		bHandled = false;
	}

	return bHandled;
}


bool CVS100::handleShiftStripFunctions( CMidiMsg* pmsg, float fValue )
{
	bool bMatch = false;

	InputBindingIterator it = m_mapShiftMsgs.find( pmsg );
	if ( m_mapShiftMsgs.end() != it )
	{
		PMBINDING& pbm = it->second;
		bMatch = true;
		if ( pmsg->IsTrigger() )
			pbm.pParam->Trigger();
		else
			pbm.pParam->SetVal( fValue );
	}
	else
	{
		DWORD dwS = getActiveStrip();

		if ( pmsg->GetId() == CID_Solo )	// read
		{
			bMatch = true;
			ClearSolo();
		}
		else if ( pmsg->GetId() == CID_Arm ) // write 
		{
			bMatch = true;
			bool b = GetWriteMode( dwS, m_eStripType );
			SetWriteMode( dwS, m_eStripType, !b );
		}
	}

	return bMatch;
}


///////////////////////////////////////////////////////////////////////
bool CVS100::handleCommandOrKey( CMidiMsg* pmsg, float fvalue )
{
	ControlId cid = (ControlId)pmsg->GetId();

	MsgIdToBADMapIterator it = m_mapButton2BAD.find( (ControlId)pmsg->GetId() );
	if ( m_mapButton2BAD.end() != it )
	{
		// get a copy ( we may modify this based on temp shift status)
		ButtonActionDefinition bad = it->second;
		if ( bad.eActionType == ButtonActionDefinition::AT_Key )
		{
			// handle as a Key
			if( !m_pSonarKeyboard )
				return true;

			if ( pmsg->IsTrigger() )
			{
				// for Triggers, do Down/Up surrounded by modifier keys
				if ( bad.wModKeys & SMK_CTRL )
				{
					TRACE(_T("Sending Ctrl Down\n"));
					m_pSonarKeyboard->KeyboardEvent( VK_CONTROL, SKE_KEYDOWN );
				}
				if ( bad.wModKeys & SMK_SHIFT )
				{
					TRACE(_T("Sending Shift Down\n"));
					m_pSonarKeyboard->KeyboardEvent( VK_SHIFT, SKE_KEYDOWN );
				}

				TRACE(_T("Sending key(%d) Down\n"), bad.dwCommandOrKey);
				m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYDOWN );
				TRACE(_T("Sending key(%d) Up\n"), bad.dwCommandOrKey);
				m_pSonarKeyboard->KeyboardEvent( bad.dwCommandOrKey, SKE_KEYUP );

				if ( bad.wModKeys & SMK_SHIFT )
				{
					TRACE(_T("Sending shift Up\n"));
					m_pSonarKeyboard->KeyboardEvent( VK_SHIFT, SKE_KEYUP );
				}
				if ( bad.wModKeys & SMK_CTRL )
				{
					TRACE(_T("Sending ctrl Up\n"));
					m_pSonarKeyboard->KeyboardEvent( VK_CONTROL, SKE_KEYUP );
				}
			}
			else
			{
				// regular key
				if ( TRUEFLOAT( fvalue ))
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
		}
		else if ( bad.eActionType == ButtonActionDefinition::AT_Command )
		{
			if ( m_pSonarCommands )
			{
				// for the view key, modify the action if the shift key is pressed
				if ( pmsg->GetId() == CID_View && m_bShiftPressed )
				{
					bad.dwCommandOrKey = CMD_VIEW_CONSOLE;
				}
				if ( bad.dwCommandOrKey == CMD_VIEW_SYNTH_RACK )
				{
					m_pSonarCommands->DoCommand( CMD_VIEW_SYNTH_RACK );
					if ( m_pSonarUIContext )
					{
						if ( !SUCCEEDED( m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, 0, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS ) ) )
							m_pSonarUIContext->SetUIContext( MIX_STRIP_RACK, -1, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS );
					}
				}
				else if ( CID_Loop == cid )
				{
					if ( m_bShiftPressed )
						m_pSonarCommands->DoCommand( CMD_SET_LOOP_FROM_SELECTION );
					else
						m_pSonarCommands->DoCommand( CMD_LOOP_TOGGLE );
				}
				// if it's a trigger or the button is being released, do the command
				else if ( pmsg->IsTrigger() || !(TRUEFLOAT(fvalue )))
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
	else if ( CID_Shift == cid )
	{
		m_bShiftPressed = TRUEFLOAT( fvalue );
		onShift();
	}

	return false;
}


///////////////////////////////////////////////////////////////////////
bool CVS100::handleBankButton( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = true;

	StripRangeMapIterator it = m_mapStripRanges.find( m_eStripType );
	if ( it != m_mapStripRanges.end() )
	{
		STRIPRANGE& sr = it->second;
		int nTrackOrg = (int)sr.dwL;

		switch( pmsg->GetId() )
		{
		case 	CID_Prev:
			if ( TRUEFLOAT( fValue ) )
			{
				if ( m_bShiftPressed )
				{
					SONAR_MIXER_STRIP e = m_eStripType == MIX_STRIP_TRACK ? MIX_STRIP_BUS : MIX_STRIP_TRACK;
					onStripType( e );
				}
				else
				{
					nTrackOrg -= 1;
					m_eRotaryMode = SRM_TrackSel;
				}
			}
			else
				m_eRotaryMode = SRM_Normal;
			break;
		case 	CID_Next:
			if ( TRUEFLOAT( fValue ) )
			{
				if ( m_bShiftPressed )
				{
					SONAR_MIXER_STRIP e = m_eStripType == MIX_STRIP_TRACK ? MIX_STRIP_BUS : MIX_STRIP_TRACK;
					onStripType( e );
				}
				else
				{
					nTrackOrg += 1;
					m_eRotaryMode = SRM_TrackSel;
				}
			}
			else
				m_eRotaryMode = SRM_Normal;
			break;
		default:
			bHandled = false;
		}

		if ( bHandled )
		{
			doSelectStrip( nTrackOrg );

			updateParamBindings();
			SetHostContextSwitch();
		}
	}

	return bHandled;
}

//---------------------------------------------------------------
// Handle a transport state button
bool CVS100::handleTransportButton( CMidiMsg* pmsg, float fValue )
{
	bool bHandled = true;
	switch( pmsg->GetId() )
	{
		case CID_Play:
			Play( );
			break;
		case CID_Stop:
			Stop();
			break;
		case CID_Record:
			Record( true );
			break;
		case CID_RTZ:
			if ( m_bShiftPressed )
				GotoPrevMarker();
			else
				GotoStart( true );
			break;
		case CID_GTE:
			if ( m_bShiftPressed )
				GotoNextMarker();
			else
				GotoEnd( true );
			break;
		case CID_Rew:
			if ( m_bShiftPressed )
			{
				if ( TRUEFLOAT( fValue ) )
					m_pSonarCommands->DoCommand( CMD_GOTO_PREV_MEAS );
			}
			else
				Rewind( TRUEFLOAT( fValue) ? 10 : 0 );
			break;
		case CID_FF:
			if ( m_bShiftPressed )
			{
				if ( TRUEFLOAT( fValue ) )
					m_pSonarCommands->DoCommand( CMD_GOTO_NEXT_MEAS );
			}
			else
				FastForward( TRUEFLOAT( fValue ) ? 10 : 0 );
			break;
		default:
			bHandled = false;
	}
	return bHandled;
}


//---------------------------------------------
void CVS100::onShift()
{
	// Shift has toggled.  Based on the shift state, put certain rotaries in FINE mode
	CMixParam::CaptureType ct = m_bShiftPressed ? CMixParam::CT_Step : CMixParam::CT_Jump;
	for ( MixParamSetIterator it = m_setFineControls.begin(); it != m_setFineControls.end(); ++it )
	{
		CMixParam* p = *it;
		p->SetCaptureType( ct );
	}
}


void CVS100::onActToggle()
{
	ControlMessageMapIterator it = m_mapControlMsgs.find( CID_ACT );
	if ( m_mapControlMsgs.end() != it )
	{
		CMidiMsg* pmsg = it->second;
		pmsg->Send( m_bACTMode ? 1.f : 0.f );
	}
}

void CVS100::onStripType( SONAR_MIXER_STRIP e )
{
	m_eStripType = e;
	
	switch( m_eStripType )
	{
	case MIX_STRIP_TRACK:
		m_pLCD->ShowText( "Track", 1, 0 );
		break;
	case MIX_STRIP_BUS:
		m_pLCD->ShowText( "Bus", 1, 0 );
		break;
	}	
	updateParamBindings();
}




//----------------------------------------------------------
void CVS100::onRotaryTrackSel( CMidiMsg* pObj, float f )
{
	StripRangeMapIterator it = m_mapStripRanges.find( m_eStripType );
	if ( it != m_mapStripRanges.end() )
	{
		STRIPRANGE& sr = it->second;
		int nTrackOrg = (int)sr.dwL;
		if ( f > .5 )
			nTrackOrg++;
		else
			nTrackOrg--;

		pObj->SetVal( .5f );	// center it again
		doSelectStrip( nTrackOrg );
	}
}

//----------------------------------------------------------
void CVS100::onJog( CMidiMsg* pObj,float f )
{
	TRACE( "Jog f=%.2f\n", f );
	JogAmount ja = JA_3;
	if ( ::fabs(f-.5f) > .03 )
		ja = JA_4;
	jog( f > .5f ? JD_CW : JD_CCW, ja);

	pObj->SetVal(.5f );
}


void CVS100::onJogMode( bool b )
{
	m_eRotaryMode = b? SRM_Jog : SRM_Normal;

	if ( SRM_Jog == m_eRotaryMode )
		startBlink( CID_RotaryAssign );
	else
		stopBlink( CID_RotaryAssign );
}


using namespace SurfaceTransportTimeUtils;
///////////////////////////////////////////////////////////////////////////////////////
void	CVS100::jog( JogDirection jd, JogAmount ja )
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
			iDir *= bScrub ? 3 : 8;
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




//----------------------------------------------------------
// Return teh index of the current active strip.
DWORD CVS100::getActiveStrip()
{
	// find the current strip range for the current strip type
	StripRangeMapIterator srit = m_mapStripRanges.find( m_eStripType );
	if ( srit == m_mapStripRanges.end() )
	{
		ASSERT(0);
		return 0;
	}
	STRIPRANGE& sr = srit->second;
	return sr.dwL;
}

// Initialize things here that are dependent upon persisted values
HRESULT CVS100::onFirstRefresh()
{
	onActToggle();
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Called after the base class connects
void CVS100::onConnect()
{
	onStripType( m_eStripType );
	buildBindings();
	requestFullAssignState();
}

/////////////////////////////////////////////////////////////////////////////
// Called before the base class disconnects
void CVS100::onDisconnect()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Always Leave with ACT mode off per RJA
	m_bACTMode = false;
	onActToggle();

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

}

//-----------------------------------------------------------------------
// Set the current active strip, update WAI and also set the
// active track in the host
void	CVS100::doSelectStrip( int ixStrip )
{
	int iMax = (int)GetStripCount( m_eStripType );
	ixStrip = min( ixStrip, iMax-1);
	ixStrip = max( 0, ixStrip );
	
	SetStripRange( (DWORD)ixStrip, m_eStripType );

	// also if the 8strip type is Track, update the host's active track
   if ( MIX_STRIP_TRACK == m_eStripType )
	{
      SetSelectedTrack( (DWORD)ixStrip );
	}
	updateParamBindings();
	RequestStatusQuery();

   // update ACT context to the track selected
   if ( m_pSonarUIContext )
      m_pSonarUIContext->SetUIContext( m_eStripType, (DWORD)ixStrip, MIX_PARAM_FX_PARAM, -1, UIA_FOCUS );
}




///////////////////////////////////////////////////////////
// Helper to delete a CMidiMsg object and remove it from the input router
void	CVS100::destroyMidiMsg( CMidiMsg*& pmsg )
{
	m_pMidiInputRouter->Remove( pmsg );
	delete pmsg;
	pmsg = NULL;
}


void CVS100::requestFullAssignState()
{
	m_FullAssignReqMidiMsg.Send( 1.f );
}

//--------------------------------------------------------------
// return the index of the current marker
DWORD CVS100::getCurrentMarker()
{
	MFX_TIME time;
	time.timeFormat = TF_MBT;
	DWORD dwIx = 0;
	if ( SUCCEEDED( m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &time ) ) )
	{
		if ( m_pSonarProject2 )
			m_pSonarProject2->GetMarkerIndexForTime( &time, &dwIx );
	}

	return dwIx;
}

//---------------------------------------------------------------
// set transport time to that of the next marker after the current now time
void CVS100::GotoNextMarker()
{
	if ( m_pSonarProject2 )
	{
		DWORD dwCount = 0;
		m_pSonarProject2->GetMarkerCount( &dwCount );
		DWORD dwIx = getCurrentMarker();

		// if we're before this marker, don't increment
		MFX_TIME timeMkr;
		timeMkr.timeFormat = TF_SECONDS;
		m_pSonarProject2->GetMarkerTime( dwIx, &timeMkr );

		MFX_TIME timeNow;
		timeNow.timeFormat = TF_SECONDS;
		m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &timeNow  );

		if ( timeNow.dSeconds >= timeMkr.dSeconds )
			++dwIx;
		if ( dwIx < dwCount )
			GotoMarker( dwIx, true );
	}
}


//---------------------------------------------------------------
// set transport time to that of the previous marker before the current now time
void CVS100::GotoPrevMarker()
{
	if ( m_pSonarProject2 )
	{
		DWORD dwCount = 0;
		m_pSonarProject2->GetMarkerCount( &dwCount );
		int iIx = (int)getCurrentMarker();

		// if we're after this marker, don't decrement.  Instead we want to
		// to TO this marker
		// if we're before this marker, don't increment
		MFX_TIME timeMkr;
		timeMkr.timeFormat = TF_TICKS;
		m_pSonarProject2->GetMarkerTime( iIx, &timeMkr );

		MFX_TIME timeNow;
		timeNow.timeFormat = TF_TICKS;
		m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &timeNow  );

		if ( timeNow.lTicks == timeMkr.lTicks )
		{
			if ( iIx > 0 )
				iIx--;
		}
		GotoMarker( (DWORD)iIx, true );
	}
}


void CVS100::Stop()
{
	if ( IsPause() )
		Pause( false );

	if ( m_pSonarTransport )
		m_pSonarTransport->SetTransportState( TRANSPORT_STATE_PLAY, FALSE );
}

void CVS100::Play( )
{
	if ( !m_pSonarTransport )
		return;

	if ( IsPlaying() )
		Pause( true );
	else if ( m_bShiftPressed )
		Audition();
	else
		m_pSonarTransport->SetTransportState( TRANSPORT_STATE_PLAY, true );
}

void CVS100::Pause( bool b )
{
	__super::Pause( b );

	if ( b )
		startBlink( CID_Play );
	else
		stopBlink( CID_Play );
}

void CVS100::Record( bool bToggle )
{
	if ( !m_pSonarTransport )
		return;

	BOOL bRec = TRUE;
	if ( bToggle )
	{
		BOOL b;
		m_pSonarTransport->GetTransportState( TRANSPORT_STATE_REC, &b );
		bRec = !b;
	}
	m_pSonarTransport->SetTransportState( TRANSPORT_STATE_REC, bRec );
}


void CVS100::VZoom( int n )
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


void CVS100::HZoom( int n )
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


/////////////////////////////////////////////////////////////////////////
HRESULT CVS100::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (pdwLen == NULL)
		return E_POINTER;

	// Strip Name
	char sz[128];

	char szStrip[128];
	DWORD cb = _countof(szStrip);
	szStrip[0] = '\0';
	cb = 127;
	m_pSonarMixer->GetMixStripName( m_eStripType, getActiveStrip(), szStrip, &cb );

	// context name
	char szContext[128];
	cb = _countof(szContext);
	szContext[0] = '\0';
   m_pSonarParamMapping->GetMapName( m_dwSurfaceID, (LPSTR) &szContext, &cb );

	::sprintf( sz, "%s ACT: %s", szStrip, szContext );

	// Return results to caller
	if (NULL == pszStatus)
	{
		*pdwLen = (DWORD)::strlen(sz) + 1;
	}
	else
	{
		::strncpy(pszStatus, sz, *pdwLen);
	}

	return S_OK;
}

