#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

#include <math.h>		// For floor()

#include "strlcpy.h"

/////////////////////////////////////////////////////////////////////////////

// A safe version of GetMixStripCount which returns 0 while projects are being loaded
//
DWORD CMackieControlBase::GetStripCount(SONAR_MIXER_STRIP eMixerStrip)
{
	DWORD dwCount = 0;

	if (m_pMixer)
	{
		HRESULT hr = m_pMixer->GetMixStripCount(eMixerStrip, &dwCount);

		// While loading a new project, dwCount is -1
		if (FAILED(hr) || 0xFFFFFFFF == dwCount)
			return 0;
	}

	return dwCount;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetStripName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									  char *pszText, DWORD *pdwLen)
{
	HRESULT hr = m_pMixer->GetMixStripName(eMixerStrip, dwStripNum, pszText, pdwLen);

	return FAILED(hr) ? false : true;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetPluginCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_FX_COUNT, 0,
										&fVal);

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetPluginName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									   DWORD dwParamNum, char *pszText, DWORD *pwdLen)
{
	HRESULT hr = m_pMixer->GetMixParamLabel(eMixerStrip, dwStripNum,
										MIX_PARAM_FX, dwParamNum,
										pszText, pwdLen);
	if (FAILED(hr))
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetPluginParamCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											  DWORD dwPluginNum)
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_FX_PARAM_COUNT, dwPluginNum,
										&fVal);

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetFilterExists(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										 DWORD dwFilterNum)
{
	if (!m_cState.HaveStripFilter())
		return false;

	if (MIX_STRIP_MASTER == eMixerStrip)
		return false;

	float fVal;

/* In case focused strip is MIDI track, that call will return 0
 *	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
 *					   MIX_PARAM_FILTER_COUNT,
 *					   mFilterLocator.GetFilterNum(eMixerStrip, dwStripNum, (SONAR_MIXER_FILTER)dwFilterNum),
 *					   &fVal);
 */
	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
					    MIX_PARAM_FILTER_PARAM_COUNT,
					    m_FilterLocator.GetFilterNum(eMixerStrip, dwStripNum, (SONAR_MIXER_FILTER)dwFilterNum),
					    &fVal);

	if (FAILED(hr))
		return false;

	return (fVal > 0.5f);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetFilterName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									   DWORD dwFilterNum, char *pszText, DWORD *pwdLen)
{
	pszText[0] = 0;

	// Force ProChannel EQ name to avoid clash with old EQ
	if ((dwFilterNum == MIX_FILTER_EQ) && m_FilterLocator.IsFlexiblePC())
	{
		*pwdLen = static_cast<DWORD>(strlcpy(pszText, "ProChannel EQ", *pwdLen));
		return true;
	}

	HRESULT hr = m_pMixer->GetMixParamLabel(eMixerStrip, dwStripNum,
						MIX_PARAM_FILTER,
						m_FilterLocator.GetFilterNum(eMixerStrip, dwStripNum, (SONAR_MIXER_FILTER)dwFilterNum),
						pszText, pwdLen);
	if (FAILED(hr) || !pszText[0])
	{
		if (!m_FilterLocator.IsFlexiblePC() || (dwFilterNum != MIX_FILTER_COMP))
			return false;
		// Can happened if focused strip is MIDI track
		// There can be one from 2 types, on any strip type
		if (GetFilterParamCount(eMixerStrip, dwStripNum, dwFilterNum) == 11)
			*pwdLen = static_cast<DWORD>(strlcpy(pszText, "Bus Comp", *pwdLen));
		else
			*pwdLen = static_cast<DWORD>(strlcpy(pszText, "Track Comp", *pwdLen));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetFilterParamCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											  DWORD dwFilterNum)
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
					    MIX_PARAM_FILTER_PARAM_COUNT,
					    m_FilterLocator.GetFilterNum(eMixerStrip, dwStripNum, (SONAR_MIXER_FILTER)dwFilterNum),
					    &fVal);

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetNumInputs(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_INPUT_MAX, 0,
										&fVal);

	if (FAILED(hr))
	{
		TRACE("CMackieControlBase::GetNumInputs() failed, hr = 0x%08x\n", hr);
		return 0;
	}

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetNumOutputs(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_OUTPUT_MAX, 0,
										&fVal);

	if (FAILED(hr))
	{
		TRACE("CMackieControlBase::GetNumOutputs() failed, hr = 0x%08x\n", hr);
		return 0;
	}

	TRACE("CMackieControlBase::GetNumOutputs(): %d, %d -> %f\n", eMixerStrip, dwStripNum, fVal);

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////


