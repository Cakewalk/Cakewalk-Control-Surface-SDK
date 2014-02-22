/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkParams.h:	Definitions of the objects that enable communication
// SONAR mix parameters.
/////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////
// CHostNotifyMulticaster:
//
// Host notifications handling. This object distributes refresh
// notifications from SONAR to any object that derives from
// CHostNotifyListener. to receive a host notification,
// the "derived" object must pass into CHostNotifyListener's constructor
// the flag combination of notifications that it wishes to receive.
// Finally, it must implement the virtual OnHostNotify with the code that
// will respond to the notification.
/////////////////////////////////////////////////////////////////////////

typedef std::set< CHostNotifyListener*, std::less< CHostNotifyListener* >, std::allocator< CHostNotifyListener* > > HostListenerSet;
typedef HostListenerSet::iterator HostListenerSetIt;

class CHostNotifyMulticaster
{
	friend class CHostNotifyListener;
	friend class CControlSurface;

public:
	
	// collects all the CHostNotifyListener (s)
	// notifies them when the host sends a notification
	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie ); // called by the surface

private:

	// private constructor because it is a singleton
	CHostNotifyMulticaster( CControlSurface *pSurface ):
		m_pSurface( pSurface )
	{
	}

	void add( CHostNotifyListener *pListener );
	void remove( CHostNotifyListener *pListener );

	HostListenerSet	m_setListeners;	// contains list of Listeners registered for notification
	CControlSurface*		m_pSurface;
};


/////////////////////////////////////////////////////////////////////////
// CHostNotifyListener:
//
// A class derived from CHostNotifyListener will be capable of receiving
// host notifications by implementing the virtual method OnHostNotify.
// The CHostNotifyListener works in conjunction with the 
//	CHostNotifyMulticaster
/////////////////////////////////////////////////////////////////////////

class CHostNotifyListener
{
	friend class CHostNotifyMulticaster;
public:
							CHostNotifyListener( DWORD fdwRefresh, CControlSurface *pSurface );
	virtual				~CHostNotifyListener();
	DWORD					GetRefreshFlags() { return m_fdwRefresh; }
	virtual void		OnHostNotify( DWORD fdwRefresh, DWORD dwCookie ) PURE;
	virtual HRESULT	RequestRefresh();

	virtual DWORD GetCookie()
	{
		return (DWORD)static_cast<CHostNotifyListener*>(this);
	}

protected:
	virtual void		onHostNotify( DWORD fdwRefresh, DWORD dwCookie );

private:
	DWORD					m_fdwRefresh;	// flag combination for the notification expected
	BOOL					m_bExpectingCookie;
	CControlSurface*	m_pSurface;

protected:
	int					m_nHostNotifyCount;
};


/////////////////////////////////////////////////////////////////////////
// CBaseMixParam:
//
// Establishes the link between a MIDI message (or set of messages) and 
// a SONAR mixer parameter.
//
//	Implements CMidiMsgListener so that MIDI input can result in a change of 
// SONAR mix parametes. Likewise, it implements CHostNotifyListener so that 
// host notifications can result in refreshing the MIDI state.
//
// The actual mapping between MIDI events and SONAR parameter changes
// has been left unimplemented, since this is done in other classes
// derived from CBaseMixParam. The reason for this is the variety of 
// parameter types available, and the many ways MIDI can be mapped to 
// interact with them.
//
// For your convenience, we have provided CMixParamFloat, CMixParamBool
// and CMixParamEnum, which cover all the available parameters in SONAR
// and the most typical ways of mapping MIDI to those parameters.
/////////////////////////////////////////////////////////////////////////

class CBaseMixParam:	public CMidiMsgListener,
							public CHostNotifyListener
{
public:
	CBaseMixParam( CControlSurface *pSurface );
	virtual ~CBaseMixParam() {};

	// accessors to determine the parameter's identity
	virtual HRESULT SetContainer( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum );

	virtual HRESULT SetContainer( DWORD dwStripNum );
	
	virtual void SetValueScribbleMsg( CMidiMsg *pMsg )
	{
		m_pMsgValueScribble = pMsg;
	}

	virtual void SetIsValueScribbleEnabled( BOOL bEnable )
	{
		m_bIsValueScribbleEnabled = bEnable;
	}

	virtual void SetIsLabelScribbleEnabled( BOOL bEnable )
	{
		m_bIsLabelScribbleEnabled = bEnable;
	}

	virtual void SetLabelScribbleMsg( CMidiMsg *pMsg )
	{
		m_pMsgLabelScribble = pMsg;
	}

	virtual HRESULT SetAlternateLabel( const char *szData );

	virtual HRESULT SetParam(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);

	virtual HRESULT SetFxParam(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		WORD wFxNum,
		WORD wFxParamNum
	);

	virtual HRESULT SetIsArmed( BOOL bIsArmed );

	virtual HRESULT GetIsArmed( BOOL *pbIsArmed );

	virtual HRESULT SetIsEnabled( BOOL bIsEnabled );

	enum ERefreshRate { RR_EVERY = 0, RR_EVEN , RR_ODD, RR_MODBASE = 500 };
	virtual HRESULT SetRefreshFrequency( ERefreshRate rr );

	BOOL GetIsEnabled()
	{
		return m_bIsEnabled;
	}

	HRESULT Snapshot();

	// CHostNotifyListener overrides
	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );
	HRESULT RequestRefresh();

	virtual void ZeroMidiState();

	// if thinning is enabled, forces the value to get sent the next time
	// a parameter change occurs.
	// otherwise thinning could prevent the change from getting sent unless
	// a value change occurred.
	virtual void Invalidate()
	{
		if (m_pMsgValueScribble)
			m_pMsgValueScribble->Invalidate();

		if (m_pMsgLabelScribble)
			m_pMsgLabelScribble->Invalidate();
	}
	
	virtual HRESULT SetMixParamValue( float fValue, SONAR_MIXER_TOUCH smtTouch );

	virtual HRESULT GetSonarParamValue( float *pfValue );

	virtual BOOL IsTouched()
	{
		return FALSE;
	}

	// validation
	HRESULT IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);

	static HRESULT IsParamArmable(
		SONAR_MIXER_PARAM mixerParam
	);

	HRESULT IsStripInRange(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum
	);

	HRESULT IsFxParamInRange(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		WORD wFxNum,
		WORD wFxParamNum
	);

	BOOL IsMidiTrack(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum
	);

