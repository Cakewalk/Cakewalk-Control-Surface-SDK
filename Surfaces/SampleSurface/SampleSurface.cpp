// ACTController.cpp : Defines the basic functionality of the "ACTController".
//

#include "stdafx.h"

#include "SampleSurface.h"
#include "SampleSurfacePropPage.h"
#include "sfkTransportTimeUtils.h"
#include "MixParam.h"


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

CSampleSurface::CSampleSurface() :
	CControlSurface()
	,m_pmsgBankL(0)
	,m_pmsgBankR(0)
	,m_pmsgTrkL(0)
	,m_pmsgTrkR(0)
	,m_pmsgPlay(0)
	,m_pmsgStop(0)
	,m_pmsgRec(0)
{ 
	::InterlockedIncrement( &g_lComponents );

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CSampleSurface::~CSampleSurface() 
{ 
	::InterlockedDecrement( &g_lComponents );
}



/////////////////////////////////////////////////////////////////////////////
// This is the IControlSurface way of obtaining the MIDI mask.  Older versions 
// of SONAR will only call this instead of IControlSurface3::GetNoEchoStatusMessages(). 
// This function has much less granularity because it only returns a bitmask for the MIDI
// channels in use.  However for a dedicated control surface this is probably just fine.
HRESULT CSampleSurface::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
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
HRESULT CSampleSurface::GetNoEchoStatusMessages(WORD** ppwStatus, DWORD* pdwCount )
{
	*pdwCount =  0;
	return S_OK;
}



//////////////////////////////////////////////////////////////////
// Set the base strip for a given strip type.
// Note: this is exposed to the host via ISurfaceParamMapping
HRESULT CSampleSurface::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
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

	if ( bChange )
	{
		updateParamBindings();
		SetHostContextSwitch();
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// Return the count of ACT parameters you are maintaining
HRESULT CSampleSurface::GetDynamicControlCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	CSFKCriticalSectionAuto csa( m_cs );

	*pdwCount = 0;

	return S_OK;
}

//////////////////////////////////////////////////////////////////
// return info about the nth ACT parameter you are maintaining
HRESULT CSampleSurface::GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *)
{
	return S_OK;
}


//////////////////////////////////////////////////////////////////
// Host is notifying us of a ACT Learn state change.
HRESULT CSampleSurface::SetLearnState( BOOL bLearning )
{
	// You should tell all CMixParam objects that are for ACT that
	// they are in Learn mode now.  This switches off any capture modes
	// while learning

	// call base class to set the global bit
	return CControlSurface::SetLearnState( bLearning );
}

