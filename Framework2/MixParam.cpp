#include "stdafx.h"


#include "MixParam.h"
#include "StringCruncher.h"
#include <math.h>

#pragma warning(disable:4100)

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
	m_eCaptureType( CT_Jump )
	,m_fLastPosition( -1.0f )
	,m_fLastSentToHost( -1.f )
	,m_bACTLearning( false )
	,m_wValHistory( VT_UNDEF )
	,m_fValCached( -1.f)
	,m_dwSetParamTimeStamp( 0 )
	,m_dwStripNumOffset(0)
	,m_eTriggerAction( TA_TOGGLE )
	,m_bDisplayValue( true )
	,m_bDisplayName( true )
	,m_bAlwaysChanging( false )
	,m_dwCrunchSize( 0 )
	,m_bThrottle( true )
{
	m_pLockStrip = NULL;
	m_pMixer = NULL;
	m_pMixer3 = NULL;
	m_pTransport = NULL;
	m_dwUniqueId = ILLEGAL_UNIQUE_ID;

	m_bHasBinding = false;
	m_bTouched = false;

	m_eMixerStrip = MIX_STRIP_TRACK;
	SetMixerParam( MIX_PARAM_VOL );
	m_dwParamNum = 0;
	m_dwPhysicalStripIndex = 0;
	m_dwStripNum = (DWORD)-1;

	m_fDefaultValue = NO_DEFAULT;

	m_bDisableWhilePlaying = false;
}

CMixParam::~CMixParam()
{
	if ( m_pMixer3 )
		m_pMixer3->Release();

	if ( m_pLockStrip )
		m_pLockStrip->Release();
}

/////////////////////////////////////////////////////////////////////////////
// Give the parameter pointers (non addref'd) to the various host interfaces
// it needs
void CMixParam::SetInterfaces(ISonarMixer *pMixer, ISonarTransport *pTransport, DWORD dwUniqueId)
{
	m_pMixer = pMixer;
	m_pMixer->QueryInterface( IID_ISonarMixer3, (void**)&m_pMixer3 );
	m_pMixer->QueryInterface( IID_IHostLockStrip, (void**)&m_pLockStrip );

	m_pTransport = pTransport;
	m_dwUniqueId = dwUniqueId;
}


/////////////////////////////////////////////////////////////////////////////
// Set the binding attributes. This identifies which host param it will control
void CMixParam::SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, 
								  SONAR_MIXER_PARAM eMixerParam, 
								  DWORD dwParamNum, 
								  float fDefault, //= NO_DEFAULT
								  BOOL bSetCapture ) //= FALSE
{
	if (m_bHasBinding == false ||
		m_eMixerStrip != eMixerStrip || m_dwStripNum != dwStripNum ||
		m_eMixerParam != eMixerParam || m_dwParamNum != dwParamNum || fDefault != m_fDefaultValue)
	{
		bool bWasTouched = m_bTouched;

		if (bWasTouched)
			Touch(false);

		m_eMixerStrip = eMixerStrip;
		SetMixerParam( eMixerParam, bSetCapture );
		m_dwParamNum = dwParamNum;

		// Set the base strip num
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

void CMixParam::SetAllParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, 
									  DWORD dwParamNum, DWORD dwStripNumOffset, DWORD dwCrunchSize /* = 0 */, BOOL bSetCapture /* = FALSE */ )
{
	if ( m_bHasBinding == false ||
		  m_eMixerStrip != eMixerStrip || m_dwStripNum != dwStripNum || m_dwCrunchSize != dwCrunchSize ||
		  m_eMixerParam != eMixerParam || m_dwParamNum != dwParamNum || m_dwStripNumOffset != dwStripNumOffset )
	{
		const bool bWasTouched = m_bTouched;

		if (bWasTouched)
			Touch(false);

		m_eMixerStrip = eMixerStrip;
		SetMixerParam( eMixerParam, bSetCapture );
		m_dwParamNum = dwParamNum;
		// Set the base strip num
		m_dwStripNum = dwStripNum;
		m_dwStripNumOffset = dwStripNumOffset;
		m_dwCrunchSize = dwCrunchSize;

		if (bWasTouched)
			Touch(true);

		m_bHasBinding = true;

		ResetHistory();
	}
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

void CMixParam::SetCompParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault ) //= NO_DEFAULT )
{
	SetParams( mixerStrip, dwStripNum, MIX_PARAM_FILTER_PARAM, MAKELONG( MIX_FILTER_COMP, wFxParamNum ), fDefault );
}

void CMixParam::SetTubeParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault ) //= NO_DEFAULT )
{
	SetParams( mixerStrip, dwStripNum, MIX_PARAM_FILTER_PARAM, MAKELONG( MIX_FILTER_SAT, wFxParamNum ), fDefault );
}

