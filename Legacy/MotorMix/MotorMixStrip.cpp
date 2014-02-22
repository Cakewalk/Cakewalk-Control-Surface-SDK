/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixStrip.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotorMixStrip.h"
#include "MotorMixSubclasses.h"
#include "StateDefs.h"

/////////////////////////////////////////////////////////////////////////
// CMMStrip:
/////////////////////////////////////////////////////////////////////////
CMMStrip::CMMStrip(
	DWORD dwHWStripNum,
	CControlSurface* pSurface
	) :
	CMixStrip( dwHWStripNum, pSurface ),
	m_pParamFader( NULL ),
	m_pParamMute( NULL ),
	m_pParamSolo( NULL ),
	m_pParamMulti( NULL ),
	m_pParamRecRdy( NULL ),
	m_pParamRotary( NULL ),
	m_msgFader( pSurface ),
	m_msgKnob( pSurface ),
	m_msgPushBtn( pSurface ),
	m_msgBtnLED( pSurface ),
	m_msgScribble( pSurface ),
	m_mixerStripMeter(MIX_STRIP_ANY),
	m_dwStripMeter( (DWORD)-1 )
{
}

////////////////////////////////////////////////////////////////////////////////
HRESULT CMMStrip::Initialize()
{
	HRESULT hr = S_OK;

	m_msgFader.SetMessageType( CMidiMsg::mtCCHiLo );
	m_msgFader.SetCCNum( m_dwHWStripNum );
	m_msgFader.SetCC2Num( 0x20 + m_dwHWStripNum  );

	m_msgKnob.SetMessageType( CMidiMsg::mtCC );
	m_msgKnob.SetCCNum( 0x40 + m_dwHWStripNum );

	m_msgPushBtn.SetMessageType( CMidiMsg::mtCCSel );
	m_msgPushBtn.SetCCSelNum( 0x0f );
	m_msgPushBtn.SetCCSelVal( m_dwHWStripNum );
	m_msgPushBtn.SetCCNum( 0x2F );

	m_msgBtnLED.SetMessageType( CMidiMsg::mtCCSel );
	m_msgBtnLED.SetCCSelNum( 0x0C );
	m_msgBtnLED.SetCCSelVal( m_dwHWStripNum );
	m_msgBtnLED.SetCCNum( 0x2C );

	// for fader touch:
	// 0x40 means pressed, 0x00 means released
	m_pParamFader = new CMMFaderParam( m_pSurface, &m_msgFader, &m_msgFader, &m_msgPushBtn, 0x40, &m_msgPushBtn, 0x00 );
	if (NULL == m_pParamFader)
		return E_OUTOFMEMORY;
	add( m_pParamFader );

	// since for all binary parameters we want toggle, we assign the on and off values to be the same.
	// for mute:
	// 0x42 means pressed, 0x02 means released. 
	m_pParamMute = new CMixParamBool( m_pSurface, &m_msgPushBtn, 0x42, 0x42, &m_msgBtnLED, 0x42, &m_msgBtnLED, 0x02 );
	if (NULL == m_pParamMute)
		return E_OUTOFMEMORY;
	add( m_pParamMute );

	// for solo:
	// 0x43 means pressed, 0x03 means released
	m_pParamSolo = new CMixParamBool( m_pSurface, &m_msgPushBtn, 0x43, 0x43, &m_msgBtnLED, 0x43, &m_msgBtnLED, 0x03 );
	if (NULL == m_pParamSolo)
		return E_OUTOFMEMORY;
	add( m_pParamSolo );

	// for multi:
	// 0x44 means pressed, 0x04 means released
	m_pParamMulti = new CMixParamBool( m_pSurface, &m_msgPushBtn, 0x44, 0x44, &m_msgBtnLED, 0x44, &m_msgBtnLED, 0x04 );
	if (NULL == m_pParamMulti)
		return E_OUTOFMEMORY;
	m_pParamMulti->SetIsEnabled( FALSE ); // we do not know the type of track yet.
	add( m_pParamMulti );

	// for rec/rdy:
	// 0x45 means pressed, 0x05 means released
	m_pParamRecRdy = new CMixParamBoolEx( m_pSurface, &m_msgPushBtn, 0x45, 0x45, &m_msgBtnLED, 0x45, &m_msgBtnLED, 0x05 );
	if (NULL == m_pParamRecRdy)
		return E_OUTOFMEMORY;
	add( m_pParamRecRdy );

	// for pan set up the rotary
	m_pParamRotary = new CMMRotaryParam( m_pSurface, m_dwHWStripNum, &m_msgKnob, NULL, NULL, 0, NULL, 0 );
	if (NULL == m_pParamRotary)
		return E_OUTOFMEMORY;
	m_pParamRotary->SetEffect( CMidiMsgListener::mdContinuousDelta );
	add( m_pParamRotary );

	// scribble message setup
	m_msgScribble.SetMessageType( CMidiMsg::mtSysXString );

	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x10, 0x00 }; // last byte is replaced with the "address" for the scribble strip.
	pczPreString[8] = BYTE(5 * m_dwHWStripNum);
	
	BYTE pczPostString[] = {0xF7};

	m_msgScribble.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribble.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribble.SetSysXTextLen( 5 ); // print five characters out each time
	m_msgScribble.SetSysXPadLen( 1 ); // force last one to be padding
	m_msgScribble.SetSysXTextFillChar( ' ' ); // padding is space bar
	m_msgScribble.SetUseTextCruncher( TRUE );
	SetScribbleMsg( &m_msgScribble );
	
	// after everything else has been set up, we may subscribe to all the states we need
	MM_CHECKHR( SubscribeToState( stContainerClass ) );
	MM_CHECKHR( SubscribeToState( stTrackRotaryMapping ) );
	MM_CHECKHR( SubscribeToState( stBusRotaryMapping ) );
	MM_CHECKHR( SubscribeToState( stSendPanOrLevel ) );
	MM_CHECKHR( SubscribeToState( stQueryState ) );
	MM_CHECKHR( SubscribeToState( stFineTweakMode ) );
	MM_CHECKHR( SubscribeToState( stInsertEditMode ) );
	MM_CHECKHR( SubscribeToState( stCurrentEffect ) );
	MM_CHECKHR( SubscribeToState( stBaseEffectParam ) );
	MM_CHECKHR( SubscribeToState( stShiftKey ) );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::refreshScribble() // overridden from CMixStrip
{
	if (GetStateForID( stQueryState ) != qsIdle)
		return;

	if (GetStateForID( stFineTweakMode ) != ftDisengaged )
	{
		// the scribble strip is in use
		return;
	}

	CMixStrip::refreshScribble();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMStrip::getScribbleText( LPSTR pszName, DWORD* pdwLen )
{
	if (GetStateForID( stInsertEditMode ) == ieEngaged)
	{
		SONAR_MIXER_STRIP curMixerStrip = m_pSurface->GetCurrentStripKind();
		DWORD	dwCurStripNum = m_pSurface->GetCurrentStrip();

		int nFx = GetStateForID( stCurrentEffect );
		int nParam = getCurrentFxParam();

		// return text for insert parameter name
		HRESULT hr = m_pParamRotary->IsFxParamInRange( curMixerStrip, dwCurStripNum, nFx, nParam );
		if (hr == S_OK)
		{
			CComPtr<ISonarMixer> pSonarMixer;
			hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (FAILED( hr ))
			{
				strcpy(pszName, "NoFx ");
			}

			MM_CHECKHR( pSonarMixer->GetMixParamLabel(	m_mixerStrip, m_pSurface->GetCurrentStrip(),	MIX_PARAM_FX_PARAM,
															MAKELONG( getCurrentFx(), getCurrentFxParam() ), pszName, pdwLen ));
		}
		else
		{
			strcpy(pszName, "NoFx ");
		}
		return S_OK;
	}
	MM_CHECKHR( CMixStrip::getScribbleText( pszName, pdwLen ) );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMStrip::OnStateChange( DWORD dwStateID, int nState )
{
	switch (dwStateID)
	{
	case stQueryState:
	case stContainerClass:
	case stBaseTrack:
	case stBaseBus:
	case stBaseMain:
	case stFineTweakMode:
	case stInsertEditMode:
		// because thinning would otherwise prevent the scribble from being sent,
		// after query states are finished, we must invalidate
		// the scribble messages so that
		// the value gets updated

		if (m_pMsgScribble)
			m_pMsgScribble->Invalidate();

		refreshScribble();
		break;
	}

	switch (dwStateID)
	{
	case stContainerClass:
	case stBaseTrack:
	case stBaseBus:
	case stBaseMain:
	case stFineTweakMode:
	case stInsertEditMode:
		remapStrip();
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapFader()
{
	m_pParamFader->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_VOL, 0 );
	m_pParamFader->SetRefreshFrequency( CBaseMixParam::RR_EVERY );
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapMute()
{
	const CBaseMixParam::ERefreshRate rr = ( m_dwStripNum % 2 == 0 ) ? CBaseMixParam::RR_EVEN : CBaseMixParam::RR_ODD;
	switch (GetStateForID( stContainerClass ))
	{
	case ccTracks:
	case ccBus:
	case ccMains:
		m_pParamMute->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_MUTE, 0 );
		m_pParamMute->SetIsEnabled( TRUE );
		m_pParamMute->SetRefreshFrequency( rr );
		break;

	default:
		_ASSERT( 0 ); // unknown state for stContainerClass
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapSolo()
{	
	const CBaseMixParam::ERefreshRate rr = ( m_dwStripNum % 2 == 0 ) ? CBaseMixParam::RR_EVEN : CBaseMixParam::RR_ODD;

	switch (GetStateForID( stContainerClass ))
	{
	case ccTracks:
	case ccBus:
		m_pParamSolo->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SOLO, 0 );
		m_pParamSolo->SetIsEnabled( TRUE );
		m_pParamSolo->SetRefreshFrequency( rr );
		break;

	case ccMains:
		m_pParamSolo->SetIsEnabled( FALSE );
		m_pParamSolo->ZeroMidiState();
		break;

	default:
		_ASSERT( 0 ); // unknown state for stContainerClass
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapRecRdy()
{
	const CBaseMixParam::ERefreshRate rr = ( m_dwStripNum % 2 == 0 ) ? CBaseMixParam::RR_EVEN : CBaseMixParam::RR_ODD;
	switch (GetStateForID( stContainerClass ))
	{
	case ccTracks:
		if (GetStateForID( stBurnButtonsChoice ) == bbOther)
		{
			m_pParamRecRdy->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_RECORD_ARM, 1 );
			m_pParamRecRdy->SetIsEnabled( TRUE );
			m_pParamRecRdy->SetRefreshFrequency( rr );
		}
		else if (GetStateForID( stBurnButtonsChoice ) == bbRecRdy)
		{
			m_pParamRecRdy->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_RECORD_ARM, 0 );
			m_pParamRecRdy->SetIsEnabled( TRUE );
			m_pParamRecRdy->SetRefreshFrequency( rr );
		}
		else
			m_pParamRecRdy->SetIsEnabled( FALSE );
		break;

	case ccBus:
		if (GetStateForID( stBurnButtonsChoice ) == bbOther)
		{
			m_pParamRecRdy->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_RECORD_ARM, 1 );
			m_pParamRecRdy->SetIsEnabled( TRUE );
			m_pParamRecRdy->SetRefreshFrequency( rr );
		}
		else
		{
			m_pParamRecRdy->SetIsEnabled( FALSE );
		}
		break;
	
	case ccMains:
		m_pParamRecRdy->SetIsEnabled( FALSE );
		break;

	default:
		_ASSERT( 0 ); // unknown state for stContainerClass
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapMulti()
{
	int nContainerState = GetStateForID( stContainerClass );
	const CBaseMixParam::ERefreshRate rr = ( m_dwStripNum % 2 == 0 ) ? CBaseMixParam::RR_EVEN : CBaseMixParam::RR_ODD;

	switch ( nContainerState )
	{
	case ccTracks:
	case ccBus:

		if (m_pParamMulti->IsMidiTrack( m_mixerStrip, m_dwStripNum ))
		{
			m_pParamMulti->SetIsEnabled( FALSE );
			return;
		}
		else
		{
			int nRotaryMapping = nContainerState == ccTracks ? GetStateForID( stTrackRotaryMapping ) : GetStateForID( stBusRotaryMapping );
		
			int nCurrentAux = 0;
			if ( ccTracks == nContainerState && nRotaryMapping >= trSendBase )
			{
				nCurrentAux = nRotaryMapping - trSendBase;
			}
			else if ( ccBus == nContainerState && nRotaryMapping >= buSendBase)
			{
				nCurrentAux = nRotaryMapping - buSendBase;
			}
			
			if ( ccTracks != nContainerState && ccBus != nContainerState )
			{
				m_pParamMulti->SetIsEnabled( FALSE );
			}
			else
			{
				m_pParamMulti->SetRefreshFrequency( rr );
				switch (GetStateForID( stMultiButtonsChoice ))
				{
				case mbFxBypassE1:
					m_pParamMulti->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_PHASE, 0 );
					m_pParamMulti->SetIsEnabled( ccTracks == nContainerState );
					break;

				case mbSMuteE2:
					m_pParamMulti->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_ENABLE, nCurrentAux );
					m_pParamMulti->SetIsEnabled( TRUE );
					break;

				case mbPrePostE3:
					m_pParamMulti->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_PREPOST, nCurrentAux );
					m_pParamMulti->SetIsEnabled( TRUE );
					break;

				case mbSelectE4:
					m_pParamMulti->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_INPUT_ECHO, 0 );
					m_pParamMulti->SetIsEnabled( ccTracks == nContainerState );
					break;

				default:
					_ASSERT( 0 ); // unknown multi buttons choice
				}
			}
		}
		break;

	case ccMains:
		m_pParamMulti->SetIsEnabled( FALSE );
		break;

	default:
		_ASSERT( 0 ); // unknown state for stContainerClass
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMStrip::OnRemapRotary()
{
	CComPtr<ISonarMixer> pIMixer;
	m_pSurface->GetSonarMixer( &pIMixer );

	BOOL bHostAudioMeters = FALSE;
	CComPtr<ISonarIdentity> pIsid;
	m_pSurface->GetSonarIdentity( &pIsid );

	if( pIsid )
		bHostAudioMeters = S_OK == pIsid->HasCapability( CAP_AUDIO_METERS );


	if ( bHostAudioMeters && meMeterOn == GetStateForID( stMeterMode ) )
	{
		m_pParamRotary->SetPointerType( 0x06 );	// VU meter style
//		m_pParamRotary->SetPointerType( 0x02 );	// perfect for VU - but right justified!!!

		m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_AUDIO_METER, m_pSurface->GetInstanceID() );
		m_pParamRotary->SetIsEnabled( TRUE );

		// if our strip is really changing, we may need to release the  meter for the
		// previous strip
		bool bChanged = m_mixerStripMeter != m_mixerStrip || m_dwStripMeter != m_dwStripNum;
		if ( bChanged )
		{
			// if the old strip was valid, resign that meter by setting its value to 0
			if ( (DWORD)-1 != m_dwStripMeter )
			{
				pIMixer->SetMixParam( m_mixerStrip, m_dwStripMeter, MIX_PARAM_AUDIO_METER, m_pSurface->GetInstanceID(), 0.0f, MIX_TOUCH_NORMAL );
			}
		}
		// and grab a new meter by setting its value to 1
		pIMixer->SetMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_AUDIO_METER, m_pSurface->GetInstanceID(), 1.0f, MIX_TOUCH_NORMAL );

		// remember the current strip type and nuber that we're metering on
		m_mixerStripMeter = m_mixerStrip;
		m_dwStripMeter = m_dwStripNum;

		// for metering, we want every refresh
		m_pParamRotary->SetRefreshFrequency( CBaseMixParam::RR_EVERY );
	}
	else
	{
		// resign a meter in the host
		if ( bHostAudioMeters )
			pIMixer->SetMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_AUDIO_METER, m_pSurface->GetInstanceID(), 0.0f, MIX_TOUCH_NORMAL );

		m_dwStripMeter = (DWORD)-1;

		// the rotaries are unique in that they can control plug in parameters as well
		switch (GetStateForID( stInsertEditMode ))
		{
		case ieEngaged:
			{
				// set the pointer to a "bar style" pointer.
				m_pParamRotary->SetPointerType( 0x00 );

				SONAR_MIXER_STRIP curMixerStrip = m_pSurface->GetCurrentStripKind();
				DWORD	dwCurStripNum = m_pSurface->GetCurrentStrip();

				// map the rotaries to the current plugin parameters						
				int nFx = GetStateForID( stCurrentEffect );
				int nParam = getCurrentFxParam();

				HRESULT hr = m_pParamRotary->IsFxParamInRange( curMixerStrip, dwCurStripNum, nFx, nParam );

				if (hr == S_OK)
				{
					m_pParamRotary->SetIsEnabled( TRUE );
					m_pParamRotary->SetFxParam( curMixerStrip, dwCurStripNum, nFx, nParam );
				}
				else
				{
					m_pParamRotary->SetIsEnabled( FALSE );
				}
			}
			break;

		case ieDisengaged:
			{
			int nContainerState = GetStateForID( stContainerClass );
			int nRotaryMapping = nContainerState == ccTracks ? GetStateForID( stTrackRotaryMapping ) : GetStateForID( stBusRotaryMapping );

			
			// map the rotaries to the current container's assigned parameter
			if ( ccTracks == nContainerState || ccBus == nContainerState )
			{
				// Tracks or Buses
				if ( trPan == nRotaryMapping || buPan == nRotaryMapping )
				{
					// set the pointer to a "pan style" pointer.
					m_pParamRotary->SetPointerType( 0x03 );
					m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_PAN, 0 );
					m_pParamRotary->SetIsEnabled( TRUE );
				}
	#if 0		// ugh!   no Input Pan parameter?
				else if ( buInputPan == nRotaryMapping )
				{
					m_pParamRotary->SetPointerType( 0x03 );
					m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_INPUT_PAN, 0 );
					m_pParamRotary->SetIsEnabled( TRUE );
				}
	#endif
				else
				{
					if (isMidiTrack())
					{
						if ( ( ( nRotaryMapping == trSendBase ) || ( nRotaryMapping == trSendBase + 1 ) )
							&& ( GetStateForID( stSendPanOrLevel ) == sendLevel ) )
						{
							// set the pointer to a "level" pointer.
							m_pParamRotary->SetPointerType( 0x00 );

							// this will result in mapping to chorus and reverb in a MIDI track
							m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_VOL, ( nRotaryMapping - trSendBase ) );
							m_pParamRotary->SetIsEnabled( TRUE );
						}
						else
						{
							m_pParamRotary->SetPointerType( GetStateForID( stSendPanOrLevel ) == sendLevel ? 0x00 : 0x03 );
							m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_VOL, ( nRotaryMapping - trSendBase ) );
							m_pParamRotary->SetIsEnabled( FALSE );
						}
					}
					else
					{
						// if it is audio:
						if (GetStateForID( stSendPanOrLevel ) == sendLevel)
						{
							// set the pointer to a "level" pointer.
							m_pParamRotary->SetPointerType( 0x00 );
							m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_VOL, ( nRotaryMapping - trSendBase ) );
						}
						else
						{
							// set the pointer to a "pan style" pointer.
							m_pParamRotary->SetPointerType( 0x03 );
							m_pParamRotary->SetParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_SEND_PAN, ( nRotaryMapping - trSendBase ) );
						}
						m_pParamRotary->SetIsEnabled( TRUE );
					}
				}
			}
			else
			{
				// Mains
				m_pParamRotary->SetPointerType( 0x03 );
				m_pParamRotary->SetIsEnabled( FALSE );
			}

			break;
			}
		default:
			_ASSERT( 0 ); // unknown insert edit mode.
		}

		// set up rotaries to refresh on even/odd refreshes based on strip count
		const CBaseMixParam::ERefreshRate rr = (m_dwStripNum % 2 == 0) ? CBaseMixParam::RR_EVEN : CBaseMixParam::RR_ODD ;
		m_pParamRotary->SetRefreshFrequency( rr );
	}
}



/////////////////////////////////////////////////////////////////////////
CMMStrip::~CMMStrip()
{
	delete m_pParamFader;
	delete m_pParamMute;
	delete m_pParamSolo;
	delete m_pParamMulti;
	delete m_pParamRecRdy;
	delete m_pParamRotary;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMMStrip::isMidiTrack()
{
	return m_pSurface->GetIsStripMidiTrack( m_mixerStrip, m_dwStripNum );
}

/////////////////////////////////////////////////////////////////////////
DWORD CMMStrip::getCurrentFxParam()
{
	return GetStateForID( stBaseEffectParam ) + m_dwHWStripNum;
}

/////////////////////////////////////////////////////////////////////////
DWORD CMMStrip::getCurrentFx()
{
	return GetStateForID( stCurrentEffect );
}

/////////////////////////////////////////////////////////////////////////