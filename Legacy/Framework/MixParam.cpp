#include "stdafx.h"


#include "MixParam.h"
#include "StringCruncher.h"
#include <math.h>

#pragma warning (disable:4100)

//--------------------------------------------------------------------------
// This is a simple Mix Parameter class which abstracts a single mixing
// parameter in the host.  The class has bindings to modify the state of any
// mixing parameter on any strip as well as any plug-in, synth, or ACT parameter
//
// Via this class you can:
// Get/Set value
// Touch/Untouch
// Write enable/disable
// Read enable/disable
//
// To use, give the class pointers to all the necessary interfaces in SetINterfaces()
// Then bind to a given parameter with the SetParams() api.
//
// This class also handles Parameter Nulling modes of "Jump", "Relative" or "Match". 
// The default mode is JUMP mode.

/////////////////////////////////////////////////////////////////////////////

CMixParam::CMixParam() :
	m_eCaptureType( CT_Jump ),
	m_fLastPosition( -1.0f ),
	m_bACTLearning( false ),
	m_wValHistory( VT_UNDEF ),
	m_fValCached( -1.f),
	m_dwSetParamTimeStamp( 0 )
{
	m_pMixer = NULL;
	m_pTransport = NULL;
	m_dwUniqueId = ILLEGAL_UNIQUE_ID;

	m_bHasBinding = false;
	m_bTouched = false;
	m_bAudioMeteringEnabled = false;

	m_eMixerStrip = MIX_STRIP_TRACK;
	m_dwStripNum = 0;
	m_eMixerParam = MIX_PARAM_VOL;
	m_dwParamNum = 0;
	m_dwStripNum = (DWORD)-1;

	m_fDefaultValue = NO_DEFAULT;

	m_bDisableWhilePlaying = false;
}

CMixParam::~CMixParam()
{
}

/////////////////////////////////////////////////////////////////////////////
// Give the parameter pointers (non addref'd) to the various host interfaces
// it needs
void CMixParam::SetInterfaces(ISonarMixer *pMixer, ISonarMixer2* pMixer2, 
										ISonarTransport *pTransport, DWORD dwUniqueId)
{
	m_pMixer = pMixer;
	m_pMixer2 = pMixer2;
	m_pTransport = pTransport;
	m_dwUniqueId = dwUniqueId;
}


/////////////////////////////////////////////////////////////////////////////
// Set the binding attributes. This identifies which host param it will control
void CMixParam::SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, 
								  SONAR_MIXER_PARAM eMixerParam, 
								  DWORD dwParamNum, 
								  float fDefault ) //= NO_DEFAULT)
{
	if (m_bHasBinding == false ||
		m_eMixerStrip != eMixerStrip || m_dwStripNum != dwStripNum ||
		m_eMixerParam != eMixerParam || m_dwParamNum != dwParamNum || fDefault != m_fDefaultValue)
	{
		bool bWasTouched = m_bTouched;

		if (bWasTouched)
			Touch(false);

		m_eMixerStrip = eMixerStrip;
		m_eMixerParam = eMixerParam;
		m_dwParamNum = dwParamNum;
		m_dwStripNum = dwStripNum;
		m_fDefaultValue = fDefault;

		if ( m_fDefaultValue != NO_DEFAULT && (m_fDefaultValue < 0.f || m_fDefaultValue > 1.f ) )
		{
			ASSERT(0);
			m_fDefaultValue = NO_DEFAULT;
		}

		if (bWasTouched)
			Touch(true);

		m_bHasBinding = true;
	}

	m_bDisableWhilePlaying = false;
}

//------------------------------------------------------------
// Convenience helper to set Fx Param attributes
void CMixParam::SetFxParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxNum, WORD wFxParamNum, float fDefault ) //= NO_DEFAULT )
{
	SetParams( mixerStrip, dwStripNum, MIX_PARAM_FX_PARAM, MAKELONG( wFxNum, wFxParamNum) ,fDefault );
}

//------------------------------------------------------------
// Convenience helper to set EQ Param attributes
void CMixParam::SetEqParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault ) //= NO_DEFAULT )
{
	SetParams( mixerStrip, dwStripNum, MIX_PARAM_FILTER_PARAM, MAKELONG( MIX_FILTER_EQ, wFxParamNum ), fDefault );
}



