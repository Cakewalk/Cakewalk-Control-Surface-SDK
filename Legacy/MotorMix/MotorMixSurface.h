/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixSurface.h:
/////////////////////////////////////////////////////////////////////////
#pragma once

class CMMSurface:	public CSurfaceImp
{
public:
	CMMSurface( CControlSurface* pSurface );
	~CMMSurface();

	// CSurfaceImp overrides
	HRESULT Initialize();
	HRESULT OnConnect();
	HRESULT OnDisconnect();
	void MakeStatusString( char *pStatus );

private:

	// state shifters that deal with sonar containers
	CStateShifter*							m_pssContainerClass;
	CStateShifter*							m_pssMeterEnable;

	CSonarContainerBankShifter*		m_pssBaseTrack;
	CSonarContainerBankShifter*		m_pssBaseMain;
	CSonarContainerBankShifter*		m_pssBaseBus;
	CSonarContainerBankShifter*		m_pssCurrentTrack;
	CSonarContainerBankShifter*		m_pssCurrentMain;
	CSonarContainerBankShifter*		m_pssCurrentBus;
	CSonarContainerBankShifter*		m_pssCurrentEffect;
	CSonarContainerBankShifter*		m_pssBaseEffectParam;

	CMixStripBank<CMMStrip>*			m_pTheBank;		// remappable bank of strips
	CStateShifter*							m_pssBankShiftMode;		// determines if bank shifts 8 by 8 or 1 by 1
	CMMContainerPicker*					m_pmmContainerPicker;	// allows choosing a current container
	
	// user query modes
	CMMTransport*			m_pMotorMixTransport;		// asks for extended transport features
	CMMLocator*				m_pMotorMixLocator;			// asks for locate choices
	CMMContainerMode*		m_pMotorMixContainerMode;	// asks: "are we looking at tracks, mains or auxes"
	CMMChooseInsert*		m_pMotorMixChooseInsert;	// asks which insert are we currently dealing with
	CMMMeterMode*			m_pqueryMeterMode;			// query asks to turn meters on or off

	CMMFineTweakMode*		m_pmmFineTweakMode;
	CMMCommand*				m_pMotorMixCommand;			// Listener for Command Functions

	CStateShifter*		m_pssTrackRotaryMapper;	// determines what the rotary mapping is when looking at tracks
	CStateShifter*		m_pssBusRotaryMapper;	// determines what the rotary mapping is when looking at auxes
	CStateShifter*		m_pssSendPanOrLevel;		// determines if the SEND parameter been controlled are pan or level
	CStateShifter*		m_pssShiftKey;				// determines the state of the shift key
	CStateShifter*		m_pssBurnButtonsChoice;	// which of the burn buttons is selected
	CStateShifter*		m_pssMultiButtonsChoice;// which of the multi buttons is selected
	CStateShifter*		m_pssQueryState;			// are we currently asking for something?
	CStateShifter*		m_pssInsertEditMode;		// turn on or off editing an Insert
	CStateShifter*		m_pssFineTweakMode;		// enable disable viewing parameter detail

	// led control multi state listener
	// object that turns leds on and off according to state changes
	CMMChoiceLedUpdater*			m_pmmChoiceLedUpdater;

	CMidiMsg				m_msgBtnUpperLeft;		// burn buttons and banks shift buttons
	CMidiMsg				m_msgBtnUpperRight;		// multi buttons
	CMidiMsg				m_msgRightBtns;			// transport buttons and escape key
	CMidiMsg				m_msgLeftBtns;				// shift button and other mode control buttons
	CMidiMsg				m_msgEncoder;				// rotary encoder
	CMidiMsg				m_msgEncoderPush;			// rotary encoder's pushbutton
	CMidiMsg				m_msgScribble;				// alphanumeric LCD
};

/////////////////////////////////////////////////////////////////////////
