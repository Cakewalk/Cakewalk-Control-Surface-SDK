// TacomaLCD.cpp : Functions for LCD display
//

#include "stdafx.h"

#include "VS100.h"
#include "MixParam.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// Use MAYBE_TRACE() for items we'd like to switch off when not actively
// working on this file.
#if 0
	#define MAYBE_TRACE TRACE
#else
	#define MAYBE_TRACE TRUE ? (void)0 : TRACE
#endif



/////////////////////////////////////////////////////////////////////
// CTimerClient
void	CVS100::Tick( DWORD id)
{
	switch( id )
	{
	case TS_ParamChanged:
		MAYBE_TRACE( "Tick() TS = TS_ParamChanged\n" );
		killTimer( m_tcbParamChange );
		updateParamStateOnLCD();
		break;

	case TS_Blink:
		onBlink();
		break;

	case TS_KeyRep:
		onKeyRep();
		break;

	case TS_Revert:
		onRevert();
		break;

	default:
		ASSERT(0);	// eh?
	}
}

//////////////////////////////////////////////////////////////////////
void CVS100::startBlink( ControlId id )
{
	bool bEmpty = false;
	{
		CSFKCriticalSectionAuto csa( m_cs );
		bEmpty = m_setBlinkControls.empty();
		m_setBlinkControls.insert( id );
	}

	if ( bEmpty )
	{
		// adding first control, start the timer
		setTimer( m_tctBlink, 250 );
		m_eBlinkState = BS_Off;
	}
}


////////////////////////////////////////////////////////////////////////
void CVS100::stopBlink( ControlId id )
{
	CSFKCriticalSectionAuto csa( m_cs );
	const bool bHadMsg = m_setBlinkControls.count( id ) > 0;
	m_setBlinkControls.erase( id );

	// and kill the LED
	ControlMessageMapIterator mit = m_mapControlMsgs.find( id );
	if ( bHadMsg && m_mapControlMsgs.end() != mit )
	{
		CMidiMsg* pmsg = mit->second;
		pmsg->Send( 0.f );
	}

	// if last control removed, stop blinking timer on next callback
	if ( m_setBlinkControls.empty() )
		m_eBlinkState = BS_None;
}

void CVS100::stopBlink()
{
	CSFKCriticalSectionAuto csa( m_cs );
	m_setBlinkControls.clear();
	m_eBlinkState = BS_None;
	killTimer( m_tctBlink );
}

/////////////////////////////////////////////////////////////////////////////
void CVS100::onBlink()
{

	if ( BS_None == m_eBlinkState )
	{
		killTimer( m_tctBlink );
	}
	else
	{
		// toggle
		m_eBlinkState = BS_On == m_eBlinkState ? BS_Off : BS_On;

		const float f = BS_On == m_eBlinkState ? 1.f : 0.f;

		std::set<CMidiMsg*> setBlink;
		{	// scope
			CSFKCriticalSectionAuto csa( m_cs );
			for ( std::set<ControlId>::iterator it = m_setBlinkControls.begin(); it != m_setBlinkControls.end(); ++it )
			{
				ControlId id = *it;
				ControlMessageMapIterator mit = m_mapControlMsgs.find( id );
				if ( m_mapControlMsgs.end() != mit )
				{
					CMidiMsg* pmsg = mit->second;
					setBlink.insert( pmsg );
				}
				else
				{
					// also look through the strip messages
					for ( InputBindingIterator it = m_mapStripMsgs.begin(); it != m_mapStripMsgs.end(); ++it )
					{
						CMidiMsg* pmsg = it->first;
						if ( pmsg->GetId() == id )
							setBlink.insert( pmsg );
					}			
				}
			}
		}

		for ( std::set<CMidiMsg*>::iterator it = setBlink.begin(); it != setBlink.end(); ++it )
		{
			CMidiMsg* pmsg = *it;
			pmsg->Send( f );
		}
	}
}


//////////////////////////////////////////////////////
void CVS100::onKeyRep()
{
	if ( m_tctKeyRep.GetTicks() > 5 )
	{
      if ( m_tctKeyRep.GetButton() == CID_ACT )
      {
         killTimer( m_tctKeyRep );
			m_tctKeyRep.SetButton( 0 );
//         updateActContextName( true );
         return;
      }
	}
}


void CVS100::setTimer( CTimerClientWithID& tc, WORD wMs )
{
	m_pTimer->SetPeriod( &tc, wMs );
	tc.SetActive( true );
}

void CVS100::killTimer( CTimerClientWithID& tc )
{
	m_pTimer->SetPeriod( &tc, 0 );
	tc.SetActive( false );
}


void CVS100::requestRevert( CMixParam* p )
{
	m_setRevertParams.insert( p );
	setTimer( m_tctRevert, 500 );
	showParam( p, PSM_Revert );
}


void CVS100::onRevert()
{
	killTimer( m_tctRevert );
	for ( std::set<CMixParam*>::iterator it = m_setRevertParams.begin(); it != m_setRevertParams.end(); ++it )
	{
		CMixParam* p = *it;
		p->RevertValue();
	}

	m_setRevertParams.clear();

	initLCDs();
}




/////////////////////////////////////////////////////////////////////
// Timer Tick Overrides

void CTimerClientWithID::Tick()
{
	if ( m_pSurface )
		m_pSurface->Tick( m_id );
}