//-----------------------------------------------------------------------
// Helper to set just the strip number
void CMixParam::SetStripNum( DWORD dwStrip )
{
	SetParams( m_eMixerStrip, dwStrip, m_eMixerParam, m_dwParamNum, m_fDefaultValue );
}

//---------------------------------------------------------------------
// Helper to set just the strip type
void CMixParam::SetStripType( SONAR_MIXER_STRIP eStrip )
{
	SetParams( eStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, m_fDefaultValue );
}

//-------------------------------------------------------------------------
// Helper to set just the param number
void	CMixParam::SetParamNum( DWORD dwParamNum )
{
	SetParams( m_eMixerStrip, m_dwStripNum, m_eMixerParam, dwParamNum, m_fDefaultValue );
}

//-------------------------------------------------------------------------
// Helper to set just the mix param
void CMixParam::SetMixParam( SONAR_MIXER_PARAM eparam )
{
	SetParams( m_eMixerStrip, m_dwStripNum, eparam, m_dwParamNum, m_fDefaultValue );
}

//----------------------------------------------------------------------
void	 CMixParam::SetStripPhysicalIndex( DWORD dwPhysicalIndex )
{
	if ( m_dwPhysicalStripIndex != dwPhysicalIndex )
	{
		bool bWasTouched = m_bTouched;

		if (bWasTouched)
			Touch(false);

		m_dwPhysicalStripIndex = dwPhysicalIndex;

		if (bWasTouched)
			Touch(true);
	}
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

	::memset(szBuf, 0, sizeof(szBuf));

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

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	HRESULT hr = m_pMixer->GetMixParamLabel( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, pszText, pdwLen );
	return ( hr );
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

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	HRESULT hr =  m_pMixer->GetMixParam(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum,
									&f );

	if ( FAILED( hr ) )
	{
		hr = ( -2.f == m_fValCached ) ? S_FALSE : hr;
		m_fValCached = -2.f;
		return hr;
	}

	scaleValueFromHost( f );

	hr = ( f == m_fValCached ) ? S_FALSE : S_OK;
	*fVal = m_fValCached = f;
	
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetVal(float f01Val, // normalized value
								  SONAR_MIXER_TOUCH eMixerTouch, // Automation touch mode
								  CMidiMsg::ValueChange vc )	// increment/decrement/none enum
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (m_bDisableWhilePlaying && isPlaying())
		return E_FAIL;

	HRESULT hr = S_OK;

	bool bSend = true;		// default to sending
	float fValSend = f01Val;	// assume sending the raw value

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	// if one of the non-jump types...
	if ( CT_Jump != m_eCaptureType && !m_bACTLearning )
	{
		float fHost = 0.0f;

		// If Match mode, we will choose to set based on the value history
		if ( CT_Match == m_eCaptureType )
		{
			DWORD dwNow = ::GetTickCount();
			hr = m_pMixer->GetMixParam( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, &fHost );

			// If some time has elapsed since the last Value Match,
			// make sure we're still within a value window. If not,
			// invalidate the history so we have to re-null
			if ( dwNow - m_dwSetParamTimeStamp > 500 )
			{
				// Some time has elapsed since last Match so...
				// compare to the host value

				// If not "close enough" we assume the rug was pulled out from under this 
				// value - ether by adjusting it in the host, or by some other surface
				// or remote control
				if ( ::fabs(f01Val - fHost) > 0.25f )
					ResetHistory();
			}

			bool bWasCrossed = m_wValHistory == VT_CROSSED;

			// If not Nulled, get the host's value and set our Below/Above
			// history bits accordingly. We'll ultimately only set the value
			// if we've crossed the host (ie nulled)
			if ( !bWasCrossed )
			{
				bWasCrossed = false;

				// Set the bit indicating above/below the host
				if ( f01Val <= fHost )
					m_wValHistory |= VT_WASBELOW;
				if ( f01Val >= fHost )
					m_wValHistory |= VT_WASABOVE;
			}

			// only send if we've crossed the host
			bSend = m_wValHistory == VT_CROSSED;
		}
		else if ( CT_Relative == m_eCaptureType )
		{
			if ( 0.0f <= m_fLastPosition )
			{
				// get the host value
				hr = m_pMixer->GetMixParam( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, &fHost );

				float fDelta = f01Val - m_fLastPosition;
				fValSend = fHost + fDelta;
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
				float fError = fabs(f01Val - fCurrent);	// error of command to current
				float fCommandDelta = f01Val - m_fLastPosition;	// change from last command
				fValSend = fCurrent + fCommandDelta * (1-fError);
				fValSend = max( 0.0f, fValSend );
				fValSend = min( 1.0f, fValSend );
			}
			bSend = true;
		}
#endif
		else if ( CMixParam::CT_Step == m_eCaptureType )
		{
			// Step mode is used for Enumerated parameters such as IO port.  It is also
			// used as a "fine" adjust mode.
			if ( vc == CMidiMsg::VC_None )
				bSend = true;
			else
			{
				float fMin = 0.f, fMax = 0.f, fStep = 0.f;
				if (!SUCCEEDED( hr = GetMinMaxStep(&fMin, &fMax, &fStep  )) )
					return hr;

				if ( !SUCCEEDED( hr = m_pMixer->GetMixParam( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, &fHost ) ) )
					return ( hr );

				fValSend = fHost + ( fStep * ( vc == CMidiMsg::VC_Decrease ? -1 : 1 ) );
				fValSend = max( fMin, fValSend );
				fValSend = min( fMax, fValSend );

				doSendToHost( eStripType, dwStripNum, fValSend, eMixerTouch );
				bSend = false;
			}
		}
	}

	if ( bSend )
	{
		scaleValueToHost( fValSend );
		if ( m_fLastSentToHost != fValSend )
			doSendToHost( eStripType, dwStripNum, fValSend, eMixerTouch );
	}

	m_fLastPosition = f01Val;

	return hr;
}


