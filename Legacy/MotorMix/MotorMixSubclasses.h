/////////////////////////////////////////////////////////////////////////
// Copyright (c) 1998-99 by Twelve Tone Systems, Inc. All rights reserved.
//
// MotorMixSubclasses.h:	Definition of various objects that are derived
// from classes supplied in the framework.
/////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////
// CMMChoiceLedUpdater:
//
// This object simply listens for state changes of several states,
// and updates the LED's on the motor mix depending on
// the current states.
// It is especially comvenient in those cases where the state of a given
// LED depends on a "truth" table from two or more StateShifters.
// Look at how we update the plug in LED for an example.
/////////////////////////////////////////////////////////////////////////

class CMMChoiceLedUpdater : public CMultiStateListener
{
public:
	CMMChoiceLedUpdater( CControlSurface *pSurface );

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nNewState );

private:

	// helpers
	void updateRotaryMappingLED();
	void updatePlugInBtnLED();

	CMidiMsg		m_msgUpperLeftLed;
	CMidiMsg		m_msgUpperRightLed;
	CMidiMsg		m_msgLowerLeftLed;
	CMidiMsg		m_msg7Segment;
	CMidiMsg		m_msgScribble;
};


//------------------------------------------------------------------------
class CMMCommand :	public CMidiMsgListener,
							public CMultiStateListener
{
public:
	CMMCommand( CControlSurface *pSurface );
	~CMMCommand();

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nNewState );

protected:

	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeInt; }

private:
	int			m_nCurShift;
	CMidiMsg		m_msgUndoSave;
};


/////////////////////////////////////////////////////////////////////////
// CRotaryEncoderStateShifter:
//
// Because the motor mix rotary encoder sends a message that depends on
// the rotational speed, we could either populate a CStateShifter
// with numerous shift triggers for each possible speed value, or
// subclass it. For simplicity, we preffer subclassing it
/////////////////////////////////////////////////////////////////////////

class CMMEncoderStateShifter: public CStateShifter
{
public:
	CMMEncoderStateShifter(
		CControlSurface *pSurface,
		CMidiMsg *pMsg,
		DWORD dwStateID,
		DWORD dwInitialState = 0
	);

	~CMMEncoderStateShifter();

	//CStateShifter overrides
	HRESULT SetNewState( int nNewState );

	int GetMaxState();
	int GetMinState() { return 0; }

private:

	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );

	CMidiMsg*					m_pMsgRotary;
	CControlSurface*		m_pSurface;
};


/////////////////////////////////////////////////////////////////////////
// CMMRotaryParam:
//
// Just like a CMixParamFloat, but can keep the little pointer bargraphs
// on the LCD updated.
/////////////////////////////////////////////////////////////////////////

class CMMRotaryParam:	public CMixParamFloat
{
public:
	CMMRotaryParam(
		CControlSurface *pSurface,
		DWORD dwHWStripNum,
		CMidiMsg *pMsgInput,
		CMidiMsg *pMsgOutput = NULL,
		CMidiMsg *pMsgCapture = NULL, DWORD dwCaptureVal = VAL_ANY,
		CMidiMsg *pMsgRelease = NULL, DWORD dwReleaseVal = VAL_ANY
		);
	~CMMRotaryParam() {}

	void SetPointerType( DWORD dwPointerType );

	// CMixParamFloat overrides
	HRESULT SetIsEnabled( BOOL bIsEnabled );

protected:

	// CMixParamFloat override
	void refreshParam( float *pfValue = NULL );

	CMidiMsg			m_msgPointerGraph;
	DWORD				m_dwHWStripNum;
};

/////////////////////////////////////////////////////////////////////////
// CMMFaderParam:
//
// This example shows how to override the applyInput/OutputMapping
// methods in CMixParamFloat. In this case, we have used it to
// match the 0 db mark in SONAR to the 0 db mark in the
// MotorMix
/////////////////////////////////////////////////////////////////////////

class CMMFaderParam:	public CMixParamFloat
{
public:
	CMMFaderParam(
		CControlSurface *pSurface,
		CMidiMsg *pMsgInput,
		CMidiMsg *pMsgOutput = NULL,
		CMidiMsg *pMsgCapture = NULL, DWORD dwCaptureVal = VAL_ANY,
		CMidiMsg *pMsgRelease = NULL, DWORD dwReleaseVal = VAL_ANY
		):
	CMixParamFloat(
		pSurface,
		pMsgInput,
		pMsgOutput,
		pMsgCapture, dwCaptureVal,
		pMsgRelease, dwReleaseVal
		)
	{
	}
	~CMMFaderParam() {}

protected:

	// CMixParamFloat overrides:
	float applyInputMapping( float fVal )
	{
		if (fVal > 95.0f/127.0f)
		{
			return (fVal - 95.0f/127.0f) * (1 - 101.0f/127.0f) / (1 - 95.0f/127.0f) + 101.0f/127.0f;
		}
		else
		{
			return fVal * (101.0f/127.0f) / (95.0f/127.0f);
		}
	}

	float applyOutputMapping( float fVal )
	{
		if (fVal > 101.0f/127.0f)
		{
			return (fVal - 101.0f/127.0f) * (1 - 95.0f/127.0f) / (1 - 101.0f/127.0f) + 95.0f/127.0f;
		}
		else
		{
			return fVal * (95.0f/127.0f) / (101.0f/127.0f);
		}
	}
};

/////////////////////////////////////////////////////////////////////////
// CMMContainerPicker:
//
// Allows the use of the select buttons on the motor mix to pick
// a current container (depending on the current ContainerClass)
/////////////////////////////////////////////////////////////////////////

class CMMContainerPicker:	public CMidiMsgListener,
									public CMultiStateListener
{
public:
	CMMContainerPicker( CControlSurface *pSurface );
	~CMMContainerPicker();

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nNewState );

protected:

	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeInt; }

	// helpers
	HRESULT refreshButtonLEDs();

	// member variables:
	CMidiMsg		m_msgSelectBtns;
	CMidiMsg		m_msgSelectLED;

	int			m_nLastLedIndex;
};


/////////////////////////////////////////////////////////////////////////
// CMMFineTweakMode:
//
// This object takes action when the FineTweakMode is engaged.
// The stFineTweakMode determines that the user wants to see
// a detail of the parameter being manipulated on the MotorMix
// screen.
// Therefore, this object must send screen updates as the parameter
// changes.
// For that reason, this object is a CLastParamChangeListener.
// That way, it gets notified every time a parameter move occurs.
/////////////////////////////////////////////////////////////////////////

class CMMFineTweakMode:	public CLastParamChangeListener,
								public CMultiStateListener,
								public CHostNotifyListener
{
public:
	CMMFineTweakMode( CControlSurface *pSurface );
	~CMMFineTweakMode();
	void OnParamChange();
	HRESULT OnStateChange( DWORD dwStateID, int nNewState );
	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );
protected:
	CMidiMsg		m_msgScribbleContainer;
	CMidiMsg		m_msgScribbleSubCont; // if this is a plugin, show which one
	CMidiMsg		m_msgScribbleParam;
	CMidiMsg		m_msgScribbleValue;
private:
	CControlSurface*	m_pSurface;
	int			m_nRefreshCount;
};

/////////////////////////////////////////////////////////////////////////
