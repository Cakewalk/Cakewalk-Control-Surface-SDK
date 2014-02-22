/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkStates.h:	Definitions of the objects that maintain and broadcast
// states and state transitions as a result of MIDI input.
/////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////
// State control:
//
// We have provided a family of classes to facilitate management of
// states (memory) that appears to be on the hardware surface.
//
// By memory we mean any state that persists such as pushing a button
// and having that button's led light up until you push it again,
// Having a set of "radio" buttons select one of several "modes", etc.
// But first, let's give some background before we go into details.
//
// The control surface's DLL is only permitted to take action 
// in the following instances:
//
// A) A MIDI event arrives and the SONAR MIDI input thread calls the
// DLL's midi input API. The MidiInputRouter relays that event
// to a CMidiMsg and from there it reaches some destination derived 
// from CMidiMsgListener that takes action.
//
// B) A change occurs in SONAR and the application calls RefreshSurface
// on any of its threads, likely the GUI's main thread.
//
// Notice that neither of those instances provide an opportunity for
// prolonged operation, since it would "hang" SONAR's threads.
//
// Instead, we must be completely "event driven" and remember the state
// we are IN so we may take action based on the present state and the
// next event.
//
// This is akin to the way digital multistate circuits work.
// They have a combinatory circuit that makes all the decision based
// on inputs that come from two places: the environment, and the memory.
//
// Well, we do not need something as full fledged as a window's message
// nor something as raw as a combinatory circuit, and that's where
// the state control family of classes come in.
//
// The overall picture is this:
// An object can be derived from either CStateListener or
// CMultiStateListener and be notified each time a StateShifter
// makes a state transition. Then this object can implement
// a behavior that is synchronized to the state transitions.
// Meanwhile, a CStateShifter maintains the state's value,
// and may transition its internal state as a response to
// external events such as incoming MIDI, or transitions from
// other states.
/////////////////////////////////////////////////////////////////////////

// forward decl
class CStateShifter;
class CMultiStateListener;
class CTimerClient;

/////////////////////////////////////////////////////////////////////////
// CTimer
//
// This object allows any CTimerClient to subscribe for timed / periodic
// notifications at specified intervals. This allows using only one
// timer in the entire surface framework.
/////////////////////////////////////////////////////////////////////////

typedef std::set< CTimerClient*, std::less< CTimerClient* >, std::allocator<CTimerClient* > > TimerClientSet;
typedef TimerClientSet::iterator TimerClientSetIt;

class CTimer
{
	friend class CTimerClient;

public:
	CTimer( CControlSurface *pSurface );
	~CTimer();

	HRESULT SetPeriod( CTimerClient *pClient, WORD wPeriod );

	void SetIsActive( BOOL bIsActive );

private:
	static void CALLBACK EXPORT timerCallback(
		UINT /*uID*/,
		UINT /*uMsg*/,
		DWORD dwUser,
		DWORD /*dw1*/,
		DWORD /*dw2*/
	);

	void setTimerActive( BOOL bIsActive );

	void onTimer();

	// member variables
	CControlSurface*	m_pSurface;
	UINT					m_uiTimerID;
	WORD					m_wTimerPeriod;
	BOOL					m_bIsTimerActive;
	TimerClientSet		m_setClients;
	CCriticalSection	m_cs;
};

/////////////////////////////////////////////////////////////////////////
// CTimerClient
//
// Base class that can subscribe for timer notifications
/////////////////////////////////////////////////////////////////////////

class CTimerClient
{
	friend class CTimer;

public:
	CTimerClient() :
		m_bOneShot( FALSE )
	{
	}

	virtual ~CTimerClient() {}

protected:
	virtual void Tick()
	{
		_ASSERT( 0 );
	}

	void SetIsOneShot( BOOL bIsOneShot )
	{
		m_bOneShot = bIsOneShot;
	}

	BOOL GetIsOneShot() { return m_bOneShot; }

private:
	WORD m_wCountTicks;
	WORD m_wNormalPeriod;
	BOOL m_bOneShot;
};

/////////////////////////////////////////////////////////////////////////
// CStateListener
//
// Base class for any class that wants to be notified of the change of
// a state. Pass the ID of the desired state (or its pointer) into
// the constructor to subscribe into that state.
/////////////////////////////////////////////////////////////////////////

class CStateListener
{
public:
	CStateListener( CControlSurface *pSurface, CStateShifter *pParent );
	CStateListener( CControlSurface *pSurface, DWORD dwStateID );
	virtual ~CStateListener();
	virtual HRESULT OnStateChange( int nNewState ) { return E_NOTIMPL; }
	DWORD GetParentStateID();
	DWORD GetParentCurrentState();
	virtual BOOL AllowShift()
	{
		return TRUE;
	}

protected:
	void setParent( CStateShifter* pParent );
	CStateShifter*	m_pParent;

private:
	CControlSurface* m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CMultiStateListener:
//
// Object capable of receiving notifications from any number of available
// CStateShifters. to subscribe for notification from a specific
// state, call SubscribeState( ID ) and pass the ID of the desired state.
/////////////////////////////////////////////////////////////////////////

// MultiStateListener aggregates a collection of state subscriptions
class CStateSubscription:	public CStateListener
{
public:
	CStateSubscription(
		CControlSurface *pSurface,
		CMultiStateListener *pMultiStateListener,
		DWORD dwStateID
	);

	HRESULT OnStateChange( int iNewState );

