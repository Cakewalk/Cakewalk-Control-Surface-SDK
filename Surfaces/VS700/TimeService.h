/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// TimeService.h:	Definitions of framework for time notification scheduling
/////////////////////////////////////////////////////////////////////////

#if !defined(TIME_SERVICE_INCLUDED)
#define TIME_SERVICE_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////
// CTimer
//
// This object allows any CTimerClient to subscribe for timed / periodic
// notifications at specified intervals. This allows using only one
// timer in the entire surface framework.
/////////////////////////////////////////////////////////////////////////

// forward
class CTimerClient;
class CTransport;
class CScheduleEntry;

/////////////////////////////////////////////////////////////////////////

class CScheduleEntry
{
public:
	CScheduleEntry(): m_pClient( 0 ){}

	bool operator <( const CScheduleEntry& rhs ) const;
	DWORD GetActionTime() const;
	CTimerClient*		m_pClient;
};

/////////////////////////////////////////////////////////////////////////

typedef std::set< CScheduleEntry > ScheduleSet;
typedef ScheduleSet::iterator ScheduleSetIt;

class CTimer
{
	friend class CTimerClient;

public:
	CTimer( UINT nTimerPeriod );
	virtual ~CTimer();

	HRESULT Schedule( CTimerClient *pClient, DWORD dwPeriod );

	void SetIsActive( BOOL bIsActive );

	UINT GetPeriod() { return m_dwTimerPeriod; }

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
	UINT					m_uiTimerID;
	DWORD					m_dwTimerPeriod;
	BOOL					m_bIsTimerActive;
	ScheduleSet			m_ScheduleSet;
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
	friend class CScheduleEntry;

public:
	CTimerClient( CTimer* pTimer ) :
		m_dwPeriod( 0 ),
		m_dwActionTime( 0 ),
		m_bOneShot( FALSE ),
		m_bWaiting( FALSE ),
		m_pTimer( pTimer )
	{
	}

	virtual ~CTimerClient()
	{
		if (m_bWaiting)
			m_pTimer->Schedule( this, 0 );
		
		m_pTimer = NULL;
	}

	CTimerClient& CTimerClient::operator=( const CTimerClient& rhs )
	{
		if(&rhs != this)
		{
			m_dwPeriod = rhs.m_dwPeriod;
			m_dwActionTime = rhs.m_dwActionTime;
			m_bOneShot = rhs.m_bOneShot;
			m_bWaiting = rhs.m_bWaiting;
			m_pTimer = rhs.m_pTimer;
		}
		return *this;
	}


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

	CTimer*	GetTimer() { return m_pTimer; }
private:


	DWORD		m_dwPeriod;
	DWORD		m_dwActionTime;
	BOOL		m_bOneShot;
	BOOL		m_bWaiting;
	CTimer*	m_pTimer;

};

/////////////////////////////////////////////////////////////////////////
#endif // !defined(TIME_SERVICE_INCLUDED)