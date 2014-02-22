/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkParams.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CHostNotifyMulticaster:
/////////////////////////////////////////////////////////////////////////
void CHostNotifyMulticaster::add( CHostNotifyListener *pListener )
{
	m_setListeners.insert(HostListenerSet::value_type( pListener ));
}

/////////////////////////////////////////////////////////////////////////
void CHostNotifyMulticaster::remove( CHostNotifyListener *pListener )
{
	HostListenerSetIt itSS = m_setListeners.find( pListener );
	if (itSS != m_setListeners.end())
	{
		m_setListeners.erase( itSS );
	}
}

/////////////////////////////////////////////////////////////////////////
void CHostNotifyMulticaster::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	HostListenerSetIt itSS;

	// for all the Listeners in our "set" notify if the flag mask matches up.
	for (itSS = m_setListeners.begin(); itSS != m_setListeners.end(); itSS++)
	{
		CHostNotifyListener* pListener = *itSS;
		DWORD fdwCurRefresh = pListener->GetRefreshFlags();
		if ((fdwCurRefresh & fdwRefresh) || dwCookie == (DWORD)pListener)
			pListener->onHostNotify( fdwRefresh, dwCookie );
	}

	m_pSurface->GetStateMgr()->deliverPostedStateChanges();
}

/////////////////////////////////////////////////////////////////////////
// CHostNotifyListener:
/////////////////////////////////////////////////////////////////////////
CHostNotifyListener::CHostNotifyListener(
	DWORD fdwRefresh,
	CControlSurface *pSurface
	) :
	m_pSurface( pSurface ),
	m_bExpectingCookie( FALSE ),
	m_nHostNotifyCount( 0 )
{
	// remember what we care to be refreshed about
	m_fdwRefresh = fdwRefresh;

	// register for notification
	m_pSurface->GetHostNotifyMulticaster()->add( this );
}

/////////////////////////////////////////////////////////////////////////
CHostNotifyListener::~CHostNotifyListener()
{
	// we are no longer needed?
	// unregister for notification
	m_pSurface->GetHostNotifyMulticaster()->remove( this );
}

/////////////////////////////////////////////////////////////////////////
void CHostNotifyListener::onHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	// if the refresh is my cookie, but I am not expecting it, ignore it.
	if (dwCookie == (DWORD)this)
	{
		if (m_bExpectingCookie)
		{
			OnHostNotify( fdwRefresh, dwCookie );
			m_bExpectingCookie = FALSE;
		}
	}
	else
	{
		// cookie is different from this
		if (dwCookie == 0 && !m_bExpectingCookie)
			OnHostNotify( fdwRefresh, dwCookie );
	}

	m_nHostNotifyCount++;
	if ( INT_MAX == m_nHostNotifyCount )
		m_nHostNotifyCount = 0;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CHostNotifyListener::RequestRefresh()
{
	if (m_bExpectingCookie)
		return S_FALSE;

	CComPtr<ISonarProject> pSonarProject;

	HRESULT hr = m_pSurface->GetSonarProject( &pSonarProject );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	hr = pSonarProject->RequestRefresh( GetCookie() );
	m_bExpectingCookie = TRUE;

	return hr;
}