	virtual BOOL AllowShift();

protected:
	CMultiStateListener*		m_pMultiStateListener;
};

typedef std::map< DWORD, CStateSubscription* > StateSubscriptionMap;
typedef StateSubscriptionMap::iterator StateSubscriptionMapIt;

class CMultiStateListener
{
public:
	CMultiStateListener( CControlSurface* pSurface ):
		m_pSurface( pSurface )
	{
	}
	virtual ~CMultiStateListener();

	HRESULT SubscribeToState( DWORD dwStateID );
	HRESULT UnsubscribeFromState( DWORD dwStateID );

	HRESULT ClearSubscriptions();

	virtual HRESULT OnStateChange( DWORD dwStateID, int nNewState )
	{
		return S_FALSE;
	}

	int GetStateForID( DWORD dwID );

	virtual BOOL AllowShift( DWORD dwStateID )
	{
		return TRUE;
	}

protected:
	StateSubscriptionMap		m_mapStateSubscriptions;
	CControlSurface*			m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CStateShifter:
//
// Object capable of reacting to MIDI input by causing a transition
// in its state. Each time its state changes, the CStateShifter
// notifies all the CStateListeners concerned.
// Additionally, the user may derive a class from CStateShifter
// that implements CMultiStateListener, and react to transitions in other
// states by producing a transition in *this* state.
// Another alternative available is to derive from CHostNotifyListener
// and induce state transitions that result from host notifications.
// Notice that when any of this methods is implemented,
// the way to produce the state transition is to call
// SetNewState ( nState ), so that state validation and
// listener notification can take place.
/////////////////////////////////////////////////////////////////////////

// definition for set of state listeners contained in the state shifter
typedef std::set< CStateListener*, std::less< CStateListener* >, std::allocator< CStateListener* > > StateListenerSet;
typedef StateListenerSet::iterator StateListenerSetIt;

class CStateCondition
{
public:
	DWORD			dwConditionID;
	int			nConditionValue;
};

typedef std::set< CStateCondition*, std::less< CStateCondition* >, std::allocator< CStateCondition* > > StateConditionSet;
typedef StateConditionSet::iterator StateConditionSetIt;

class CStateConditionConjunction
{
public:
	~CStateConditionConjunction();
	BOOL Conjunction( CControlSurface* pSurface );
	void AddCondition( DWORD dwConditionID, int nConditionValue );
	void Clear();
	StateConditionSet	setConjunction;
};

// definition for a map of triggers to state transitions contained in the state shifter.
class CStateShiftVal: public CStateConditionConjunction // identifies a state transition
{
public:
	int					nShift;
	BOOL					bAbsolute;

};

typedef CStateConditionConjunction* CONDITION_HANDLE;

typedef std::vector< CStateShiftVal* >	ShiftValVector;
typedef ShiftValVector::iterator ShiftValVectorIt;

typedef std::map< CMidiTrigger, ShiftValVector > ShiftMap;
typedef ShiftMap::iterator ShiftMapIt;

#define SHIFTER_NO_CONDITION 0

class CStateShifter:		public CMidiMsgListener,
								public CMultiStateListener
{
	friend class CStateListener;
	friend class CStateMgr;

public:
	CStateShifter( CControlSurface *pSurface, DWORD dwStateID, DWORD dwInitialState = 0 );

	virtual ~CStateShifter();

	HRESULT AddShift(
		CMidiMsg *pMsg, DWORD dwVal,		// MIDI event's identity
		int nShift, BOOL bAbsolute = TRUE,		// where to shift (shift to absolute position or inc/dec?)
		CONDITION_HANDLE *phCondition = NULL
	);

	HRESULT AddShiftCondition(
		CONDITION_HANDLE hShift,
		DWORD dwConditionID,
		int nConditionValue
	);

	void ClearShifts();

	int GetCurrentState();

	// if getMin and getMax get overriden, theese no longer apply
	void SetMinState( int nMin ) { m_nMin = nMin; }
	void SetMaxState( int nMax ) { m_nMax = nMax; }

	void SetWrapAround( BOOL bWrap ) { m_bWrap = bWrap;}

	DWORD GetStateID() { return m_dwStateID; }

	virtual HRESULT SetNewState( int nNewState );

	// determines whether state transitions are allowed
	virtual HRESULT SetIsEnabled( BOOL bIsEnabled )
	{
		m_bIsEnabled = bIsEnabled;
		return S_OK;
	}

	// when enabled, notifications that do not result in value change
	// will not be sent to listeners (this is for optimizing StateShifters
	// that may get excessive notifications that result in no change,
	// and is disabled by default)
	virtual void SetThinNotify( BOOL bThin ) { m_bIsThinNotify = bThin; }

	virtual HRESULT SetIsPersistent( BOOL bIsPersistent )
	{
		m_bIsPersistent = bIsPersistent;
		return S_OK;
	}

	virtual BOOL GetIsPersistent() { return m_bIsPersistent; }

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nNewState ) { return S_FALSE; }

	virtual int GetMaxState() { return m_nMax; }
	virtual int GetMinState() { return m_nMin; }

	virtual void SetEngageListenerEnable( BOOL bEngage )
	{
		m_bListenerEnableEngaged = bEngage;
	}

protected:
	// helpers
	HRESULT	notifyListeners( int nNewState );
	BOOL		areListenersReceptive();
	
	BOOL		isThinNotify() { return m_bIsThinNotify; }

	// CMidiMsgListener override
	HRESULT	setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeInt; }

	// helpers
	BOOL			add( CStateListener *pBank );
	BOOL			remove( CStateListener *pBank );

	// members
	StateListenerSet	m_setListeners;
	ShiftMap				m_mapShifts;		// contains all the "shifts" (pairs of message and state transition)
	BOOL					m_bWrap;				// determines if the state wraps around beyond max
	BOOL					m_bListenerEnableEngaged; // when true, listeners can deny permision to shift.

