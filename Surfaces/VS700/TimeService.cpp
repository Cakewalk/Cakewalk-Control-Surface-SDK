/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// TimeService.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TimeService.h"
#include "Transport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CTimer
/////////////////////////////////////////////////////////////////////////
CTimer::CTimer( UINT nResolution ):
	m_bIsTimerActive( FALSE ),
	m_uiTimerID( 0 )
{
	TIMECAPS timeDevCaps;
	timeGetDevCaps( &timeDevCaps, sizeof( timeDevCaps ) );
	m_dwTimerPeriod = max( timeDevCaps.wPeriodMin, nResolution );
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
	reinterpret_cast<CTimer*>(dwUser)->onTimer();
}

/////////////////////////////////////////////////////////////////////////
void CTimer::setTimerActive( BOOL bIsActive )
{
	BOOL bWasActive = m_bIsTimerActive;
	if (bIsActive && !bWasActive ) // if it is not active but we want to make it so
	{
		MMRESULT mr = timeBeginPeriod( m_dwTimerPeriod );
		ASSERT( mr != TIMERR_NOCANDO );

		m_uiTimerID = timeSetEvent(
			m_dwTimerPeriod,
			1,								// was: m_dwTimerPeriod,
			(LPTIMECALLBACK)timerCallback,
			(DWORD)this,
			TIME_PERIODIC | TIME_CALLBACK_FUNCTION
		);
 
		if (m_uiTimerID != NULL)
			m_bIsTimerActive = TRUE;
	}
	else if (!bIsActive && bWasActive) // if it is active and we want to make it inactive
	{
		timeKillEvent( m_uiTimerID );
		m_bIsTimerActive = FALSE;

		MMRESULT mr = timeEndPeriod( m_dwTimerPeriod );
		ASSERT( mr != TIMERR_NOCANDO );
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CTimer::Schedule( CTimerClient *pClient, DWORD dwPeriod )
{
	CCriticalSectionAuto lock( m_cs );

	if (pClient == NULL)
		return E_POINTER;

	if (dwPeriod != 0)	// activation
	{
		// schedule or reschedule service to a client
		if (pClient->m_bWaiting)
		{
			// reschedule
			CScheduleEntry key;
			key.m_pClient = pClient;
			ScheduleSetIt itFind = m_ScheduleSet.find( key );
			if (itFind != m_ScheduleSet.end())
			{
				// found, reschedule
				m_ScheduleSet.erase( itFind );

				pClient->m_bWaiting = TRUE;
				pClient->m_dwPeriod = max( dwPeriod, m_dwTimerPeriod );
				pClient->m_dwActionTime = timeGetTime() + pClient->m_dwPeriod;

				key.m_pClient = pClient;
				m_ScheduleSet.insert( key );
				
				return S_OK;
			}
			else
			{
				// not found, return error
				return E_UNEXPECTED;
			}
		}
		else // not waiting...
		{
			// schedule a new entry

			pClient->m_bWaiting = TRUE;
			pClient->m_dwPeriod = max( dwPeriod, m_dwTimerPeriod );
			pClient->m_dwActionTime = timeGetTime() + pClient->m_dwPeriod;

			CScheduleEntry key;
			key.m_pClient = pClient;
			m_ScheduleSet.insert( key );

			setTimerActive( TRUE );
			return S_OK;
		}
	}
	else // period is zero
	{
		// a period of zero is a deactivation
		CScheduleEntry key;
		key.m_pClient = pClient;
		key.m_pClient->m_bWaiting = FALSE;

		ScheduleSetIt itFind = m_ScheduleSet.find( key );
		if (itFind != m_ScheduleSet.end())
		{
			m_ScheduleSet.erase( itFind );

			if (m_ScheduleSet.size() == 0)
			{
				setTimerActive( FALSE );
			}
			return S_OK;
		}
		else
			return S_FALSE; // not found for deactivation
	}

}

/////////////////////////////////////////////////////////////////////////
void CTimer::onTimer()
{
	ScheduleSet setRemove;
	ScheduleSet setRepeat;
	ScheduleSet setTick;

	{
		// service clients
		CCriticalSectionAuto lock( m_cs );


		ScheduleSetIt it = m_ScheduleSet.begin();
		while( it != m_ScheduleSet.end() && (*it).GetActionTime() <= timeGetTime())
		{
			CTimerClient *pClient = (*it).m_pClient;

			CScheduleEntry key;
			key.m_pClient = pClient;

			setTick.insert( key );

			if (pClient->GetIsOneShot())
			{
				// one shot, must remove
				setRemove.insert( key );
			}
			else
			{
				// not one shot, must repeat
				setRepeat.insert( key );
			}
			it++;
		}

		// remove the one shots:
		for (it = setRemove.begin(); it != setRemove.end(); it++)
		{
			CScheduleEntry key = (*it);
			ScheduleSetIt itFind = m_ScheduleSet.find( key );
			if (itFind != m_ScheduleSet.end())
			{
				// if found
				key.m_pClient->m_bWaiting = FALSE;
				m_ScheduleSet.erase( itFind );
			}
			else
				ASSERT( 0 ); // it should have been found
		}

		// reschedule the repeats:
		for (it = setRepeat.begin(); it != setRepeat.end(); it++)
		{
			CScheduleEntry key = (*it);
			ScheduleSetIt itFind = m_ScheduleSet.find( key );
			if (itFind != m_ScheduleSet.end())
			{
				// if found
				m_ScheduleSet.erase( itFind ); // remove it because it will be rescheduled
				key.m_pClient->m_dwActionTime += key.m_pClient->m_dwPeriod;
				m_ScheduleSet.insert( key );
			}
			else
				ASSERT( 0 ); // it should have been found
		}

		if (m_ScheduleSet.size() == 0)
		{
			setTimerActive( FALSE );
		}


	}

	// tick everybody
	for (ScheduleSetIt it = setTick.begin(); it != setTick.end(); it++)
	{
		CScheduleEntry key = (*it);
		key.m_pClient->Tick();
	}
}

/////////////////////////////////////////////////////////////////////////

DWORD CScheduleEntry::GetActionTime() const
{
	ASSERT( m_pClient );
	return m_pClient->m_dwActionTime;
}

/////////////////////////////////////////////////////////////////////////

bool CScheduleEntry::operator <( const CScheduleEntry& rhs ) const
{
	ASSERT( m_pClient );

	if (m_pClient->m_dwActionTime < rhs.m_pClient->m_dwActionTime )
		return TRUE;
	else if (m_pClient->m_dwActionTime == rhs.m_pClient->m_dwActionTime )
	{
		return m_pClient < rhs.m_pClient;
	}
	else
		return FALSE;
}

