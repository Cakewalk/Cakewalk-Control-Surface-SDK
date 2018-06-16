#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

#include "MackieControlLCDDisplay.h"
#include "MackieControlVPotDisplay.h"
#include "MackieControlFader.h"
#include "MackieControlXT.h"

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::OnRefreshSurface(DWORD fdwRefresh, bool bForceSend)
{
//	TRACE("CMackieControlMaster::OnRefreshSurface()\n");

	// First see if there's any temp text we need to display
	CheckForTempDisplayText();

	// Make sure the parameter offset is always in range too
//	ShiftParamNumOffset(0);		// Maybe not such a good idea

	// Update the XT section
	CMackieControlXT::OnRefreshSurface(fdwRefresh, bForceSend);

	// Now update the master section
	CCriticalSectionAuto csa(m_cState.GetCS());

	ReconfigureMaster(bForceSend);

	// Do the reconfigure before this so that the setup is up to date
	UpdateToolbarDisplay(bForceSend);

	UpdateLEDs(bForceSend);
	UpdateTransportLEDs(bForceSend);
	UpdateVSelectDisplay(bForceSend);
	UpdateTimeCodeDisplay(bForceSend);

	if (!m_cState.GetDisableFaders())
		UpdateMasterFader(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::CheckForTempDisplayText()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	char *szText  = m_cState.GetTempDisplayText();

	if (szText[0] != 0)
	{
		TempDisplay(TEMP_DISPLAY_TIMEOUT, szText);

		m_cState.SetTempDisplayText("");
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::UpdateLEDs(bool bForceSend)
{
	SetLED(MC_PARAM,		m_cState.GetAssignment() == MCS_ASSIGNMENT_PARAMETER,	bForceSend);
	SetLED(MC_SEND,			m_cState.GetAssignment() == MCS_ASSIGNMENT_SEND,		bForceSend);
	SetLED(MC_PAN,			m_cState.GetAssignment() == MCS_ASSIGNMENT_PAN,			bForceSend);
	SetLED(MC_PLUG_INS,		m_cState.GetAssignment() == MCS_ASSIGNMENT_PLUGIN,		bForceSend);
	SetLED(MC_EQ,			m_cState.GetAssignment() == MCS_ASSIGNMENT_EQ_FREQ_GAIN ||
							m_cState.GetAssignment() == MCS_ASSIGNMENT_EQ,			bForceSend);
	SetLED(MC_DYNAMICS,		m_cState.GetAssignment() == MCS_ASSIGNMENT_DYNAMICS,	bForceSend);

	if (MCS_FLIP_NORMAL == m_cState.GetFlipMode())
		SetLED(MC_FLIP,		0,														bForceSend);
	else if (MCS_FLIP_DUPLICATE == m_cState.GetFlipMode())
		SetLED(MC_FLIP,		2,														bForceSend);
	else
		SetLED(MC_FLIP,		1,														bForceSend);

	if (m_cState.GetModifiers(MCS_MODIFIER_EDIT))
		SetLED(MC_EDIT,		1,														bForceSend);
	else if (m_cState.GetModifiers(MCS_MODIFIER_NUMERIC))
		SetLED(MC_EDIT,		2,														bForceSend);
	else
		SetLED(MC_EDIT,		0,														bForceSend);

	SetLED(MC_READ_OFF,		m_bSwitches[MC_READ_OFF],								bForceSend);
	SetLED(MC_SNAPSHOT,		m_bSwitches[MC_SNAPSHOT],								bForceSend);
	SetLED(MC_DISARM,		m_bSwitches[MC_DISARM],									bForceSend);
	SetLED(MC_OFFSET,		m_bSwitches[MC_OFFSET],									bForceSend);

	SetLED(MC_TRACK,		m_cState.GetMixerStrip() == MIX_STRIP_TRACK,			bForceSend);
	SetLED(MC_AUX,			m_cState.GetMixerStrip() == m_cState.BusType(),			bForceSend);
	SetLED(MC_MAIN,			m_cState.GetMixerStrip() == m_cState.MasterType(),		bForceSend);

	SetLED(MC_SAVE,			m_bSwitches[MC_SAVE],									bForceSend);

	SetLED(MC_MARKER,		m_cState.GetNavigationMode() == MCS_NAVIGATION_MARKER,	bForceSend);
	SetLED(MC_LOOP,			m_cState.GetNavigationMode() == MCS_NAVIGATION_LOOP,	bForceSend);
	SetLED(MC_SELECT,		m_cState.GetNavigationMode() == MCS_NAVIGATION_SELECT,	bForceSend);
	SetLED(MC_PUNCH,		m_cState.GetNavigationMode() == MCS_NAVIGATION_PUNCH,	bForceSend);

	SetLED(MC_JOG_PRM,		m_cState.GetJogParamMode(),								bForceSend);
	SetLED(MC_HOME,			m_bSwitches[MC_HOME],									bForceSend);

	SetLED(MC_CURSOR_ZOOM,	(BYTE)m_cState.GetModifiers(MCS_MODIFIER_ZOOM),				bForceSend);

	SetLED(MC_SMPTE,		m_cState.GetDisplaySMPTE(),								bForceSend);
	SetLED(MC_BEATS,		!m_cState.GetDisplaySMPTE(),							bForceSend);

	SetLED(MC_RUDE,			GetRudeSoloStatus() ? 2 : 0,							bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::UpdateTransportLEDs(bool bForceSend)
{
	bool bPlaying = GetTransportState(TRANSPORT_STATE_PLAY);

	SetLED(MC_LOOP_ON_OFF,	GetTransportState(TRANSPORT_STATE_LOOP),	bForceSend);
	SetLED(MC_REWIND,		m_bSwitches[MC_REWIND],						bForceSend);
	SetLED(MC_FAST_FORWARD,	m_bSwitches[MC_FAST_FORWARD],				bForceSend);
	SetLED(MC_STOP,			!bPlaying,									bForceSend);
	SetLED(MC_PLAY,			bPlaying,									bForceSend);
	SetLED(MC_RECORD,		GetTransportState(TRANSPORT_STATE_REC),		bForceSend);
	SetLED(MC_SCRUB,		GetTransportState(TRANSPORT_STATE_SCRUB),	bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::UpdateVSelectDisplay(bool bForceSend)
{
	bool bDot = (m_cState.GetAssignmentMode() == MCS_ASSIGNMENT_CHANNEL_STRIP);

	// Display from '1' to '0'
	DWORD dwPluginNum = m_cState.GetPluginNumOffset() + 1;
	if (dwPluginNum > 9)
		dwPluginNum -= 10;

	switch (m_cState.GetAssignment())
	{
		case MCS_ASSIGNMENT_PARAMETER:
			switch (m_cState.GetMixerStrip())
			{
				case MIX_STRIP_TRACK:
					m_cVSelectDisplay[0].SetChar('T', false, bForceSend);
					m_cVSelectDisplay[1].SetChar('r', bDot, bForceSend);
					break;

				case MIX_STRIP_AUX:
					m_cVSelectDisplay[0].SetChar('A', false, bForceSend);
					m_cVSelectDisplay[1].SetChar('u', bDot, bForceSend);
					break;

				case MIX_STRIP_MAIN:
					m_cVSelectDisplay[0].SetChar('V', false, bForceSend);
					m_cVSelectDisplay[1].SetChar('m', bDot, bForceSend);
					break;

				case MIX_STRIP_BUS:
					m_cVSelectDisplay[0].SetChar('B', false, bForceSend);
					m_cVSelectDisplay[1].SetChar('u', bDot, bForceSend);
					break;

				case MIX_STRIP_MASTER:
					m_cVSelectDisplay[0].SetChar('O', false, bForceSend);
					m_cVSelectDisplay[1].SetChar('p', bDot, bForceSend);
					break;

				default:
					break;
			}
			break;

		case MCS_ASSIGNMENT_PAN:
			m_cVSelectDisplay[0].SetChar('P', false, bForceSend);
			m_cVSelectDisplay[1].SetChar('n', bDot, bForceSend);
			break;

		case MCS_ASSIGNMENT_SEND:
			m_cVSelectDisplay[0].SetChar('S', false, bForceSend);
			m_cVSelectDisplay[1].SetChar('e', bDot, bForceSend);
			break;

		case MCS_ASSIGNMENT_PLUGIN:
			m_cVSelectDisplay[0].SetChar('P', false, bForceSend);
			m_cVSelectDisplay[1].SetChar((char)dwPluginNum + '0', bDot, bForceSend);
			break;

		case MCS_ASSIGNMENT_EQ:
			m_cVSelectDisplay[0].SetChar('E', false, bForceSend);
			m_cVSelectDisplay[1].SetChar((char)dwPluginNum + '0', bDot, bForceSend);
			break;

		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:
			m_cVSelectDisplay[0].SetChar('F', false, bForceSend);
			m_cVSelectDisplay[1].SetChar((char)dwPluginNum + '0', bDot, bForceSend);
			break;

		case MCS_ASSIGNMENT_DYNAMICS:
			m_cVSelectDisplay[0].SetChar('D', false, bForceSend);
			m_cVSelectDisplay[1].SetChar((char)dwPluginNum + '0', bDot, bForceSend);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::UpdateTimeCodeDisplay(bool bForceSend)
{
	MFX_TIME timeCurrent;

	// First check to see if it's changed
	timeCurrent.timeFormat = TF_SECONDS;
	HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &timeCurrent);

	if (FAILED(hr) || m_cState.GetLastDisplayTime() == timeCurrent.dSeconds)
		return;

	m_cState.SetLastDisplayTime(timeCurrent.dSeconds);

	// It has...

	char sBuf[32];

	if (m_cState.GetDisplaySMPTE())
	{
		timeCurrent.timeFormat = TF_SMPTE_REL;
		HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &timeCurrent);

		if (FAILED(hr))
			return;

//		TRACE("SMPTE: %d %d %d %d %d\n", timeCurrent.smpte.nHour, timeCurrent.smpte.nMin,
//										timeCurrent.smpte.nSec, timeCurrent.smpte.nFrame,
//										timeCurrent.smpte.nSub400);

		if (timeCurrent.smpte.nSub400 >= 200)
			timeCurrent.smpte.nFrame++;

		char cFrameMax = 0;
		switch (timeCurrent.smpte.fps)
		{
			case FPS_24:			cFrameMax = 24;		break;
			case FPS_25:			cFrameMax = 25;		break;
			case FPS_2997:			cFrameMax = 29;		break;
			case FPS_2997_DROP:		cFrameMax = 29;		break;
			case FPS_30:			cFrameMax = 30;		break;
			case FPS_30_DROP:		cFrameMax = 30;		break;

			default:
				return;
		}

		if (timeCurrent.smpte.nFrame >= cFrameMax)
		{
			timeCurrent.smpte.nFrame = 0;
			timeCurrent.smpte.nSec++;
		}
		if (timeCurrent.smpte.nSec >= 60)
		{
			timeCurrent.smpte.nSec = 0;
			timeCurrent.smpte.nMin++;
		}
		if (timeCurrent.smpte.nMin >= 60)
		{
			timeCurrent.smpte.nMin = 0;
			timeCurrent.smpte.nHour++;
		}

		char sHour[8];
		snprintf(sHour, sizeof(sHour), "%02d", timeCurrent.smpte.nHour);

		char sFrame[8];
		snprintf(sFrame, sizeof(sFrame), "%02d", timeCurrent.smpte.nFrame);

		snprintf(sBuf, sizeof(sBuf), "%3s.%02d.%02d.%3s", sHour, timeCurrent.smpte.nMin,
											timeCurrent.smpte.nSec, sFrame);
	}
	else
	{
		timeCurrent.timeFormat = TF_MBT;
		HRESULT hr = m_pTransport->GetTransportTime(TRANSPORT_TIME_CURSOR, &timeCurrent);

		if (FAILED(hr))
			return;

		snprintf(sBuf, sizeof(sBuf), "%3d%02d  %03d", timeCurrent.mbt.nMeas,
										timeCurrent.mbt.nBeat,
										timeCurrent.mbt.nTick);
	}

	char *s = sBuf;

	for (int n = 0; n < NUM_TIME_CODE_DISPLAY_CELLS; n++)
	{
		if (*s)
		{
			bool bDot = (s[1] == '.');

			m_cTimeCodeDisplay[n].SetChar(*s, bDot, bForceSend);

			if (bDot)
				s++;
			s++;
		}
		else
		{
			m_cTimeCodeDisplay[n].SetChar(' ', false, bForceSend);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::UpdateMasterFader(bool bForceSend)
{
	if (!HaveMixerStrips())
		return;

	float fVal;

	HRESULT hr = m_SwMasterFader.GetNormalizedVal(&fVal);

	if (FAILED(hr))
		fVal = 0.0f;

	m_HwMasterFader.SetVal(fVal, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////