	int					m_nMax;
	int					m_nMin;
	DWORD					m_dwStateID;

	BOOL					m_bIsThinNotify;
	BOOL					m_bIsEnabled;
	BOOL					m_bIsPersistent;

	int					m_nCurrentState; // do not try to induce state transitions by directly changing
	// this member variable, instead call SetNewState.

	CControlSurface*		m_pSurface;

private:
	CCriticalSection		m_cs;
};

/////////////////////////////////////////////////////////////////////////
// CStateMgr:
//
// Keeps a collection of CStateShifters for the purpose of persistence.
// When a CStateShifter is created it is assigned an ID. It is important
// to make sure that the ID's are all unique and that they do not change
// once a file employing such states has reached the general public.
// The state ID's are used as chunk ID's for persistence, so that
// the user may add new states, or even reorder the time of creation
// of states without worring about breaking persistence.
// It also declares a protected clas CDirtyState that implements
// CMultiStateListener for the purpose of flagging the dirty bit.
/////////////////////////////////////////////////////////////////////////

typedef std::map< DWORD, CStateShifter* > StateMap;
typedef StateMap::iterator StateMapIt;

typedef std::map< DWORD, int > StateChanges;
typedef StateChanges::iterator StateChangesIt;


class CTimedTransition
{
public:
	CTimedTransition()
	{
	}

	BOOL IsDue()
	{
		return (m_dwWhen <= timeGetTime());
	}

	bool operator <( const CTimedTransition &rhs ) const
	{
		if (m_dwWhen < rhs.m_dwWhen)
			return TRUE;
		else
			return FALSE;
	}

	DWORD		m_dwWhen;
	DWORD		m_dwID;
	int		m_nState;
};

typedef std::priority_queue<CTimedTransition> ShiftSchedule;

class CStateMgr : public CTimerClient
{
	friend class CStateShifter;
	friend class CMidiInputRouter;
	friend class CHostNotifyMulticaster;
	friend class CControlSurface;

public:
	CStateShifter* GetShifterFromID( DWORD dwStateID );
	void PostStateChange( DWORD dwStateID, int nNewState );
	void ScheduleStateChange(
		DWORD dwStateID,
		int nNewState,
		DWORD dwElapsed,
		BOOL bReplace = FALSE // determines if we find a scheduled change for the specified
		// ID in the queue, and it is the top one in the queue,
		// should we replace it with this one instead of inserting a new one.
		);

	void SetDirty() { m_bIsDirty = TRUE; }
	HRESULT IsDirty();
	HRESULT Load( IStream *pstm, const CLSID *pClsid );
	HRESULT Save( IStream *pstm, BOOL fClearDirty, const CLSID *pClsid );
	BOOL IsLoading() { return m_bIsLoading; }

	void Tick()
	{
		onScheduleTimer();
	}

protected:
	void deliverPostedStateChanges();

	void onScheduleTimer();

	class CDirtyState : public CMultiStateListener
	{
	public:
		CDirtyState( CControlSurface* pSurface ):
			CMultiStateListener( pSurface )
		{
		}

		// CMultiStateListener override
		HRESULT OnStateChange( DWORD dwStateID, int nNewState );
	};

	class CIsLoading
	{
	public:
		CIsLoading( BOOL *pbIsLoading )
		{
			m_pbIsLoading = pbIsLoading;
			if (m_pbIsLoading)
				*m_pbIsLoading = TRUE;
		}
		~CIsLoading()
		{
			if (m_pbIsLoading)
				*m_pbIsLoading = FALSE;
		}
	private:
		BOOL* m_pbIsLoading;
	};

private:

	// private constructor because it is a singleton
	CStateMgr( CControlSurface *pSurface );
	~CStateMgr();

	BOOL onStateCreated( CStateShifter *pState );
	BOOL onStateDestroyed( CStateShifter *pState );

	StateMap				m_mapStates; // map of states by ID
	BOOL					m_bIsDirty; // dirty flag
	CDirtyState			m_dirtyState; // multi state listener for setting the dirty bit
	StateChanges		m_DeferredStateChanges; // map of state transitions by ID
	ShiftSchedule		m_ScheduledStateChanges; // queue of scheduled state changes
	CCriticalSection	m_cs; // critical section to protect schedule
	BOOL					m_bIsLoading; // loading flag
	CControlSurface*	m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CSonarContainerBankShifter:
//
// Special kind of state shifter aware of min max limits in SONAR.
/////////////////////////////////////////////////////////////////////////

class CSonarContainerBankShifter:	public CStateShifter,
												public CHostNotifyListener
{
public:
	CSonarContainerBankShifter(
		CControlSurface *pSurface,
		DWORD dwStateID,
		int nBankWidth,
		DWORD dwInitialState = 0,
		WORD	wBankID = 0,
		WORD	wSlaveToBankID = 0xffff,
		BOOL	bUseRangeBasedMaximums = FALSE
	);

	~CSonarContainerBankShifter() {}

	// CStateShifter overrides
	HRESULT SetNewState( int nNewState );

	HRESULT OnStateChange( DWORD dwStateID, int nNewState );
	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );

	void SetUseAlignedBanks( BOOL bAligned )
	{
		m_bAligned = bAligned;
	}

protected:
	void maybeUpdateSonarStatus( int nNewState, BOOL bInvalidate = FALSE );
	int GetMaxState();
	int GetMinState();