protected:

	// overrides for CMidiMsgListener
	HRESULT setValue( CMidiMsg *pMsg, float fVal );	// sets the current value
	HRESULT setValue( CMidiMsg *pMsg, BOOL bVal );	// sets the current value
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );// sets the current value

	HRESULT getValueType( EValueType eType );

	// this method will be called whenever parameter identity changes
	// and whenever SONAR notifications arrive.
	virtual void refreshParam( float *pfValue = NULL ) {}
	virtual void renderParam( float fValue )
	{
		renderScribbles( fValue );
	}
	virtual void renderScribbles( float fValue );
	void maybeRefreshParam( float *pfValue = NULL );

protected: // member variables
	// parameter identity
	BOOL						m_bIsEnabled;
	SONAR_MIXER_STRIP		m_mixerStrip;
	DWORD						m_dwStripNum;
	SONAR_MIXER_PARAM		m_mixerParam;
	DWORD						m_dwParamNum;
	CMidiMsg*				m_pMsgValueScribble;
	CMidiMsg*				m_pMsgLabelScribble;
	BOOL						m_bIsValueScribbleEnabled;
	BOOL						m_bIsLabelScribbleEnabled;
	char*						m_szAlternateLabel;
	CCriticalSection		m_cs;
	CControlSurface*		m_pSurface;
	ERefreshRate			m_eRefreshRate;
};


/////////////////////////////////////////////////////////////////////////
// CMixParamFloat:
//
// Derived from CBaseMixParam, allows controlling continuous (float)
// parameters in SONAR via MIDI, as well as feedback MIDI messages.
//
// The optional "valued" input message specified what MIDI message's
// value will control the SONAR parameter. The optional capture and
// release messages may trigger a capture and release in the mix
// parameter (e.g. for touch sensitive faders)
//
// You may want output only in the case of VU meters. In that case you
// would naturally supply an output parameter, and supply NULL for
// pMsgInput.
//
// For feedback (e.g. motor surfaces) the optional "valued" output
// message identifies the kind of MIDI message sent out when the
// parameter is refreshed.
//
// If you require a special kind of taper between the SONAR float
// parameter and MIDI input/output, you can create a class
// derived from CMixParamFloat and override the methods
// applyInputMapping and applyOutput mapping, so that one is the
// inverse function of the other, that is:
// x = applyInputMapping( applyOutputMapping ( x ) )
/////////////////////////////////////////////////////////////////////////

class CMixParamFloat: public CBaseMixParam
{
public:
	CMixParamFloat(
		CControlSurface *pSurface,
		CMidiMsg *pMsgInput,
		CMidiMsg *pMsgOutput = NULL,
		CMidiMsg *pMsgCapture = NULL, DWORD dwCaptureVal = VAL_ANY,
		CMidiMsg *pMsgRelease = NULL, DWORD dwReleaseVal = VAL_ANY
		);

	virtual ~CMixParamFloat();

	virtual HRESULT SetParam(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);

	HRESULT Touch( BOOL bTouch );

	virtual void ZeroMidiState()
	{
		if (m_pMsgOut)
			m_pMsgOut->Send( (float)0 );

		CBaseMixParam::ZeroMidiState();
	}

	virtual BOOL IsTouched()
	{
		return m_bIsTouched;
	}

	// CBaseMixParam overrides
	void Invalidate();