/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetDisableWhilePlaying(bool bDisable)
{
	m_bDisableWhilePlaying = true;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::ClearBinding()
{
	m_bHasBinding = false;
}



/////////////////////////////////////////////////////////////////////////////
// Return a shortened version of the param name
HRESULT CMixParam::GetCrunchedParamLabel(LPSTR pszText, DWORD dwLen)
{
	pszText[0] = 0;

	char szBuf[128];
	DWORD dwTmpLen = sizeof(szBuf);

	HRESULT hr = GetParamLabel(szBuf, &dwTmpLen);

	if (FAILED(hr))
		return hr;

	CStringCruncher cCruncher;

	cCruncher.CrunchString(szBuf, pszText, dwLen);

	pszText[dwLen] = 0;

	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// 
HRESULT CMixParam::GetParamLabel(LPSTR pszText, DWORD *pdwLen)
{
	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->GetMixParamLabel(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////
// Get the parameter value from the host. 
// Compare to the latest Cached value and return S_OK if changed, else
// S_FALSE;
HRESULT CMixParam::GetVal(float *fVal)
{
	if (!m_bHasBinding)
		return E_FAIL;

	float f = 0.f;
	HRESULT hr =  m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									&f );

	if ( FAILED( hr ) )
		return hr;

	hr = f == m_fValCached ? S_FALSE : S_OK;
	m_fValCached = f;
	*fVal = m_fValCached;
	
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (m_bDisableWhilePlaying && isPlaying())
		return E_FAIL;

	HRESULT hr = S_OK;

	bool bSend = true;
	float fValSend = fVal;

	// if one of the non-jump types...
	if ( CT_Jump != m_eCaptureType && !m_bACTLearning )
	{
		float fCurrent = 0.0f;
		hr = m_pMixer->GetMixParam( m_eMixerStrip, m_dwStripNum,
										m_eMixerParam, m_dwParamNum, &fCurrent );

		if ( FAILED( hr ) )
			return hr;

		// If Match mode, we will choose to set based on the value history
		if ( CT_Match == m_eCaptureType )
		{
			float fHost = 0.0f;
			DWORD dwNow = ::GetTickCount();

			// If some time has elapsed since the last Value Match,
			// make sure we're still within a value window. If not,
			// invalidate the history so we have to re-null
			if ( dwNow - m_dwSetParamTimeStamp > 200 )
			{
				// Some time has elapsed since last Match so...
				// get the host value
				hr = m_pMixer->GetMixParam( m_eMixerStrip, m_dwStripNum,
												m_eMixerParam, m_dwParamNum, &fHost );

				// If not "close enough" we assume the rug was pulled out from under this 
				// value - ether by adjusting it in the host, or by some other surface
				// or remote control
				if ( ::fabs(fVal - fHost) > 0.1f )
					ResetHistory();
			}

			bool bWasCrossed = m_wValHistory == VT_CROSSED;

			// If not Nulled, get the host's value and set our Below/Above
			// history bits accordingly. We'll ultimately only set the value
			// if we've crossed the host (ie nulled)
			if ( !bWasCrossed )
			{
				bWasCrossed = false;

				// get the host value
				hr = m_pMixer->GetMixParam( m_eMixerStrip, m_dwStripNum,
												m_eMixerParam, m_dwParamNum, &fHost );

				if ( FAILED( hr ) )
					return hr;

				// Set the bit indicating above/below the host
				if ( fVal <= fHost )
					m_wValHistory |= VT_WASBELOW;
				if ( fVal >= fHost )
					m_wValHistory |= VT_WASABOVE;
			}

			// only send if we've crossed the host
			bSend = m_wValHistory == VT_CROSSED;
		}
		else if ( CT_Relative == m_eCaptureType )
		{
			if ( 0.0f <= m_fLastPosition )
			{
				float fDelta = fVal - m_fLastPosition;
				fValSend = fCurrent + fDelta;
				fValSend = max( 0.0f, fValSend );
				fValSend = min( 1.0f, fValSend );
			}
			bSend = true;
		}
#if 0	// would be nice to get this working right
		else if ( CMixParam::CT_Converge == m_eCaptureType )
		{
			if ( 0.0f <= m_fLastPosition )
			{
				float fError = fabs(fVal - fCurrent);	// error of command to current
				float fCommandDelta = fVal - m_fLastPosition;	// change from last command
				fValSend = fCurrent + fCommandDelta * (1-fError);
				fValSend = max( 0.0f, fValSend );
				fValSend = min( 1.0f, fValSend );
			}
			bSend = true;
		}
#endif
	}

	if ( bSend )
	{
		m_dwSetParamTimeStamp = ::GetTickCount();
		hr = m_pMixer->SetMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, fValSend, eMixerTouch);
	}

	m_fLastPosition = fVal;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ToggleBooleanParam(SONAR_MIXER_TOUCH eMixerTouch)
{
	float fVal;

	HRESULT hr = GetVal(&fVal);

	if (FAILED(hr))
	{
		// If it's a dynamic control, don't worry if the GetVal()
		// failed, just set it to something so that ACT learn detects it
		if (m_eMixerParam == MIX_PARAM_DYN_MAP)
			return SetVal(1.0f, eMixerTouch);

		return hr;
	}

	return SetVal((fVal >= 0.5f) ? 0.0f : 1.0f, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetValueText(LPSTR pszText, DWORD *pdwLen)
{
	pszText[0] = 0;

	float fVal;

	HRESULT hr = GetVal(&fVal);

	if (FAILED(hr))
		return hr;

	return GetValueText(fVal, pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetValueText(float fVal, LPSTR pszText, DWORD *pdwLen)
{
	if (!m_bHasBinding)
		return E_FAIL;

	HRESULT hr = m_pMixer->GetMixParamValueText(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									fVal, pszText, pdwLen);

	if ( FAILED( hr ) )
		*pszText = '\0';

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Touch(bool bTouchState)
{
	// Record this even if we're not currently bound to anything so that if we
	// are bound after a reconfigure, we can update the touch state correctly
	m_bTouched = bTouchState;

	if (!m_bHasBinding)
		return E_FAIL;

	if ( !bTouchState )
		m_fValCached = -1.f;

	return m_pMixer->TouchMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									bTouchState ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetWrite(bool *pbArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	BOOL bArm;

	HRESULT hr = m_pMixer->GetArmMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									&bArm);

	*pbArm = (FALSE != bArm);

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetWrite(bool bArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (isPlaying())
		return E_FAIL;

	return m_pMixer->SetArmMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									bArm ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetWriteAll(bool *pbArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	BOOL bArm;

	HRESULT hr = m_pMixer->GetArmMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_ANY, 0,
									&bArm);

	*pbArm = (FALSE != bArm);

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetWriteAll(bool bArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (isPlaying())
		return E_FAIL;

	return m_pMixer->SetArmMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_ANY, 0,
									bArm ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetRead(bool* pb)
{
	if ( !m_pMixer2 )
		return E_NOINTERFACE;
	if (!m_bHasBinding)
		return E_FAIL;
	if (isPlaying())
		return E_FAIL;

	BOOL b = FALSE;
	HRESULT hr = m_pMixer2->GetReadMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum, &b);
	*pb = b ? true : false;
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetRead(bool b )
{
	if ( !m_pMixer2 )
		return E_NOINTERFACE;
	if (!m_bHasBinding)
		return E_FAIL;
	if (isPlaying())
		return E_FAIL;

	return m_pMixer2->SetReadMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, m_dwParamNum,
									b ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetReadAll( bool* pb )
{
	if ( !m_pMixer2 )
		return E_NOINTERFACE;
	if (!m_bHasBinding)
		return E_FAIL;
	if (isPlaying())
		return E_FAIL;

	BOOL b = FALSE;
	HRESULT hr = m_pMixer2->GetReadMixParam(m_eMixerStrip, m_dwStripNum,
										MIX_PARAM_ANY, 0, &b);

	*pb = b ? true : false;
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetReadAll( bool b )
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (isPlaying())
		return E_FAIL;

	return m_pMixer2->SetReadMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_ANY, 0,
									b ? TRUE : FALSE);
}


/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Adjust( float fDelta )
{
	float fVal;

	HRESULT hr = GetVal(&fVal);

	if (FAILED(hr))
		return hr;

	fVal += fDelta;

	if (fVal < 0.f)
		fVal = 0.f;
	else if (fVal > 1.f)
		fVal = 1.f;

	return SetVal(fVal, MIX_TOUCH_NORMAL);
}


/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetToDefaultValue()
{
	if (NO_DEFAULT == m_fDefaultValue)
		return E_FAIL;

	return SetVal(m_fDefaultValue, MIX_TOUCH_NORMAL);
}



/////////////////////////////////////////////////////////////////////////////

bool CMixParam::isPlaying()
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(TRANSPORT_STATE_PLAY, &bVal);

	if (SUCCEEDED(hr) && TRUE == bVal)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////