	int			m_nBankWidth;
	WORD			m_wBankID;
	WORD			m_wSlaveToBankID;
	BOOL			m_bAligned;
	BOOL			m_bUseRangeBasedMaximums;

private:
	CControlSurface*	m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CStateShifterMux: (multiplexer)
//
// Manages the enable state of a collection of state shifters
// by enabling only one of them at a time as a response of some
// other state shifter's state.
//
// Don't be intimidated by the long name. This object's job is very
// small. As you may have noticed, the state shifters can be enabled
// or disabled. This has many possible uses.
//
// Imagine the following scenario: Your control surface has a set of buttons
// labled "cursor" which do many different things in many different situations.
// You wish to use the cursor buttons to change the current container
// For that, you have already created several instances of the 
// CSonarContainerBankShifter, each dealing with each kind of containers.
// Now you realize that all of those instances must respond to the same
// MIDI triggers (the triggers associated with the pair of buttons)...
//	but you are not shifting tracks and auxes and mains all at the same time
// right? Instead, you want to deal with auxes only on certain situations,
// and with tracks on other situations, and so on, yet all of them
// respond to the same pair of buttons!
// So how do we get out of this one?
//
// That is where CStateShifterMultiplexer comes in.
// You will already have defined some CStateShifter that knows
// what kind of container is being "looked at", depending on its state.
// Let's suppose that this state shifter is identified as the stContainerClass.
// and can be in a state such as ccTracks, ccMains, ccAuxes, etc.
//
// We also assume you already have a handful of CSonarContainerBankShifter
// instances, each dealing with a different type of container,
// and all of which listening into the same pair of cursor buttons.
// Now you create a CStateShifterMultiplexer m_pssmContClassMux
//
// You can add one by one into the multiplexer each of your
// CSonarContainerBankShifters, and the ID and state of the stateshifter
// that will enable it by making a succesion of calls like this:
//
// prototype:
// AddShifter( CStateShifter* pShifter, DWORD dwStateEnable );
//
// example:
// AddShifter( m_pssBaseTrack, ccTracks );
// AddShifter( m_pssBaseBus, ccBus );
// AddShifter( m_pssBaseMain, ccMains );
/////////////////////////////////////////////////////////////////////////

class CStateShifterMux : public CStateListener
{
public:
	// pass in the ID of the state that controls the mux switch
	CStateShifterMux( CControlSurface *pSurface, DWORD dwStateID );

	// associate each shifter with a position for this mux switch
	HRESULT AddShifter(
		CStateShifter* pShifter,
		DWORD dwStateEnable 
	);
	// depending on the switch's position the shifters will be enabled
	// and disabled so that only the one that matches the
	// current position is enabled.

