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
#include "MackieControlC4.h"
#include "MackieControlC4PropPage.h"

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::ReconfigureC4(bool bForce)
{
	// Check what this version of SONAR can do
	m_cState.DetermineCapabilities(m_pMixer, m_pSonarIdentity);

	bool bReconfigured = false;

	switch (m_eSplitMode)
	{
		default:
		case C4_SPLIT_NONE:
			bReconfigured =  ReconfigureC4Half(C4_UPPER, 0, 3, bForce);
			break;

		case C4_SPLIT_1_3:
			bReconfigured =  ReconfigureC4Half(C4_UPPER, 0, 0, bForce);
			bReconfigured |= ReconfigureC4Half(C4_LOWER, 1, 3, bForce);
			break;

		case C4_SPLIT_2_2:
			bReconfigured =  ReconfigureC4Half(C4_UPPER, 0, 1, bForce);
			bReconfigured |= ReconfigureC4Half(C4_LOWER, 2, 3, bForce);
			break;
		
		case C4_SPLIT_3_1:
			bReconfigured =  ReconfigureC4Half(C4_UPPER, 0, 2, bForce);
			bReconfigured |= ReconfigureC4Half(C4_LOWER, 3, 3, bForce);
			break;
	}

	// Metering
	if (bReconfigured && m_cState.HaveMeters())
	{
		// The C4 effectively acts as two control surfaces, but has to share
		// a single unique ID. It's possible (even likely) that the two halves
		// will view the same tracks, but with only one half having metering
		// enabled. 
		
		// We need to avoid one half disabling the meters in use by the other,
		// so first go through and disable any meters that are no longer
		// in use...
		int m = 0;

		for (m = 0; m < NUM_ROWS; m++)
		{
			C4SplitSection eSplit = GetSplitForRow(m);

			bool bEnableMeters = (MCS_ASSIGNMENT_MUTLI_CHANNEL == GetAssignmentMode(eSplit) &&
									GetDisplayLevelMeters(eSplit));

			if (!bEnableMeters)
			{
				for (int n = 0; n < NUM_COLS; n++)
					m_SwStrip[m][n].SetMetering(false);
			}
		}

		// ...and then go through and again and enable any meters which are
		// required.
		for (m = 0; m < NUM_ROWS; m++)
		{
			C4SplitSection eSplit = GetSplitForRow(m);

			bool bEnableMeters = (MCS_ASSIGNMENT_MUTLI_CHANNEL == GetAssignmentMode(eSplit) &&
									GetDisplayLevelMeters(eSplit));

			if (bEnableMeters)
			{
				for (int n = 0; n < NUM_COLS; n++)
					m_SwStrip[m][n].SetMetering(true);
			}
		}
	}

	if (bReconfigured)
		OnContextSwitch();

	// Remember the counter values for next time
	m_dwC4UpdateCount = m_cState.GetC4UpdateCount();
	m_dwC4PreviousUpdateCount = m_dwC4UpdateCount;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlC4::ReconfigureC4Half(C4SplitSection eSplit, int first, int last, bool bForce)
{
	SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);
	DWORD dwStripCount = GetStripCount(eMixerStrip);

	// Tracks can be deleted, so fix the selected track index if necessary
	if (GetSelectedStripNum(eSplit) >= dwStripCount && dwStripCount > 0)
		SetSelectedStripNum(eSplit, dwStripCount - 1, m_pMixer);

	Assignment eAssignment = GetAssignment(eSplit);
	AssignmentMode eAssignmentMode = GetAssignmentMode(eSplit);
	DWORD dwSelectedStripNum = GetSelectedStripNum(eSplit);
	DWORD dwStripNumOffset = GetStripNumOffset(eSplit);
	DWORD dwPluginCount = 0;

	// If the assignment changed in channel strip mode it's
	// nice to tell the user what they're editing
	if (m_ePreviousAssignment[eSplit] != eAssignment &&
		eAssignmentMode == MCS_ASSIGNMENT_CHANNEL_STRIP)
	{
		TempDisplaySelectedTrackName(eSplit);
	}

	// Check for changes in the number of plugins
	if (IsAPluginMode(eAssignment))
	{
		if (MCS_ASSIGNMENT_MUTLI_CHANNEL == eAssignmentMode)
		{
			DWORD N = dwStripNumOffset;

			for (int m = first; m <= last; m++)
			{
				for (int n = 0; n < NUM_COLS; n++)
				{
					dwPluginCount += GetPluginCount(eMixerStrip, N);
					N++;
				}
			}
		}
		else
		{
			dwPluginCount = GetPluginCount(eMixerStrip, dwSelectedStripNum);
		}
	}
	else
	{
		dwPluginCount = m_dwPreviousPluginCount[eSplit];
	}

	// Check to see if an Audio and MIDI track have been swapped
	if (!bForce)
	{
		for (int m = first; m <= last; m++)
		{
			for (int n = 0; n < NUM_COLS; n++)
			{
				if (!m_SwStrip[m][n].CheckBinding())
				{
					TRACE("CMackieControlC4: binding changed required\n");
					bForce = true;
					break;
				}
			}
		}
	}

	// Do we need to reconfigure?
	if (!bForce && 
		m_cState.GetC4UpdateCount() == m_dwC4UpdateCount &&
		m_dwC4PreviousUpdateCount == m_dwC4UpdateCount &&
		dwStripCount == m_dwPreviousStripCount[eSplit] &&
		dwPluginCount == m_dwPreviousPluginCount[eSplit])
		return false;

	TRACE("CMackieControlC4::ReconfigureC4(): [0x%08X] Reconfiguring\n", this);

	void (CMackieControlC4::* pConf)(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);

	switch (eAssignment)
	{
		case MCS_ASSIGNMENT_PARAMETER:
			switch (eMixerStrip)
			{
				case MIX_STRIP_TRACK:		pConf = &CMackieControlC4::ConfParameterTrack;		break;
				case MIX_STRIP_AUX:			pConf = &CMackieControlC4::ConfParameterAux;		break;
				case MIX_STRIP_MAIN:		pConf = &CMackieControlC4::ConfParameterMain;		break;
				case MIX_STRIP_BUS:			pConf = &CMackieControlC4::ConfParameterBus;		break;
				case MIX_STRIP_MASTER:		pConf = &CMackieControlC4::ConfParameterMaster;	break;
				default:
					TRACE("CMackieControlC4::ReconfigureC4(): Error: unknown strip type!\n");
					return false;
			}
			break;
		case MCS_ASSIGNMENT_PAN:			pConf = &CMackieControlC4::ConfPan;				break;
		case MCS_ASSIGNMENT_SEND:			pConf = &CMackieControlC4::ConfSend;				break;
		case MCS_ASSIGNMENT_PLUGIN:			pConf = &CMackieControlC4::ConfPlugin;				break;
		case MCS_ASSIGNMENT_EQ:				pConf = &CMackieControlC4::ConfEQ;					break;
		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:	pConf = &CMackieControlC4::ConfEQ;					break;
		case MCS_ASSIGNMENT_DYNAMICS:		pConf = &CMackieControlC4::ConfDynamics;			break;
		default:
			TRACE("CMackieControlC4::ReconfigureC4(): Error: unknown assignment!\n");
			return false;
	}

	DWORD dwPluginNumOffset = GetPluginNumOffset(eSplit);
	DWORD dwParamNumOffset = GetParamNumOffset(eSplit);

	// These are not dependent on the assignment or assignment mode
	{
		DWORD N = dwStripNumOffset;

		for (int m = first; m <= last; m++)
		{
			for (int n = 0; n < NUM_COLS; n++)
			{
				if (N < dwStripCount)
				{
					// This one isn't mapped to any controls, but it's used to determine
					// if the strip exists and hence if values should be displayed
					//
					// We can't just use m_SwFader because of the flip modes
					//
					m_SwStrip[m][n].SetParams(eMixerStrip, N, MIX_PARAM_VOL, 0);
				}
				else
				{
					m_SwStrip[m][n].ClearBinding();
				}

				N++;
			}
		}
	}

	// Only check for M1 - this stops the C4 using the M2-M4 quick access bindings
	DWORD dwModifiers = GetModifiers(MCS_MODIFIER_M1);

	// VPots are dependent on the assignment mode
	if (MCS_ASSIGNMENT_MUTLI_CHANNEL == eAssignmentMode)
	{
		DWORD N = dwStripNumOffset;

		for (int m = first; m <= last; m++)
		{
			for (int n = 0; n < NUM_COLS; n++)
			{
				if (N < dwStripCount)
					(this->*pConf)(eMixerStrip, N, dwPluginNumOffset, dwParamNumOffset, dwModifiers, &m_SwVPot[m][n]);
				else
					m_SwVPot[m][n].ClearBinding();

				N++;
			}
		}
	}
	else if (MCS_ASSIGNMENT_CHANNEL_STRIP == eAssignmentMode)
	{
		DWORD N = dwParamNumOffset;

		for (int m = first; m <= last; m++)
		{
			for (int n = 0; n < NUM_COLS; n++)
			{
				if (dwSelectedStripNum < dwStripCount)
					(this->*pConf)(eMixerStrip, dwSelectedStripNum, dwPluginNumOffset, N, dwModifiers, &m_SwVPot[m][n]);
				else
					m_SwVPot[m][n].ClearBinding();

				N++;
			}
		}
	}

	// Remember the counter values for next time
	m_dwPreviousStripCount[eSplit] = dwStripCount;
	m_dwPreviousPluginCount[eSplit] = dwPluginCount;
	m_ePreviousAssignment[eSplit] = eAssignment;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
