/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkStates.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SfkStateDefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CTimer
/////////////////////////////////////////////////////////////////////////
CTimer::CTimer( CControlSurface *pSurface ):
	m_pSurface( pSurface ),
	m_bIsTimerActive( FALSE )
{
	TIMECAPS timeDevCaps;
	timeGetDevCaps( &timeDevCaps, sizeof( timeDevCaps ) );
	m_wTimerPeriod = max( timeDevCaps.wPeriodMin, 10 );
}

/////////////////////////////////////////////////////////////////////////
CTimer::~CTimer()
{
	setTimerActive( FALSE );
}

/////////////////////////////////////////////////////////////////////////
void CTimer::SetIsActive( BOOL bIsActive )
{
	setTimerActive( bIsActive );
}

/////////////////////////////////////////////////////////////////////////
void CALLBACK EXPORT CTimer::timerCallback(
		UINT /*uID*/,
		UINT /*uMsg*/,
		DWORD dwUser,
		DWORD /*dw1*/,
		DWORD /*dw2*/
	)
{
	reinterpret_cast<CControlSurface*>(dwUser)->GetTimer()->onTimer();
}

/////////////////////////////////////////////////////////////////////////
void CTimer::setTimerActive( BOOL bIsActive )
{
	BOOL bWasActive = m_bIsTimerActive;
	if (bIsActive && !bWasActive ) // if it is not active but we want to make it so
	{
		MMRESULT mr = timeBeginPeriod( m_wTimerPeriod );
		_ASSERT( mr != TIMERR_NOCANDO );

		m_uiTimerID = timeSetEvent(
			10,
			m_wTimerPeriod,
			(LPTIMECALLBACK)timerCallback,
			(DWORD)m_pSurface,
			TIME_PERIODIC | TIME_CALLBACK_FUNCTION
		);
 
		if (m_uiTimerID != NULL)
			m_bIsTimerActive = TRUE;
	}
	else if (!bIsActive && bWasActive) // if it is active and we want to make it inactive
	{
		timeKillEvent( m_uiTimerID );
		m_bIsTimerActive = FALSE;

		MMRESULT mr = timeEndPeriod( m_wTimerPeriod );
		_ASSERT( mr != TIMERR_NOCANDO );
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CTimer::SetPeriod( CTimerClient *pClient, WORD wPeriod )
{
	CCriticalSectionAuto lock( m_cs );

	if (pClient == NULL)
		return E_POINTER;

	TimerClientSetIt itFind = m_setClients.find( pClient );
	
	if (wPeriod == 0)
	{
		// remove from service
		if (itFind != m_setClients.end())
		{
			m_setClients.erase( itFind );

			if (m_setClients.size() == 0)
			{
				setTimerActive( FALSE );
			}

			return S_OK;
		}
		else
			return S_FALSE;
	}
	else
	{
		// add or update for service
		if (itFind == m_setClients.end())
		{
			m_setClients.insert( TimerClientSet::value_type( pClient ) );
			setTimerActive( TRUE );
		}

		pClient->m_wNormalPeriod = max( wPeriod / m_wTimerPeriod, 1 ) ;
		pClient->m_wCountTicks = 0;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CTimer::onTimer()
{
	// service all clients
	TimerClientSet setRemove;
	TimerClientSet setTick;
	TimerClientSetIt it;
	{
		CCriticalSectionAuto lock( m_cs );
		// check what to do with each timer
		for (it = m_setClients.begin(); it != m_setClients.end(); it++)
		{
			// write each state ID and its state.
			CTimerClient *pClient = (*it);
			pClient->m_wCountTicks++;
			if (pClient->m_wCountTicks >= pClient->m_wNormalPeriod)
			{
				// rollover
				pClient->m_wCountTicks = 0;
				setTick.insert( TimerClientSet::value_type( pClient ) );

				if (pClient->GetIsOneShot())
				{
					setRemove.insert( TimerClientSet::value_type( pClient ) );
				}
			}
		}


		// remove the one shots
		for (it = setRemove.begin(); it != setRemove.end(); it++)
		{
			CTimerClient *pClient = (*it);
			TimerClientSetIt itFind = m_setClients.find( pClient );
			if (itFind != m_setClients.end())
			{
				// if found
				m_setClients.erase( itFind );
			}
		}

		if (m_setClients.size() == 0)
		{
			setTimerActive( FALSE );
		}

	}

	// tick them (outside of the critical section)
	for ( it = setTick.begin(); it != setTick.end(); it++)
	{
		CTimerClient *pClient = (*it);
		pClient->Tick();
	}
}

/////////////////////////////////////////////////////////////////////////
// CStateListener:
/////////////////////////////////////////////////////////////////////////
CStateListener::CStateListener( CControlSurface *pSurface, CStateShifter *pParent ) :
	m_pSurface( pSurface )
{
	setParent( pParent );
}

/////////////////////////////////////////////////////////////////////////
CStateListener::CStateListener( CControlSurface *pSurface, DWORD dwStateID ) :
	m_pSurface( pSurface )
{
	CStateShifter *pParent = m_pSurface->GetStateMgr()->GetShifterFromID( dwStateID );
	setParent( pParent );
}

/////////////////////////////////////////////////////////////////////////
DWORD CStateListener::GetParentCurrentState()
{
	return m_pParent->GetCurrentState();
}

/////////////////////////////////////////////////////////////////////////
void CStateListener::setParent( CStateShifter* pParent )
{
	_ASSERT( pParent ); // make sure that the state shifter this listener
	// slaves to has been created BEFORE this listener.

	if (pParent != NULL)
	{
		m_pParent = pParent;
		m_pParent->add( this );// add myself to the bank shifter

		// let actual implementations decide whether they want to get the current state.
	}
}
/////////////////////////////////////////////////////////////////////////
CStateListener::~CStateListener()
{
	m_pParent->remove( this );
}

/////////////////////////////////////////////////////////////////////////
DWORD CStateListener::GetParentStateID()
{
	if (m_pParent != NULL)
		return m_pParent->GetStateID();
	else
	{
		_ASSERT( 0 );
		return -1;
	}
}

/////////////////////////////////////////////////////////////////////////
// CMultiStateListener:
/////////////////////////////////////////////////////////////////////////
// CStateSubscription is used by CMultiStateListener
CStateSubscription::CStateSubscription(
			CControlSurface *pSurface,
			CMultiStateListener *pMultiStateListener,
			DWORD dwStateID
		) :
	m_pMultiStateListener( pMultiStateListener ),
	CStateListener( pSurface, dwStateID )
{
	OnStateChange( GetParentCurrentState() );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateSubscription::OnStateChange( int nNewState )
{
	if (m_pMultiStateListener)
		return m_pMultiStateListener->OnStateChange( GetParentStateID(), nNewState );
	else
	{
		_ASSERT( 0 ); // the pointer to m_pMultiStateListener has not been set.
		return E_UNEXPECTED;
	}
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateSubscription::AllowShift()
{
	return m_pMultiStateListener->AllowShift( GetParentStateID() );
}

/////////////////////////////////////////////////////////////////////////
CMultiStateListener::~CMultiStateListener()
{
	HRESULT hr = ClearSubscriptions();
	_ASSERT( SUCCEEDED( hr ) );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMultiStateListener::ClearSubscriptions()
{
	StateSubscriptionMapIt it;
	for (it = m_mapStateSubscriptions.begin(); it != m_mapStateSubscriptions.end(); it++)
	{
		delete (*it).second;
	}

	m_mapStateSubscriptions.clear();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMultiStateListener::SubscribeToState( DWORD dwStateID )
{
	StateSubscriptionMapIt it = m_mapStateSubscriptions.find( dwStateID );

	if (it == m_mapStateSubscriptions.end()) // if not found insert it
	{
		CStateSubscription *pSubscription = new CStateSubscription( m_pSurface, this, dwStateID );
		m_mapStateSubscriptions.insert(StateSubscriptionMap::value_type( dwStateID, pSubscription ) );
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMultiStateListener::UnsubscribeFromState( DWORD dwStateID )
{
	StateSubscriptionMapIt it = m_mapStateSubscriptions.find( dwStateID );
	if (it != m_mapStateSubscriptions.end()) // should be found... 
	{
		delete (*it).second;
		m_mapStateSubscriptions.erase( it );
		
		return S_OK;
	}
	else
	{
		// not found
		_ASSERT( 0 );
		return E_UNEXPECTED;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
int CMultiStateListener::GetStateForID( DWORD dwID )
{
	CStateShifter *pShifter = m_pSurface->GetStateMgr()->GetShifterFromID( dwID );

	_ASSERT( pShifter );

	if (pShifter)
		return pShifter->GetCurrentState();
	else
		return -1;
}

/////////////////////////////////////////////////////////////////////////
// CStateShifter:
/////////////////////////////////////////////////////////////////////////
CStateShifter::CStateShifter(
	CControlSurface *pSurface,
	DWORD dwStateID,
	DWORD dwInitialState /* = 0 */
	) :
	CMultiStateListener( pSurface ),
	m_pSurface( pSurface ),
	m_bWrap( FALSE ),
	m_nMin( 0 ),
	m_nMax( 0 ),
	m_dwStateID( dwStateID ),
	m_bIsEnabled( TRUE ),
	m_bIsPersistent( TRUE ),
	m_bListenerEnableEngaged( FALSE ),
	m_bIsThinNotify( FALSE )
{
	notifyListeners( dwInitialState );
	m_pSurface->GetStateMgr()->onStateCreated( this );
}

/////////////////////////////////////////////////////////////////////////
CStateShifter::~CStateShifter()
{
	m_pSurface->GetStateMgr()->onStateDestroyed( this );
	ClearShifts();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifter::AddShift(
		CMidiMsg *pMsg, DWORD dwVal,
		int nShift, BOOL bAbsolute /* = TRUE */,
		CONDITION_HANDLE *phCondition /* = NULL */
	)
{
	_ASSERT( pMsg );

	CMidiTrigger key;
	key.m_dwVal = dwVal;
	key.m_pMsg = pMsg;

	ShiftMapIt itSM;
	itSM = m_mapShifts.find( key );

	// create it...
	CStateShiftVal *pVal = new CStateShiftVal;
	pVal->bAbsolute = bAbsolute;
	pVal->nShift = nShift;

	if (itSM == m_mapShifts.end()) // if trigger not found
	{
		// insert it...
		ShiftValVector vector;
		vector.push_back( ShiftValVector::value_type( pVal ) );
		if (phCondition)
			*phCondition = pVal;

		m_mapShifts.insert( ShiftMap::value_type( key, vector ) );

		pMsg->AddListener( this );
	}
	else // trigger was found. Add another shift for this trigger
	{		
		ShiftValVector &vector = (*itSM).second;
		vector.push_back( ShiftValVector::value_type( pVal ) );

		if (phCondition)
			*phCondition = pVal;
	}

	if (bAbsolute)
	{
		if (nShift > GetMaxState())
			SetMaxState( nShift );
		if (nShift < GetMinState())
			SetMinState( nShift );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifter::AddShiftCondition( CONDITION_HANDLE hCondition, DWORD dwConditionID, int nConditionValue )
{
	_ASSERT( hCondition );
	if ( hCondition == NULL )
		return E_INVALIDARG;

	hCondition->AddCondition( dwConditionID, nConditionValue );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CStateShifter::ClearShifts()
{
	ShiftMapIt it;

	for (it = m_mapShifts.begin(); it != m_mapShifts.end(); it++)
	{
		it->first.m_pMsg->RemoveListener( this );
		ShiftValVector &vector = it->second;
		for (ShiftValVectorIt itSvv = vector.begin(); itSvv != vector.end(); itSvv++)
		{
			CStateShiftVal *pVal = (*itSvv);
			delete pVal;
		}
	}

	m_mapShifts.clear();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifter::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	// determine where to shift to
	int nNewState = 0;

	CMidiTrigger key;
	key.m_pMsg = pMsg;
	key.m_dwVal = dwVal;

	ShiftMapIt itSM = m_mapShifts.find( key );

	if (itSM == m_mapShifts.end())
		return S_FALSE;	// we have received an unintended MIDI message

	ShiftValVector &vector = itSM->second; // handy ref

	for (ShiftValVectorIt itVector = vector.begin(); itVector != vector.end(); itVector++)
	{
		CStateShiftVal *pVal = (*itVector);
		
		if (pVal->Conjunction( m_pSurface ) == FALSE)
			continue;

		if (pVal->bAbsolute)
			nNewState = pVal->nShift;
		else
			nNewState = m_nCurrentState + pVal->nShift;

		return SetNewState( nNewState );
	}
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
void CStateConditionConjunction::AddCondition( DWORD dwConditionID, int nConditionValue )
{
	CStateCondition *pscCondition = new CStateCondition;
	pscCondition->dwConditionID = dwConditionID;
	pscCondition->nConditionValue = nConditionValue;
	setConjunction.insert( StateConditionSet::value_type( pscCondition ) );
}

/////////////////////////////////////////////////////////////////////////
CStateConditionConjunction::~CStateConditionConjunction()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////
void CStateConditionConjunction::Clear()
{
	for (StateConditionSetIt it = setConjunction.begin(); it != setConjunction.end(); it++)
	{
		CStateCondition *pscCondition = (*it);
		delete pscCondition;
	}
	setConjunction.clear();
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateConditionConjunction::Conjunction( CControlSurface* pSurface )
{
	for ( StateConditionSetIt it = setConjunction.begin(); it != setConjunction.end(); it++)
	{
		DWORD dwStateID = (*it)->dwConditionID;
		CStateShifter *pShifter = pSurface->GetStateMgr()->GetShifterFromID( dwStateID );

		if (pShifter == NULL)
		{
			_ASSERT( 0 );
			return FALSE;
		}

		int nCurState = pShifter->GetCurrentState();

		if (nCurState != (*it)->nConditionValue)
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
int CStateShifter::GetCurrentState()
{
	CCriticalSectionAuto lock( m_cs );

	return m_nCurrentState;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifter::SetNewState( int nNewState )
{
	CCriticalSectionAuto lock( m_cs );

	if (!m_bIsEnabled)
		return S_FALSE;

	// TODO: implement wrap around
	// rangecheck the state transition
	if (m_bWrap)
	{
		_ASSERT( GetMinState() <= GetMaxState() );
		if (GetMinState() < GetMaxState())
		{
			DWORD dwRange = ( GetMaxState() + 1 ) - GetMinState();
			DWORD dwNormalState = nNewState - GetMinState();
			DWORD dwWrappedNormalState = dwNormalState % dwRange;

			nNewState = dwWrappedNormalState + GetMinState();
		}
	}
	else
	{
		if (nNewState > GetMaxState())
			nNewState = GetMaxState();

		if (nNewState < GetMinState())
			nNewState = GetMinState();
	}

	// make the shift
	HRESULT hr = notifyListeners( nNewState );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	return hr;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifter::notifyListeners( int nNewState )
{
	// perform some notification "thinning"
	if ((nNewState == GetCurrentState()) && isThinNotify())
		return S_FALSE;

	if (m_bListenerEnableEngaged)
	{
		// if the listeners do not allow a change, we exit
		if (!areListenersReceptive())
			return S_FALSE;
	}

	// first set the state.. Listeners may want to "ask for it"
	// instead of acting on the StateHandler's input parameter.
	m_nCurrentState = nNewState;

	StateListenerSetIt itBS;
	HRESULT hr = E_FAIL;
	for (itBS = m_setListeners.begin(); itBS != m_setListeners.end(); itBS++)
	{
		hr = (*itBS)->OnStateChange( nNewState );
		_ASSERT( SUCCEEDED( hr ) );
		// continue notifying other listeners
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateShifter::areListenersReceptive()
{
	StateListenerSetIt itBS;

	for (itBS = m_setListeners.begin(); itBS != m_setListeners.end(); itBS++)
	{
		if (!(*itBS)->AllowShift())
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateShifter::add( CStateListener *pListener)
{
	// ensure it is not already in there
	StateListenerSetIt itBS = m_setListeners.find( pListener );

	if (itBS == m_setListeners.end()) // if not found
	{
		// do not let this guy know the current state yet.
		// the actual derived class has probably not been constructed yet.
		m_setListeners.insert( StateListenerSet::value_type( pListener ));
		return TRUE;
	}
	
	// it should not already be in the set
	_ASSERT( 0 );
	return FALSE;	
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateShifter::remove( CStateListener *pListener )
{
	StateListenerSetIt itBS = m_setListeners.find( pListener );

	if (itBS != m_setListeners.end()) // if found
	{
		m_setListeners.erase( itBS );
		return TRUE;
	}

	// it should have been found
	_ASSERT( 0 );
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
// CStateMgr:
/////////////////////////////////////////////////////////////////////////
CStateMgr::CStateMgr( CControlSurface *pSurface ) :
	m_pSurface( pSurface ),
	m_bIsDirty( FALSE ),
	m_bIsLoading( FALSE ),
	m_dirtyState( pSurface )
{
}

/////////////////////////////////////////////////////////////////////////
CStateMgr::~CStateMgr()
{
	CCriticalSectionAuto lock( m_cs );

	if (!m_ScheduledStateChanges.empty())
	{
		// if the queue is not empty, kill the timer
		HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, 0 );
		_ASSERT( SUCCEEDED( hr ) );
	}
}

/////////////////////////////////////////////////////////////////////////
CStateShifter* CStateMgr::GetShifterFromID( DWORD dwStateID )
{
	StateMapIt it = m_mapStates.find( dwStateID );

	if (it != m_mapStates.end()) // if found
	{
		return (*it).second;
	}

	// if not found:
	return NULL;
}

/////////////////////////////////////////////////////////////////////////
void CStateMgr::PostStateChange( DWORD dwStateID, int nNewState )
{
	// PostStateChange may get called on any thread, but the state changes
	// themselves may only happen within any of the sonar threads.
	StateChangesIt it = m_DeferredStateChanges.find( dwStateID );
	if (it == m_DeferredStateChanges.end())
	{
		m_DeferredStateChanges.insert( StateChanges::value_type( dwStateID, nNewState ) );
		return;
	}
	else
	{
		(*it).second = nNewState;
	}
}

/////////////////////////////////////////////////////////////////////////
void CStateMgr::ScheduleStateChange(
		DWORD dwStateID,
		int nNewState,
		DWORD dwElapsed,
		BOOL bReplace /*=FALSE*/
	)
{
	BOOL bScheduleWasEmpty = FALSE;

	// scope of critical section
	{
		CCriticalSectionAuto lock( m_cs );

		// add an entry to the schedule of state changes
		DWORD dwNow = timeGetTime();
		DWORD dwWhen = dwNow + dwElapsed;

		// if the list had zero entries, set up the timer.
		if (m_ScheduledStateChanges.empty())
		{
			bScheduleWasEmpty = TRUE;
		}
		else
		{
			if (m_ScheduledStateChanges.top().m_dwID == dwStateID && bReplace)
			{
				m_ScheduledStateChanges.top().m_nState = nNewState;
				m_ScheduledStateChanges.top().m_dwWhen = dwWhen;
				return;
			}
		}

		// enqueue scheduled state change
		CTimedTransition ttNew;
		ttNew.m_dwID = dwStateID;
		ttNew.m_nState = nNewState;
		ttNew.m_dwWhen = dwWhen;

		m_ScheduledStateChanges.push( ttNew );
	}

	if (bScheduleWasEmpty)
	{
		HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, 10 );
		_ASSERT( SUCCEEDED( hr ) );
	}
}

/////////////////////////////////////////////////////////////////////////
void CStateMgr::onScheduleTimer()
{
	// deliver scheduled state changes

	// get critical section:
	CCriticalSectionAuto lock( m_cs );

	while (!m_ScheduledStateChanges.empty())
	{
		if (m_ScheduledStateChanges.top().IsDue())
		{
			// do not change the state directly.
			// For thread safety, post the state change and let it happen
			// in any of the SONAR threads (when MIDI arrives or when a
			// refresh is issued)
			CStateMgr::PostStateChange(
				m_ScheduledStateChanges.top().m_dwID,
				m_ScheduledStateChanges.top().m_nState
			);

			m_ScheduledStateChanges.pop();
		}
		else
			break;
	}

	if (m_ScheduledStateChanges.empty())
	{
		// if the queue is empty, kill the timer
		HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, 0 );
		_ASSERT( SUCCEEDED( hr ) );
	}
}

/////////////////////////////////////////////////////////////////////////
void CStateMgr::deliverPostedStateChanges()
{
	// deliver deferred state transitions
	StateChangesIt it;
	for (it = m_DeferredStateChanges.begin(); it != m_DeferredStateChanges.end(); it++)
	{
		DWORD dwState = it->first;
		CStateShifter *pShifter = GetShifterFromID( dwState );
		if (pShifter)
		{
			int nNewState = it->second;
			pShifter->SetNewState( nNewState );
		}
	}
	m_DeferredStateChanges.clear();
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateMgr::onStateCreated( CStateShifter *pShifter )
{
	_ASSERT( pShifter );
	DWORD dwStateID = pShifter->GetStateID();

	StateMapIt it = m_mapStates.find( dwStateID );
	if (it == m_mapStates.end()) // should not be found... unless ID collides
	{
		m_mapStates.insert( StateMap::value_type( dwStateID, pShifter ) );
		m_dirtyState.SubscribeToState( dwStateID );
		return TRUE;
	}
	else
	{
		// ID collided
		_ASSERT( 0 );
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
BOOL CStateMgr::onStateDestroyed( CStateShifter *pShifter )
{
	_ASSERT( pShifter );
	DWORD dwStateID = pShifter->GetStateID();

	StateMapIt it = m_mapStates.find( dwStateID );
	if (it != m_mapStates.end()) // should be found... 
	{
		m_mapStates.erase( it );
		m_dirtyState.UnsubscribeFromState( dwStateID );
		return TRUE;
	}
	else
	{
		// ID not found
		_ASSERT( 0 );
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateMgr::CDirtyState::OnStateChange( DWORD dwStateID, int nNewState )
{
	CStateShifter *pShifter = m_pSurface->GetStateMgr()->GetShifterFromID( dwStateID );
	if (pShifter == NULL)
	{
		_ASSERT( 0 ); // this should be impossible
		return E_UNEXPECTED;
	}

	if (pShifter->GetIsPersistent())
		m_pSurface->GetStateMgr()->SetDirty();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateMgr::IsDirty()
{
	if (m_bIsDirty)
		return S_OK;
	else
		return S_FALSE;
}

#define LOAD_HEADER 1 // comment this line out so that this build can load only the
/////////////////////////////////////////////////////////////////////////
HRESULT CStateMgr::Load( IStream *pstm, const CLSID *pClsid )
{
	CIsLoading loading( &m_bIsLoading );

	int nCount;
	ULONG cbRead = 0;

	if (pstm == NULL)
		return E_POINTER;

	if (pClsid == NULL)
		return E_POINTER;

#ifdef LOAD_HEADER
	// begin header
	// read the signature: should match our own clsid
	CLSID clsidSignature;
	pstm->Read( &clsidSignature, sizeof( CLSID ), &cbRead );
	if (cbRead != sizeof( CLSID ))
		return E_UNEXPECTED;

	if (clsidSignature != *pClsid)
		return E_UNEXPECTED;

	// read file version number:
	DWORD dwGenSurfaceVersion; // arbitrary first version
	pstm->Read( &dwGenSurfaceVersion, sizeof( dwGenSurfaceVersion ), &cbRead );
	if (cbRead != sizeof( dwGenSurfaceVersion ))
		return E_UNEXPECTED;

	if (dwGenSurfaceVersion != 1)
		return E_UNEXPECTED;
	// read the size of this file:
	// but first find out where are we right now:
	ULARGE_INTEGER libFileSizeOffset;
	LARGE_INTEGER libZero;
	libZero.QuadPart = 0;
	HRESULT hr = pstm->Seek( libZero, STREAM_SEEK_CUR, &libFileSizeOffset );
	if (FAILED( hr ))
		return hr;

	// Read the file size, keep it around until the end for a comparison
	DWORD dwFileSize;
	pstm->Read( &dwFileSize, sizeof( dwFileSize ), &cbRead );
	if (cbRead != sizeof( dwFileSize ))
		return E_UNEXPECTED;
	// header finished.
#endif // LOAD_HEADER

	pstm->Read( &nCount, sizeof( nCount ), &cbRead );
	if (cbRead != sizeof( nCount ))
		return E_UNEXPECTED;

	for (int ix = 0; ix < nCount; ix++)
	{
		// this relies on the assumption that states have already been created.
		DWORD dwStateID = 0;
		pstm->Read( &dwStateID, sizeof( DWORD ), &cbRead );
		if (cbRead != sizeof( DWORD ))
		{
			_ASSERT( 0 ); // could not read ID
			return E_UNEXPECTED;
		}

		DWORD dwState = 0;
		pstm->Read( &dwState, sizeof( DWORD ), &cbRead );
		if (cbRead != sizeof( DWORD ))
		{
			_ASSERT( 0 ); // could not read state
			return E_UNEXPECTED;
		}

		StateMapIt it = m_mapStates.find( dwStateID );
		if (it != m_mapStates.end()) // if found:
		{
			CStateShifter *pState = (*it).second;
			if (pState->GetIsPersistent())
			{
				HRESULT hr = pState->SetNewState( dwState );
				_ASSERT( SUCCEEDED( hr ));
			}
		}
		// otherwise, don't worry, this is a deprecated state.
	}

#ifdef LOAD_HEADER
	// begin footer
	// check the file size:
	// find out where are we right now:
	ULARGE_INTEGER libEndOfFileOffset;
	hr = pstm->Seek( libZero, STREAM_SEEK_CUR, &libEndOfFileOffset );
	if (FAILED( hr ))
		return hr;


	DWORD dwActualFileSize = (DWORD)(libEndOfFileOffset.QuadPart - libFileSizeOffset.QuadPart);
	if (dwActualFileSize != dwFileSize)
		return E_UNEXPECTED;
	// footer finished
#endif // LOAD_HEADER

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateMgr::Save( IStream *pstm, BOOL fClearDirty, const CLSID *pClsid )
{
	if (pstm == NULL)
		return E_POINTER;

	if (pClsid == NULL)
		return E_POINTER;

	ULONG cbWritten = 0;

	// begin header
	// write a signature: (use our own clsid for this)
	pstm->Write( pClsid, sizeof( CLSID ), &cbWritten );
	if (cbWritten != sizeof( CLSID ))
		return E_UNEXPECTED;

	// write file version number:
	DWORD dwStateMgrVersion = 1; // arbitrary first version
	pstm->Write( &dwStateMgrVersion, sizeof( DWORD ), &cbWritten );
	if (cbWritten != sizeof( DWORD ))
		return E_UNEXPECTED;

	// write the size of this file:
	// leave this gap open so we can get back to it and write the
	// file size once finished.

	// but first find out where are we right now:
	ULARGE_INTEGER libFileSizeOffset;
	LARGE_INTEGER libZero;
	libZero.QuadPart = 0;
	HRESULT hr = pstm->Seek( libZero, STREAM_SEEK_CUR, &libFileSizeOffset );
	if (FAILED( hr ))
		return hr;

	// write a fake file size for now
	DWORD dwFileSizeFoo = 0;
	pstm->Write( &dwFileSizeFoo, sizeof( dwFileSizeFoo ), &cbWritten );
	if (cbWritten != sizeof( dwFileSizeFoo ))
		return E_UNEXPECTED;
	// header finished.

	// write the number of states
	int nCount = (int)m_mapStates.size();

	pstm->Write( &nCount, sizeof( nCount ), &cbWritten );
	if (cbWritten != sizeof( nCount ))
		return E_UNEXPECTED;

	StateMapIt it;
	for (it = m_mapStates.begin(); it != m_mapStates.end(); it++)
	{
		// write each state ID and its state.
		DWORD dwStateID = (*it).first;
		CStateShifter *pState = (*it).second;

		DWORD dwState = pState->GetCurrentState();

		pstm->Write( &dwStateID, sizeof(DWORD), &cbWritten );
		if (cbWritten != sizeof(DWORD))
			return E_UNEXPECTED;

		pstm->Write( &dwState, sizeof(DWORD), &cbWritten );
		if (cbWritten != sizeof(DWORD))
			return E_UNEXPECTED;
	}

	// begin footer:
	// find out where are we right now:
	ULARGE_INTEGER libEndOfFileOffset;
	hr = pstm->Seek( libZero, STREAM_SEEK_CUR, &libEndOfFileOffset );
	if (FAILED( hr ))
		return hr;

	// jump to the position where the file size should be
	LARGE_INTEGER libJumpBack;
	libJumpBack.QuadPart = libFileSizeOffset.QuadPart - libEndOfFileOffset.QuadPart;

	hr = pstm->Seek( libJumpBack, STREAM_SEEK_CUR, NULL );
	if (FAILED( hr ))
		return hr;

	// write the actual file size now
	dwFileSizeFoo = (DWORD)(libEndOfFileOffset.QuadPart - libFileSizeOffset.QuadPart);
	pstm->Write( &dwFileSizeFoo, sizeof( dwFileSizeFoo ), &cbWritten );
	if (cbWritten != sizeof( dwFileSizeFoo ))
		return E_UNEXPECTED;

	ULARGE_INTEGER libPostFileSizeOffset;
	hr = pstm->Seek( libZero, STREAM_SEEK_CUR, &libPostFileSizeOffset );
	if (FAILED( hr ))
		return hr;

	LARGE_INTEGER libJumpForth;
	libJumpForth.QuadPart = libEndOfFileOffset.QuadPart - libPostFileSizeOffset.QuadPart;

	// return to position at the end of file.
	hr = pstm->Seek( libJumpForth, STREAM_SEEK_CUR, NULL );
	if (FAILED( hr ))
		return hr;
	// footer finished.

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
// CSonarContainerBankShifter:
/////////////////////////////////////////////////////////////////////////
CSonarContainerBankShifter::CSonarContainerBankShifter(
	CControlSurface *pSurface,
	DWORD dwStateID,
	int nBankWidth,
	DWORD dwInitialState, /*= 0*/
	WORD	wBankID, /*= 0*/
	WORD	wSlaveToBankID, /* = 0xffff */
	BOOL	bUseRangeBasedMaximums /* = FALSE*/
	) : 
	m_pSurface( pSurface ),
	CStateShifter( pSurface, MAKELONG( dwStateID, wBankID ), dwInitialState ),
	CHostNotifyListener( REFRESH_F_OTHER, pSurface ), // TODO: confirm this is the correct refresh notification
	m_wBankID( wBankID ),
	m_wSlaveToBankID( wSlaveToBankID ),
	m_bAligned( FALSE ),
	m_nBankWidth( nBankWidth ),
	m_bUseRangeBasedMaximums( bUseRangeBasedMaximums )
{
	if (m_wSlaveToBankID == 0xffff)
		m_wSlaveToBankID = wBankID;

	_ASSERT( m_nBankWidth > 0 );
	if (GetStateID() == MAKELONG( stBaseEffect, m_wBankID ) )
	{
		HRESULT hr = SubscribeToState( MAKELONG( stCurrentTrack, m_wSlaveToBankID ) );
		_ASSERT( SUCCEEDED( hr ) ); // there has to be an instance of a CSonarContainerBankShifter
		// that has as its ID stCurrentTrack, so we can deal with the effects on "THAT" track.

	}

	if (GetStateID() == MAKELONG( stBaseEffectParam, m_wBankID ) )
	{
		HRESULT hr = SubscribeToState( MAKELONG( stCurrentTrack, m_wSlaveToBankID ) );
		_ASSERT( SUCCEEDED( hr ) ); // there has to be an instance of a CSonarContainerBankShifter
		// that has as its ID stCurrentTrack, so we can deal with the effects on "THAT" track.

		hr = SubscribeToState( MAKELONG( stCurrentMain, m_wSlaveToBankID ) );
		_ASSERT( SUCCEEDED( hr ) ); // there has to be an instance of a CSonarContainerBankShifter
		// that has as its ID stCurrentMain, so we can deal with the effects on "THAT" main.

		hr = SubscribeToState( MAKELONG( stCurrentBus, m_wSlaveToBankID ) );
		_ASSERT( SUCCEEDED( hr ) ); // there has to be an instance of a CSonarContainerBankShifter
		// that has as its ID stCurrentAux, so we can deal with the effects on "THAT" aux.

		if (!m_bUseRangeBasedMaximums)
		{
			hr = SubscribeToState( MAKELONG( stCurrentEffect, m_wSlaveToBankID ) );
			_ASSERT( SUCCEEDED( hr ) ); // Likewise (see above) there has to be a
			// CSonarContainerBankShifter instance that has as its ID the stCurrentEffect.
			// So that we can deal with the current parameter on "THAT" effect.
		}
	}

	SubscribeToState( MAKELONG( stContainerClass, m_wSlaveToBankID ) );
}

/////////////////////////////////////////////////////////////////////////
void CSonarContainerBankShifter::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	// we can thin this one
	if ( m_nHostNotifyCount % 2 != 0 )
		return;

	if (m_pSurface->GetStateMgr()->IsLoading())
		return;

	DWORD dwMatchID;
	switch (GetStateForID( MAKELONG( stContainerClass, m_wSlaveToBankID )))
	{
	case ccTracks:
		dwMatchID = stBaseTrack;
		break;

	case ccBus:
		dwMatchID = stBaseBus;
		break;

	case ccMains:
		dwMatchID = stBaseMain;
		break;

	default:
		_ASSERT ( 0 ); // unknown container kind
	}

	if (MAKELONG( dwMatchID, m_wBankID ) == GetStateID() && dwCookie == 0)
	{
		// containers may have changed. revalidate:
		m_pSurface->GetStateMgr()->PostStateChange( GetStateID(), GetCurrentState() );
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CSonarContainerBankShifter::SetNewState( int nNewState )
{
	// When loading, sonar may not have yet defined its containers,
	// and a validation of them may fail.
	// Instead, simply set the state variable without validating it
	// and wait until the next call to OnHostNotify to validate
	if (m_pSurface->GetStateMgr()->IsLoading())
	{
		m_nCurrentState = nNewState;
		return S_FALSE;
	}
	else
	{
		maybeUpdateSonarStatus( nNewState );
		HRESULT hr = CStateShifter::SetNewState( nNewState );
		m_pSurface->UpdateHostContext();
		return hr;
	}
}

/////////////////////////////////////////////////////////////////////////
void CSonarContainerBankShifter::maybeUpdateSonarStatus( int nNewState, BOOL bInvalidate /*=FALSE*/ )
{
	if (m_nCurrentState == nNewState && bInvalidate == FALSE)
		return;

	ISonarProject* pSonarProject = NULL;
	HRESULT hr = m_pSurface->GetSonarProject( &pSonarProject );
	if (FAILED( hr ))
		return;	// nothing we can do

	hr = pSonarProject->OnNewStatus( 0 );

	_ASSERT( SUCCEEDED ( hr ) ) ;
	pSonarProject->Release();
}

/////////////////////////////////////////////////////////////////////////
int CSonarContainerBankShifter::GetMaxState()
{
	DWORD dwMax = 0;

	// determine what kind of strip goes with the current state for our parent:
	// (our parent should be the stContainerClass)
	SONAR_MIXER_STRIP stripKind;
	int nCurrentContainer = 0;
	int nBaseContainer = 0;
	DWORD nContainerCount = 0;
	CStateShifter *pBaseContainerShifter = NULL;

	// get a pointer to the sonar mixer
	ISonarMixer *pSonarMixer = NULL;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return E_UNEXPECTED;


	switch (GetStateID() & 0xFFFF)
	{
	case stBaseEffect:
	case stCurrentEffect:
	case stBaseEffectParam:
	case stCurrentEffectParam:
		switch (GetStateForID( MAKELONG( stContainerClass, m_wSlaveToBankID ) ) )
		{
		case ccTracks:
			stripKind = MIX_STRIP_TRACK;
			nCurrentContainer = GetStateForID( MAKELONG( stCurrentTrack, m_wSlaveToBankID ) );
			nBaseContainer = GetStateForID( MAKELONG( stBaseTrack, m_wSlaveToBankID ) );
			pBaseContainerShifter = m_pSurface->GetStateMgr()->GetShifterFromID( MAKELONG( stBaseTrack, m_wSlaveToBankID ) );
			break;

		case ccBus:
			stripKind = MIX_STRIP_BUS;
			nCurrentContainer = GetStateForID( MAKELONG( stCurrentBus, m_wSlaveToBankID ) );
			nBaseContainer = GetStateForID( MAKELONG( stBaseBus, m_wSlaveToBankID ) );
			pBaseContainerShifter = m_pSurface->GetStateMgr()->GetShifterFromID( MAKELONG( stBaseBus, m_wSlaveToBankID ) );
			break;

		case ccMains:
			stripKind = MIX_STRIP_MASTER;
			nCurrentContainer = GetStateForID( MAKELONG( stCurrentMain, m_wSlaveToBankID ) );
			nBaseContainer = GetStateForID( MAKELONG( stBaseMain, m_wSlaveToBankID ) );
			pBaseContainerShifter = m_pSurface->GetStateMgr()->GetShifterFromID( MAKELONG( stBaseMain, m_wSlaveToBankID ) );
			break;

		default:
			_ASSERT ( 0 ); // unknown container kind
		}

		pSonarMixer->GetMixStripCount( stripKind, &nContainerCount );
	}

	switch (GetStateID() & 0xFFFF)
	{
	case stBaseTrack:
	case stCurrentTrack:
		pSonarMixer->GetMixStripCount( MIX_STRIP_TRACK, &dwMax );		
		break;

	case stBaseBus:
	case stCurrentBus:
		pSonarMixer->GetMixStripCount( MIX_STRIP_BUS, &dwMax );
		break;

	case stBaseMain:
	case stCurrentMain:
		pSonarMixer->GetMixStripCount( MIX_STRIP_MASTER, &dwMax );
		break;

	case stBaseEffect:
	case stCurrentEffect:
		{
			float fMax = 0;

			if (m_bUseRangeBasedMaximums)
			{
				for (int ix = 0; ix < (int)nContainerCount; ix++)
				{
					float fTemp = 0;
					pSonarMixer->GetMixParam(
						stripKind,
						ix,
						MIX_PARAM_FX_COUNT,
						0,
						&fTemp
					);

					if (fTemp > fMax)
						fMax = fTemp;
				}
			}
			else
			{
				pSonarMixer->GetMixParam(
					stripKind,
					nCurrentContainer,
					MIX_PARAM_FX_COUNT,
					0,
					&fMax
				);
			}
			dwMax = (DWORD)fMax;
		}
		break;

	case stBaseEffectParam:
	case stCurrentEffectParam:
		{
			float fMax = 0;

			if (m_bUseRangeBasedMaximums)
			{
				for (int ix = 0; ix < (int)nContainerCount; ix++)
				{
					float fTemp = 0;
					pSonarMixer->GetMixParam(
						stripKind,
						ix,
						MIX_PARAM_FX_COUNT,
						0,
						&fTemp
					);

					int nFxCount = (int)fTemp;
					for (int jx = 0; jx < nFxCount; jx++)
					{
						float fTemp = 0;
						pSonarMixer->GetMixParam(
							stripKind,
							ix,
							MIX_PARAM_FX_PARAM_COUNT,
							jx,
							&fTemp
						);
						if (fTemp > fMax)
							fMax = fTemp;
					}
				}
			}
			else
			{
				// if there are no effects do not bother to ask the count of parameters
				pSonarMixer->GetMixParam(
					stripKind,
					nCurrentContainer,
					MIX_PARAM_FX_COUNT,
					0,
					&fMax
				);
				if ((DWORD)fMax == 0)
				{
					dwMax = 0;
				}
				else
				{
					pSonarMixer->GetMixParam(
						stripKind,
						nCurrentContainer,
						MIX_PARAM_FX_PARAM_COUNT,
						GetStateForID( MAKELONG( stCurrentEffect, m_wSlaveToBankID ) ),
						&fMax
					);
				}
			}

			dwMax = (DWORD)fMax;
		}
		break;

	default:
		_ASSERT( 0 ); // unknown state for stContainerMode
	}

	pSonarMixer->Release();

	switch (GetStateID() & 0xFFFF)
	{
	case stBaseTrack:
	case stBaseBus:
	case stBaseMain:
	case stBaseEffect:
	case stBaseEffectParam:
		{
			if (m_bAligned)
			{
				// use bank width alignment
				_ASSERT( m_nBankWidth > 0 );
				dwMax = ( (dwMax - 1) / m_nBankWidth ) * m_nBankWidth;
			}
			else
				dwMax = max( (int)0, (int)(dwMax - m_nBankWidth) );
		}
		break;

	case stCurrentTrack:
	case stCurrentBus:
	case stCurrentMain:
	case stCurrentEffect:
	case stCurrentEffectParam:
		dwMax = max( (int)0, (int)(dwMax - 1) );
		break;

	default:
		_ASSERT( 0 ); // unknown ID for a CSonarContainerBankShifter
	}

	return dwMax;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CSonarContainerBankShifter::OnStateChange( DWORD dwStateID, int nNewState )
{
	if (dwStateID == MAKELONG( stContainerClass, m_wSlaveToBankID))
	{
		// containers may have changed. revalidate:
		m_pSurface->GetStateMgr()->PostStateChange( GetStateID(), GetCurrentState() );
		maybeUpdateSonarStatus( nNewState, TRUE );
	}

	return CStateShifter::OnStateChange( dwStateID, nNewState );
}

/////////////////////////////////////////////////////////////////////////
int CSonarContainerBankShifter::GetMinState()
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////
// CStateShifterMux:
/////////////////////////////////////////////////////////////////////////
CStateShifterMux::CStateShifterMux( CControlSurface *pSurface, DWORD dwStateID ) :
	CStateListener( pSurface, dwStateID )
{
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifterMux::AddShifter(
		CStateShifter* pShifter,
		DWORD dwStateEnable 
	)
{
	StateMapIt it = m_mapStates.find( dwStateEnable );
	if (it == m_mapStates.end())
	{
		m_mapStates.insert( StateMap::value_type( dwStateEnable, pShifter ) );
		return S_OK;
	}
	else
		return E_UNEXPECTED;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CStateShifterMux::OnStateChange( int nNewState )
{
	for (StateMapIt it = m_mapStates.begin(); it != m_mapStates.end(); it++)
	{
		if ((*it).first == (DWORD)nNewState)
		{
			(*it).second->SetIsEnabled( TRUE );
		}
		else
		{
			(*it).second->SetIsEnabled( FALSE );
		}
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
// CMixStrip:
/////////////////////////////////////////////////////////////////////////
CMixStrip::CMixStrip(
	DWORD dwHWStripNum,
	CControlSurface *pSurface
	):
	m_pSurface( pSurface ),
	CHostNotifyListener( REFRESH_F_MIXER, pSurface ),
	CMultiStateListener( pSurface ),
	m_pMsgScribble( NULL ),
	m_bIsScribbleEnabled( TRUE ),
	m_mixerStrip( MIX_STRIP_TRACK ),
	m_dwStripNum( dwHWStripNum )
{
	m_dwHWStripNum = dwHWStripNum;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixStrip::SetContainer( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	m_mixerStrip = mixerStrip;
	m_dwStripNum = dwStripNum;

	refreshScribble();
	return remapStrip();
}

/////////////////////////////////////////////////////////////////////////
BOOL CMixStrip::IsAnyParamTouched()
{
	for (ParamSetIt itPM = m_setParams.begin(); itPM != m_setParams.end(); itPM++)
	{
		CBaseMixParam *pParam = (*itPM); // handy ref
		if (pParam->IsTouched())
			return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixStrip::remapStrip()
{
	HRESULT hr = S_OK;

	for (ParamSetIt itPM = m_setParams.begin(); itPM != m_setParams.end(); itPM++)
	{
		CBaseMixParam *pParam = (*itPM); // handy ref
		hr = remapStripParam( pParam );
		if (hr != S_OK)
		{
			HRESULT hrLocal = pParam->SetContainer( m_mixerStrip, m_dwStripNum );
			if (hrLocal != S_OK)
			{
				// out of range
				hr = S_FALSE;
			}
		}
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMixStrip::add( CBaseMixParam *pParam )
{
	BOOL bRet = FALSE;

	ParamSetIt itFindParam = m_setParams.find( pParam );
	if (itFindParam == m_setParams.end()) // if it is unique
	{
		m_setParams.insert(ParamSet::value_type( pParam ) );
		bRet = TRUE;
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////
void CMixStrip::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	// interlace the scribble updates based on the strip number even/odd
	if ( m_nHostNotifyCount % 2 == m_dwHWStripNum % 2 )
		refreshScribble();
}

/////////////////////////////////////////////////////////////////////////
void CMixStrip::refreshScribble()
{
	if (m_pMsgScribble != NULL && m_bIsScribbleEnabled)
	{
		// since we have a scribble message (for strip identification)
		// we will send it.
		char szStripName[64];
		DWORD dwLength = 64;

		HRESULT hr = getScribbleText( szStripName, &dwLength );
		if (hr == S_OK)
			m_pMsgScribble->SendText( (DWORD)strlen( szStripName ), szStripName );
		else
			m_pMsgScribble->SendText( 0, "" );
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixStrip::getScribbleText( LPSTR pszName, DWORD* pdwLen )
{
	ISonarMixer *pSonarMixer = NULL;

	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
	{
		_ASSERT( 0 ); // could not get the sonar mixer
		return E_UNEXPECTED;
	}

	pSonarMixer->Release();

	return pSonarMixer->GetMixStripName( m_mixerStrip, m_dwStripNum, pszName, pdwLen );
}

/////////////////////////////////////////////////////////////////////////
// CTransport
/////////////////////////////////////////////////////////////////////////
CTransport::CTransport( CControlSurface *pSurface ):
	CHostNotifyListener( REFRESH_F_TRANSPORT, pSurface ),
	m_pSurface( pSurface ),
	m_pShuttleState( NULL ),
	m_dwTimerCounter( 0 ),
	m_tState( tsInvalid ),
	m_pListener( NULL ),
	m_bIsPlaying( FALSE ),
	m_bIsEnabled( TRUE )
{
	// initialize the jog time increment to a default value
	m_JogTimeIncrement.timeFormat = TF_MBT;
	m_JogTimeIncrement.mbt.nMeas = 1;
	m_JogTimeIncrement.mbt.nBeat = 0;
	m_JogTimeIncrement.mbt.nTick = 0;
}

/////////////////////////////////////////////////////////////////////////
CTransport::~CTransport()
{
	ClearAll();
}

/////////////////////////////////////////////////////////////////////////
void CTransport::ClearAll()
{
	HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, 0 );

	// cleanup and unregister from listening
	m_trigPlay.SetMsg( NULL, 0, this );
	m_trigRecord.SetMsg( NULL, 0, this );
	m_trigRecAuto.SetMsg( NULL, 0, this );
	m_trigStop.SetMsg( NULL, 0, this );
	m_trigGotoTop.SetMsg( NULL, 0, this );
	m_trigGotoEnd.SetMsg( NULL, 0, this );
	m_trigJogFwd.SetMsg( NULL, 0, this );
	m_trigJogBck.SetMsg( NULL, 0, this );
	m_trigPanic.SetMsg( NULL, 0, this );
	m_trigRunAudio.SetMsg( NULL, 0, this );
	m_trigRejectLoopTake.SetMsg( NULL, 0, this );
	m_trigFetchLoopFrom.SetMsg( NULL, 0, this );
	m_trigFetchLoopThru.SetMsg( NULL, 0, this );
	m_trigFetchPunchFrom.SetMsg( NULL, 0, this );
	m_trigFetchPunchThru.SetMsg( NULL, 0, this );
	m_trigPrevMarker.SetMsg( NULL, 0, this );
	m_trigNextMarker.SetMsg( NULL, 0, this );
	m_trigScrubIn.SetMsg( NULL, 0, this );
	m_trigScrubOut.SetMsg( NULL, 0, this );

	ClearShuttle();
}

/////////////////////////////////////////////////////////////////////////
void CTransport::ClearShuttle()
{
	m_pShuttleState = NULL;

	for (ShuttleMapIt it = m_mapShuttle.begin(); it != m_mapShuttle.end(); it++)
	{
		CMidiTrigger trig = (*it).first;
		trig.SetMsg( NULL, 0, this );

		delete (*it).second;
	}

	m_mapShuttle.clear();
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetPlayMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigPlay.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetRecordMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigRecord.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetWriteMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigRecAuto.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetStopMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigStop.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetPanicMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigPanic.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetRunAudioMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigRunAudio.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetRejectLoopTakeMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigRejectLoopTake.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetFetchPunchFromMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigFetchPunchFrom.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetFetchPunchThruMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigFetchPunchThru.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetFetchLoopFromMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigFetchLoopFrom.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetFetchLoopThruMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigFetchLoopThru.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetGotoTopMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigGotoTop.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetGotoEndMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigGotoEnd.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetJogFwdMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigJogFwd.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetJogBckMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigJogBck.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetNextMarkerMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigNextMarker.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetPrevMarkerMsg( CMidiMsg *pInMsg, DWORD dwInVal /* = VAL_ANY */ )
{
	m_trigPrevMarker.SetMsg( pInMsg, dwInVal, this );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::SetScrubMsg( CMidiMsg *pInMsg, DWORD dwInVal, CMidiMsg *pOutMsg, DWORD dwOutVal )
{
	m_trigScrubIn.SetMsg( pInMsg, dwInVal, this );
	m_trigScrubOut.SetMsg( pOutMsg, dwOutVal, this );
}
							  
/////////////////////////////////////////////////////////////////////////
HRESULT CTransport::AddShuttle(
		DWORD dwPeriod, MFX_TIME tAmount, BOOL bNegative, DWORD dwWait,
		CMidiMsg *pMsgPress, DWORD dwValPress,
		CMidiMsg *pMsgRelease, DWORD dwValRelease,
		CONDITION_HANDLE *phCondition /* = NULL */
	)
{
	m_pShuttleState = NULL;

	_ASSERT( pMsgPress );
	_ASSERT( pMsgRelease );
	_ASSERT( dwPeriod > 0 );

	CMidiTrigger keyPrs;
	keyPrs.SetMsg( pMsgPress, dwValPress, this );

	CMidiTrigger keyRel;
	keyRel.SetMsg( pMsgRelease, dwValRelease, this );

	ShuttleMapIt itSMPrs;
	itSMPrs = m_mapShuttle.find( keyPrs );

	ShuttleMapIt itSMRel;
	itSMRel = m_mapShuttle.find( keyRel );

	if (itSMPrs == m_mapShuttle.end() && itSMRel == m_mapShuttle.end())
	{
		// insert it...
		CShuttleVal *pVal = new CShuttleVal();
		pVal->bNegative = bNegative;
		pVal->dwPeriod = dwPeriod;
		pVal->tAmount = tAmount;
		pVal->dwWait = dwWait;
		pVal->eAction = CShuttleVal::Pressed;
		m_mapShuttle.insert( ShuttleMap::value_type( keyPrs, pVal ) );
		if (phCondition)
		{
			*phCondition = pVal;
		}

		pVal = new CShuttleVal();
		pVal->bNegative = bNegative;
		pVal->dwPeriod = dwPeriod;
		pVal->tAmount = tAmount;
		pVal->dwWait = dwWait;
		pVal->eAction = CShuttleVal::Released;
		m_mapShuttle.insert( ShuttleMap::value_type( keyRel, pVal ) );
		return S_OK;
	}
	else
		// it was already there
		return E_INVALIDARG;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CTransport::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	if (m_bIsEnabled == FALSE) // if disabled
	{
		return S_FALSE;
	}

	if (m_sccGlobal.Conjunction( m_pSurface ) == FALSE)
	{
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	CMidiTrigger key;
	key.m_dwVal = dwVal;
	key.m_pMsg = pMsg;

	ShuttleMapIt it = m_mapShuttle.find( key );
	if (it != m_mapShuttle.end()) // if found
	{
		BOOL bEvaluateShuttle = TRUE;
		// update shuttle state
		if ((*it).second->eAction == CShuttleVal::Pressed)
		{
			// evaluate the condition only for "pressed"
			// release must always be unconditional to prevent
			// "stuck" states
			if ((*it).second->Conjunction( m_pSurface ) == FALSE)
				bEvaluateShuttle = FALSE;
		}

		if (bEvaluateShuttle)
		{
			m_pShuttleState = (*it).second;

			_ASSERT( m_pShuttleState );
			if (m_pShuttleState == NULL)
				return E_UNEXPECTED;

			if (m_pShuttleState->eAction == CShuttleVal::Pressed)
			{
				// reset timer
				m_dwTimerCounter = 0;

				// initial button push:
				shuttleAdvance();
			}

			// let the timer trigger the auto repeats
			WORD wPeriod = 0;
			if (m_pShuttleState->eAction == CShuttleVal::Pressed)
				wPeriod = 10;

			hr = m_pSurface->GetTimer()->SetPeriod( this, wPeriod );
			_ASSERT( SUCCEEDED( hr ) );
			hr = S_OK;
		}
	}

	// evaluate transport operations other than shuttle
	ISonarTransport *pSonarTransport = NULL;
	hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		return E_UNEXPECTED;
	}

	ISonarCommands *pSonarCommands = 0;
	hr = m_pSurface->GetSonarCommands( &pSonarCommands );
	if (FAILED( hr ))
	{
		return E_UNEXPECTED;
	}

	if (m_trigPlay.Test( pMsg, dwVal ) && m_sccPlay.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// play
		hr = pSonarTransport->SetTransportState(
			TRANSPORT_STATE_PLAY,
			TRUE
			);
	}
	else if (m_trigStop.Test( pMsg, dwVal ) && m_sccStop.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// stop
		hr = pSonarTransport->SetTransportState(
			TRANSPORT_STATE_PLAY,
			FALSE
			);
	}
	else if (m_trigPanic.Test( pMsg, dwVal ) && m_sccPanic.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// panic
		hr = pSonarCommands->DoCommand( CMD_REALTIME_PANIC );
	}
	else if (m_trigRunAudio.Test( pMsg, dwVal ) && m_sccRunAudio.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// panic
		hr = pSonarCommands->DoCommand( CMD_REALTIME_AUDIO_RUNNING );
	}
	else if (m_trigRejectLoopTake.Test( pMsg, dwVal ) && m_sccRejectLoopTake.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// panic
		hr = pSonarCommands->DoCommand( CMD_REALTIME_REJECT_LOOP_TAKE );
	}
	else if (m_trigFetchLoopFrom.Test( pMsg, dwVal ) && m_sccFetchLoopFrom.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// FetchLoopFrom
		MFX_TIME tWhen;
		tWhen.timeFormat = TF_SECONDS;
		pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );
		hr = pSonarTransport->SetTransportTime( TRANSPORT_TIME_LOOP_IN, &tWhen );
	}
	else if (m_trigFetchLoopThru.Test( pMsg, dwVal ) && m_sccFetchLoopThru.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// FetchLoopThru
		MFX_TIME tWhen;
		tWhen.timeFormat = TF_SECONDS;
		pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );
		hr = pSonarTransport->SetTransportTime( TRANSPORT_TIME_LOOP_OUT, &tWhen );
	}
	else if (m_trigFetchPunchFrom.Test( pMsg, dwVal ) && m_sccFetchPunchFrom.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// FetchPunchFrom
		MFX_TIME tWhen;
		tWhen.timeFormat = TF_SECONDS;
		pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );
		hr = pSonarTransport->SetTransportTime( TRANSPORT_TIME_PUNCH_IN, &tWhen );
	}
	else if (m_trigFetchPunchThru.Test( pMsg, dwVal ) && m_sccFetchPunchThru.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		// FetchPunchThru
		MFX_TIME tWhen;
		tWhen.timeFormat = TF_SECONDS;
		pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );
		hr = pSonarTransport->SetTransportTime( TRANSPORT_TIME_PUNCH_OUT, &tWhen );
	}
	else if (m_trigRecord.Test( pMsg, dwVal ) && m_sccRecord.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		hr = pSonarTransport->SetTransportState( TRANSPORT_STATE_REC, TRUE);
	}
	else if (m_trigRecAuto.Test( pMsg, dwVal ) && m_sccRecAuto.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		BOOL b = FALSE;
		hr = pSonarTransport->GetTransportState(TRANSPORT_STATE_REC_AUTOMATION, &b);
		if ( SUCCEEDED( hr ) )
			hr = pSonarTransport->SetTransportState(TRANSPORT_STATE_REC_AUTOMATION, !b);
	}
	else if (m_trigJogFwd.Test( pMsg, dwVal ) && m_sccJogFwd.Conjunction( m_pSurface ))
	{
		Jog( 1 );
	}
	else if (m_trigJogBck.Test( pMsg, dwVal ) && m_sccJogBck.Conjunction( m_pSurface ))
	{
		Jog( -1 );
	}
	else if (m_trigPrevMarker.Test( pMsg, dwVal ) && m_sccPrevMarker.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		hr = pSonarCommands->DoCommand( CMD_MARKER_PREVIOUS );
	}
	else if (m_trigNextMarker.Test( pMsg, dwVal ) && m_sccNextMarker.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		hr = pSonarCommands->DoCommand( CMD_MARKER_NEXT );
	}
	else if (m_trigGotoTop.Test( pMsg, dwVal ) && m_sccGotoTop.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		hr = pSonarCommands->DoCommand( CMD_GOTO_START );
	}
	else if (m_trigGotoEnd.Test( pMsg, dwVal ) && m_sccGotoEnd.Conjunction( m_pSurface ))
	{
		maybeResetShuttle();

		hr = pSonarCommands->DoCommand( CMD_GOTO_END );
	}
	else if (m_trigScrubIn.Test( pMsg, dwVal ) && m_sccScrub.Conjunction ( m_pSurface ))
	{
		// scrub in
		maybeResetShuttle();

		hr = pSonarTransport->SetTransportState(
			TRANSPORT_STATE_SCRUB,
			TRUE
			);
	}
	else if (m_trigScrubOut.Test( pMsg, dwVal )) // out from a pushbutton is always unconditional
	{
		// scrub out
		maybeResetShuttle();

		hr = pSonarTransport->SetTransportState(
			TRANSPORT_STATE_SCRUB,
			FALSE
			);
	}
	else
		hr = S_FALSE;

	pSonarCommands->Release();
	pSonarTransport->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////////////
void CTransport::Jog( int nIncrements )
{
	if (nIncrements == 0)
		return;

	maybeResetShuttle();

	int nCount = abs( nIncrements );
	for (int ix = 0; ix < nCount; ix++)
	{
		timeAdvance( m_JogTimeIncrement, (nIncrements < 0) ); // (false forward true back)
	}
}

/////////////////////////////////////////////////////////////////////////
void CTransport::maybeResetShuttle()
{
	m_pShuttleState = NULL;
	HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, 0 );
	_ASSERT( SUCCEEDED( hr ) );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::onShuttleTimer()
{
	if (m_pShuttleState == NULL)
		return;

	if (m_pShuttleState->eAction == CShuttleVal::Released)
	{
		// nothing to do
		return;
	}

	if (m_pShuttleState->eAction == CShuttleVal::Pressed)
	{
		m_dwTimerCounter ++;

		// subsequent "auto repeats"
		if (m_dwTimerCounter >= m_pShuttleState->dwWait)
		{
			_ASSERT( m_pShuttleState->dwPeriod > 0 );
			// every time we cycle by a period, nudge the now time
			if ( ( ( m_dwTimerCounter - m_pShuttleState->dwWait ) % m_pShuttleState->dwPeriod ) == 0 )
			{
				shuttleAdvance();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
void CTransport::shuttleAdvance()
{
	if (m_pShuttleState == NULL)
		return;

	timeAdvance( m_pShuttleState->tAmount, m_pShuttleState->bNegative );
}

/////////////////////////////////////////////////////////////////////////
void CTransport::timeAdvance( MFX_TIME timeIncrement, BOOL bNegative )
{
	// advance by the specified amount in the specified direction
	// the amount and direction are specified in the shuttle's state.
	ISonarTransport *pSonarTransport = NULL;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	_ASSERT( SUCCEEDED( hr ) );

	MFX_TIME timeCurrent;
	timeCurrent.timeFormat = timeIncrement.timeFormat;

	hr = pSonarTransport->GetTransportTime(
		TRANSPORT_TIME_CURSOR,
		&timeCurrent
	);
	_ASSERT( SUCCEEDED( hr ) );

	switch (timeIncrement.timeFormat)
	{
	case TF_SECONDS:
		if (bNegative)
			timeCurrent.dSeconds -= timeIncrement.dSeconds;
		else
			timeCurrent.dSeconds += timeIncrement.dSeconds;

		// prevent time from going negative:
		timeCurrent.dSeconds = max( 0, timeCurrent.dSeconds );
		break;

	case TF_SAMPLES:
		if (bNegative)
			timeCurrent.llSamples -= timeIncrement.llSamples;
		else
			timeCurrent.llSamples += timeIncrement.llSamples;

		// prevent time from going negative:
		timeCurrent.llSamples = max( 0, timeCurrent.llSamples );
		break;

	case TF_TICKS:
		if (bNegative)
			timeCurrent.lTicks -= timeIncrement.lTicks;
		else
			timeCurrent.lTicks += timeIncrement.lTicks;

		// prevent time from going negative:
		timeCurrent.lTicks = max( 0, timeCurrent.lTicks );
		break;

	case TF_UTICKS:
		if (bNegative)
			timeCurrent.llUTicks -= timeIncrement.llUTicks;
		else
			timeCurrent.llUTicks += timeIncrement.llUTicks;

		// prevent time from going negative:
		timeCurrent.llUTicks = max( 0, timeCurrent.llUTicks );
		break;

	case TF_FRAMES:
		if (bNegative)
			timeCurrent.frames.lFrame -= timeIncrement.frames.lFrame;
		else
			timeCurrent.frames.lFrame += timeIncrement.frames.lFrame;

		// prevent time from going negative:
		timeCurrent.frames.lFrame = max( 0, timeCurrent.frames.lFrame );
		break;
		// NOTE: for this application, since we are nudging the transport
		// time, we ignore the FPS specified in the time increment.

	case TF_MBT:

		if (bNegative)
		{
			if (timeIncrement.mbt.nMeas != 0)
			{
				if (timeCurrent.mbt.nBeat == 1 && timeCurrent.mbt.nTick == 0)
					timeCurrent.mbt.nMeas -= timeIncrement.mbt.nMeas;

				timeCurrent.mbt.nMeas = max( 1, timeCurrent.mbt.nMeas );
				timeCurrent.mbt.nBeat = 1;
			}
			if (timeIncrement.mbt.nBeat != 0)
			{
				if (timeCurrent.mbt.nTick == 0)
					timeCurrent.mbt.nBeat -= timeIncrement.mbt.nBeat;

				if (timeCurrent.mbt.nMeas == 1)
					timeCurrent.mbt.nBeat = max( 1, timeCurrent.mbt.nBeat );
			}
			if (timeIncrement.mbt.nTick != 0)
				timeCurrent.mbt.nTick -= timeIncrement.mbt.nTick;
			else
				timeCurrent.mbt.nTick = 0;
		}
		else
		{
			if (timeIncrement.mbt.nMeas != 0)
			{
				timeCurrent.mbt.nMeas += timeIncrement.mbt.nMeas;
				timeCurrent.mbt.nBeat = 1;
			}
			if (timeIncrement.mbt.nBeat != 0)
			{
				timeCurrent.mbt.nBeat += timeIncrement.mbt.nBeat;
			}

			if (timeIncrement.mbt.nTick != 0)
				timeCurrent.mbt.nTick += timeIncrement.mbt.nTick;
			else
				timeCurrent.mbt.nTick = 0;
		}

		// prevent time from going negative:
		timeCurrent.dSeconds = max( 0, timeCurrent.dSeconds );
		break;

	default:
		_ASSERT( 0 ); // unsupported time format
	}

	hr = pSonarTransport->SetTransportTime(
		TRANSPORT_TIME_CURSOR,
		&timeCurrent
	);
	_ASSERT( SUCCEEDED( hr ) );

	pSonarTransport->Release();

}
/////////////////////////////////////////////////////////////////////////
void CTransport::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	// only if REFRESH_F_TRANSPORT bit is set
	if ( 0 == (fdwRefresh & REFRESH_F_TRANSPORT) )
		return;

	// evaluate transport operations other than shuttle
	ISonarTransport *pSonarTransport = NULL;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	// check for any changes in play or record state
	BOOL bStateRecAuto = FALSE;
	BOOL bStateRec = FALSE;
	BOOL bStatePlay = FALSE;

	ETransportState tState = tsInvalid;

	hr = pSonarTransport->GetTransportState(
		TRANSPORT_STATE_REC_AUTOMATION,
		&bStateRecAuto
		);

	if (bStateRecAuto)
	{
		// we are recording automation
		tState = tsRecordAutomation;
	}
	else
	{
		hr = pSonarTransport->GetTransportState(
			TRANSPORT_STATE_REC,
			&bStateRec
			);

		if (bStateRec)
		{
			// we are recording
			tState = tsRecord;
		}
		else
		{
			hr = pSonarTransport->GetTransportState(
				TRANSPORT_STATE_PLAY,
				&bStatePlay
				);
			if (bStatePlay)
			{
				// we are playing automation
				tState = tsPlay;
			}
			else
			{
				// transport must be stopped or shuttling
				if (m_pShuttleState != NULL && m_pShuttleState->eAction == CShuttleVal::Pressed)
				{
					if (m_pShuttleState->bNegative)
						tState = tsShuttleRewind;
					else
						tState = tsShuttleForward;
				}
				else
					tState = tsStop;
			}
		}
	}

	if (tState != m_tState)
	{
		// notify transport change
		m_tState = tState;
		if (m_pListener)
		{
			m_pListener->OnTransportUpdate( m_tState );
		}

		if (tState == tsPlay)
			m_bIsPlaying = TRUE;
		if (tState == tsStop)
			m_bIsPlaying = FALSE;		
	}

	pSonarTransport->Release();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CTransportListener::Initialize()
{
	CTransportListener *pOldListener = m_pSurface->GetTransport()->getListener();
	if (pOldListener != NULL)
	{
		return E_UNEXPECTED;
	}
	m_pSurface->GetTransport()->setListener( this );
	return S_OK;
}

CTransportListener::~CTransportListener()
{
	if (m_pSurface == NULL)
		return;

	if (m_pSurface->GetTransport() == NULL)
		return;

	m_pSurface->GetTransport()->setListener( NULL );
}

/////////////////////////////////////////////////////////////////////////
// CSonarCommander
/////////////////////////////////////////////////////////////////////////
CSonarCommander::CSonarCommander( CControlSurface* pSurface ):
	m_pSurface( pSurface )
{
}

/////////////////////////////////////////////////////////////////////////
CSonarCommander::~CSonarCommander()
{
	ClearCommands();
}

/////////////////////////////////////////////////////////////////////////
void CSonarCommander::ClearCommands()
{
	for (CommandTriggerMapIt it = m_mapCommandTriggers.begin(); it != m_mapCommandTriggers.end(); it++)
	{
		CMidiTrigger trig = (*it).first;
		trig.SetMsg( NULL, 0, this );
		
		CommanderValSet &set = it->second;
		for (CommanderValSetIt itCvs = set.begin(); itCvs != set.end(); itCvs++)
		{
			CCommanderVal *pVal = (*itCvs);
			delete pVal;
		}
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CSonarCommander::AddCommand(
	CMidiMsg *pMsg, DWORD dwVal,
	DWORD dwCmdID,
	CONDITION_HANDLE *phCondition /* = NULL */
)
{
	_ASSERT( pMsg );

	CMidiTrigger trig;
	trig.m_dwVal = dwVal;
	trig.m_pMsg = pMsg;

	CommandTriggerMapIt it;
	it = m_mapCommandTriggers.find( trig );

	// create it
	CCommanderVal *pVal = new CCommanderVal;
	pVal->dwCmdID = dwCmdID;


	if (it == m_mapCommandTriggers.end()) // if trigger not found
	{
		// insert it...
		CommanderValSet set;
		set.insert( CommanderValSet::value_type( pVal ) );
		if (phCondition)
			*phCondition = pVal;

		m_mapCommandTriggers.insert( CommandTriggerMap::value_type( trig, set ) );

		pMsg->AddListener( this );

		return S_OK;
	}
	else // trigger was found. Add another command entry for this trigger
	{		
		CommanderValSet &set = (*it).second;
		CommanderValSetIt itSet;
		itSet = set.find( pVal );
		if (itSet == set.end())
		{
			set.insert( CommanderValSet::value_type( pVal ) );
			if (phCondition)
				*phCondition = pVal;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}

}

/////////////////////////////////////////////////////////////////////////
HRESULT CSonarCommander::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	CMidiTrigger key;
	key.m_pMsg = pMsg;
	key.m_dwVal = dwVal;

	CommandTriggerMapIt itCM = m_mapCommandTriggers.find( key );

	if (itCM == m_mapCommandTriggers.end())
		return S_FALSE;	// we have received an unintended MIDI message

	CommanderValSet &set = itCM->second; // handy ref

	for (CommanderValSetIt itSet = set.begin(); itSet != set.end(); itSet++)
	{
		CCommanderVal *pVal = (*itSet);
		
		if (pVal->Conjunction( m_pSurface ) == FALSE)
			continue;

		// issue the command
		return DoCommand( pVal->dwCmdID );
	}
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CSonarCommander::DoCommand( DWORD dwCmdID )
{
	ISonarCommands *pSonarCommands = 0;
	HRESULT hr = m_pSurface->GetSonarCommands( &pSonarCommands );
	if (FAILED( hr ))
		return hr;

	hr = pSonarCommands->DoCommand( dwCmdID );
	pSonarCommands->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////
// CLastParamChangeListener:
/////////////////////////////////////////////////////////////////////////
CLastParamChangeListener::CLastParamChangeListener( CControlSurface *pSurface ):
	m_pSurface( pSurface )
{
	m_pSurface->GetLastParamChange()->add( this );
}

/////////////////////////////////////////////////////////////////////////
CLastParamChangeListener::~CLastParamChangeListener()
{
	m_pSurface->GetLastParamChange()->remove( this );
}

/////////////////////////////////////////////////////////////////////////
// CThrottle
/////////////////////////////////////////////////////////////////////////
CThrottle::CThrottle( CControlSurface *pSurface, DWORD dwThrottleTime, EThrottleMode eMode /* = thAstable */ ) :
	m_dwThrottleTime( dwThrottleTime ),
	m_bWaiting( FALSE ),
	m_eMode( eMode ),
	m_pSurface( pSurface )
{
	SetIsOneShot( TRUE );
}

/////////////////////////////////////////////////////////////////////////
void CThrottle::OnEventToThrottle()
{
	if (m_dwThrottleTime != 0) // use throttle
	{
		// trigger the throttle timer
		// the queue is empty, set up the timer
		if (!m_bWaiting || m_eMode == thMonostable)
		{
			m_bWaiting = TRUE;

			HRESULT hr = m_pSurface->GetTimer()->SetPeriod( this, (WORD)m_dwThrottleTime );
			_ASSERT( SUCCEEDED( hr ) );
		}
	}
	else
		OnThrottleElapsed();
}

/////////////////////////////////////////////////////////////////////////
// CLastParamChange:
/////////////////////////////////////////////////////////////////////////
CLastParamChange::CLastParamChange( CControlSurface *pSurface ):
	m_pSurface( pSurface ),
	m_bIsDefined( FALSE ),
	m_pParam( NULL ),
	CThrottle( pSurface, 20 )
{
	SetIsOneShot( TRUE );
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::SetThrottleTime( DWORD dwThrottleTime )
{
	m_dwThrottleTime = dwThrottleTime;
}

/////////////////////////////////////////////////////////////////////////
BOOL CLastParamChange::GetLastParamID(
		SONAR_MIXER_STRIP *pmixerStrip,
		DWORD *pdwStripNum,
		SONAR_MIXER_PARAM *pmixerParam,
		DWORD *pdwParamNum
	)
{
	if (m_bIsDefined)
	{
		if (pmixerStrip)
			*pmixerStrip = m_mixerStrip;

		if (pdwStripNum)
			*pdwStripNum = m_dwStripNum;

		if (pmixerParam)
			*pmixerParam = m_mixerParam;

		if (pdwParamNum)
			*pdwParamNum = m_dwParamNum;

		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
BOOL CLastParamChange::GetLastMixParam(
		CBaseMixParam **ppParam
	)
{
	if (m_bIsDefined)
	{
		if (ppParam)
		{
			_ASSERT( m_pParam );
			*ppParam = m_pParam;
		}

		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
BOOL CLastParamChange::IsDefined()
{
	return m_bIsDefined;
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::setLastParamID(
		CBaseMixParam *pParam,
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	m_pParam = pParam;
	m_mixerStrip = mixerStrip;
	m_dwStripNum = dwStripNum;
	m_mixerParam = mixerParam;
	m_dwParamNum = dwParamNum;
	m_bIsDefined = TRUE;
	
	OnEventToThrottle();
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::OnThrottleElapsed()
{
	notifyListeners();
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::add( CLastParamChangeListener* pListener )
{
	m_setListeners.insert(ParamListenerSet::value_type( pListener ));
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::remove( CLastParamChangeListener* pListener )
{
	ParamListenerSetIt itSS = m_setListeners.find( pListener );
	if (itSS != m_setListeners.end())
	{
		m_setListeners.erase( itSS );
	}
}

/////////////////////////////////////////////////////////////////////////
void CLastParamChange::notifyListeners()
{
	ParamListenerSetIt itSS;

	// for all the Listeners in our "set" notify if the flag mask matches up.
	for (itSS = m_setListeners.begin(); itSS != m_setListeners.end(); itSS++)
	{
		(*itSS)->OnParamChange();
	}
}

/////////////////////////////////////////////////////////////////////////