	HRESULT OnStateChange( int nNewState );
private:
	StateMap			m_mapStates;
};

/////////////////////////////////////////////////////////////////////////
// CMixStrip parameter map macros:
/////////////////////////////////////////////////////////////////////////

#define BEGIN_PARAM_REMAP_MAP\
	HRESULT remapStripParam( CBaseMixParam *pParam )\
	{\
		if (pParam == NULL) return E_UNEXPECTED;\

#define PARAM_REMAP_MAP(theParamPtr, OnRemapParam) \
	else if (pParam == (CBaseMixParam*)theParamPtr)\
		OnRemapParam();\

#define END_PARAM_REMAP_MAP return S_OK; }

/////////////////////////////////////////////////////////////////////////
// CMixStrip:
//
// Collection of MixParams that belong to a hardware strip.
//
// Most control surfaces have the concept of a bank of one or more strips
// which can be dynamically assigned. The mapping of Sonar parameters
// to each hardware strip is normally replicated across the entire bank
// of strips.
//
// This class provides a single place where that mapping can be establised
// (the hardware strip number is left as an input parameter)
//
// It also provides an accessor SetContainer() by which
// the user may change the software strip number to which CMixStrip mapps.
// This method takes care of remapping any contained parameters.
//
// Finally, it also provides a hook for refreshing strip specific information
// into the MIDI device (e.g. refreshing a scribble strip)
//
// To effectively use CMixStrip you must do the following:
//		1. Derive a class CMyMixStrip for your specific application
//		2. Override the constructor and inside it create
//			all the various CMixParam's that may be needed
//			along with all the CMidiMsg's that may be used with the CMixParam's
//		3. Be sure to add each CMixParam by calling add( pMyMixParam )
//		4. Manage the lifetime of the associated CMidiMsg's in any appropriate way
//			(e.g. declare pointers to them as member variables of CMyMixStrip)
//		5. [Optionally] set a scribble message to be refreshed with the strip's name
//
//	Remember, CMidiMgs's used for CMixParam's must remain active until the strip is
// destroyed, therefore, clean them up in the destructor.
// 
// CMixStrip will automatically provide for you the clean up
// of CMixParam's added, and software container reparenting.
//
/////////////////////////////////////////////////////////////////////////

typedef std::set< CBaseMixParam*, std::less< CBaseMixParam* >, std::allocator<CBaseMixParam* > > ParamSet;
typedef ParamSet::iterator ParamSetIt;

class CMixStrip:	public CHostNotifyListener,
						public CMultiStateListener
{
public:
	CMixStrip(
		DWORD dwHWStripNum,
		CControlSurface *pSurface
	);

	virtual HRESULT Initialize() { return S_OK; }

	void SetScribbleMsg( CMidiMsg *pMsg ) { m_pMsgScribble = pMsg; }
	void SetIsScribbleEnabled( BOOL bEnable )
	{
		m_bIsScribbleEnabled = bEnable;
	}

	// accessors to determine the strip's identity
	virtual HRESULT SetContainer( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum );
	virtual BOOL IsAnyParamTouched();
	
	// NOTE: since the identity of the parameters inside the strip
	// is dependent on the kind of mixerStrip, we will not allow
	// that to be changed other than from inside CMixStrip itself
	// or classes derived from it.

	virtual void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );

	// helper for bank containment
	DWORD GetHWStripNum() { return m_dwHWStripNum; }

	HRESULT OnStateChange( DWORD dwStateID, DWORD dwState ) { return E_NOTIMPL; }

protected:
	BOOL add( CBaseMixParam *pParam );
	virtual void refreshScribble();
	virtual HRESULT remapStrip();
	virtual HRESULT remapStripParam( CBaseMixParam *pParam ) { return S_FALSE; }
	virtual HRESULT getScribbleText( LPSTR pszName, DWORD* pdwLen );

	// strip identity
	SONAR_MIXER_STRIP		m_mixerStrip;
	DWORD						m_dwStripNum;		// software strip mapped to this
	DWORD						m_dwHWStripNum;	// hardware strip mapped to this

	// MIDI identity
	CMidiMsg*				m_pMsgScribble;
	BOOL						m_bIsScribbleEnabled;

	ParamSet					m_setParams;

	CControlSurface*		m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
class CHostStateBit : public CHostNotifyListener,
							public CMidiMsgListener
{
public:
	CHostStateBit();
	~CHostStateBit();
	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );
};

/////////////////////////////////////////////////////////////////////////
// CMixStripBank<T>
//
// A container for multiple contiguous strips that can be reparented
// as a bank to any range of software strips.
//
// If you recall from the description of CMixStrip, you may remember that
// you would have likely declared a CMyCoolMixStrip, and in the constructor
// you create all the parameters required for your strip, as well as CMidiMsgs.
//
// This constructor is parametrized to take an index (the index of the hardware strip)
// and you likely took advantage of that index to write strip construction code
// that works for any of the strips in the bank...
//
// What the CMixStripBank<T> does is provide you with a container of an array of
// strips that can be all reparented by just making one call.
// To declare a bank you would write something like this:
//
// CMixStripBank< CMyCoolMixStrip > MyStripBank( 8 );
//
// look at the 8 in the constructor's argument list, specifying a bank of 8 strips
//
// The cool thing about CMixStripBank is that, since it is a CContainerBank
// the CContainerBankShifter, the CContainerBankShifter may automatically take care
// of reparenting your strip, and any other strips by calling into this single
// method.
//
// This all makes the assumption that your hardware supports only one bank
// if it supports multiple banks, you will need to use the
// bank ID parameter (defaulted to 1) otherwise, don't even worry about it.
//
/////////////////////////////////////////////////////////////////////////

typedef std::vector< CMixStrip* >				StripVector;
typedef StripVector::iterator						StripVectorIt;

#define BANK_FOLLOWS_TRACKS	1
#define BANK_FOLLOWS_MAINS		2
#define BANK_FOLLOWS_AUXES		4
#define BANK_FOLLOWS_ALL		-1

template<class T>
class CMixStripBank : public CMultiStateListener
{
public:
	CMixStripBank<T>(
		int nWidth,
		CControlSurface *pSurface,
		WORD wBankID = 0
		) : 
		m_nWidth( nWidth ),
		m_wBankID( wBankID ),
		m_nLastStrip( -1 ),
		m_nLastStripKind( -1 ),
		CMultiStateListener( pSurface )
	{
	}

	~CMixStripBank()
	{

		for (StripVectorIt it = m_vectorStrips.begin(); it != m_vectorStrips.end(); it++)
		{
			CMixStrip *pStrip = *it;
			_ASSERT( pStrip );
			delete pStrip;
		}
	}

	HRESULT Initialize( DWORD dwCoClassMask = BANK_FOLLOWS_TRACKS )
	{
		HRESULT hr;
		for (int ix = 0; ix < m_nWidth; ix++)
		{
			CMixStrip* pStrip = new T( ix, m_pSurface );
			if (NULL == pStrip)
				return E_OUTOFMEMORY;
			if (FAILED( hr = pStrip->Initialize() ))
				return hr;
			m_vectorStrips.push_back( pStrip );
		}
		if (FAILED( hr = SubscribeToState( stContainerClass ) ))
			return hr;
		if (dwCoClassMask & BANK_FOLLOWS_TRACKS)
		{
			if (FAILED( hr = SubscribeToState( MAKELONG( stBaseTrack, m_wBankID ) ) ))
				return hr;
		}
		if (dwCoClassMask & BANK_FOLLOWS_AUXES)
		{
			if (FAILED( hr = SubscribeToState( MAKELONG( stBaseBus, m_wBankID ) ) ))
				return hr;
		}
		if (dwCoClassMask & BANK_FOLLOWS_MAINS)
		{
			if (FAILED( hr = SubscribeToState( MAKELONG( stBaseMain, m_wBankID ) ) ))
				return hr;
		}

		return S_OK;
	}

	BOOL AllowShift( DWORD dwStateID )
	{
		// if any strip contains touched controls we must disallow shift.

		for (int ix = 0; ix < m_nWidth; ix++)
		{
			CMixStrip *pStrip = m_vectorStrips[ix]; // handy ref
			if (pStrip->IsAnyParamTouched())
				return FALSE;
		}
		return TRUE;
	}