DWORD CMackieControlBase::GetNumSends(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
//	return GetStripCount(m_cState.BusType());

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_SENDCOUNT, 0,
										&fVal);

	if (FAILED(hr))
	{
		// Probably not SONAR 4...
		TRACE("GetNumSends() failed, hr = 0x%08x, using GetMaxNumSends()\n", hr);
		return GetMaxNumSends(eMixerStrip);
	}

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetMaxNumSends(SONAR_MIXER_STRIP eMixerStrip)
{
	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:
		case MIX_STRIP_BUS:
			return GetStripCount(m_cState.BusType()) + GetStripCount(m_cState.MasterType());

		case MIX_STRIP_AUX:
			return GetStripCount(MIX_STRIP_AUX);

		case MIX_STRIP_MAIN:
		case MIX_STRIP_MASTER:
			return 0;

		default:
			TRACE("CMackieControlBase::GetMaxSends(): Error: unknown strip type!\n");
			break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetSelected()
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(MIX_STRIP_TRACK, 0,
										MIX_PARAM_SELECTED, 0,
										&fVal);

	if (FAILED(hr))
	{
		TRACE("CMackieControlBase::GetSelected() failed, hr = 0x%08x\n", hr);
		return 0;
	}

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::IsProjectLoaded()
{
	char szName[128];
	DWORD dwLen = sizeof(szName);

	HRESULT hr = m_pProject->GetProjectName(szName, &dwLen);

	if (FAILED(hr))
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::HaveMixerStrips()
{
	DWORD dwCount;

	HRESULT hr = m_pMixer->GetMixStripCount(MIX_STRIP_TRACK, &dwCount);

	// While loading a new project, dwCount is -1
	if (FAILED(hr) || 0xFFFFFFFF == dwCount)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::IsAPluginMode(Assignment eAssignment)
{
	switch (eAssignment)
	{
		case MCS_ASSIGNMENT_PLUGIN:
		case MCS_ASSIGNMENT_EQ:
		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:
		case MCS_ASSIGNMENT_DYNAMICS:
			return true;

		default:
			return false;
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::IsMIDI(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
	bool bIsMIDI = false;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_IS_MIDI, 0,
										&fVal);

	if (SUCCEEDED(hr))
		bIsMIDI = (fVal >= 0.5f);

	return bIsMIDI;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetRudeSoloStatus()
{
	if (m_cState.HaveStripAny())
	{
		float fVal;

		HRESULT hr = m_pMixer->GetMixParam(MIX_STRIP_ANY, 0, MIX_PARAM_SOLO, 0, &fVal);

		if (FAILED(hr))
		{
			// This will happen when no project is loaded
			return false;
		}

		return (fVal >= 0.5);
	}

	bool bRudeOn = false;

	DWORD dwCount = GetStripCount(MIX_STRIP_TRACK);

	// Look for any channels with solo enabled
	for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
	{
		float fVal;

		if (m_pMixer->GetMixParam(MIX_STRIP_TRACK, dwChan, MIX_PARAM_SOLO, 0, &fVal) == S_OK)
		{
			if (fVal >= 0.5)
			{
				bRudeOn = true;
				break;
			}
		}
	}

	if (!bRudeOn && m_cState.HaveBuses())
	{
		// Check buses too
		dwCount = GetStripCount(MIX_STRIP_BUS);

		// Look for any channels with solo enabled
		for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
		{
			float fVal;

			if (m_pMixer->GetMixParam(MIX_STRIP_BUS, dwChan, MIX_PARAM_SOLO, 0, &fVal) == S_OK)
			{
				if (fVal >= 0.5)
				{
					bRudeOn = true;
					break;
				}
			}
		}
	}

	return bRudeOn;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ClearAllMutes()
{
//	TRACE("CMackieControlBase::ClearAllMutes()\n");

	DWORD dwCount = GetStripCount(MIX_STRIP_TRACK);

	for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
		m_pMixer->SetMixParam(MIX_STRIP_TRACK, dwChan, MIX_PARAM_MUTE, 0, 0.0f, MIX_TOUCH_NORMAL);

	if (m_cState.HaveBuses())
	{
		// Buses too
		dwCount = GetStripCount(MIX_STRIP_BUS);

		for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
			m_pMixer->SetMixParam(MIX_STRIP_BUS, dwChan, MIX_PARAM_MUTE, 0, 0.0f, MIX_TOUCH_NORMAL);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ClearAllSolos()
{
//	TRACE("CMackieControlBase::ClearAllSolos()\n");

	DWORD dwCount = GetStripCount(MIX_STRIP_TRACK);

	for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
		m_pMixer->SetMixParam(MIX_STRIP_TRACK, dwChan, MIX_PARAM_SOLO, 0, 0.0f, MIX_TOUCH_NORMAL);

	if (m_cState.HaveBuses())
	{
		// Buses too
		dwCount = GetStripCount(MIX_STRIP_BUS);

		for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
			m_pMixer->SetMixParam(MIX_STRIP_BUS, dwChan, MIX_PARAM_SOLO, 0, 0.0f, MIX_TOUCH_NORMAL);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::DisarmAllTracks()
{
//	TRACE("CMackieControlBase::DisarmAllTracks()\n");

	DWORD dwCount = GetStripCount(MIX_STRIP_TRACK);

	for (DWORD dwChan = 0; dwChan < dwCount; dwChan++)
		m_pMixer->SetMixParam(MIX_STRIP_TRACK, dwChan, MIX_PARAM_RECORD_ARM, 0, 0.0f, MIX_TOUCH_NORMAL);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ToggleLoopMode()
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(TRANSPORT_STATE_LOOP, &bVal);

	if (FAILED(hr))
		return;

	bVal = !bVal;

	m_pTransport->SetTransportState(TRANSPORT_STATE_LOOP, bVal);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ToggleScrubMode()
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(TRANSPORT_STATE_SCRUB, &bVal);

	if (FAILED(hr))
		return;

	bVal = !bVal;

	m_pTransport->SetTransportState(TRANSPORT_STATE_SCRUB, bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetTransportState(SONAR_TRANSPORT_STATE eState)
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(eState, &bVal);

	if (FAILED(hr))
		return false;

	return bVal ? true : false;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetTransportState(SONAR_TRANSPORT_STATE eState, bool bVal)
{
	m_pTransport->SetTransportState(eState, bVal ? TRUE : FALSE);
}


/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::IsRecording()
{
	return GetTransportState(TRANSPORT_STATE_REC);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetTimeCursor(SONAR_TRANSPORT_TIME eTime)
{
	if (TRANSPORT_TIME_CURSOR == eTime)
		return;

	MFX_TIME mfxTime;

	mfxTime.timeFormat = TF_SECONDS;

	HRESULT hr = m_pTransport->GetTransportTime(eTime, &mfxTime);

	if (FAILED(hr))
		return;

	m_pTransport->SetTransportTime(TRANSPORT_TIME_CURSOR, &mfxTime);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetMarker(SONAR_TRANSPORT_TIME eTime)
{
	if (TRANSPORT_TIME_CURSOR == eTime)
		return;

	MFX_TIME mfxTime;

	mfxTime.timeFormat = TF_SECONDS;

	HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &mfxTime);

	if (FAILED(hr))
		return;

	m_pTransport->SetTransportTime(eTime, &mfxTime);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::FakeKeyPress(bool bShift, bool bCtrl, bool bAlt, int nVirtKey)
{
//	TRACE("CMackieControlBase::FakeKeyPress(): shift: %d, ctrl: %d, alt: %d, key: %d (%c)\n",
//			bShift, bCtrl, bAlt, nVirtKey, nVirtKey);

	if (bShift)	m_pKeyboard->KeyboardEvent(VK_SHIFT, SKE_KEYDOWN);
	if (bCtrl)	m_pKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYDOWN);
	if (bAlt)	m_pKeyboard->KeyboardEvent(VK_MENU, SKE_KEYDOWN);

	if (nVirtKey >= 0)
		m_pKeyboard->KeyboardEvent(nVirtKey, SKE_KEYDOWNUP);

	if (bAlt)	m_pKeyboard->KeyboardEvent(VK_MENU, SKE_KEYUP);
	if (bCtrl)	m_pKeyboard->KeyboardEvent(VK_CONTROL, SKE_KEYUP);
	if (bShift)	m_pKeyboard->KeyboardEvent(VK_SHIFT, SKE_KEYUP);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetRelayClick(bool bOn)
{
	TRACE("CMackieControlBase::SetRelayClick()\n");

	if (0x00 == m_bDeviceType)
		return;

	BYTE pbRelayClick[] = { 0xF0, 0x00, 0x00, 0x66, m_bDeviceType, 0x0A,
							static_cast<BYTE>(bOn ? 0x01 : 0x00), 0xF7 };

	SendMidiLong(sizeof(pbRelayClick), pbRelayClick);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::NudgeTimeCursor(JogResolution eJogResolution, Direction eDir)
{
//	TRACE("CMackieControlBase::NudgeTimeCursor()\n");

	MFX_TIME mfxTime;

	if (eJogResolution < JOG_HOURS)
		mfxTime.timeFormat = TF_MBT;
	else
		mfxTime.timeFormat = TF_SECONDS;

	HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &mfxTime);

	if (FAILED(hr))
		return;

	// Fixup for when the displayed time is (incorrectly?) rounded up
	switch (eJogResolution)
	{
		case JOG_HOURS:
		case JOG_MINUTES:
		case JOG_SECONDS:
			if (DIR_FORWARD == eDir)
			{
				double c = ceil(mfxTime.dSeconds);
				double d = c - mfxTime.dSeconds;

//				TRACE("time: %.20f, ceil: %.20f, diff: %g\n", mfxTime.dSeconds, c, d);

				if (d > 0.0 && d < 1e-8)
				{
					mfxTime.dSeconds = c;
//					TRACE("FIXUP (forward), now %.20f\n", mfxTime.dSeconds);
				}
			}
			else
			{
				double f = floor(mfxTime.dSeconds);
				double d = mfxTime.dSeconds - f;

//				TRACE("time: %.20f, floor: %.20f, diff : %g\n", mfxTime.dSeconds, f, d);

				if (d > 0.0 && d < 1e-8)
				{
					mfxTime.dSeconds = f;
//					TRACE("FIXUP (backward), now %.20f\n", mfxTime.dSeconds);
				}
			}
			break;

		default:
			break;
	}

	int iIncrement = (DIR_FORWARD == eDir) ? 1 : -1;

	switch (eJogResolution)
	{
		case JOG_MEASURES:
			mfxTime.mbt.nMeas += iIncrement;
			mfxTime.mbt.nBeat = 1;
			mfxTime.mbt.nTick = 0;
			break;

		case JOG_BEATS:
			mfxTime.mbt.nBeat += iIncrement;
			mfxTime.mbt.nTick = 0;
			break;

		case JOG_TICKS:
			mfxTime.mbt.nTick += iIncrement;
			break;

		case JOG_HOURS:
			if (DIR_FORWARD == eDir)
				mfxTime.dSeconds = 3600 * (floor(mfxTime.dSeconds / 3600) + 1);
			else
				mfxTime.dSeconds = 3600 * (ceil(mfxTime.dSeconds / 3600) - 1);
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_MINUTES:
			if (DIR_FORWARD == eDir)
				mfxTime.dSeconds = 60 * (floor(mfxTime.dSeconds / 60) + 1);
			else
				mfxTime.dSeconds = 60 * (ceil(mfxTime.dSeconds / 60) - 1);
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_SECONDS:
			if (DIR_FORWARD == eDir)
				mfxTime.dSeconds = floor(mfxTime.dSeconds) + 1;
			else
				mfxTime.dSeconds = ceil(mfxTime.dSeconds) - 1;
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_FRAMES:
			{
				MFX_TIME tmpTime;

				tmpTime.timeFormat = TF_SMPTE;

				HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &tmpTime);

				if (FAILED(hr))
					return;

				double dFramePeriod;

				switch (tmpTime.smpte.fps)
				{
					case FPS_24:			dFramePeriod = 1.0 / 24.0;		break;
					case FPS_25:			dFramePeriod = 1.0 / 25.0;		break;
					case FPS_2997:			dFramePeriod = 1.0 / 29.97;		break;
					case FPS_2997_DROP:		dFramePeriod = 1.0 / 29.97;		break;
					case FPS_30:			dFramePeriod = 1.0 / 30.0;		break;
					case FPS_30_DROP:		dFramePeriod = 1.0 / 30.0;		break;

					default:
						return;
				}

				mfxTime.dSeconds += (double)iIncrement * dFramePeriod;
				mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			}
			break;

		default:
			return;
	}

	m_pTransport->SetTransportTime(TRANSPORT_TIME_CURSOR, &mfxTime);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::OnPlayPressed()
{
	if (GetTransportState(TRANSPORT_STATE_PLAY))
		DoCommand(NEW_CMD_STOP_WITH_NOW_MARKER);		// Pause
	else
		SetTransportState(TRANSPORT_STATE_PLAY, true);	// Play
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::OnContextSwitch()
{
	TRACE("CMackieControlBase::OnContextSwitch()\n");

	if (m_pParamMapping)
		m_pParamMapping->OnContextSwitch(m_dwUniqueId);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::LimitAndSetStripNumOffset(int iStripNumOffset)
{
	int iNumStrips = GetStripCount(m_cState.GetMixerStrip());
	int iLastFader = m_cState.GetLastFaderNumber();

	if (iStripNumOffset + iLastFader >= iNumStrips)
		iStripNumOffset = iNumStrips - iLastFader - 1;

	if (iStripNumOffset < 0)
		iStripNumOffset = 0;

	m_cState.SetStripNumOffset(iStripNumOffset);
}

/////////////////////////////////////////////////////////////////////////////
