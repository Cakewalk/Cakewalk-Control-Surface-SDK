/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixSurface.cpp : Methods in CControlSurface specific to MotorMix
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotorMixStrip.h"
#include "MotorMixQueries.h"
#include "MotorMixSubclasses.h"
#include "MotorMixSurface.h"
#include "StateDefs.h"


/////////////////////////////////////////////////////////////////////////
// SurfaceImp factory
CSurfaceImp* CControlSurface::createSurfaceImp()
{
	return new CMMSurface( this );
}

/////////////////////////////////////////////////////////////////////////
// CMMSurface:
/////////////////////////////////////////////////////////////////////////
CMMSurface::CMMSurface( CControlSurface* pSurface ) :
	CSurfaceImp( pSurface ),
	m_pTheBank( NULL ),
	m_pssBaseTrack( NULL ),
	m_pssBaseMain( NULL ),
	m_pssBaseBus( NULL ),
	m_pssCurrentTrack( NULL ),
	m_pssCurrentMain( NULL ),
	m_pssCurrentBus( NULL ),
	m_pssCurrentEffect( NULL ),
	m_pssBaseEffectParam( NULL ),
	m_pssTrackRotaryMapper( NULL ),
	m_pssBusRotaryMapper( NULL ),
	m_pssSendPanOrLevel( NULL ),
	m_pmmChoiceLedUpdater( NULL ),
	m_pssBankShiftMode( NULL ),
	m_pssBurnButtonsChoice( NULL ),
	m_pssMultiButtonsChoice( NULL ),
	m_pssContainerClass( NULL ),
	m_pssMeterEnable( NULL ),
	m_pssShiftKey( NULL ),
	m_pssQueryState( NULL ),
	m_pMotorMixTransport( NULL ),
	m_pqueryMeterMode( NULL ),
	m_pMotorMixLocator( NULL ),
	m_pMotorMixChooseInsert( NULL ),
	m_pMotorMixContainerMode( NULL ),
	m_pssInsertEditMode( NULL ),
	m_pssFineTweakMode( NULL ),
	m_pmmContainerPicker( NULL ),
	m_msgBtnUpperLeft( pSurface ),
	m_msgBtnUpperRight( pSurface ),
	m_msgRightBtns( pSurface ),
	m_msgLeftBtns( pSurface ),
	m_msgEncoder( pSurface ),
	m_msgEncoderPush( pSurface ),
	m_msgScribble( pSurface )
{
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMSurface::Initialize()
{
	BOOL bHostAudioMeters = FALSE;
	CComPtr<ISonarIdentity> pIsid;
	MM_CHECKHR( m_pSurface->GetSonarIdentity( &pIsid ) );

	if( pIsid )
		bHostAudioMeters = S_OK == pIsid->HasCapability( CAP_AUDIO_METERS );

	// state shifter which remembers
	// which kind of container is being looked at (mains, auxes or tracks)
	m_pssContainerClass = new CStateShifter( m_pSurface, stContainerClass, ccTracks );
	if (m_pssContainerClass == NULL)
		return E_OUTOFMEMORY;
	m_pssContainerClass->SetMinState( ccTracks );
	m_pssContainerClass->SetMaxState( ccMains );


	// if the host has audio meters, make a state shifter and query object for metering
	if ( bHostAudioMeters )
	{
		m_pssMeterEnable = new CStateShifter( m_pSurface, stMeterMode, meMeterOff );
		if ( !m_pssMeterEnable )
			return E_OUTOFMEMORY;
		m_pssMeterEnable->SetMinState( meMeterOff );
		m_pssMeterEnable->SetMaxState( meMeterOn );

	}

	// we will not map specific buttons to do the state transitions, for the
	// container class state shifter.
	// instead one of the motor mix queries will control this setting


	// m_msgBtnUpperLeft all the buttons on the upper left rectangle on the MotorMix.
	// This includes the arrows (left right, up down)
	// and the Burn buttons.
	m_msgBtnUpperLeft.SetMessageType( CMidiMsg::mtCCSel );
	m_msgBtnUpperLeft.SetCCSelNum( 0x0f );
	m_msgBtnUpperLeft.SetCCSelVal( 0x0A );
	m_msgBtnUpperLeft.SetCCNum( 0x2F );

	m_pssShiftKey = new CStateShifter( m_pSurface, stShiftKey, skShiftOut );
	if (m_pssShiftKey == NULL)
		return E_OUTOFMEMORY;


	// By default, every state is persistent. This means that
	// reopening a previously saved project will restore the
	// state of each shifter to the same as when saved.
	// However, because of the transient nature of some states, we
	// do not want all of them persistent. Such as the shift key:
	//
	// Keep in mind we do save all states. The persistent flag
	// only takes action at *LOAD* time. So, if in the future you change
	// your mind and decide a state was meant to be persistent,
	// it is not too late to change it back because its value was
	// saved all along.
	m_pssShiftKey->SetIsPersistent( FALSE );
	m_msgLeftBtns.SetMessageType( CMidiMsg::mtCCSel );
	m_msgLeftBtns.SetCCSelNum( 0x0f );
	m_msgLeftBtns.SetCCSelVal( 0x08 );
	m_msgLeftBtns.SetCCNum( 0x2F );


	// for the shift key, 40 is in, 00 is out
	m_pssShiftKey->AddShift( &m_msgLeftBtns, 0x40, skShiftIn, TRUE );
	m_pssShiftKey->AddShift( &m_msgLeftBtns, 0x00, skShiftOut, TRUE );

	// this message is used for the transport and for activating the
	// MotorMixTransportExtension, when used with the shift key.
	m_msgRightBtns.SetMessageType( CMidiMsg::mtCCSel );
	m_msgRightBtns.SetCCNum( 0x2f );
	m_msgRightBtns.SetCCSelNum( 0x0f );
	m_msgRightBtns.SetCCSelVal( 0x09 );

	m_pssQueryState = new CStateShifter( m_pSurface, stQueryState, qsIdle );
	if (m_pssQueryState == NULL)
		return E_OUTOFMEMORY;

	m_pssQueryState->SetIsPersistent( FALSE );

	CONDITION_HANDLE hCondition;

	m_pssQueryState->AddShift( &m_msgRightBtns, 0x40 /*esc btn*/, qsIdle, TRUE, &hCondition );
	m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );
	m_pssQueryState->AddShift( &m_msgRightBtns, 0x47 /*play btn*/, qsTransport, TRUE, &hCondition );
	m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );
	m_pssQueryState->AddShift( &m_msgRightBtns, 0x46 /*stop btn*/, qsLocator, TRUE, &hCondition );
	m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );

	// Meter Mode query
	if ( bHostAudioMeters )
	{
		m_pssQueryState->AddShift( &m_msgRightBtns, 0x45 /*FFwd(monitor) button*/, qsMeterMode, TRUE, &hCondition );
		m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );
	}

	m_pssQueryState->AddShift( &m_msgLeftBtns, 0x47 /*mode btn*/, qsContainerMode, TRUE, &hCondition );
	m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );
	m_pssQueryState->AddShift( &m_msgLeftBtns, 0x45 /*plugin btn*/, qsChooseInsert, TRUE, &hCondition );
	m_pssQueryState->AddShiftCondition( hCondition, stShiftKey, skShiftIn );


	m_pMotorMixTransport = new CMMTransport( m_pSurface );
	if (m_pMotorMixTransport == NULL)
		return E_OUTOFMEMORY;

	if ( bHostAudioMeters )
	{
		m_pqueryMeterMode = new CMMMeterMode( m_pSurface );
		if ( !m_pqueryMeterMode	)
			return E_OUTOFMEMORY;
	}

	m_pMotorMixLocator = new CMMLocator( m_pSurface );
	if (m_pMotorMixLocator == NULL)
		return E_OUTOFMEMORY;

	m_pMotorMixContainerMode = new CMMContainerMode( m_pSurface );
	if (m_pMotorMixContainerMode == NULL)
		return E_OUTOFMEMORY;

	// the insert edit mode can be in either of two states: Engaged and Disengaged.
	// It is toggled by pushing the PlugIn button while the shift key is NOT pressed.
	// It will ultimately control whether the mix strip knobs are remapped to
	// control plugin parameters. Likewise, scribble display should reflect
	// the parameters being edited.
	m_pssInsertEditMode = new CStateShifter( m_pSurface, stInsertEditMode, ieDisengaged );
	if (m_pssInsertEditMode == NULL)
		return E_OUTOFMEMORY;

	m_pssInsertEditMode->AddShift( &m_msgLeftBtns, 0x45, 1, FALSE, &hCondition );
	m_pssInsertEditMode->AddShiftCondition( hCondition, stShiftKey, skShiftOut );
	m_pssInsertEditMode->SetMinState( ieDisengaged );
	m_pssInsertEditMode->SetMaxState( ieEngaged );
	m_pssInsertEditMode->SetWrapAround( TRUE );

	// setup encoder pushbutton
	m_msgEncoderPush.SetMessageType( CMidiMsg::mtCC );
	m_msgEncoderPush.SetCCNum( 0x49 );

	m_pssSendPanOrLevel = new CStateShifter( m_pSurface, stSendPanOrLevel, sendPan );
	if (m_pssSendPanOrLevel == NULL)
		return E_OUTOFMEMORY;

	m_pssSendPanOrLevel->AddShift( &m_msgEncoderPush, 0x01, 1, FALSE );
	m_pssSendPanOrLevel->SetMinState( sendPan );
	m_pssSendPanOrLevel->SetMaxState( sendLevel );
	m_pssSendPanOrLevel->SetWrapAround( TRUE );

	// encoder message for rotary encoder
	m_msgEncoder.SetMessageType( CMidiMsg::mtCC );
	m_msgEncoder.SetCCNum( 0x48 );

	m_pssTrackRotaryMapper = new CMMEncoderStateShifter( m_pSurface, &m_msgEncoder, stTrackRotaryMapping );
	if (m_pssTrackRotaryMapper == NULL)
		return E_OUTOFMEMORY;

	m_pssBusRotaryMapper = new CMMEncoderStateShifter( m_pSurface, &m_msgEncoder, stBusRotaryMapping );
	if (m_pssBusRotaryMapper == NULL)
		return E_OUTOFMEMORY;

	// This state shifter's state indicates whether "bank mode"
	// is enabled in the motor mix.
	// it has two possible states: smByBanks (enabled)
	// and smOneByOne (disabled)
	// its purpose is to determine whether track bank shifting
	// occurs one by one or eight by eight.
	// The entity that listens for its state and acts upon it
	// is the CMMBankShifter
	m_pssBankShiftMode = new CStateShifter( m_pSurface, stBankShiftMode, smByBanks );
	if (m_pssBankShiftMode == NULL)
		return E_OUTOFMEMORY;

	// the m_pssBankShiftMode
	m_pssBankShiftMode->AddShift( &m_msgBtnUpperLeft, 0x42, 1, FALSE );
	m_pssBankShiftMode->SetMinState( smByBanks );
	m_pssBankShiftMode->SetMaxState( smOneByOne );
	m_pssBankShiftMode->SetWrapAround( TRUE );

	// CMMBankShifter is just like the SonarBank shifter
	// except that it listens for stBankShiftMode and 
	// adjusts its inc/dec amounts accordingly.
	// When the stBankShiftMode equals smOneByOne, the increment/dec
	// are 1/-1
	// otherwise, they are 8/-8
	// keep in mind that StateListeners must always be created and
	// destroyed while their master CStateShifters are alive and well
	// Otherwise, the automatic subscription that takes place upon
	// construction will fail and crash.
	// (likewise for the destruction's unsubscribe)
	m_msgBtnUpperLeft.SetMessageType( CMidiMsg::mtCCSel );
	m_msgBtnUpperLeft.SetCCSelNum( 0x0f );
	m_msgBtnUpperLeft.SetCCSelVal( 0x0A );
	m_msgBtnUpperLeft.SetCCNum( 0x2F );

	m_pssBaseTrack = new CSonarContainerBankShifter( m_pSurface, stBaseTrack, BANK_WIDTH );
	if (m_pssBaseTrack == NULL)
		return E_OUTOFMEMORY;

	m_pssBaseTrack->AddShift( &m_msgBtnUpperLeft, 0x40, -1, FALSE, &hCondition );
	m_pssBaseTrack->AddShiftCondition( hCondition, stContainerClass, ccTracks );
	m_pssBaseTrack->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );

	m_pssBaseTrack->AddShift( &m_msgBtnUpperLeft, 0x41, 1, FALSE, &hCondition );
	m_pssBaseTrack->AddShiftCondition( hCondition, stContainerClass, ccTracks );
	m_pssBaseTrack->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );


	m_pssBaseTrack->AddShift( &m_msgBtnUpperLeft, 0x40, -8, FALSE, &hCondition );
	m_pssBaseTrack->AddShiftCondition( hCondition, stContainerClass, ccTracks );
	m_pssBaseTrack->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	m_pssBaseTrack->AddShift( &m_msgBtnUpperLeft, 0x41, 8, FALSE, &hCondition );
	m_pssBaseTrack->AddShiftCondition( hCondition, stContainerClass, ccTracks );
	m_pssBaseTrack->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	m_pssBaseBus = new CSonarContainerBankShifter( m_pSurface, stBaseBus, BANK_WIDTH );
	if (m_pssBaseBus == NULL)
		return E_OUTOFMEMORY;

	m_pssBaseBus->AddShift( &m_msgBtnUpperLeft, 0x40, -1, FALSE, &hCondition );
	m_pssBaseBus->AddShiftCondition( hCondition, stContainerClass, ccBus );
	m_pssBaseBus->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );

	m_pssBaseBus->AddShift( &m_msgBtnUpperLeft, 0x41, 1, FALSE, &hCondition );
	m_pssBaseBus->AddShiftCondition( hCondition, stContainerClass, ccBus );
	m_pssBaseBus->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );

	m_pssBaseBus->AddShift( &m_msgBtnUpperLeft, 0x40, -8, FALSE, &hCondition );
	m_pssBaseBus->AddShiftCondition( hCondition, stContainerClass, ccBus );
	m_pssBaseBus->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	m_pssBaseBus->AddShift( &m_msgBtnUpperLeft, 0x41, 8, FALSE, &hCondition );
	m_pssBaseBus->AddShiftCondition( hCondition, stContainerClass, ccBus );
	m_pssBaseBus->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	m_pssBaseMain = new CSonarContainerBankShifter( m_pSurface, stBaseMain, BANK_WIDTH );
	if (m_pssBaseMain == NULL)
		return E_OUTOFMEMORY;

	m_pssBaseMain->AddShift( &m_msgBtnUpperLeft, 0x40, -1, FALSE, &hCondition );
	m_pssBaseMain->AddShiftCondition( hCondition, stContainerClass, ccMains );
	m_pssBaseMain->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );

	m_pssBaseMain->AddShift( &m_msgBtnUpperLeft, 0x41, 1, FALSE, &hCondition );
	m_pssBaseMain->AddShiftCondition( hCondition, stContainerClass, ccMains );
	m_pssBaseMain->AddShiftCondition( hCondition, stBankShiftMode, smOneByOne );

	m_pssBaseMain->AddShift( &m_msgBtnUpperLeft, 0x40, -8, FALSE, &hCondition );
	m_pssBaseMain->AddShiftCondition( hCondition, stContainerClass, ccMains );
	m_pssBaseMain->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	m_pssBaseMain->AddShift( &m_msgBtnUpperLeft, 0x41, 8, FALSE, &hCondition );
	m_pssBaseMain->AddShiftCondition( hCondition, stContainerClass, ccMains );
	m_pssBaseMain->AddShiftCondition( hCondition, stBankShiftMode, smByBanks );

	// The various CurrentXXX container states determine the container on which an operation
	// is being done. For example: currently enabling the plugin mode will display the
	// parameters for the current plugin on the current container.
	m_pssCurrentTrack = new CSonarContainerBankShifter( m_pSurface, stCurrentTrack, 8, 0 );
	if (m_pssCurrentTrack == NULL)
		return E_OUTOFMEMORY;

	m_pssCurrentBus = new CSonarContainerBankShifter( m_pSurface, stCurrentBus, 8, 0);
	if (m_pssCurrentBus == NULL)
		return E_OUTOFMEMORY;

	m_pssCurrentMain = new CSonarContainerBankShifter( m_pSurface, stCurrentMain, 8, 0);
	if (m_pssCurrentMain == NULL)
		return E_OUTOFMEMORY;

	m_pssCurrentEffect = new CSonarContainerBankShifter( m_pSurface, stCurrentEffect, 8, 0);
	if (m_pssCurrentEffect == NULL)
		return E_OUTOFMEMORY;

	m_pssBaseEffectParam = new CSonarContainerBankShifter( m_pSurface, stBaseEffectParam, 8, 0);
	if (m_pssBaseEffectParam == NULL)
		return E_OUTOFMEMORY;

	m_pMotorMixChooseInsert = new CMMChooseInsert( m_pSurface );
	if (m_pMotorMixChooseInsert == NULL)
		return E_OUTOFMEMORY;

	// allows the user to pick the current container with the select buttons
	m_pmmContainerPicker = new CMMContainerPicker( m_pSurface );
	if (m_pmmContainerPicker == NULL)
		return E_OUTOFMEMORY;

	// The fine tweak mode is engaged when the user pushes the alt/fine key
	// while holding the shift key. It determines what we display in the
	// scribble strip. When engaged, we display the latest parameter change
	// in a detailed manner, and, in doing so, we use as much as we need
	// of the display.
	m_pssFineTweakMode = new CStateShifter( m_pSurface, stFineTweakMode, ftDisengaged );
	if (m_pssFineTweakMode == NULL)
		return E_OUTOFMEMORY;

	m_pssFineTweakMode->SetIsPersistent( FALSE );

	// for the alt/fine key, 43 is in, 03 is out
	m_pssFineTweakMode->AddShift( &m_msgLeftBtns, 0x43, ftEngaged, TRUE, &hCondition );
	m_pssFineTweakMode->AddShiftCondition( hCondition, stShiftKey, skShiftIn );

	// during the button's release it does not matter if shift is in or out.
	m_pssFineTweakMode->AddShift( &m_msgLeftBtns, 0x03, ftDisengaged, TRUE );

	// This is the object that acutally reacts to the fine tweak mode state
	// by putting on the screen a detail of the the last parameter change.
	m_pmmFineTweakMode = new CMMFineTweakMode( m_pSurface );
	if (m_pmmFineTweakMode == NULL)
		return E_OUTOFMEMORY;

	// This state shifter specifies the choice in the burn buttons.
	// The burn buttons are the three small black buttons
	// on the upper left.
	// The burn buttons are supposed to determine the function
	// of the record buttons (grayed ones) in each strip.
	m_pssBurnButtonsChoice = new CStateShifter( m_pSurface, stBurnButtonsChoice );
	if (m_pssBurnButtonsChoice == NULL)
		return E_OUTOFMEMORY;

	m_pssBurnButtonsChoice->AddShift( &m_msgBtnUpperLeft, 0x44, bbRecRdy, TRUE );
	m_pssBurnButtonsChoice->AddShift( &m_msgBtnUpperLeft, 0x45, bbWrite, TRUE );
	m_pssBurnButtonsChoice->AddShift( &m_msgBtnUpperLeft, 0x46, bbOther, TRUE );

	// This state shifter specifies the choice in the multi buttons.
	// The multi buttons are the four small black buttons
	// on the upper right.
	// The multi buttons are supposed to determine the function
	// of the multi buttons (crossed by a black line) in each strip.
	m_pssMultiButtonsChoice = new CStateShifter( m_pSurface, stMultiButtonsChoice );
	if (m_pssMultiButtonsChoice == NULL)
		return E_OUTOFMEMORY;

	m_msgBtnUpperRight.SetMessageType( CMidiMsg::mtCCSel );
	m_msgBtnUpperRight.SetCCSelNum( 0x0f );
	m_msgBtnUpperRight.SetCCSelVal( 0x0B );
	m_msgBtnUpperRight.SetCCNum( 0x2F );

	m_pssMultiButtonsChoice->AddShift( &m_msgBtnUpperRight, 0x40, mbFxBypassE1, TRUE );
	m_pssMultiButtonsChoice->AddShift( &m_msgBtnUpperRight, 0x41, mbSMuteE2, TRUE );
	m_pssMultiButtonsChoice->AddShift( &m_msgBtnUpperRight, 0x42, mbPrePostE3, TRUE );
	m_pssMultiButtonsChoice->AddShift( &m_msgBtnUpperRight, 0x43, mbSelectE4, TRUE );

	// This template class does two things.
	// First, it creates eight motor mix strips (or any kind strips) and
	// Second, it calls set container on them when the state shifter
	// for base track undergoes a shift.
	// make sure that it is created after states for base main and aux have been created
	m_pTheBank = new CMixStripBank<CMMStrip>( BANK_WIDTH, m_pSurface );
	if (m_pTheBank == NULL)
		return E_OUTOFMEMORY;
	HRESULT hr = m_pTheBank->Initialize( BANK_FOLLOWS_ALL );
	if (FAILED( hr ))
		return hr;

	// The CChoiceLedUpdater keeps LED's on the surface updated depending on state
	// transitions from all pertinent states. For that reason, it is created
	// after all oft he state shifters have been created.
	// Likewise, it is destroyed before the state shifters are.
	m_pmmChoiceLedUpdater = new CMMChoiceLedUpdater( m_pSurface );
	if (m_pmmChoiceLedUpdater == NULL)
		return E_OUTOFMEMORY;

	hCondition = m_pSurface->GetTransport()->GetGlobalConditionHandle();
	hCondition->AddCondition( stShiftKey, skShiftOut );
	m_pSurface->GetTransport()->SetPlayMsg( &m_msgRightBtns, 0x47 );
	m_pSurface->GetTransport()->SetStopMsg( &m_msgRightBtns, 0x46 );

	MFX_TIME tAmount;
	tAmount.timeFormat = TF_MBT;
	tAmount.mbt.nMeas = 1;
	tAmount.mbt.nBeat = 0;
	tAmount.mbt.nTick = 0;


	// for the shuttle control we want:
	// auto repeat every seventy miliseconds after waiting 0.4 sec.

	// rewind shuttle
	m_pSurface->GetTransport()->AddShuttle( 8 /* eighty milliseconds*/, tAmount, TRUE, 40 /* 0.4 sec */, &m_msgRightBtns, 0x44, &m_msgRightBtns, 0x04 );

	// forward shuttle
	m_pSurface->GetTransport()->AddShuttle( 8 /* eighty milliseconds*/, tAmount, FALSE, 40 /* 0.4 sec */, &m_msgRightBtns, 0x45, &m_msgRightBtns, 0x05 );

	// set scribble message for "goodbye" message
	m_msgScribble.SetMessageType( CMidiMsg::mtSysXString );
	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x10, 0x00 };	
	BYTE pczPostString[] = {0xF7};

	m_msgScribble.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribble.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribble.SetSysXTextLen( 40 );
	m_msgScribble.SetSysXTextFillChar( 0x20 );
	m_msgScribble.SetUseTextCruncher( TRUE );


	m_pMotorMixCommand = new CMMCommand( m_pSurface );
	if ( !m_pMotorMixCommand )
		return E_OUTOFMEMORY;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