	// CMultiStateListener override 
	HRESULT OnStateChange( DWORD dwStateID, int nNewState )
	{
		HRESULT hr = E_FAIL;

		SONAR_MIXER_STRIP mixerStrip = MIX_STRIP_TRACK;

		int nBaseContainer = 0;
		BOOL bBaseContainerDefined = FALSE;

		switch (GetStateForID( MAKELONG( stContainerClass, m_wBankID ) ))
		{
		case ccTracks:
			mixerStrip = MIX_STRIP_TRACK;
			nBaseContainer = GetStateForID( MAKELONG( stBaseTrack, m_wBankID ) );
			bBaseContainerDefined = TRUE;

			break;

		case ccBus:
			mixerStrip = MIX_STRIP_BUS;
			nBaseContainer = GetStateForID( MAKELONG( stBaseBus, m_wBankID ) );
			bBaseContainerDefined = TRUE;
			break;

		case ccMains:
			mixerStrip = MIX_STRIP_MASTER;
			nBaseContainer = GetStateForID( MAKELONG( stBaseMain, m_wBankID ) );
			bBaseContainerDefined = TRUE;
			break;

		default:
			_ASSERT( 0 ); // unknown strip container class
		}

		if (!bBaseContainerDefined)
			return S_FALSE;

		for (int ix = 0; ix < m_nWidth; ix++)
		{
			CMixStrip *pStrip = m_vectorStrips[ix]; // handy ref

			DWORD dwStripIndex = pStrip->GetHWStripNum();
			hr = pStrip->SetContainer( mixerStrip, nBaseContainer + dwStripIndex );
			if (FAILED( hr ))
				return E_UNEXPECTED;
		}
		
		return S_OK;
	}

protected:
	StripVector		m_vectorStrips;
	int				m_nWidth;
	WORD				m_wBankID;

	// variables for updating SONAR mapping status
	int				m_nLastStrip;
	int				m_nLastStripKind;
};

/////////////////////////////////////////////////////////////////////////
class CShuttleVal:	public CStateConditionConjunction
{
public:
	enum EAction
	{
		Pressed,
		Released
	};

	EAction		eAction;
	DWORD			dwPeriod;
	DWORD			dwWait;
	MFX_TIME		tAmount;
	BOOL			bNegative;
};

/////////////////////////////////////////////////////////////////////////
// CTransportListener
//
// Base class for a component that may be passed into the transport
// for updates on the transport status. A derived TransportListener
// may take actions such as updating output MIDI status as a response
// of transport notifications
/////////////////////////////////////////////////////////////////////////

enum ETransportState
{
	tsInvalid,
	tsStop,
	tsPlay,
	tsRecord,
	tsRecordAutomation,
	tsShuttleForward,
	tsShuttleRewind
};

/////////////////////////////////////////////////////////////////////////
class CTransportListener
{
public:
	CTransportListener( CControlSurface *pSurface )
	{
		m_pSurface = pSurface;
	}

	virtual HRESULT Initialize();
	~CTransportListener();

	virtual HRESULT OnTransportUpdate( ETransportState tNext ) PURE;

protected:
	CControlSurface*		m_pSurface;

};

/////////////////////////////////////////////////////////////////////////
// CTransport
//
// Provides the link between input MIDI messages and the SONAR transport.
/////////////////////////////////////////////////////////////////////////

typedef std::map< CMidiTrigger, CShuttleVal*> ShuttleMap;
typedef ShuttleMap::iterator ShuttleMapIt;

/////////////////////////////////////////////////////////////////////////
class CTransport:	public CTimerClient,
						public CMidiMsgListener,
						public CHostNotifyListener
{
	friend class CControlSurface;
	friend class CTransportListener;

public:

	void SetRecordMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetWriteMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetPlayMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetStopMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetPanicMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetRunAudioMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetRejectLoopTakeMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetFetchLoopFromMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetFetchLoopThruMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetFetchPunchFromMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetFetchPunchThruMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetGotoTopMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetGotoEndMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetJogFwdMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetJogBckMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);
	void SetJogAmount( MFX_TIME JogTimeIncrement )
	{
		m_JogTimeIncrement = JogTimeIncrement;
	}