//////////////////////////////////////////////////////////////////////
// Create a mix param on the heap and put it in the every mix param set
CMixParam* CSampleSurface::createMixParam()
{
	CMixParam* pParam = new CMixParam;
	pParam->SetInterfaces( m_pSonarMixer, m_pSonarTransport, m_dwSurfaceID );
	m_setEveryMixparam.insert( pParam );
	return pParam;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Build all MIDI and Parameter Binding tables
HRESULT	CSampleSurface::buildBindings()
{
	// Build the strip ranges
	m_mapStripRanges.clear();
	STRIPRANGE sr;
	sr.dwL = 0;					// todo: persisted?
	sr.dwH = sr.dwL + 7; 
	m_mapStripRanges[MIX_STRIP_TRACK] = sr;

	buildStripBindings();
	buildMiscBindings();
	buildTransportBindings();

	updateParamBindings();

	return CControlSurface::buildBindings();
}


//--------------------------------------------------------
// Create the Midi message and Parameter objects for
// all per-strip parameters.  
HRESULT CSampleSurface::buildStripBindings()
{
	// mackie faders
	for ( int i = 0; i < 8; i++ )
	{
		PMBINDING pmb;
		pmb.nRefreshMod = RM_EVERY;	// faders have motors - update them as often as possible

		// create a mix param for vol. This will have TWO Input messages associated
		// with it (fader, touch switch), and one output message (motor).
		pmb.pParam = createMixParam();

		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_VOL, 0, 101.0f/127 );

		// Fader Input Messages
		CMidiMsg* pmsg = new CMidiMsg(this);
		pmsg->SetMessageType( CMidiMsg::mtWheel );
		pmsg->SetChannel( i );
		m_pMidiInputRouter->Add( pmsg );

		// Fader Motor output messages (use input)
		pmb.pMsgOut = pmsg;

		m_mapStripContinuous[pmsg] = pmb;

		// Fader touch switch
		pmb.pMsgOut = NULL;	// no output
		CMidiMsg* pmsgT = new CMidiMsg(this);
		pmsgT->SetMessageType( CMidiMsg::mtNote );
		pmsgT->SetChannel(0);	// all 0
		pmsgT->SetNoteNum( 0x68 + i );
		m_pMidiInputRouter->Add( pmsgT );

		m_mapFaderTouch[pmsgT] = pmb;

		// the rest of the params can be interlaced
		pmb.nRefreshMod = (i % 2) == 0 ? RM_EVEN : RM_ODD;

		////////////////////////////////////////////
		// Encoder
		// create a mix param for vol. This will have TWO Input messages associated
		// with it (fader, touch switch), and one output message (motor).
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_PAN, 0, 101.0f/127 );

		// Fader Input Messages
		CMidiMsg* pmsgR = new CMidiMsg(this);
		pmsgR->SetMessageType( CMidiMsg::mtCC );
		pmsgR->SetChannel( 0 );
		pmsgR->SetCCNum( 0x10 + i );
		CMidiMsg::ValueRange vrCCW, vrCW;
		vrCCW.dwL = 65;
		vrCCW.dwH = 127;
		vrCW.dwL = 1;
		vrCW.dwH = 64;


		pmsgR->SetValueMode( CMidiMsg::VM_DELTA_ACCEL, &vrCCW, &vrCW );
		m_pMidiInputRouter->Add( pmsgR );

		// Encoder Led ring
		pmb.pMsgOut = pmsgR;	// use input

		m_mapStripContinuous[pmsgR] = pmb;


		////////////////////////////////////////////
		// MUTES
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_MUTE, 0 );

		pmsg = new CMidiMsg(this);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x10 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)
		m_mapStripTrigger[pmsg] = pmb;


		////////////////////////////////////////////
		// SOLO
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_SOLO, 0 );

		pmsg = new CMidiMsg(this);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x08 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

		m_mapStripTrigger[pmsg] = pmb;

		////////////////////////////////////////////
		// ARM
		pmb.pParam = createMixParam();
		pmb.pParam->SetParams( MIX_STRIP_TRACK, i, MIX_PARAM_RECORD_ARM, 0 );

		pmsg = new CMidiMsg(this);
		pmsg->SetMessageType( CMidiMsg::mtNote );
		pmsg->SetChannel(0);	// all 0
		pmsg->SetNoteNum( 0x00 + i );
		pmsg->SetIsTrigger( TRUE, 0x7f );
		m_pMidiInputRouter->Add( pmsg );
		pmb.pMsgOut = pmsg;	// light for the switch (use input msg)

		m_mapStripTrigger[pmsg] = pmb;

	}
	return S_OK;
}

///////////////////////////////////////////////////////////////
// Build midi messages for various misc function buttons on the surface
HRESULT CSampleSurface::buildMiscBindings()
{
	////////////////////////////////////////
	// Track Bank switching
	m_pmsgBankL = new CMidiMsg( this );
	m_setBankSwitches.insert( m_pmsgBankL );
	m_pmsgBankL->SetMessageType( CMidiMsg::mtNote );
	m_pmsgBankL->SetChannel(0);
	m_pmsgBankL->SetNoteNum( 0x2e );
	m_pMidiInputRouter->Add( m_pmsgBankL );

	m_pmsgBankR = new CMidiMsg( this );
	m_setBankSwitches.insert( m_pmsgBankR );
	m_pmsgBankR->SetMessageType( CMidiMsg::mtNote );
	m_pmsgBankR->SetChannel(0);
	m_pmsgBankR->SetNoteNum( 0x2f );
	m_pMidiInputRouter->Add( m_pmsgBankR );

	m_pmsgTrkL = new CMidiMsg( this );
	m_setBankSwitches.insert( m_pmsgTrkL );
	m_pmsgTrkL->SetMessageType( CMidiMsg::mtNote );
	m_pmsgTrkL->SetChannel(0);
	m_pmsgTrkL->SetNoteNum( 0x30 );
	m_pMidiInputRouter->Add( m_pmsgTrkL );

	m_pmsgTrkR = new CMidiMsg( this );
	m_setBankSwitches.insert( m_pmsgTrkR );
	m_pmsgTrkR->SetMessageType( CMidiMsg::mtNote );
	m_pmsgTrkR->SetChannel(0);
	m_pmsgTrkR->SetNoteNum( 0x31 );
	m_pMidiInputRouter->Add( m_pmsgTrkR );


	return S_OK;
}