void CMixParam::doSendToHost( SONAR_MIXER_STRIP eStripType, DWORD dwStripNum, float fVal, SONAR_MIXER_TOUCH eMixerTouch )
{
	m_dwSetParamTimeStamp = ::GetTickCount();
	m_pMixer->SetMixParam(eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, fVal, eMixerTouch );
	m_fLastSentToHost = fVal;
	m_fValCached = -1;
}

///////////////////////////////////////////////////////
void	CMixParam::ResetHistory()
{ 
	m_wValHistory = VT_UNDEF; 
	m_fValCached = -1.f; 
	m_fLastPosition = -1.f;
	m_fLastSentToHost = -1.f;
}


CMixParam::VALUE_HISTORY CMixParam::GetNullStatusForValue( float fValue )
{
	float fValCur = 0;
	GetVal( &fValCur );

	if ( fValue < fValCur - .02f )
		return VT_WASBELOW;
	if ( fValue > fValCur + .02f )
		return VT_WASABOVE;
	return VT_CROSSED;
}

/////////////////////////////////////////////////////////////////////////
// Certain params are not normalized 0..1 such as I/O port assignments
void CMixParam::scaleValueToHost( float& fVal )
{
	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;

	float fMax = 1.f;
	switch (m_eMixerParam)
	{
	case MIX_PARAM_PAN:	// panning - employ a slight detent in center
		if ( fVal > .495f && fVal < .505f )
			fVal = .5f;
		break;
	case MIX_PARAM_INPUT:
		GetStripInfo( &eStripType, &dwStripNum );
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_INPUT_MAX, 0, &fMax );
		// fMax is the number of real ports.  There is also an implied "none" port which 
		// has a host index of -1
		fVal = fVal * fMax - 1.f;
		if ( fVal > 0.f )
			fVal = (float)(int)(fVal + .5);
		break;
	case MIX_PARAM_OUTPUT:
		GetStripInfo( &eStripType, &dwStripNum );
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_OUTPUT_MAX, 0, &fMax );
		fVal *= (fMax - 1.f);
		fVal = (float)(int)(fVal + .5);
		break;
	case MIX_PARAM_SEND_OUTPUT:
		GetStripInfo( &eStripType, &dwStripNum );
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_SEND_OUTPUT_MAX, m_dwParamNum, &fMax );
		fVal *= (fMax - 1.f);
		fVal = (float)(int)(fVal + .5);
		break;
	case MIX_PARAM_INTERLEAVE:
		fVal = fVal < .5f ? 1.f : 2.f;
		break;
	case MIX_PARAM_BANK:
		fVal *= 16383.f;
		break;
	case MIX_PARAM_PATCH:
		fVal *= 127.f;
		break;
   case MIX_PARAM_SURROUND_ANGLE:
   case MIX_PARAM_SURROUND_SENDANGLE:
      fVal = 1.f - fVal;
      break;
	}
}