	void SetNextMarkerMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);

	void SetPrevMarkerMsg(
		CMidiMsg *pInMsg, DWORD dwInVal = VAL_ANY
	);

	void SetScrubMsg( CMidiMsg *pInMsg, DWORD dwInVal, CMidiMsg *pOutMsg, DWORD dwOutVal );


	// Shuttle is the transport movement where a button is pressed (or wheel turned)
	// and for as long as it remains engaged we continue to move the now time.
	//
	// Initially, when the "engage" message is received, we move immediately by the stAmount.
	// After that, we issue repeated movements by the same amount, until 
	// the button or wheel is released.
	//
	// AddShuttle adds an entry to the list of commands that can shuttle the
	// transport back and forth.
	// The dwPeriod expresses hundreths of a second between updates.
	// a typical value would be 5 to 10 (from 50 milliseconds to 100)
	// The stAmount specified how far to move on each update.
	// Since we provide a sign flag (bNegative), it is possible to express a "rewind" type action.
	//
	// the shuttle wait time specifies how long to wait before 
	// auto repeat kicks in. dwWait is hundreths of a second. A typical value would be 30 to 70.
	// (from 300 milliseconds to 700)
	// When using Shuttle wheels, it is desirable to have no wait time at all, and instead
	// to have the wheel trigger a low rate when a small displacement is sensed.

	HRESULT AddShuttle(
		DWORD dwPeriod, MFX_TIME tAmount, BOOL bNegative, DWORD dwWait,
		CMidiMsg *pMsgPress, DWORD dwValPress,
		CMidiMsg *pMsgRelease, DWORD dwValRelease,
		CONDITION_HANDLE *phCondition = NULL
	);

	// clears the list of shuttle entries
	void ClearShuttle();

	// restore the transport to the condition right after creation
	void ClearAll();

	void OnHostNotify( DWORD fdwRefresh, DWORD dwCookie );

	void SetIsEnabled( BOOL bIsEnabled )
	{
		m_bIsEnabled = bIsEnabled;
	}
	
	void Tick()
	{
		onShuttleTimer();
	}

	// methods to make actions conditional to states
	CONDITION_HANDLE GetGlobalConditionHandle() { return &m_sccGlobal; }
	CONDITION_HANDLE GetRecordConditionHandle() { return &m_sccRecord; }
	CONDITION_HANDLE GetWriteConditionHandle() { return &m_sccRecAuto; }
	CONDITION_HANDLE GetPlayConditionHandle() { return &m_sccPlay; }
	CONDITION_HANDLE GetStopConditionHandle() { return &m_sccStop; }
	CONDITION_HANDLE GetPanicConditionHandle() { return &m_sccPanic; }
	CONDITION_HANDLE GetRunAudioConditionHandle() { return &m_sccRunAudio; }
	CONDITION_HANDLE GetRejectLoopTakeConditionHandle() { return &m_sccRejectLoopTake; }
	CONDITION_HANDLE GetFetchPunchFromConditionHandle() { return &m_sccFetchPunchFrom; }
	CONDITION_HANDLE GetFetchPunchThruConditionHandle() { return &m_sccFetchPunchThru; }
	CONDITION_HANDLE GetFetchLoopFromConditionHandle() { return &m_sccFetchLoopFrom; }
	CONDITION_HANDLE GetFetchLoopThruConditionHandle() { return &m_sccFetchLoopThru; }
	CONDITION_HANDLE GetGotoTopConditionHandle() { return &m_sccGotoTop; }
	CONDITION_HANDLE GetGotoEndConditionHandle() { return &m_sccGotoEnd; }
	CONDITION_HANDLE GetJogFwdConditionHandle() { return &m_sccJogFwd; }
	CONDITION_HANDLE GetJogBckConditionHandle() { return &m_sccJogBck; }
	CONDITION_HANDLE GetPrevMarkerConditionHandle() { return &m_sccPrevMarker; }
	CONDITION_HANDLE GetNextMarkerConditionHandle() { return &m_sccNextMarker; }
	CONDITION_HANDLE GetScrubConditionHandle() { return &m_sccScrub; }
	
	
	void Jog( int nIncrements );

	BOOL GetIsPlaying() { return m_bIsPlaying; }

protected:
	void onShuttleTimer();
	void shuttleAdvance();
	void timeAdvance( MFX_TIME timeIncrement, BOOL bNegative );

	// CMidiMsgListener overrides
	EValueType getValueType() { return TypeInt; }
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );

	void maybeResetShuttle();
	CTransportListener* getListener() { return m_pListener; }

	void setListener( CTransportListener *pListener )
	{
		m_pListener = pListener;
	}

private:

	// private constructor and destructor because it is a singleton
	CTransport( CControlSurface* pSurface );
	virtual ~CTransport();

	static CTransport*		m_instance;
	ShuttleMap					m_mapShuttle;
	CShuttleVal*				m_pShuttleState;
	DWORD							m_dwTimerCounter;
	BOOL							m_bIsEnabled;

	CMidiTrigger				m_trigRecord;
	CMidiTrigger				m_trigRecAuto;
	CMidiTrigger				m_trigPlay;
	CMidiTrigger				m_trigStop;
	CMidiTrigger				m_trigPanic;
	CMidiTrigger				m_trigRunAudio;
	CMidiTrigger				m_trigRejectLoopTake;
	CMidiTrigger				m_trigFetchLoopFrom;
	CMidiTrigger				m_trigFetchLoopThru;
	CMidiTrigger				m_trigFetchPunchFrom;
	CMidiTrigger				m_trigFetchPunchThru;

	CMidiTrigger				m_trigGotoTop;
	CMidiTrigger				m_trigGotoEnd;
	
	CMidiTrigger				m_trigJogFwd;
	CMidiTrigger				m_trigJogBck;

	CMidiTrigger				m_trigPrevMarker;
	CMidiTrigger				m_trigNextMarker;

	CMidiTrigger				m_trigScrubIn;
	CMidiTrigger				m_trigScrubOut;

	// conditions
	CStateConditionConjunction	m_sccGlobal;

	CStateConditionConjunction	m_sccRecord;
	CStateConditionConjunction	m_sccRecAuto;
	CStateConditionConjunction	m_sccPlay;
	CStateConditionConjunction	m_sccStop;
	CStateConditionConjunction	m_sccPanic;
	CStateConditionConjunction	m_sccRunAudio;
	CStateConditionConjunction	m_sccRejectLoopTake;
	CStateConditionConjunction	m_sccFetchLoopFrom;
	CStateConditionConjunction	m_sccFetchLoopThru;
	CStateConditionConjunction	m_sccFetchPunchFrom;
	CStateConditionConjunction	m_sccFetchPunchThru;
	CStateConditionConjunction	m_sccGotoTop;
	CStateConditionConjunction	m_sccGotoEnd;

	CStateConditionConjunction	m_sccJogFwd;
	CStateConditionConjunction	m_sccJogBck;

	CStateConditionConjunction	m_sccPrevMarker;
	CStateConditionConjunction	m_sccNextMarker;

	CStateConditionConjunction	m_sccScrub;

	MFX_TIME						m_JogTimeIncrement;
	CControlSurface*			m_pSurface;
	ETransportState			m_tState;
	CTransportListener*		m_pListener;
	BOOL							m_bIsPlaying;
};

