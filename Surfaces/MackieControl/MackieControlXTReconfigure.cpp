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
#include "MackieControlXTPropPage.h"

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::ReconfigureXT(bool bForce)
{
	// Check what this version of SONAR can do
	m_cState.DetermineCapabilities(m_pMixer, m_pSonarIdentity);

	SONAR_MIXER_STRIP eMixerStrip = m_cState.GetMixerStrip();
	DWORD dwStripCount = GetStripCount(eMixerStrip);

	// Read back the selected track and updated the surface
	if (eMixerStrip == MIX_STRIP_TRACK && m_cState.GetSelectHighlightsTrack() )
		m_cState.SetSelectedStripNum(GetSelected(), m_pMixer);

	// Tracks can be deleted, so fix the selected track index if necessary
	if (m_cState.GetSelectedStripNum() >= dwStripCount && dwStripCount > 0)
		m_cState.SetSelectedStripNum(dwStripCount - 1, m_pMixer);

	Assignment eAssignment = m_cState.GetAssignment();
	AssignmentMode eAssignmentMode = m_cState.GetAssignmentMode();
	DWORD dwSelectedStripNum = m_cState.GetSelectedStripNum();
	DWORD dwStripNumOffset = m_cState.GetStripNumOffset();
	DWORD dwPluginCount = 0;

	// Check for changes in the number of plugins
	if (IsAPluginMode(eAssignment))
	{
		if (MCS_ASSIGNMENT_MUTLI_CHANNEL == eAssignmentMode)
		{
			for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
			{
				DWORD N = m_dwUnitStripNumOffset + dwStripNumOffset + n;

				dwPluginCount += GetPluginCount(eMixerStrip, N);
			}
		}
		else
		{
			dwPluginCount = GetPluginCount(eMixerStrip, dwSelectedStripNum);
		}
	}
	else
	{
		dwPluginCount = m_dwPreviousPluginCount;
	}

	// Check to see if an Audio and MIDI track have been swapped
	if (!bForce)
	{
		for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		{
			if (!m_SwStrip[n].CheckBinding())
			{
				TRACE("CMackieControlXT: binding changed required\n");
				bForce = true;
				break;
			}
		}
	}

	// Do we need to reconfigure?
	if (!bForce && 
		m_cState.GetXTUpdateCount() == m_dwXTUpdateCount &&
		dwStripCount == m_dwPreviousStripCount &&
		dwPluginCount == m_dwPreviousPluginCount)
		return;

	TRACE("CMackieControlXT::ReconfigureXT(): [0x%08X] Reconfiguring, %d\n", this, m_dwUnitStripNumOffset);

	void (CMackieControlXT::* pConf)(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);

	switch (eAssignment)
	{
		case MCS_ASSIGNMENT_PARAMETER:
			switch (eMixerStrip)
			{
			case MIX_STRIP_TRACK:		pConf = &CMackieControlXT::ConfParameterTrack;		break;
				case MIX_STRIP_AUX:			pConf = &CMackieControlXT::ConfParameterAux;		break;
				case MIX_STRIP_MAIN:		pConf = &CMackieControlXT::ConfParameterMain;		break;
				case MIX_STRIP_BUS:			pConf = &CMackieControlXT::ConfParameterBus;		break;
				case MIX_STRIP_MASTER:		pConf = &CMackieControlXT::ConfParameterMaster;	break;
				default:
					TRACE("CMackieControlXT::ReconfigureXT(): Error: unknown strip type!\n");
					return;
			}
			break;
		case MCS_ASSIGNMENT_PAN:			pConf = &CMackieControlXT::ConfPan;				break;
		case MCS_ASSIGNMENT_SEND:			pConf = &CMackieControlXT::ConfSend;				break;
		case MCS_ASSIGNMENT_PLUGIN:			pConf = &CMackieControlXT::ConfPlugin;				break;
		case MCS_ASSIGNMENT_EQ:				pConf = &CMackieControlXT::ConfEQ;					break;
		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:	pConf = &CMackieControlXT::ConfNoBindings;			break;
		case MCS_ASSIGNMENT_DYNAMICS:		pConf = &CMackieControlXT::ConfDynamics;			break;
		default:
			TRACE("CMackieControlXT::ReconfigureXT(): Error: unknown assignment!\n");
			return;
	}

	DWORD dwPluginNumOffset = m_cState.GetPluginNumOffset();
	DWORD dwParamNumOffset = m_cState.GetParamNumOffset();

	// Flip the faders and VPots?
	FlipMode eFlipMode = m_cState.GetFlipMode();

	CMixParam *pSwVPot;
	CMixParam *pSwFader;

	switch (eFlipMode)
	{
		case MCS_FLIP_NORMAL:
		case MCS_FLIP_DUPLICATE:
			pSwVPot	= &m_SwVPot[0];
			pSwFader = &m_SwFader[0];
			break;

		case MCS_FLIP_FLIP:
			pSwVPot	= &m_SwFader[0];
			pSwFader = &m_SwVPot[0];
			break;

		default:
			TRACE("CMackieControlXT::ReconfigureXT(): Error: unknown flip mode!\n");
			return;
	}

	int n = 0;

	// These are not dependent on the assignment or assignment mode
	for (n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		DWORD N = m_dwUnitStripNumOffset + dwStripNumOffset + n;

		if (N < dwStripCount)
		{
			// This one isn't mapped to any controls, but it's used to determine
			// if the strip exists and hence if values should be displayed
			//
			// We can't just use m_SwFader because of the flip modes
			//
			m_SwStrip[n].SetParams(eMixerStrip, N, MIX_PARAM_VOL, 0);
		}
		else
		{
			m_SwStrip[n].ClearBinding();
		}

		// These depend on the mixer strip type
		if (MIX_STRIP_TRACK == eMixerStrip && N < dwStripCount)
		{
			// Rec
			m_SwRec[n].SetParams(eMixerStrip, N, MIX_PARAM_RECORD_ARM, 0);
			m_SwRec[n].SetAttribs(DT_BOOL);

			// Solo
			m_SwSolo[n].SetParams(eMixerStrip, N, MIX_PARAM_SOLO, 0);
			m_SwSolo[n].SetAttribs(DT_BOOL);

			// Mute
			m_SwMute[n].SetParams(eMixerStrip, N, MIX_PARAM_MUTE, 0);
			m_SwMute[n].SetAttribs(DT_BOOL);

			// Archive
			m_SwArchive[n].SetParams(eMixerStrip, N, MIX_PARAM_ARCHIVE, 0);
			m_SwArchive[n].SetAttribs(DT_BOOL);

			// Input Echo
			if (m_cState.HaveBuses())
			{
				m_SwInputEcho[n].SetParams(eMixerStrip, N, MIX_PARAM_INPUT_ECHO, 0);
				m_SwInputEcho[n].SetAttribs(DT_BOOL);
			}
			else
			{
				m_SwInputEcho[n].ClearBinding();
			}
		}
		else if (MIX_STRIP_BUS == eMixerStrip && N < dwStripCount)
		{
			// Rec
			m_SwRec[n].ClearBinding();

			// Solo
			m_SwSolo[n].SetParams(eMixerStrip, N, MIX_PARAM_SOLO, 0);
			m_SwSolo[n].SetAttribs(DT_BOOL);

			// Mute
			m_SwMute[n].SetParams(eMixerStrip, N, MIX_PARAM_MUTE, 0);
			m_SwMute[n].SetAttribs(DT_BOOL);

			// Archive
			m_SwArchive[n].ClearBinding();

			// Input Echo
			m_SwInputEcho[n].ClearBinding();
		}
		else if (MIX_STRIP_MASTER == eMixerStrip && N < dwStripCount)
		{
			// Rec
			m_SwRec[n].ClearBinding();

			// Solo
			m_SwSolo[n].ClearBinding();

			// Mute
			m_SwMute[n].SetParams(eMixerStrip, N, MIX_PARAM_MUTE, 0);
			m_SwMute[n].SetAttribs(DT_BOOL);

			// Archive
			m_SwArchive[n].ClearBinding();

			// Input Echo
			m_SwInputEcho[n].ClearBinding();
		}
		else
		{
			m_SwRec[n].ClearBinding();
			m_SwSolo[n].ClearBinding();
			m_SwMute[n].ClearBinding();
			m_SwArchive[n].ClearBinding();
			m_SwInputEcho[n].ClearBinding();
		}
	}

	// Metering
	if (m_cState.HaveMeters())
	{
		bool bEnableMeters = (m_cState.GetDisplayLevelMeters() != METERS_OFF);

		for (n = 0; n < NUM_MAIN_CHANNELS; n++)
			m_SwStrip[n].SetMetering(bEnableMeters);
	}

	if (m_cState.GetAssignment() == MCS_ASSIGNMENT_EQ_FREQ_GAIN)
	{
		for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		{
			if (dwSelectedStripNum < dwStripCount)
			{
				DWORD N = m_dwUnitStripNumOffset + dwParamNumOffset + n;

				if (MCS_FLIP_DUPLICATE == eFlipMode)
				{
					ConfEQGainFreq(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, NULL, pSwFader + n);
					ConfEQGainFreq(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, NULL, pSwVPot + n);
				}
				else
				{
					ConfEQGainFreq(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, pSwFader + n, pSwVPot + n);
				}
			}
			else
			{
				pSwFader[n].ClearBinding();
				pSwVPot[n].ClearBinding();
			}
		}
	}
	else
	{
		DWORD dwModifiers = m_cState.GetModifiers(M1_TO_M4_MASK);

		// Faders
		if (MCS_FLIP_DUPLICATE != eFlipMode)
		{
			for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
			{
				DWORD N = m_dwUnitStripNumOffset + dwStripNumOffset + n;

				if (N < dwStripCount)
					(this->*pConf)(eMixerStrip, N, dwPluginNumOffset, IDX_DEFAULT_FADER, dwModifiers, pSwFader + n);
				else
					pSwFader[n].ClearBinding();
			}
		}

		// VPots are dependent on the assignment mode
		if (MCS_ASSIGNMENT_MUTLI_CHANNEL == eAssignmentMode)
		{
			for (n = 0; n < NUM_MAIN_CHANNELS; n++)
			{
				DWORD N = m_dwUnitStripNumOffset + dwStripNumOffset + n;

				if (N < dwStripCount)
					(this->*pConf)(eMixerStrip, N, dwPluginNumOffset, dwParamNumOffset, dwModifiers, pSwVPot + n);
				else
					pSwVPot[n].ClearBinding();

				if (MCS_FLIP_DUPLICATE == eFlipMode)
				{
					if (N < dwStripCount)
						(this->*pConf)(eMixerStrip, N, dwPluginNumOffset, dwParamNumOffset, dwModifiers, pSwFader + n);
					else
						pSwFader[n].ClearBinding();
				}
			}
		}
		else if (MCS_ASSIGNMENT_CHANNEL_STRIP == eAssignmentMode)
		{
			for (n = 0; n < NUM_MAIN_CHANNELS; n++)
			{
				DWORD N = m_dwUnitStripNumOffset + dwParamNumOffset + n;

				if (dwSelectedStripNum < dwStripCount)
					(this->*pConf)(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, dwModifiers, pSwVPot + n);
				else
					pSwVPot[n].ClearBinding();

				if (MCS_FLIP_DUPLICATE == eFlipMode)
				{
					if (dwSelectedStripNum < dwStripCount)
						(this->*pConf)(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, dwModifiers, pSwFader + n);
					else
						pSwFader[n].ClearBinding();
				}
			}
		}
	}

	OnContextSwitch();

	// Remember the counter values for next time
	m_dwXTUpdateCount = m_cState.GetXTUpdateCount();
	m_dwPreviousStripCount = dwStripCount;
	m_dwPreviousPluginCount = dwPluginCount;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::ConfEQGainFreq(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									  DWORD dwPluginNum, DWORD dwIndex, CMixParam *pFader, CMixParam *pVPot)
{
	CPluginProperties cPluginProps;

	if (!GetPluginProperties(eMixerStrip, dwStripNum, &dwPluginNum, PT_EQ, &cPluginProps))
	{
		if (pFader)	pFader->ClearBinding();
		if (pVPot)	pVPot->ClearBinding();

		return;
	}

	// Fader == Gain
	if (pFader)
	{
		SetParamToProperty(eMixerStrip, dwStripNum, dwIndex, dwPluginNum, &cPluginProps.m_mapParamPropsGain, pFader);
	}

	// VPot == Freq
	if (pVPot)
	{
		mapParameterProperties *pParamProps;
		bool bAllowFineResolution = true;

		switch (m_cState.GetModifiers(M1_TO_M4_MASK))
		{
			case MCS_MODIFIER_NONE:
				pParamProps = &cPluginProps.m_mapParamPropsCourseFreq;
				break;

			case MCS_MODIFIER_M1:
				if (cPluginProps.m_mapParamPropsFineFreq.empty())
				{
					pParamProps = &cPluginProps.m_mapParamPropsCourseFreq;
					bAllowFineResolution = true;
				}
				else
				{
					pParamProps = &cPluginProps.m_mapParamPropsFineFreq;
					bAllowFineResolution = false;
				}
				break;

			case MCS_MODIFIER_M2:
			case MCS_MODIFIER_M1 | MCS_MODIFIER_M2:
				pParamProps = &cPluginProps.m_mapParamPropsQ;
				break;

			case MCS_MODIFIER_M4:
			case MCS_MODIFIER_M1 | MCS_MODIFIER_M4:
				pParamProps = &cPluginProps.m_mapParamPropsBandEnable;
				break;

			default:
				pVPot->ClearBinding();
				return;
		}
		
		SetParamToProperty(eMixerStrip, dwStripNum, dwIndex, dwPluginNum, pParamProps, pVPot);
		pVPot->SetAllowFineResolution(bAllowFineResolution);
	}
}

/////////////////////////////////////////////////////////////////////////////