/////////////////////////////////////////////////////////////////////////
// Certain params are not normalized 0..1 such as I/O port assignments
void CMixParam::scaleValueFromHost( float& fVal )
{
	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	float fMax = 1.f;
	switch (m_eMixerParam)
	{
	case MIX_PARAM_INPUT:
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_INPUT_MAX, 0, &fMax );
		// don't subtract 1 from the max in this case because we need the range to be wide
		// enough for the NONE case (which is not counted for in MIX_PARAM_INPUT_MAX)

		// special case for scaling the input param
		if ( fMax > 0.f )
			fVal = (fVal + 1) / fMax;

		fVal = max( 0.f, fVal );
		fVal = min( 1.f, fVal );

		return;

		break;
	case MIX_PARAM_OUTPUT:
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_OUTPUT_MAX, 0, &fMax );
		fMax -= 1.f;
		break;
	case MIX_PARAM_SEND_OUTPUT:
		m_pMixer->GetMixParam( eStripType, dwStripNum, MIX_PARAM_SEND_OUTPUT_MAX, m_dwParamNum, &fMax );
		fMax -= 1.f;
		break;
	case MIX_PARAM_INTERLEAVE:
		fVal = fVal > 1 ? 1.f : 0.f;
		return;
		/////////
	case MIX_PARAM_BANK:
		fMax = 16383.f;
		break;
	case MIX_PARAM_PATCH:
		fMax = 127.f;
		break;
   case MIX_PARAM_SURROUND_ANGLE:
   case MIX_PARAM_SURROUND_SENDANGLE:
      fVal = 1.f - fVal;
      break;
	}

	if ( fMax > 0.f )
		fVal /= (fMax);

	fVal = max( 0.f, fVal );
	fVal = min( 1.f, fVal );
}