/////////////////////////////////////////////////////////////////////////
// CSonarCommander:
//
// Allows assigning conditional triggers to SonarCommand ID's.
// This way, a specified trigger can invoke the desired SONAR command.
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
class CCommanderVal:	public CStateConditionConjunction
{
public:
	DWORD dwCmdID;
};

typedef std::set< CCommanderVal*, std::less< CCommanderVal* >, std::allocator< CCommanderVal* > > CommanderValSet;
typedef CommanderValSet::iterator CommanderValSetIt;

typedef std::map< CMidiTrigger, CommanderValSet> CommandTriggerMap;
typedef CommandTriggerMap::iterator CommandTriggerMapIt;

class CSonarCommander:	public CMidiMsgListener
{
public:
	CSonarCommander( CControlSurface* pSurface );
	virtual ~CSonarCommander();
	HRESULT AddCommand(
		CMidiMsg *pMsg, DWORD dwVal,
		DWORD dwCmdID,
		CONDITION_HANDLE *phCondition = NULL
	);

	void ClearCommands();

	HRESULT DoCommand( DWORD dwCmdID );

protected:

	// CMidiMsgListener overrides
	EValueType getValueType() { return TypeInt; }
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );

private:
	CStateConditionConjunction	m_sccGlobal;
	CommandTriggerMap		m_mapCommandTriggers;
	CControlSurface*		m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CThrottle:
//
// This object can reduce (throttle) the rate of a given notification to
// not exceed a specified period. If an object needs to react to a 
// particular notification, but its reactions cannot occur at an unlimited
// rate, the CThrottle can be used so that reactions occur only at specified
// intervals. This is helpful in cases where the response to a notification
// involves dense MIDI output.
//
// For example, if an object needs to react to a notification in a "throttled"
// manner, it can derive CThrottle, specify the minimum period for notifications.
// In the method that would have otherwise handled the notifications, it will call
// OnEventToThrottle (a method in CThrottle).

// Last, override OnThrottleElapsed so that the actual notification response takes
// place on this method.
// 
// The class CLastParamChange is a good example for an object utilizing this.
/////////////////////////////////////////////////////////////////////////

class CThrottle: public CTimerClient
{
public:
	enum EThrottleMode
	{
		thMonostable, // every time the event occurs, the timer is reset, thus extending
			// the time for the next notification.
			// therefore, a throttle elapsed notification is only sent dwThrottleTime
			// after no "events to throttle" have been received.
		thAstable, // resetting the timer while waiting is prevented. Thus, throttle elapsed
		// notifications will be triggered periodically (i.e., reception of a newer
		// "event to throttle" will not reschedule an already waiting timer request.
	};
	CThrottle( CControlSurface *pSurface, DWORD dwThrottleTime, EThrottleMode eMode = thAstable );

	void SetThrottleTime( DWORD dwThrottleTime )
	{
		_ASSERT( dwThrottleTime >= 0 );
		m_dwThrottleTime = dwThrottleTime;
	}

	void OnEventToThrottle();

	void Tick()
	{
		OnThrottleElapsed();
		m_bWaiting = FALSE;
	}

	virtual void OnThrottleElapsed() PURE;

protected:
	DWORD					m_dwThrottleTime;
	EThrottleMode		m_eMode;
	BOOL					m_bWaiting;
	CControlSurface*	m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
// CLastParamChangeListener:
//
// An object derived from it will be notified each time a parameter change
// is sent to SONAR from the framework.
// This can be handy if the developer wants a feature on the surface where
// the unit shows a "detail" of what was the last parameter change.
/////////////////////////////////////////////////////////////////////////

class CLastParamChangeListener
{
public:
	CLastParamChangeListener( CControlSurface *pSurface );
	virtual ~CLastParamChangeListener();
	virtual void OnParamChange() PURE;

private:
	CControlSurface*		m_pSurface;
};

/////////////////////////////////////////////////////////////////////////
//	CLastParamChange:
//
// A multicaster that relays notifications of the last parameter change
// sent to SONAR.
/////////////////////////////////////////////////////////////////////////

typedef std::set< CLastParamChangeListener*, std::less< CLastParamChangeListener* >, std::allocator< CLastParamChangeListener* > > ParamListenerSet;
typedef ParamListenerSet::iterator ParamListenerSetIt;

class CLastParamChange:	public CThrottle
{
	friend class CBaseMixParam;
	friend class CLastParamChangeListener;
	friend class CControlSurface;

public:

	BOOL GetLastParamID( SONAR_MIXER_STRIP *pmixerStrip, DWORD *pdwStripNum, SONAR_MIXER_PARAM *pmixerParam, DWORD *pdwParamNum );
	BOOL GetLastMixParam( CBaseMixParam **ppParam );

	void SetThrottleTime( DWORD dwThrottleTime );
	BOOL IsDefined();

	virtual void OnThrottleElapsed();

private:

	CLastParamChange( CControlSurface *pSurface );

	// advise and unadvise methods for listeners
	void add( CLastParamChangeListener* pListener );
	void remove( CLastParamChangeListener* pListener );

	// helpers
	void notifyListeners();

	// called by CMixParamXXX
	void setLastParamID( CBaseMixParam *pParam, SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum );

	CBaseMixParam*					m_pParam;
	SONAR_MIXER_STRIP				m_mixerStrip;
	DWORD								m_dwStripNum;
	SONAR_MIXER_PARAM				m_mixerParam;
	DWORD								m_dwParamNum;
	BOOL								m_bIsDefined;
	ParamListenerSet				m_setListeners;
   CControlSurface*				m_pSurface;
};

/////////////////////////////////////////////////////////////////////////