#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "StringCruncher.h"

/////////////////////////////////////////////////////////////////////////////

CMixParam::CMixParam()
{
	m_pMixer = NULL;
	m_pTransport = NULL;
	m_dwUniqueId = ILLEGAL_UNIQUE_ID;

	m_pFilterLocator = NULL;

	m_bHasBinding = false;
	m_bTouched = false;
	m_bAudioMeteringEnabled = false;
	m_bWasMIDI = false;

	m_eMixerStrip = MIX_STRIP_TRACK;
	m_dwStripNum = 0;
	m_eMixerParam = MIX_PARAM_VOL;
	m_dwParamNum = 0;

	m_eDataType = DT_LEVEL;
	m_fDefaultValue = NO_DEFAULT;
	m_fStepSize = 0.05f;
	m_fMin = 0.0f;
	m_fMax = 1.0f;

	m_szAlternateLabel[0] = 0;
	m_bDisableWhilePlaying = false;
	m_bAllowFineResolution = true;
}

CMixParam::~CMixParam()
{
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::Setup(ISonarMixer *pMixer, ISonarTransport *pTransport, FilterLocator *pFilterLocator, DWORD dwUniqueId)
{
	m_pMixer = pMixer;
	m_pTransport = pTransport;
	m_dwUniqueId = dwUniqueId;
	m_pFilterLocator = pFilterLocator;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
							SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum)
{
	if (m_bHasBinding == false ||
		m_eMixerStrip != eMixerStrip || m_dwStripNum != dwStripNum ||
		m_eMixerParam != eMixerParam || m_dwParamNum != dwParamNum)
	{
		bool bWasTouched = m_bTouched;

		if (bWasTouched)
			Touch(false);

		SetMetering(false);

		m_eMixerStrip = eMixerStrip;
		m_dwStripNum = dwStripNum;
		m_eMixerParam = eMixerParam;
		m_dwParamNum = dwParamNum;

		if (bWasTouched)
			Touch(true);

		m_bHasBinding = true;
	}

	m_szAlternateLabel[0] = 0;
	m_bDisableWhilePlaying = false;
	m_bAllowFineResolution = true;

	// Remember if it was a MIDI track
	m_bWasMIDI = IsMIDITrack();
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetAttribs(DataType eDataType, float fDefaultValue, float fStepSize,
							float fMin, float fMax)
{
	m_eDataType = eDataType;
	m_fDefaultValue = fDefaultValue;

	switch (eDataType)
	{
		case DT_BOOL:
		case DT_SELECTOR:
		case DT_INTERLEAVE:
			m_fStepSize = 1.0f;
			m_bAllowFineResolution = false;
			break;

		default:
			m_fStepSize = fStepSize;
			break;
	}

	m_fMin = fMin;
	m_fMax = fMax;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetAlternateLabel(const char *szText)
{
	::strlcpy(m_szAlternateLabel, szText, sizeof(m_szAlternateLabel));
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetDisableWhilePlaying(bool bDisable)
{
	m_bDisableWhilePlaying = true;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetAllowFineResolution(bool bAllowFineResolution)
{
	m_bAllowFineResolution = bAllowFineResolution;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::ClearBinding()
{
	SetMetering(false);
	m_bHasBinding = false;
	m_szAlternateLabel[0] = 0;
	m_bWasMIDI = false;
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::ClearBindingDisplayDash()
{
	ClearBinding();
	::strlcpy(m_szAlternateLabel, "  --", sizeof(m_szAlternateLabel));
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetCrunchedStripName(LPSTR pszText, DWORD dwLen)
{
	pszText[0] = 0;

	char szBuf[128];
	DWORD dwTmpLen = sizeof(szBuf);

	HRESULT hr = GetStripName(szBuf, &dwTmpLen);

	if (FAILED(hr))
		return hr;

	CStringCruncher cCruncher;

	cCruncher.CrunchString(szBuf, pszText, dwLen);

	pszText[dwLen] = 0;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

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

HRESULT CMixParam::GetCrunchedValueText(LPSTR pszText, DWORD dwLen)
{
	pszText[0] = 0;

	char szBuf[128];
	DWORD dwTmpLen = sizeof(szBuf);

	HRESULT hr = GetValueText(szBuf, &dwTmpLen);

	if (FAILED(hr))
		return hr;

	CStringCruncher cCruncher;

	cCruncher.CrunchString(szBuf, pszText, dwLen);

	pszText[dwLen] = 0;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetNormalizedVal(float *fVal, bool *bDot)
{
	float fTmp;

	HRESULT hr = GetVal(&fTmp);

	if (FAILED(hr))
		return hr;

	// Rescale to be between 0 and 1
	*fVal = (fTmp - m_fMin) / (m_fMax - m_fMin);

	if (bDot)
	{
		switch (m_eDataType)
		{
			case DT_NO_LEDS:
			case DT_BOOL:
			case DT_SELECTOR:
			case DT_LEVEL:
			case DT_INTERLEAVE:
			case DT_SPREAD:
				*bDot = false;
				break;

			case DT_BOOST_CUT:
			case DT_PAN:
				{
					float fDiff = *fVal - 0.5f;

					if (fDiff < 0)
						fDiff = -fDiff;

					*bDot = fDiff < 0.005f;
				}
				break;

			default:
				*bDot = false;
				TRACE("CMixParam::GetNormalizedVal(): Error: unknown DataType\n");
				break;
		}
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetNormalizedVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch)
{
	// Rescale to be between fMin and fMax
	fVal = (fVal * (m_fMax - m_fMin)) + m_fMin;

	return SetVal(fVal, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////
DWORD	CMixParam::GetParamNum()
{
	if (!m_pFilterLocator)
		return m_dwParamNum;

	if (m_eMixerParam == MIX_PARAM_FILTER)
		return m_pFilterLocator->GetFilterNum(m_eMixerStrip, m_dwStripNum, (SONAR_MIXER_FILTER)m_dwParamNum);

	if (m_eMixerParam == MIX_PARAM_FILTER_PARAM)
	{
		SONAR_MIXER_FILTER eFilter = (SONAR_MIXER_FILTER)LOWORD(m_dwParamNum);
		DWORD dwFilterNum = m_pFilterLocator->GetFilterNum(m_eMixerStrip, m_dwStripNum, eFilter);
		return MAKELONG(dwFilterNum, HIWORD(m_dwParamNum));
	}

	return m_dwParamNum;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetStripName(LPSTR pszText, DWORD *pdwLen)
{
	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->GetMixStripName(m_eMixerStrip, m_dwStripNum,
									pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetStripName(LPSTR pszText)
{
	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->SetMixStripName(m_eMixerStrip, m_dwStripNum,
									pszText);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetParamLabel(LPSTR pszText, DWORD *pdwLen)
{
	if (0 != m_szAlternateLabel[0])
	{
		if (pszText && pdwLen)
			::strlcpy(pszText, m_szAlternateLabel, *pdwLen);

		else if (pdwLen)
			*pdwLen = (DWORD)::strlen(m_szAlternateLabel) + 1;

		return S_OK;
	}

	if (!m_bHasBinding)
		return E_FAIL;

	// AZ: A workaround for Sonar crashing when accessing ProChannel compressor's "Type" label
	if ((m_eMixerParam == MIX_PARAM_FILTER_PARAM) && (m_dwParamNum == MAKELONG(MIX_FILTER_COMP, 0)))
	{
		::strlcpy(pszText, "Type", *pdwLen);
		return S_OK;
	}

	return m_pMixer->GetMixParamLabel(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetVal(float *fVal)
{
	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									fVal);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (m_bDisableWhilePlaying && IsPlaying())
		return E_FAIL;

	return m_pMixer->SetMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									fVal, eMixerTouch);
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

	return m_pMixer->GetMixParamValueText(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									fVal, pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Touch(bool bTouchState)
{
	// Record this even if we're not currently bound to anything so that if we
	// are bound after a reconfigure, we can update the touch state correctly
	m_bTouched = bTouchState;

	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->TouchMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									bTouchState ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetArm(bool *pbArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	BOOL bArm;

	HRESULT hr = m_pMixer->GetArmMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									&bArm);

	*pbArm = (FALSE != bArm);

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetArm(bool bArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (IsPlaying())
		return E_FAIL;

	return m_pMixer->SetArmMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum(),
									bArm ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::GetArmAll(bool *pbArm)
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

HRESULT CMixParam::SetArmAll(bool bArm)
{
	if (!m_bHasBinding)
		return E_FAIL;

	if (IsPlaying())
		return E_FAIL;

	return m_pMixer->SetArmMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_ANY, 0,
									bArm ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Snapshot()
{
	if (!m_bHasBinding)
		return E_FAIL;

	return m_pMixer->SnapshotMixParam(m_eMixerStrip, m_dwStripNum,
									m_eMixerParam, GetParamNum());
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ToggleBooleanParam(SONAR_MIXER_TOUCH eMixerTouch)
{
	float fVal;

	HRESULT hr = GetVal(&fVal);

	if (FAILED(hr))
		return hr;

	return SetVal((fVal >= 0.5f) ? 0.0f : 1.0f, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::Adjust(float fDelta, SONAR_MIXER_TOUCH eMixerTouch)
{
	float fVal;

	HRESULT hr = GetVal(&fVal);

	if (FAILED(hr))
		return hr;

	fVal += fDelta;

	if (fVal < m_fMin)
		fVal = m_fMin;
	else if (fVal > m_fMax)
		fVal = m_fMax;

	return SetVal(fVal, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ToggleArm()
{
	bool bArm;

	HRESULT hr = GetArm(&bArm);

	if (FAILED(hr))
		return hr;

	return SetArm(!bArm);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ToggleArmAll()
{
	bool bArm;

	HRESULT hr = GetArmAll(&bArm);

	if (FAILED(hr))
		return hr;

	return SetArmAll(!bArm);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ToggleOrSetToDefault(SONAR_MIXER_TOUCH eMixerTouch)
{
	// Toggle if it's a bool or interleave type
	if (DT_BOOL == m_eDataType)
	{
		return ToggleBooleanParam(eMixerTouch);
	}
	else if (DT_INTERLEAVE == m_eDataType)
	{
		float fVal;

		HRESULT hr = GetVal(&fVal);

		if (FAILED(hr))
			return hr;

		if (1.0f == fVal)
			fVal = 2.0f;
		else
			fVal = 1.0f;

		return SetVal(fVal, eMixerTouch);
	}

	if (NO_DEFAULT == m_fDefaultValue)
		return E_FAIL;

	return SetVal(m_fDefaultValue, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::SetToDefaultValue(SONAR_MIXER_TOUCH eMixerTouch)
{
	if (NO_DEFAULT == m_fDefaultValue)
		return E_FAIL;

	return SetVal(m_fDefaultValue, eMixerTouch);
}

/////////////////////////////////////////////////////////////////////////////

// Is this a MIDI track?
//
// Returns true if it is, false if it's not a MIDI track or an invalid track number
//
bool CMixParam::IsMIDITrack()
{
	if (!m_bHasBinding || m_eMixerStrip != MIX_STRIP_TRACK)
		return false;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_IS_MIDI, 0,
									&fVal);

	if (FAILED(hr))
		return false;

	return (fVal >= 0.5f);
}

/////////////////////////////////////////////////////////////////////////////

// Is this an audio track?
//
// Returns true if it is, false if it's not an audio track or an invalid track number
//
bool CMixParam::IsAudioTrack()
{
	if (!m_bHasBinding)
		return false;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_IS_MIDI, 0,
									&fVal);

	if (FAILED(hr))
		return false;

	return (fVal < 0.5f);
}

/////////////////////////////////////////////////////////////////////////////

bool CMixParam::StripExists()
{
	if (!m_bHasBinding)
		return false;

	DWORD dwCount = 0;

	HRESULT hr = m_pMixer->GetMixStripCount(m_eMixerStrip, &dwCount);

	// While loading a new project, dwCount is -1
	if (FAILED(hr) || 0xFFFFFFFF == dwCount)
		return false;

	return (m_dwStripNum < dwCount);
}

/////////////////////////////////////////////////////////////////////////////

bool CMixParam::IsPlaying()
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(TRANSPORT_STATE_PLAY, &bVal);

	if (SUCCEEDED(hr) && TRUE == bVal)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////

bool CMixParam::IsArchived()
{
	if (!m_bHasBinding)
		return false;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
									MIX_PARAM_ARCHIVE, 0,
									&fVal);

	if (FAILED(hr))
		return false;

	return (fVal >= 0.5f);
}

/////////////////////////////////////////////////////////////////////////////

void CMixParam::SetMetering(bool bOn)
{
	if (!m_bHasBinding || m_dwUniqueId == ILLEGAL_UNIQUE_ID)
	{
		m_bAudioMeteringEnabled = false;
		return;
	}

//	TRACE("m_dwUniqueId = 0x%08X\n", m_dwUniqueId);

	float fVal = (bOn) ? 1.0f : 0.0f;

	HRESULT hr = m_pMixer->SetMixParam(m_eMixerStrip, m_dwStripNum, MIX_PARAM_AUDIO_METER, m_dwUniqueId, fVal, MIX_TOUCH_NORMAL);

	m_bAudioMeteringEnabled = (bOn) ? (SUCCEEDED(hr)) : false;

//	TRACE("m_bAudioMeteringEnabled = %d\n", m_bAudioMeteringEnabled);
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMixParam::ReadMeter(float *fVal)
{
	if (!m_bHasBinding || !m_bAudioMeteringEnabled)
		return E_FAIL;

	HRESULT hr = m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum, MIX_PARAM_AUDIO_METER, m_dwUniqueId, fVal);

#if 0
	if (FAILED(hr))
	{
		TRACE("ReadMeter failed\n");
	}
	else
	{
		TRACE("ReadMeter returnd %f\n", *fVal);
	}
#endif

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

// Check to see if the binding still matches
//
// Returns false if there's a mismatch
//
bool CMixParam::CheckBinding()
{
	// No binding, no problem
	if (!m_bHasBinding)
		return true;

	// Only tracks can contain MIDI
	if (m_eMixerStrip != MIX_STRIP_TRACK)
		return true;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(m_eMixerStrip, m_dwStripNum,
										MIX_PARAM_IS_MIDI, 0,
										&fVal);

	// Strip doesn't exist, so no problem
	if (FAILED(hr))
		return true;

	bool bIsMIDI = (fVal >= 0.5f);

	// Was MIDI, and still is, no problem
	if (m_bWasMIDI && bIsMIDI)
		return true;

	// Was audio, and still is, no problem
	if (!m_bWasMIDI && !bIsMIDI)
		return true;

	// Mismatch -> problem
	return false;
}

/////////////////////////////////////////////////////////////////////////////