HRESULT CSampleSurface::buildTransportBindings()
{

	m_pmsgPlay = new CMidiMsg( this );
	m_setTransportSwitches.insert( m_pmsgPlay );
	m_pmsgPlay->SetMessageType( CMidiMsg::mtNote );
	m_pmsgPlay->SetChannel(0);
	m_pmsgPlay->SetNoteNum( 0x5e );
	m_pmsgPlay->SetIsTrigger( true, 0x7f );
	m_pMidiInputRouter->Add( m_pmsgPlay );

	m_pmsgStop = new CMidiMsg( this );
	m_setTransportSwitches.insert( m_pmsgStop );
	m_pmsgStop->SetMessageType( CMidiMsg::mtNote );
	m_pmsgStop->SetChannel(0);
	m_pmsgStop->SetNoteNum( 0x5d );
	m_pmsgStop->SetIsTrigger( true, 0x7f );
	m_pMidiInputRouter->Add( m_pmsgStop );

	m_pmsgRec = new CMidiMsg( this );
	m_setTransportSwitches.insert( m_pmsgRec );
	m_pmsgRec->SetMessageType( CMidiMsg::mtNote );
	m_pmsgRec->SetChannel(0);
	m_pmsgRec->SetNoteNum( 0x5f );
	m_pmsgRec->SetIsTrigger( true, 0x7f );
	m_pMidiInputRouter->Add( m_pmsgRec );
	
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// For whatever reason, the host parameter bindings have changed.
// This may be because the track bank has shifted, or the ACT context has changed.
HRESULT CSampleSurface::updateParamBindings()
{
	// get Track strip range
	StripRangeMapIterator it = m_mapStripRanges.find( MIX_STRIP_TRACK );
	if ( it == m_mapStripRanges.end() )
		return E_UNEXPECTED;		// ranges not set up yet

	STRIPRANGE& sr = it->second;

	// For any strip oriented parameters, set the strip offset to match our current bank
	for ( InputBindingIterator it = m_mapStripContinuous.begin(); it != m_mapStripContinuous.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		pmb.pParam->SetStripNumOffset( sr.dwL );
	}

	for ( InputBindingIterator it = m_mapStripTrigger.begin(); it != m_mapStripTrigger.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		pmb.pParam->SetStripNumOffset( sr.dwL );
	}

	// ACT and other non-strip params should be rebound too


	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// The Call back from a MIDI Input Message.  Your job here is to figure out what
// to do in the the host based on which param called you back, and what its
// value is.
HRESULT	CSampleSurface::OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue )
{
	bool bMatch = false;
	InputBindingIterator it = m_mapStripContinuous.find( pObj );
	if ( m_mapStripContinuous.end() != it )
	{
		bMatch = true;
		PMBINDING& pmb = it->second;
		pmb.pParam->SetVal( fValue );
	}

	if ( !bMatch )
	{
		// Look in the strip trigger map
		it = m_mapStripTrigger.find( pObj );
		if ( m_mapStripTrigger.end() != it )
		{
			bMatch = true;
			PMBINDING& pmb = it->second;
			pmb.pParam->ToggleBooleanParam();
		}
	}

	if ( !bMatch )
	{
		// No match, try the Fader Touch switches
		it = m_mapFaderTouch.find( pObj );
		if ( m_mapFaderTouch.end() != it )
		{
			PMBINDING& pmb = it->second;
			pmb.pParam->Touch( fValue > .1f );
		}
	}
	if ( !bMatch )
	{
		if ( m_setBankSwitches.count( pObj ) > 0 )
		{
			bMatch = true;

			StripRangeMapIterator it = m_mapStripRanges.find( MIX_STRIP_TRACK );
			if ( it != m_mapStripRanges.end() )
			{
				STRIPRANGE& sr = it->second;
				int nTrackOrg = (int)sr.dwL;

				// track bank switches?	// opt: put these in a map to quickly narrow them down?
				if ( pObj == m_pmsgBankL )
					nTrackOrg -= 8;
				else if ( pObj == m_pmsgBankR )
					nTrackOrg += 8;
				else if ( pObj == m_pmsgTrkL )
					nTrackOrg -= 1;
				else if ( pObj == m_pmsgTrkR )
					nTrackOrg += 1;

				nTrackOrg = min( (int)(GetStripCount( MIX_STRIP_TRACK ) - 8), nTrackOrg );
				nTrackOrg = max( 0, nTrackOrg );

				updateParamBindings();
				SetHostContextSwitch();
			}
		}
		if ( !bMatch )
		{
			if ( m_setTransportSwitches.count( pObj ) > 0 )
			{
				bMatch = true;
				if ( pObj == m_pmsgPlay )
				{
					Play( true );
				}
				else if ( pObj == m_pmsgStop )
				{
					Stop();
				}
				else if ( pObj == m_pmsgRec )
				{
					Record( true );
				}
			}
		}
	}

	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// Called after the base class connects
void CSampleSurface::onConnect()
{
	buildBindings();
}

/////////////////////////////////////////////////////////////////////////////
// Called before the base class disconnects
void CSampleSurface::onDisconnect()
{
	// Destroy all mix params and MIDI message objects
	for ( InputBindingIterator it = m_mapStripContinuous.begin(); it != m_mapStripContinuous.end(); ++it )
		deleteBinding( it );
	m_mapStripContinuous.clear();

	// Strip trigger map
	for ( InputBindingIterator it = m_mapStripTrigger.begin(); it != m_mapStripTrigger.end(); ++it )
		deleteBinding( it );
	m_mapStripTrigger.clear();

	// Fader touch map
	for ( InputBindingIterator it = m_mapFaderTouch.begin(); it != m_mapFaderTouch.end(); ++it )
		deleteBinding( it );


	// misc Midi Mesages
	destroyMidiMsg( m_pmsgBankL );
	destroyMidiMsg( m_pmsgBankR );
	destroyMidiMsg( m_pmsgTrkL );
	destroyMidiMsg( m_pmsgTrkR );

	// transport Midi Messages
	destroyMidiMsg( m_pmsgPlay );
	destroyMidiMsg( m_pmsgStop );
	destroyMidiMsg( m_pmsgRec );


	// now clean up the every mix param collection
	for ( MixParamSetIterator it = m_setEveryMixparam.begin(); it != m_setEveryMixparam.end(); ++it )
	{
		delete *it;
	}
	m_setEveryMixparam.clear();
}


///////////////////////////////////////////////////////////
// Helper to delete a CMidiMsg object and remove it from the input router
void	CSampleSurface::destroyMidiMsg( CMidiMsg*& pmsg )
{
	m_pMidiInputRouter->Remove( pmsg );
	delete pmsg;
	pmsg = NULL;
}



//////////////////////////////////////////////////////////////////
// Helper to deleting a Pamater-Message binding given an 
// iterator from a binding map
void CSampleSurface::deleteBinding( InputBindingIterator it )
{
	PMBINDING& pmb = it->second;
	CMidiMsg* pMsgIn = it->first;

	// Input and output message may be the same.  Don't delete same
	// pointer twice
	if ( pMsgIn != pmb.pMsgOut )
		delete pmb.pMsgOut;

	destroyMidiMsg( pMsgIn );
}



HRESULT CSampleSurface::SetHostContextSwitch( )
{
	// More important for ACT controls in Match Mode.  Reset the
	// value history on a context switch so that Nulling get's re-discovered

	return CControlSurface::SetHostContextSwitch();
}


void CSampleSurface::Stop()
{
	if ( m_pSonarTransport )
		m_pSonarTransport->SetTransportState( TRANSPORT_STATE_PLAY, FALSE );
}

void CSampleSurface::Play( bool bToggle )
{
	if ( !m_pSonarTransport )
		return;

	BOOL bPlay = TRUE;
	if ( bToggle )
	{
		BOOL b;
		m_pSonarTransport->GetTransportState( TRANSPORT_STATE_PLAY, &b );
		bPlay = !b;
	}
	m_pSonarTransport->SetTransportState( TRANSPORT_STATE_PLAY, bPlay );
}

void CSampleSurface::Record( bool bToggle )
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


void CSampleSurface::VZoom( int n )
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


void CSampleSurface::HZoom( int n )
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