/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Trigger()
{
	m_fLastSentToHost = -1.f;
	switch( m_eTriggerAction )
	{
	case TA_DEFAULT:
		SetToDefaultValue();
		break;
	case TA_TOGGLE:
		ToggleBooleanParam();
		break;
	}
	return S_OK;
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

	// attempt to use a cached value for the text representation.
	// This saves us an extra call to the host for value lookup.
	// This implies that one should not ask for a text representation of
	// the value until you've either sent to the host or refreshed from 
	// the host.
	float f = m_fValCached;

	// if cached host value is invalid, try the cached sent value
	if (f < 0.f)
	{
		if ( ( f = m_fLastSentToHost ) >= 0.f )
			scaleValueFromHost( f );
	}

	// if value is invalid, it means no one has either sent or received 
	// a value from the host yet.
	if ( f < 0.f )
	{
		HRESULT hr;
		if ( !SUCCEEDED( hr = GetVal( &f ) ) )
			return hr;
	}

	return GetValueText(f, pszText, pdwLen);
}


HRESULT CMixParam::GetCrunchedValueText(LPSTR pszText, DWORD dwLen)
{
	pszText[0] = '\0';

	char szBuf[128];
	DWORD dwTmpLen = sizeof(szBuf);
	HRESULT hr = GetValueText( szBuf, &dwTmpLen );

	CStringCruncher cCruncher;
	cCruncher.CrunchString(szBuf, pszText, dwLen);
	pszText[dwLen] = '\0';

	return hr;
}