CMMSurface::~CMMSurface()
{
	delete m_pMotorMixCommand;
	delete m_pmmChoiceLedUpdater;
	delete m_pTheBank;
	delete m_pmmContainerPicker;
	delete m_pMotorMixChooseInsert;
	delete m_pssBaseTrack;
	delete m_pssBaseMain;
	delete m_pssBaseBus;
	delete m_pssBaseEffectParam;
	delete m_pssCurrentTrack;
	delete m_pssCurrentMain;
	delete m_pssCurrentBus;
	delete m_pssCurrentEffect;
	delete m_pssSendPanOrLevel;
	delete m_pssBusRotaryMapper;
	delete m_pssTrackRotaryMapper;
	delete m_pssBankShiftMode;
	delete m_pssBurnButtonsChoice;
	delete m_pssMultiButtonsChoice;
	delete m_pmmFineTweakMode;
	delete m_pssFineTweakMode;
	delete m_pssInsertEditMode;
	delete m_pMotorMixContainerMode;
	delete m_pMotorMixLocator;
	delete m_pMotorMixTransport;
	delete m_pqueryMeterMode;

	delete m_pssQueryState;
	delete m_pssShiftKey;
	delete m_pssContainerClass;
	delete m_pssMeterEnable;
}

/////////////////////////////////////////////////////////////////////////
void CMMSurface::MakeStatusString( char *pStatus )
{
	CString strStrip;
	switch (m_pSurface->GetCurrentStripKind())
	{
	case MIX_STRIP_TRACK:
		strStrip = "Tracks ";
		break;

	case MIX_STRIP_BUS:
		strStrip = "Buses ";
		break;

	case MIX_STRIP_MASTER:
		strStrip = "Mains ";
		break;

	default:
		_ASSERT( 0 ); // unknown strip kind
	}

	CString strRange;
	int nBegin = m_pSurface->GetBaseStrip() + 1;
	int nEnd = m_pSurface->GetBaseStrip() + BANK_WIDTH;
	strRange.Format("%d - %d", nBegin, nEnd );

	strStrip = strStrip + strRange;

	strcpy( pStatus, strStrip );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMSurface::OnConnect()
{
	// escape out of any transient modes, just to have a tidy look upon closure.
	CStateShifter *pShifter = m_pSurface->GetStateMgr()->GetShifterFromID( stQueryState );
	if (pShifter)
		pShifter->SetNewState( qsIdle );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMSurface::OnDisconnect()
{
	// escape out of any transient modes, just to have a tidy look upon closure.
	CStateShifter *pShifter = m_pSurface->GetStateMgr()->GetShifterFromID( stQueryState );
	if (pShifter)
		pShifter->SetNewState( qsIdle );

	// send a session closure message so people know MotorMix is not online anymore
	m_msgScribble.SendText( 40, "     ---- SONAR Session Closed ----     " );

	return S_OK;
}


