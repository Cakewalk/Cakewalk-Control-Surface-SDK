#include "stdafx.h"
#include "VS100.h"
#include "MixParam.h"
#include "LCDTextWriter.h"
#include "StringCruncher.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Handles Refreshing the surface to match the host states
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------
// The host is asking us to refresh.  Here we should query the host for
// the values we're interested in and set our states
HRESULT CVS100::onRefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	// use the m_dwRefreshCount member to thin out updates as needed. For example,
	// motors and meters should be updated as often as possible.  However, switches,
	// and text names can be updated less often.  You can also use this to interlace
	// updates (even/odd strips get updated every other refresh)

	int nRefreshMod = ( (m_dwRefreshCount % 2 ) == 0 ) ? RM_EVEN : RM_ODD;

	InputBindingMap*	aMaps[] = { &m_mapACT, &m_mapStripMsgs };

	// For all of our parameter binding sets,  Query the host for changes and possibly broadcast
	// them out any associated output messages. Also update the input message state so that
	// their current value matches the host
   for ( size_t iMap = 0; iMap < _countof( aMaps ); iMap++ )
   {
	   InputBindingMap* pMap = aMaps[iMap];
	   if ( !pMap )
		   continue;
		for ( InputBindingIterator it = pMap->begin(); it != pMap->end(); ++it )
		{
			PMBINDING& pmb = it->second;
			CMidiMsg* pmsgIn = it->first;
			if ( pmb.nRefreshMod == RM_EVERY || pmb.nRefreshMod == nRefreshMod )
			{
				float fVal = 0.f;
				HRESULT hrChange = pmb.pParam->GetVal( &fVal );
				if ( S_OK == hrChange )
				{
					// is there an output message?  Broadcast
					if ( pmb.pMsgOut )
					{
						if ( m_bEnableMotors || CID_Fader != pmsgIn->GetId() )
							pmb.pMsgOut->Send( fVal );
					}
					// In any case, set input state so that delta's work
					bool bSetVal = true;
					if ( CID_ValueEnc == pmsgIn->GetId() && m_bShiftPressed )
						bSetVal = false;
					else if ( pmsgIn->GetId() == CID_StripRotary && SRM_Normal != m_eRotaryMode )
						bSetVal = false;
					if ( bSetVal )
						pmsgIn->SetVal( fVal );	

					if ( pmsgIn->GetId() == CID_StripRotary )
						showEncoderParam( pmb.pParam );
				}
			}
		}
	}

	// meters:  Note.  If your surface does not have any sort of meter, just skip this.
	std::vector<float> vMeters;
	for ( StripRangeMapIterator it = m_mapStripRanges.begin(); it != m_mapStripRanges.end(); ++it )
	{
		STRIPRANGE& sr = it->second;
		SONAR_MIXER_STRIP eStrip = it->first;

		for ( DWORD i = sr.dwL; i <= sr.dwH; i++ )
		{
			GetMeterValues( eStrip, i, &vMeters, 1 );
			// Todo:  Update your meter objects.  This should be done on every refresh
		}
	}

	// update LCDs
	refreshLCDs();

	// Button states
	refreshLEDs();

	return S_OK;
}

HRESULT CVS100::refreshLCDs()
{
	CStringCruncher cCruncher;

	// Strip Name
	char sz[128];
	char szDest[8];
	DWORD cb = _countof(sz);
	sz[0] = '\0';
	cb = 127;
	m_pSonarMixer->GetMixStripName( m_eStripType, this->getActiveStrip(), sz, &cb );
	cCruncher.CrunchString( sz, szDest, 7 );
	szDest[7] = '\0';
	m_pLCD->ShowText( szDest, 1, 1 );

	// context name
	cb = _countof(sz);
	sz[0] = '\0';
   m_pSonarParamMapping->GetMapName( m_dwSurfaceID, (LPSTR) &sz, &cb );
	cCruncher.CrunchString( sz, szDest, 7 );
	szDest[7] = '\0';
	m_pLCD->ShowText( szDest, 0, 1 );

	return S_OK;
}


HRESULT CVS100::refreshLEDs()
{
	// Button states
	if ( m_dwRefreshCount % 2 == 0 )
	{
		for ( ControlMessageMapIterator it = m_mapControlMsgs.begin(); it != m_mapControlMsgs.end(); it++ )
		{
			CMidiMsg* pmsg = it->second;
			ControlId id = it->first;

			if ( CID_Play == id || CID_Stop == id )
			{
				if ( IsScrub() )
					startBlink( id );
				else if ( IsPause() && CID_Play == id )
					startBlink( id );
				else
					stopBlink( id );
			}


			// if not blinking, set the state
			if ( m_setBlinkControls.count( id ) == 0 )
			{
				switch( id )
				{
					case CID_Play:
						pmsg->Send( IsPlaying() ? 1.f : 0.f );
						break;
					case CID_Stop:
						pmsg->Send( IsPlaying() ? 0.f : 1.f );
						break;
					case CID_Record:
						pmsg->Send( IsRecording() ? 1.f : 0.f );
						break;
					case CID_FF:
						pmsg->Send( IsFastForward() ? 1.f : 0.f );
						break;
					case CID_Rew:
						pmsg->Send( IsRewind() ? 1.f : 0.f );
						break;
				}
			}
		}
	}
	return S_OK;
}