/////////////////////////////////////////////////////////////////////////
// CBaseMixParam:
/////////////////////////////////////////////////////////////////////////
CBaseMixParam::CBaseMixParam( CControlSurface *pSurface ) :
	CHostNotifyListener( REFRESH_F_MIXER, pSurface ),
	m_bIsEnabled( TRUE ),
	m_mixerStrip( MIX_STRIP_TRACK ),
	m_dwStripNum( 0 ),
	m_mixerParam( MIX_PARAM_VOL ),
	m_dwParamNum( 0 ),
	m_pMsgValueScribble( NULL ),
	m_bIsValueScribbleEnabled( TRUE ),
	m_pMsgLabelScribble( NULL ),
	m_bIsLabelScribbleEnabled( TRUE ),
	m_szAlternateLabel( NULL ),
	m_eRefreshRate( RR_EVERY ),
	m_pSurface( pSurface )
{
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetContainer( DWORD dwStripNum )
{
	CCriticalSectionAuto lock( m_cs );

	// change only the container number...
	if (IsStripInRange( m_mixerStrip, dwStripNum ) != S_OK)
		return E_INVALIDARG;

	m_dwStripNum = dwStripNum;

	HRESULT hr = RequestRefresh();
	_ASSERT( SUCCEEDED( hr ) );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetContainer( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( mixerStrip, dwStripNum, m_mixerParam, m_dwParamNum );

	m_mixerStrip = mixerStrip;
	m_dwStripNum = dwStripNum;
	
	if (hrParamIsValid != S_OK)
		return S_FALSE;
	else
	{
		HRESULT hr = RequestRefresh();
		_ASSERT( SUCCEEDED( hr ) );

		return S_OK;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetParam(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, mixerParam, dwParamNum );

	m_mixerStrip = mixerStrip;
	m_dwStripNum = dwStripNum;
	m_mixerParam = mixerParam;
	m_dwParamNum = dwParamNum;

	if (hrParamIsValid != S_OK)
		return S_FALSE;
	else
	{
		HRESULT hr = RequestRefresh();
		_ASSERT( SUCCEEDED( hr ) );

		return S_OK;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetFxParam(
	SONAR_MIXER_STRIP mixerStrip,
	DWORD dwStripNum,
	WORD wFxNum,
	WORD wFxParamNum
	)
{
	return SetParam(
		mixerStrip,
		dwStripNum,
		MIX_PARAM_FX_PARAM,
		MAKELONG( wFxNum, wFxParamNum )
	);
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetIsArmed( BOOL bIsArmed )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

	if (hrParamIsValid != S_OK)
		return E_FAIL;

	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	hr = pSonarMixer->SetArmMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, bIsArmed );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::GetIsArmed( BOOL *pbIsArmed )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

	if (hrParamIsValid != S_OK)
		return E_FAIL;

	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	hr = pSonarMixer->GetArmMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, pbIsArmed );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::Snapshot()
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

	if (hrParamIsValid != S_OK)
		return E_FAIL;

	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	hr = pSonarMixer->SnapshotMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
BOOL CBaseMixParam::IsMidiTrack( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	return m_pSurface->GetIsStripMidiTrack( mixerStrip, dwStripNum );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::IsStripInRange( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hr;

	CComPtr<ISonarMixer> pSonarMixer;
	hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return hr;
		
	DWORD dwCount = 0;
	pSonarMixer->GetMixStripCount( mixerStrip, &dwCount );
	// TODO: are parameters ZERO based?
	if (dwStripNum >= dwCount)
		return S_FALSE;
	else
		return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::IsFxParamInRange(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		WORD wFxNum,
		WORD wFxParamNum
	)
{
	CCriticalSectionAuto lock( m_cs );

	BOOL bParamIsValid = FALSE;

	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
	{
		_ASSERT( 0 ); // could not get pointer to ISonarMixer.
		return E_UNEXPECTED;
	}

	float fMax = 0;
	pSonarMixer->GetMixParam( mixerStrip, dwStripNum, MIX_PARAM_FX_COUNT, 0, &fMax );

	// is the FX num within range?
	if (0 <= wFxNum && wFxNum < (WORD)fMax)
	{
		pSonarMixer->GetMixParam( mixerStrip, dwStripNum, MIX_PARAM_FX_PARAM_COUNT, wFxNum, &fMax );

		// is the fxParamNum for THIS effect within range?
		if (0 <= wFxParamNum && wFxParamNum < (WORD)fMax)
			bParamIsValid = TRUE;
	}

	if (bParamIsValid)
		return S_OK;
	else
		return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	// for dynamic params, almost anything goes
	if ( MIX_PARAM_DYN_MAP == mixerParam )
		return S_OK;

	CCriticalSectionAuto lock( m_cs );

	if (IsStripInRange( mixerStrip, dwStripNum ) != S_OK)
	{
		return S_FALSE;
	}

	// validate parameter
	BOOL bParamIsValid = FALSE;

	switch(mixerStrip)
	{
	case MIX_STRIP_TRACK:
		{
			float fIsMidi = 0;

			CComPtr<ISonarMixer> pSonarMixer;
			HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (FAILED( hr ))
				return E_UNEXPECTED;

			hr = pSonarMixer->GetMixParam( mixerStrip, dwStripNum, MIX_PARAM_IS_MIDI, 0, &fIsMidi );

			if (FAILED( hr ))
				return hr;

			if (fIsMidi > 0.5) // if the track is MIDI
			{
				// include "MIDI only" parameters.
				switch (mixerParam)
				{
				case MIX_PARAM_BANK:
				case MIX_PARAM_PATCH:
				case MIX_PARAM_KEY_OFFSET:
				case MIX_PARAM_VEL_OFFSET:
				case MIX_PARAM_MIDI_METER:
					bParamIsValid = TRUE;
					break;

				case MIX_PARAM_SEND_VOL:
					bParamIsValid = (dwParamNum == 0 || dwParamNum == 1);
					break;
				}
			}
			else // (track must be audio)
			{
				// include "audio only" parameters.
				switch (mixerParam)
				{
				case MIX_PARAM_VOL_TRIM:
				case MIX_PARAM_PHASE:
				case MIX_PARAM_INTERLEAVE:
				case MIX_PARAM_SEND_ENABLE:
				case MIX_PARAM_SEND_MUTE:
				case MIX_PARAM_SEND_VOL:
				case MIX_PARAM_SEND_PAN:
				case MIX_PARAM_SEND_PREPOST:
				case MIX_PARAM_AUDIO_METER:
					bParamIsValid = TRUE;
					break;
				case MIX_PARAM_FX_PARAM:
					bParamIsValid = S_OK == IsFxParamInRange( mixerStrip, dwStripNum, LOWORD( dwParamNum ), HIWORD( dwParamNum ) );
				}
			}
			
			switch (mixerParam)
			{
			// any track
			case MIX_PARAM_VOL:
			case MIX_PARAM_PAN:
			case MIX_PARAM_MUTE:
			case MIX_PARAM_SOLO:
			case MIX_PARAM_ARCHIVE:
			case MIX_PARAM_RECORD_ARM:
			case MIX_PARAM_INPUT_ECHO:
			case MIX_PARAM_INPUT_MAX:
			case MIX_PARAM_INPUT:
			case MIX_PARAM_OUTPUT_MAX:
			case MIX_PARAM_OUTPUT:
			case MIX_PARAM_SELECTED:
				bParamIsValid = TRUE;
			}
		}
		break;

	case MIX_STRIP_BUS:
		{
			switch (mixerParam)
			{
			case MIX_PARAM_MUTE:
			case MIX_PARAM_SOLO:
			case MIX_PARAM_SEND_VOL:
			case MIX_PARAM_SEND_PAN:
			case MIX_PARAM_VOL:
			case MIX_PARAM_PAN:
			case MIX_PARAM_SEND_ENABLE:
			case MIX_PARAM_SEND_PREPOST:

			case MIX_PARAM_OUTPUT_MAX:
			case MIX_PARAM_OUTPUT:

			case MIX_PARAM_AUDIO_METER:
				bParamIsValid = TRUE;
				break;

			case MIX_PARAM_FX_PARAM:
				bParamIsValid = S_OK == IsFxParamInRange( mixerStrip, dwStripNum, LOWORD( dwParamNum ), HIWORD( dwParamNum ) );
				break;
			}

		}
		break;

	case MIX_STRIP_MASTER:
		{
			switch (mixerParam)
			{
			case MIX_PARAM_VOL:
			case MIX_PARAM_MUTE:
			case MIX_PARAM_AUDIO_METER:
				bParamIsValid = TRUE;
			}
		}
		break;
	}

	if (bParamIsValid)
		return S_OK;
	else
		return S_FALSE;
}


/////////////////////////////////////////////////////////////////////////
void CBaseMixParam::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto lock( m_cs );
	
	maybeRefreshParam();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetMixParamValue( float fValue, SONAR_MIXER_TOUCH smtTouch )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

	if (hrParamIsValid != S_OK)
		return hrParamIsValid;

	HRESULT hrRet = S_OK;
	if ( S_OK == m_pSurface->ParamInContext( m_mixerParam ) )
	{
		// respond to the value change
		CComPtr<ISonarMixer> pSonarMixer;
		HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
		if (FAILED( hr ))
			return E_UNEXPECTED;

		hrRet = pSonarMixer->SetMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, fValue, smtTouch );

		hr = RequestRefresh();
		_ASSERT( SUCCEEDED( hr ) );

		m_pSurface->GetLastParamChange()->setLastParamID( this, m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );
	}
	return hrRet;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::GetSonarParamValue( float *pfValue )
{
	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return hr;

	hr = pSonarMixer->GetMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, pfValue );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetIsEnabled( BOOL bIsEnabled )
{
	if (bIsEnabled != m_bIsEnabled)
	{
		m_bIsEnabled = bIsEnabled;

		if (bIsEnabled)
		{
			HRESULT hr = RequestRefresh();
			_ASSERT( SUCCEEDED( hr ) );
		}
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::RequestRefresh()
{
	return CHostNotifyListener::RequestRefresh();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::setValue( CMidiMsg *pMsg, float fVal )
{
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::setValue( CMidiMsg *pMsg, BOOL bVal )
{
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::getValueType( EValueType eType )
{
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
void CBaseMixParam::maybeRefreshParam( float *pfValue /* =NULL */ )
{
	CCriticalSectionAuto lock( m_cs );	
	
	// maybe filter out this refresh based on Even/Odd/Mod refresh count
	if ( RR_ODD == m_eRefreshRate )
	{
		if ( m_nHostNotifyCount % 2 != 0 )
			return;
	}
	else if ( RR_EVEN == m_eRefreshRate )
	{
		if ( m_nHostNotifyCount % 2 == 0 )
			return;
	}
	else if ( RR_MODBASE <= m_eRefreshRate )
	{
		int nMod = m_eRefreshRate - RR_MODBASE + 1;
		if ( m_nHostNotifyCount % nMod == 0 )
			return;
	}


	if (m_bIsEnabled)
	{	
		HRESULT hrParamIsValid = IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum );

		if (hrParamIsValid == S_OK)
		{
			refreshParam( pfValue );
		}
		else
		{
			float fVal = 0;
			refreshParam( &fVal );
		}
	}
}

////////////////////////////////////////////////////////////////////////
// Parameters can be given a Refresh Rate of RR_EVEN, RR_ODD,
// RR_EVERY, or RR_MODBASE + n.  This can be used to "interlace"
// or otherwise thin out refresh requests to the host.  Typically
// non motorized parameter updates can be refreshed less frequently
// than on every refresh with no noticeable loss in response.
// RR_EVERY (the default) will update only on every refresh.
// RR_EVEN and RR_ODD will update only on even/odd refreshes
// RR_MOBASE can be used to update only on every Nth refresh
// where N is added to RR_MODBASE.  For example, to update on every
// 5th refresh, call SetRefreshFrequency( RR_MODBASE + 5 )
HRESULT CBaseMixParam::SetRefreshFrequency( ERefreshRate rr )
{
	m_eRefreshRate = rr;
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////
HRESULT CBaseMixParam::SetAlternateLabel( const char *szData )
{
	if (m_szAlternateLabel)
	{
		::CoTaskMemFree( m_szAlternateLabel );
		m_szAlternateLabel = NULL;
	}

	if (szData == NULL)
		return S_FALSE;

	DWORD dwLen = (DWORD)strlen( szData );

	if (dwLen == 0)
		return S_FALSE;
	
	m_szAlternateLabel = (char*)::CoTaskMemAlloc( dwLen + 1 ); // allocate space for string plus NULL

	_ASSERT( m_szAlternateLabel );
	if (!m_szAlternateLabel)
		return E_OUTOFMEMORY;

	strncpy( m_szAlternateLabel, szData, dwLen );
	m_szAlternateLabel[ dwLen ] = 0; // put a null terminator

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CBaseMixParam::renderScribbles( float fValue )
{
	if (m_pMsgValueScribble && m_bIsValueScribbleEnabled)
	{
		char szParamText[64];
		DWORD dwLength = 64;

		CComPtr<ISonarMixer> pSonarMixer;
		HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
		if (SUCCEEDED( hr ))
		{
			hr = pSonarMixer->GetMixParamValueText( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, fValue, szParamText, &dwLength );

			if (SUCCEEDED( hr ))
				m_pMsgValueScribble->SendText( (DWORD)strlen( szParamText ), szParamText );
			else
			{
				m_pMsgValueScribble->SendText( 3, "n/a" );
			}
		}
	}
	if (m_pMsgLabelScribble && m_bIsLabelScribbleEnabled)
	{
		char szParamText[64];
		DWORD dwLength = 64;

		HRESULT hr = E_FAIL;
		if (m_szAlternateLabel)
		{
			strcpy( szParamText, m_szAlternateLabel );
			hr = S_OK;
		}
		else
		{		
			CComPtr<ISonarMixer> pSonarMixer;
			hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (SUCCEEDED( hr ))
			{
				hr = pSonarMixer->GetMixParamLabel( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, szParamText, &dwLength );
			}
		}

		if (SUCCEEDED( hr ))
			m_pMsgLabelScribble->SendText( (DWORD)strlen( szParamText ), szParamText );
		else
			m_pMsgLabelScribble->SendText( 3, "n/a" );

	}
}

/////////////////////////////////////////////////////////////////////////
void CBaseMixParam::ZeroMidiState()
{
	if (m_pMsgValueScribble && m_bIsValueScribbleEnabled)
		m_pMsgValueScribble->SendText( 3, "n/a" );

	if (m_pMsgLabelScribble && m_bIsLabelScribbleEnabled)
		m_pMsgLabelScribble->SendText( 3, "n/a" );
}

/////////////////////////////////////////////////////////////////////////
// CMixParamFloat:
/////////////////////////////////////////////////////////////////////////
CMixParamFloat::CMixParamFloat(
		CControlSurface *pSurface,
		CMidiMsg *pMsgInput,
		CMidiMsg *pMsgOutput /*= NULL */,
		CMidiMsg *pMsgCapture /* = NULL */, DWORD dwCaptureVal /* = VAL_ANY */,
		CMidiMsg *pMsgRelease /* = NULL */, DWORD dwReleaseVal /* = VAL_ANY */
		):
	CBaseMixParam( pSurface ),
	m_bIsTouched( FALSE )
{
	_ASSERT( pMsgInput );

	m_pMsgIn = pMsgInput;

	m_pMsgOut = pMsgOutput;

	m_trigCapture.m_pMsg = pMsgCapture;
	m_trigCapture.m_dwVal = dwCaptureVal;

	m_trigRelease.m_pMsg = pMsgRelease;
	m_trigRelease.m_dwVal = dwReleaseVal;

	if (pMsgOutput == NULL)
	{
		// message is not motorized. Enable nulling:
		SetUseNulling( TRUE );
	}

	// tell this message that "this" cares for input.
	HRESULT hr = pMsgInput->AddListener( this );
	_ASSERT( SUCCEEDED( hr ));

	if (pMsgCapture)
	{
		hr = pMsgCapture->AddListener( this );
		_ASSERT( SUCCEEDED( hr ));
	}

	if (pMsgRelease)
	{
		hr = pMsgRelease->AddListener( this );
		_ASSERT( SUCCEEDED( hr ));
	}

	// output message is, obviously, not meant for input,
	// therefore, we do not call SetListener.
}

/////////////////////////////////////////////////////////////////////////
CMixParamFloat::~CMixParamFloat()
{
	// unregister for listening
	HRESULT hr;

	hr = m_pMsgIn->RemoveListener( this );
	_ASSERT( SUCCEEDED( hr ));

	if (m_trigCapture.m_pMsg != NULL)
	{
		hr = m_trigCapture.m_pMsg->RemoveListener( this );
		_ASSERT( SUCCEEDED( hr ));
	}

	if (m_trigRelease.m_pMsg != NULL)
	{
		hr = m_trigRelease.m_pMsg->RemoveListener( this );
		_ASSERT( SUCCEEDED( hr ));
	}
}

/////////////////////////////////////////////////////////////////////////
void CMixParamFloat::Invalidate()
{
	if (m_pMsgOut)
		m_pMsgOut->Invalidate();

	CBaseMixParam::Invalidate();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::IsParamFloat( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum )
{
	switch (mixerParam)
	{
	case MIX_PARAM_VOL:
	case MIX_PARAM_PAN:
	case MIX_PARAM_VOL_TRIM:
	case MIX_PARAM_SEND_VOL:
	case MIX_PARAM_SEND_PAN:
	case MIX_PARAM_AUDIO_METER:
	case MIX_PARAM_MIDI_METER:
	case MIX_PARAM_FX_PARAM:
		return S_OK;
	default:
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	if (CBaseMixParam::IsParamValid( mixerStrip, dwStripNum, mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	if (IsParamFloat( mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::setValue( CMidiMsg *pMsg, float fVal )
{
	// handle value changes (determined by parametrized MIDI message)
	if (m_bIsEnabled == FALSE)
		return S_FALSE;

	if (pMsg == m_pMsgIn)
	{
		fVal = applyInputMapping( fVal );
		HRESULT hr = SetMixParamValue( fVal,
			( ( hasTouchMsgs() && m_bParamIsTouchable ) ? MIX_TOUCH_MANUAL : MIX_TOUCH_TIMEOUT)
			);

		return hr;
	}
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::SetParam(
	SONAR_MIXER_STRIP mixerStrip,
	DWORD dwStripNum,
	SONAR_MIXER_PARAM mixerParam,
	DWORD dwParamNum
)
{
	if (mixerStrip == m_mixerStrip &&
							dwStripNum == m_dwStripNum &&
							mixerParam == m_mixerParam &&
							dwParamNum == m_dwParamNum)
		return S_FALSE;

	CCriticalSectionAuto lock( m_cs );

	HRESULT hr = E_FAIL;

	if (m_bIsTouched)
	{
		return E_FAIL;
	}

	return CBaseMixParam::SetParam( mixerStrip, dwStripNum, mixerParam, dwParamNum );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	CCriticalSectionAuto lock( m_cs );

	// handle capture messages (events triggered by specific MIDI message and value)
	if (m_bIsEnabled == FALSE)
		return S_FALSE;

	if (!hasTouchMsgs())
		return S_FALSE;

	if (m_trigCapture.Test( pMsg, dwVal ))
	{
		HRESULT hr = Touch( TRUE );
		if (hr != S_OK)
			m_bParamIsTouchable = FALSE;
		else
		{
			m_bParamIsTouchable = TRUE;
			m_bIsTouched = TRUE;
		}
		return hr;
	}

	if (m_trigRelease.Test( pMsg, dwVal ) && m_bParamIsTouchable)
	{
		m_bIsTouched = FALSE;
		return Touch( FALSE );
	}
	
	return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMixParamFloat::hasTouchMsgs()
{
	if (m_trigCapture.m_pMsg == NULL)
		return FALSE;
	if (m_trigRelease.m_pMsg == NULL)
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamFloat::Touch( BOOL bTouch )
{
	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return E_UNEXPECTED;

	hr = pSonarMixer->TouchMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, bTouch );
	return hr;
}

/////////////////////////////////////////////////////////////////////////
void CMixParamFloat::refreshParam( float *pfVal /* = NULL */)
{
	// update the local parameter, and maybe send it out.
	float fValue = 0;

	HRESULT hr = E_FAIL;

	if (!pfVal)
		hr = GetSonarParamValue( &fValue );
	else
	{
		fValue = *pfVal;
		hr = S_OK;
	}

	if (FAILED( hr ))
	{
		ZeroMidiState();
		return;
	}

	renderParam( fValue );

	// update the CMidiMsgListener
	if (m_pMsgIn)
	{
		setLastValue( fValue, m_pMsgIn );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMixParamFloat::renderParam( float fValue )
{
	float fMappedValue = applyOutputMapping( fValue );

	if (m_pMsgOut != NULL)
	{
		m_pMsgOut->Send( fMappedValue ); // TODO: we may need echo suppression here.
	}

	CBaseMixParam::renderParam( fValue );
}
/////////////////////////////////////////////////////////////////////////
// CMixParamBool:
/////////////////////////////////////////////////////////////////////////
#if		TRUE == (-1)
#define BOOL_UNDEFINED  (TRUE - 1)
#else
#define BOOL_UNDEFINED  (TRUE + 1)
#endif

CMixParamBool::CMixParamBool(
		CControlSurface *pSurface,
		CMidiMsg *pMsgIn, DWORD dwInOnVal, DWORD dwInOffVal,
		CMidiMsg *pMsgOutOn /* = NULL */, DWORD dwOutOnVal /* = VAL_ANY */,
		CMidiMsg *pMsgOutOff /* = NULL */, DWORD dwOutOffVal /* = VAL_ANY */
		) :
	m_pMsgOutOff( NULL ),
	m_pMsgOutOn( NULL ),
	m_bLastValue( BOOL_UNDEFINED ),
	CBaseMixParam( pSurface )
{
	_ASSERT( pMsgIn != NULL );

	m_pMsgIn = pMsgIn;

	HRESULT hr = m_pMsgIn->AddListener( this );
	_ASSERT( SUCCEEDED( hr ));

	if (dwInOnVal != dwInOffVal)
	{
		SetEffect( mdOnOff );
		SetOnOffValues( dwInOnVal, dwInOffVal );
	}
	else
	{
		SetEffect( mdToggle );
		SetToggleValue( dwInOnVal );
	}

	// initialize pointers to output messages
	if (pMsgOutOn)
	{
		m_pMsgOutOn = pMsgOutOn;
		m_dwOutOnVal = dwOutOnVal;
	}

	if (pMsgOutOff)
	{
		m_pMsgOutOff = pMsgOutOff;
		m_dwOutOffVal = dwOutOffVal;
	}

	if ( ( pMsgOutOn != pMsgOutOff ) && pMsgOutOn && pMsgOutOff )
	{
		// because different messages are used for on and off, we would not
		// want to use thinning.
		pMsgOutOn->SetIsOutputThin( FALSE );
		pMsgOutOff->SetIsOutputThin( FALSE );
	}
}

/////////////////////////////////////////////////////////////////////////
CMixParamBool::~CMixParamBool()
{
	// unregister for listening
	HRESULT hr = m_pMsgIn->RemoveListener( this );
	_ASSERT( SUCCEEDED( hr ));
}

/////////////////////////////////////////////////////////////////////////
void CMixParamBool::Invalidate()
{
	if (m_pMsgOutOn)
		m_pMsgOutOn->Invalidate();

	if (m_pMsgOutOff)
		m_pMsgOutOff->Invalidate();

	CBaseMixParam::Invalidate();

	m_bLastValue = BOOL_UNDEFINED;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamBool::IsParamBool( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum )
{
	switch (mixerParam)
	{
	case MIX_PARAM_MUTE:
	case MIX_PARAM_SOLO:
	case MIX_PARAM_ARCHIVE:
	case MIX_PARAM_RECORD_ARM:
	case MIX_PARAM_PHASE:
	case MIX_PARAM_IS_MIDI:
	case MIX_PARAM_SELECTED:
	case MIX_PARAM_SEND_ENABLE:
	case MIX_PARAM_SEND_MUTE:
	case MIX_PARAM_SEND_PREPOST:
		return S_OK;
	default:
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamBool::IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	if (CBaseMixParam::IsParamValid( mixerStrip, dwStripNum, mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	if (IsParamBool( mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamBool::setValue( CMidiMsg *pMsg, BOOL bVal )
{
	if (m_bIsEnabled == FALSE)
		return S_FALSE;

	HRESULT hr = SetMixParamValue( float((bVal == TRUE) ? 1 : 0), MIX_TOUCH_NORMAL );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
void CMixParamBool::refreshParam( float *pfVal /* = NULL */)
{
	// update the local parameter, and maybe send it out.
	float fValue = 0;

	HRESULT hr = E_FAIL;

	if (!pfVal)
		hr = GetSonarParamValue( &fValue );
	else
	{
		fValue = *pfVal;
		hr = S_OK;
	}

	if (FAILED( hr ))
	{
		ZeroMidiState();
		return;
	}

	renderParam( fValue );

	// update the CMidiMsgListener
	setLastValue( fValue > 0.5 );
}

/////////////////////////////////////////////////////////////////////////
void CMixParamBool::renderParam( float fValue )
{
	// we avoid sending the same value repeatedly if there has been no change.
	if (fValue > 0.5) // is the value "ON"?
	{
		if (m_pMsgOutOn != NULL && (m_bLastValue != TRUE))
			m_pMsgOutOn->Send( m_dwOutOnVal );
		m_bLastValue = TRUE;
	}
	else
	{
		if (m_pMsgOutOff != NULL && (m_bLastValue != FALSE))
			m_pMsgOutOff->Send( m_dwOutOffVal );
		m_bLastValue = FALSE;
	}

	CBaseMixParam::renderParam( fValue );
}

/////////////////////////////////////////////////////////////////////////
// CMixParamEnum:
/////////////////////////////////////////////////////////////////////////
CMixParamEnum::CMixParamEnum(
		CControlSurface *pSurface,
		CMidiMsg *pMsg,
		DWORD dwIncVal,
		DWORD dwDecVal,
		DWORD dwLgeIncVal /* = VAL_ANY */, DWORD dwLgeIncAmount /* = 0*/,
		DWORD dwLgeDecVal /* = VAL_ANY */, DWORD dwLgeDecAmount /* = 0*/
	):
	CBaseMixParam( pSurface )
{
	_ASSERT( pMsg != NULL ); // message is required

	m_pMsgIn = pMsg;

	SetEffect( CMidiMsgListener::mdDelta );
	HRESULT hr = m_pMsgIn->AddListener( this );
	_ASSERT( SUCCEEDED( hr ) );

	AddDelta( dwIncVal, 1 );
	AddDelta( dwDecVal, -1 );

	if (dwLgeIncVal != VAL_ANY)
		AddDelta( dwLgeIncVal, dwLgeIncAmount );

	if (dwLgeDecVal != VAL_ANY)
		AddDelta( dwLgeDecVal, 0 - dwLgeDecAmount );

}

/////////////////////////////////////////////////////////////////////////
CMixParamEnum::~CMixParamEnum()
{
	// unregister for listening
	HRESULT hr = m_pMsgIn->RemoveListener( this );
	_ASSERT( SUCCEEDED( hr ) );
}

/////////////////////////////////////////////////////////////////////////
void CMixParamEnum::Invalidate()
{
	CBaseMixParam::Invalidate();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamEnum::IsParamEnum( SONAR_MIXER_PARAM mixerParam, DWORD dwParamNum )
{
	switch (mixerParam)
	{
	case MIX_PARAM_INTERLEAVE:
	case MIX_PARAM_INPUT_MAX:
	case MIX_PARAM_INPUT:
	case MIX_PARAM_OUTPUT_MAX:
	case MIX_PARAM_OUTPUT:
	case MIX_PARAM_BANK:
	case MIX_PARAM_PATCH:
	case MIX_PARAM_KEY_OFFSET:
	case MIX_PARAM_VEL_OFFSET:
		return S_OK;
	default:
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamEnum::IsParamValid(
		SONAR_MIXER_STRIP mixerStrip,
		DWORD dwStripNum,
		SONAR_MIXER_PARAM mixerParam,
		DWORD dwParamNum
	)
{
	if (CBaseMixParam::IsParamValid( mixerStrip, dwStripNum, mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	if (IsParamEnum( mixerParam, dwParamNum ) != S_OK)
		return S_FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamEnum::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	if (m_bIsEnabled == FALSE)
		return S_FALSE;

	HRESULT hr = SetMixParamValue( (float)(int /*force it to become signed*/)dwVal, MIX_TOUCH_NORMAL );

	return hr;
}

/////////////////////////////////////////////////////////////////////////
DWORD CMixParamEnum::getMin()
{
	// determine min based on parameter type
	if (m_bIsEnabled == FALSE)
		return 0;

	switch (m_mixerParam)
	{
	case MIX_PARAM_INTERLEAVE:
		return 0;

	case MIX_PARAM_INPUT:
	case MIX_PARAM_OUTPUT:
	case MIX_PARAM_BANK:
	case MIX_PARAM_PATCH:
		return -1;

	case MIX_PARAM_KEY_OFFSET:
	case MIX_PARAM_VEL_OFFSET:
		return -127;
		
	default:
		_ASSERT( 0 ); // unsupported parameter
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
DWORD CMixParamEnum::getMax()
{
	// determine max based on parameter type
	
	if (m_bIsEnabled == FALSE)
		return 0;

	switch (m_mixerParam)
	{
	case MIX_PARAM_INTERLEAVE:
		return 2;
	case MIX_PARAM_INPUT:
		{
			CComPtr<ISonarMixer> pSonarMixer;
			HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (FAILED( hr ))
				_ASSERT( 0 ); // could not get pointer to ISonarMixer.

			float fMax;
			pSonarMixer->GetMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_INPUT_MAX, 0, &fMax );

			return (DWORD)fMax - 1;
		}
	case MIX_PARAM_OUTPUT:
		{
			CComPtr<ISonarMixer> pSonarMixer;
			HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (FAILED( hr ))
				_ASSERT( 0 ); // could not get pointer to ISonarMixer.

			float fMax;
			pSonarMixer->GetMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_OUTPUT_MAX, 0, &fMax );

			return (DWORD)fMax - 1;
		}
	case MIX_PARAM_BANK:
		return 16383;
	case MIX_PARAM_PATCH:
		return 127;

	case MIX_PARAM_KEY_OFFSET:
	case MIX_PARAM_VEL_OFFSET:
		return 127;
		
	default:
		_ASSERT( 0 ); // unsupported parameter
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
void CMixParamEnum::refreshParam( float *pfVal /* = NULL */ )
{
	// update the local parameter, and maybe send it out.
	float fValue = 0;

	HRESULT hr = E_FAIL;

	if (!pfVal)
		hr = GetSonarParamValue( &fValue );
	else
	{
		fValue = *pfVal;
		hr = S_OK;
	}

	if (FAILED( hr ))
	{
		ZeroMidiState();
		return;
	}

	renderParam( fValue );

	// update the CMidiMsgListener
	setLastValue( (DWORD)fValue );
}

/////////////////////////////////////////////////////////////////////////
void CMixParamEnum::renderParam( float fValue )
{
	// must subclass to define output rendering beyond base behavior (scribble)
	CBaseMixParam::renderParam( fValue );
}

/////////////////////////////////////////////////////////////////////////
// CMixParamBoolEx:
/////////////////////////////////////////////////////////////////////////

BOOL CMixParamBoolEx::isRecordArm()
{
	if (m_dwParamNum == 1 && m_mixerParam == MIX_PARAM_RECORD_ARM)
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMixParamBoolEx::SetMixParamValue( float fValue, SONAR_MIXER_TOUCH smtTouch )
{
	if (isRecordArm())
	{
		CComPtr<ISonarMixer> pSonarMixer;
		HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
		_ASSERT( SUCCEEDED( hr ) );

		BOOL bArm = fValue > 0.5f;

		// use MIX_PARAM_ANY to arm all params on this strip
		return pSonarMixer->SetArmMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_ANY, 0, bArm );
	}
	else
		return CMixParamBool::SetMixParamValue( fValue, smtTouch );
}

/////////////////////////////////////////////////////////////////////////
void CMixParamBoolEx::refreshParam( float *pfValue /*= NULL*/ )
{
	if (isRecordArm())
	{
		float fValue = 0;
		CComPtr<ISonarMixer> pSonarMixer;
		HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
		_ASSERT( SUCCEEDED( hr ) );

		BOOL bArm = FALSE;
		pSonarMixer->GetArmMixParam( m_mixerStrip, m_dwStripNum, MIX_PARAM_ANY, 0, &bArm );
		fValue = bArm ? 1.0f : 0.0f;
		CMixParamBool::refreshParam( &fValue );
		if ( pfValue )
			*pfValue = fValue;
}
	else
		CMixParamBool::refreshParam( pfValue );
}

/////////////////////////////////////////////////////////////////////////