HRESULT CMixParam::GetCrunchedValueText(float fVal, LPSTR pszText, DWORD dwLen)
{
	pszText[0] = '\0';

	char szBuf[128];
	DWORD dwTmpLen = sizeof(szBuf);
	HRESULT hr = GetValueText( fVal, szBuf, &dwTmpLen );

	CStringCruncher cCruncher;
	cCruncher.CrunchString(szBuf, pszText, dwLen);
	pszText[dwLen] = '\0';

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetValueText(float fVal, LPSTR pszText, DWORD *pdwLen)
{
	if (!m_bHasBinding)
		return E_FAIL;

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	scaleValueToHost( fVal );
	HRESULT hr = m_pMixer->GetMixParamValueText(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum,
									fVal, pszText, pdwLen);

	if ( FAILED( hr ) )
		*pszText = '\0';

//	TRACE( "Travel: %.2f  Label:  %s\n", fVal, pszText );

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

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	return m_pMixer->TouchMixParam(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum,
									bTouchState ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetWrite(bool *pbArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	BOOL bArm;

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	HRESULT hr = m_pMixer->GetArmMixParam(eStripType, dwStripNum,
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

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	return m_pMixer->SetArmMixParam(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum,
									bArm ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetRead(bool* pb)
{
	if (!m_bHasBinding)
		return E_FAIL;

	ISonarMixer2*	pMixer2 = NULL;
	if ( FAILED( this->m_pMixer->QueryInterface( IID_ISonarMixer2, (void**)&pMixer2 ) ) )
		return E_NOINTERFACE;

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	BOOL b = FALSE;
	HRESULT hr = pMixer2->GetReadMixParam(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum, &b);
	pMixer2->Release();

	*pb = b ? true : false;
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetRead(bool b )
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (isPlaying())
		return E_FAIL;

	ISonarMixer2*	pMixer2 = NULL;
	if ( FAILED( this->m_pMixer->QueryInterface( IID_ISonarMixer2, (void**)&pMixer2 ) ) )
		return E_NOINTERFACE;

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	HRESULT hr = pMixer2->SetReadMixParam(eStripType, dwStripNum,
									m_eMixerParam, m_dwParamNum,
									b ? TRUE : FALSE);

	pMixer2->Release();
	return hr;
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

	float fValSend = m_fDefaultValue;

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	scaleValueToHost( fValSend );
	doSendToHost( eStripType, dwStripNum, fValSend, MIX_TOUCH_NORMAL );

	m_fLastPosition = m_fDefaultValue;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Use ISonarMixer3 to revert to the previous punch value
HRESULT CMixParam::RevertValue()
{
	if ( !m_pMixer3 )
	{
		// Relies on ISonarMixer3
		if ( FAILED( m_pMixer->QueryInterface( IID_ISonarMixer3, (void**)&m_pMixer3 ) ) )
			return E_NOINTERFACE;
	}

	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );

	float fRev = 0.f;
	HRESULT hr = m_pMixer3->GetMixParamRevertValue( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, &fRev );

	if ( FAILED( hr ) )
		return hr;

	return SetVal( fRev );
}

/////////////////////////////////////////////////////////////////////////////
// For bank switching
void CMixParam::SetStripNumOffset( DWORD dwOffset ) 
{
	m_dwStripNumOffset = dwOffset; 
	ResetHistory(); 
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

// Return the Strip Type and Index. Ask the host if this physical strip is 
// locked and if so, override our local strip type and index with that from
// the host's locking data.
void CMixParam::GetStripInfo( SONAR_MIXER_STRIP* peType, DWORD* pdwStripNum )
{
	if ( m_pLockStrip )
	{
		SONAR_MIXER_STRIP eType;
		DWORD dwStripNum = 0;
		if ( S_OK == m_pLockStrip->GetLockedStripInfo( m_dwUniqueId, m_dwPhysicalStripIndex, &eType, &dwStripNum ) )
		{
			*peType = eType;
			*pdwStripNum = dwStripNum;
			return;
		}
	}

	*peType = m_eMixerStrip;
	*pdwStripNum = m_dwStripNum + m_dwStripNumOffset;
}

void CMixParam::SetMixerParam( SONAR_MIXER_PARAM param, BOOL bChangeCapture /* = FALSE */ )
{
	m_eMixerParam = param;
	if ( bChangeCapture )
		SetCaptureType( GetDefaultCaptureType () );
}


HRESULT CMixParam::GetMinMaxStep( float *pfMin, float *pfMax, float *pfStep )
{
	HRESULT hr = S_OK;

	if ( !m_pMixer3 )
		return ( E_NOINTERFACE );

	float fMin = 0.f, fMax = 0.f, fStep = 0.f;
	SONAR_MIXER_STRIP eStripType;
	DWORD dwStripNum = 0;
	GetStripInfo( &eStripType, &dwStripNum );
	if ( !SUCCEEDED( hr = m_pMixer3->GetMixParamStepSize( eStripType, dwStripNum, m_eMixerParam, m_dwParamNum, &fMin, &fMax, &fStep ) ) )
		return ( hr );

	if ( pfMin )
		*pfMin = fMin;
	if ( pfMax )
		*pfMax = fMax;
	if ( pfStep)
		*pfStep = fStep;

	return ( hr );
}

CMixParam::CaptureType CMixParam::GetDefaultCaptureType( void )
{
	switch ( m_eMixerParam )
	{
		case MIX_PARAM_INPUT_ECHO:
		case MIX_PARAM_LAYER_MUTE:
		case MIX_PARAM_LAYER_SOLO:
		case MIX_PARAM_MUTE:
		case MIX_PARAM_SOLO:
		case MIX_PARAM_ARCHIVE:
		case MIX_PARAM_RECORD_ARM:
		case MIX_PARAM_OUTPUT:
		case MIX_PARAM_INPUT:
		case MIX_PARAM_PHASE:
		case MIX_PARAM_INTERLEAVE:
		case MIX_PARAM_SEND_PREPOST:
		case MIX_PARAM_SEND_OUTPUT:
		case MIX_PARAM_SEND_ENABLE:
		case MIX_PARAM_SEND_MUTE:
		case MIX_PARAM_SEND_SOLO:
			return ( CMixParam::CT_Step );
		break;
		default:
			return ( CMixParam::CT_Jump );
		break;
	}
}