	// validation
	static HRESULT IsParamFloat( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum );

protected:
	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, float fVal );
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeFloat; }
	BOOL	hasTouchMsgs();
	
	// CBaseMixParam overrides
	HRESULT IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);
	virtual void refreshParam( float *pfValue = NULL );
	virtual void renderParam( float fValue );

	// this is where "dead bands" and such are applied, by deriving CMixParamFloat
	virtual float applyInputMapping( float fVal ) { return fVal; }
	virtual float applyOutputMapping( float fVal ) { return fVal; }

	// MIDI identity
	CMidiMsg*				m_pMsgIn;
	CMidiMsg*				m_pMsgOut;
	CMidiTrigger			m_trigCapture;
	CMidiTrigger			m_trigRelease;

	BOOL						m_bParamIsTouchable; // touch may fail if a param isn't
	// if it fails, we should use auto touch when setting the param, and skip
	// the call to "untouch"

	BOOL						m_bIsTouched;
};


/////////////////////////////////////////////////////////////////////////
// CMixParamBool:
//
// Derived from CBaseMixParam, allows controlling binary (BOOL)
// parameters in SONAR via MIDI, as well as feedback MIDI messages
// to update LEDs etc.
//
// Input messages for On and Off are required as well as a specific
// value to trigger the event (the value may be VAL_ANY, so any value
// of the message will trigger the event.)
//
// The CMidiMsg for input On and Off may be the same (assuming the value
// determines the on off state.) The same applies to output.
/////////////////////////////////////////////////////////////////////////

class CMixParamBool: public CBaseMixParam
{
public:
	CMixParamBool(
		CControlSurface *pSurface,
		CMidiMsg *pMsgIn, DWORD dwInOnVal, DWORD dwInOffVal,
		CMidiMsg *pMsgOutOn = NULL, DWORD dwOutOnVal = VAL_ANY,
		CMidiMsg *pMsgOutOff = NULL, DWORD dwOutOffVal = VAL_ANY
	);

	virtual ~CMixParamBool();

	// CBaseMixParam overrides
	void Invalidate();

	virtual void ZeroMidiState()
	{
		if (m_pMsgOutOff)
			m_pMsgOutOff->Send( m_dwOutOffVal );

		CBaseMixParam::ZeroMidiState();
	}

	// validation
	static HRESULT IsParamBool( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum );

protected:
	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, BOOL bVal );
	EValueType getValueType() { return TypeBool; }

	// CBaseMixParam overrides
	HRESULT IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);
	virtual void refreshParam( float *pfValue = NULL );
	virtual void renderParam( float fValue );

	// MIDI identity
	CMidiMsg*		m_pMsgIn;

	CMidiMsg*		m_pMsgOutOn;
	DWORD				m_dwOutOnVal;
	
	CMidiMsg*		m_pMsgOutOff;
	DWORD				m_dwOutOffVal;

	BOOL				m_bLastValue;
};


/////////////////////////////////////////////////////////////////////////
// CMixParamEnum:
//
// Derived from CBaseMixParam, allows controlling finite discrete
// (enum and int) parameters in SONAR via MIDI, as well as providing
// a hook for possibly feedback on parameter change.
/////////////////////////////////////////////////////////////////////////

class CMixParamEnum: public CBaseMixParam
{
public:
	CMixParamEnum(
		CControlSurface *pSurface,
		CMidiMsg *pMsg,
		DWORD dwIncVal,
		DWORD dwDecVal,
		DWORD dwLgeIncVal = VAL_ANY, DWORD dwLgeIncAmount = 0,
		DWORD dwLgeDecVal = VAL_ANY, DWORD dwLgeDecAmount = 0
	);

	virtual ~CMixParamEnum();

	// CBaseMixParam overrides
	void Invalidate();

	// validation
	static HRESULT IsParamEnum( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum );

protected:
	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeInt; }

	DWORD getMin();
	DWORD getMax();

	// CBaseMixParam overrides
	HRESULT IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	);
	virtual void refreshParam( float *pfValue = NULL );
	virtual void renderParam( float fValue );

	CMidiMsg*		m_pMsgIn;
};

/////////////////////////////////////////////////////////////////////////


static SONAR_MIXER_PARAM s_arArmableParams[] = {
		MIX_PARAM_VOL,
		MIX_PARAM_PAN,
		MIX_PARAM_MUTE,
		MIX_PARAM_FX_PARAM,
		MIX_PARAM_SEND_VOL,
		MIX_PARAM_SEND_PAN
	};

static int s_nCountArmable = 6;

class CMixParamBoolEx:	public CMixParamBool
{
public:
	CMixParamBoolEx(
		CControlSurface *pSurface,
		CMidiMsg *pMsgIn, DWORD dwInOnVal, DWORD dwInOffVal,
		CMidiMsg *pMsgOutOn = NULL, DWORD dwOutOnVal = VAL_ANY,
		CMidiMsg *pMsgOutOff = NULL, DWORD dwOutOffVal = VAL_ANY
		):
		CMixParamBool(
			pSurface,
			pMsgIn, dwInOnVal, dwInOffVal,
			pMsgOutOn, dwOutOnVal,
			pMsgOutOff, dwOutOffVal
			)
	{
	}

	HRESULT SetMixParamValue( float fValue, SONAR_MIXER_TOUCH smtTouch );

protected:
	virtual void refreshParam( float *pfValue = NULL );
	BOOL isRecordArm();

};
