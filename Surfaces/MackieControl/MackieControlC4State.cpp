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

// The current mixer strip (Track, Bus or Master)
//
// Linked to the main unit unless we're in locked mode

SONAR_MIXER_STRIP CMackieControlC4::GetMixerStrip(C4SplitSection eSplit)
{
	if (m_bLockMode[eSplit])
		return m_eMixerStrip[eSplit];
	else
		return m_cState.GetMixerStrip();
}

void CMackieControlC4::SetMixerStrip(C4SplitSection eSplit, SONAR_MIXER_STRIP eMixerStrip)
{
	if (m_bLockMode[eSplit])
	{
		if (m_eMixerStrip[eSplit] != eMixerStrip)
		{
			m_eMixerStrip[eSplit] = eMixerStrip;

			m_cState.BumpToolbarUpdateCount();
			m_dwC4UpdateCount++;
		}
	}
	else
	{
		m_cState.SetMixerStrip(eMixerStrip);
	}
}

/////////////////////////////////////////////////////////////////////////////

// The current assignment (Parameter, Pan, Send, Plugin, Eq or Dynamics)
//
// Linked to the main unit unless we're in locked mode

Assignment CMackieControlC4::GetAssignment(C4SplitSection eSplit)
{
	if (m_eAssignment[eSplit] == MCS_ASSIGNMENT_EQ_FREQ_GAIN)
		return MCS_ASSIGNMENT_EQ;

	return m_eAssignment[eSplit];
}

void CMackieControlC4::SetAssignment(C4SplitSection eSplit, Assignment eAssignment)
{
	if (m_eAssignment[eSplit] != eAssignment)
	{
		m_eAssignment[eSplit] = eAssignment;

		m_cState.BumpToolbarUpdateCount();
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

// The current assignment mode (multi channel or channel strip)
//
// Always local

AssignmentMode CMackieControlC4::GetAssignmentMode(C4SplitSection eSplit)
{
	return m_eAssignmentMode[eSplit];
}

/////////////////////////////////////////////////////////////////////////////

// The current plugin offset

// Always local

DWORD CMackieControlC4::GetPluginNumOffset(C4SplitSection eSplit)
{
	return m_dwPluginNumOffset[eSplit][GetMixerStrip(eSplit)][GetAssignment(eSplit)];
}

void CMackieControlC4::ShiftPluginNumOffset(C4SplitSection eSplit, int iAmount)
{
	if (IsAPluginMode(GetAssignment(eSplit)))
	{
		int iPluginNumOffset = GetPluginNumOffset(eSplit) + iAmount;

		if (iPluginNumOffset < 0)
			iPluginNumOffset = 0;
		else if (iPluginNumOffset > 9)
			iPluginNumOffset = 9;

		if (GetPluginNumOffset(eSplit) != iPluginNumOffset)
		{
			SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);
			Assignment eAssignment = GetAssignment(eSplit);

			if (MCS_ASSIGNMENT_EQ == eAssignment || MCS_ASSIGNMENT_EQ_FREQ_GAIN == eAssignment)
			{
				// These two shouldn't have separate offsets
				m_dwPluginNumOffset[eSplit][eMixerStrip][MCS_ASSIGNMENT_EQ] = iPluginNumOffset;
				m_dwPluginNumOffset[eSplit][eMixerStrip][MCS_ASSIGNMENT_EQ_FREQ_GAIN] = iPluginNumOffset;
			}
			else
			{
				m_dwPluginNumOffset[eSplit][eMixerStrip][eAssignment] = iPluginNumOffset;
			}

			m_dwC4UpdateCount++;
		}

		TempDisplaySelectedTrackName(eSplit);
	}
}

/////////////////////////////////////////////////////////////////////////////

// The current parameter offset

// Always local

DWORD CMackieControlC4::GetParamNumOffset(C4SplitSection eSplit)
{
	return m_dwParamNumOffset[eSplit][GetMixerStrip(eSplit)][GetAssignment(eSplit)][GetAssignmentMode(eSplit)];
}

void CMackieControlC4::ShiftParamNumOffset(C4SplitSection eSplit, int iAmount)
{
	int iParamNumOffset = GetParamNumOffset(eSplit) + iAmount;

	SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);
	Assignment eAssignment = GetAssignment(eSplit);
	AssignmentMode eAssignmentMode = GetAssignmentMode(eSplit);

	int iNumParams = GetNumParams(eMixerStrip, GetSelectedStripNum(eSplit), GetPluginNumOffset(eSplit), eAssignment);

	if (MCS_ASSIGNMENT_CHANNEL_STRIP == eAssignmentMode)
	{
		int iLastVPot = GetNumVPotsInSplit(eSplit) - 1;

		if (iParamNumOffset + iLastVPot >= iNumParams)
			iParamNumOffset = iNumParams - iLastVPot - 1;
	}
	else	// MCS_ASSIGNMENT_MUTLI_CHANNEL
	{
		if (iParamNumOffset >= iNumParams)
			iParamNumOffset = iNumParams - 1;
	}

	if (iParamNumOffset < 0)
		iParamNumOffset = 0;

	if (m_dwParamNumOffset[eSplit][eMixerStrip][eAssignment][eAssignmentMode] != iParamNumOffset)
	{
		m_dwParamNumOffset[eSplit][eMixerStrip][eAssignment][eAssignmentMode] = iParamNumOffset;

		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

// The current strip offset
//
// Always local

DWORD CMackieControlC4::GetStripNumOffset(C4SplitSection eSplit)
{
	return m_dwStripNumOffset[eSplit][GetMixerStrip(eSplit)];
}

void CMackieControlC4::ShiftStripNumOffset(C4SplitSection eSplit, int iAmount)
{
	TRACE("CMackieControlC4::ShiftStripNumOffset(%d)\n", iAmount);

	LimitAndSetStripNumOffset(eSplit, GetStripNumOffset(eSplit) + iAmount);
}

void CMackieControlC4::LimitAndSetStripNumOffset(C4SplitSection eSplit, int iStripNumOffset)
{
	SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);

	int iNumStrips = GetStripCount(eMixerStrip);
	int iLastVPot = GetNumVPotsInSplit(eSplit) - 1;

	if (iStripNumOffset + iLastVPot >= iNumStrips)
		iStripNumOffset = iNumStrips - iLastVPot - 1;

	if (iStripNumOffset < 0)
		iStripNumOffset = 0;

	if (m_dwStripNumOffset[eSplit][eMixerStrip] != iStripNumOffset)
	{
		m_dwStripNumOffset[eSplit][eMixerStrip] = iStripNumOffset;

		m_cState.BumpToolbarUpdateCount();
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

// The selected strip
//
// Linked to the main unit unless we're in locked mode

// Note there's currently only one of these, there should probably be
// seperate ones for each strip type

DWORD CMackieControlC4::GetSelectedStripNum(C4SplitSection eSplit)
{
	if (m_bLockMode[eSplit])
		return m_dwSelectedStripNum[eSplit];
	else
		return m_cState.GetSelectedStripNum();
}

void CMackieControlC4::ShiftSelectedTrack(C4SplitSection eSplit, int iAmount)
{
	TRACE("CMackieControlC4::ShiftSelectedTrack(%d)\n", iAmount);

	LimitAndSetSelectedTrack(eSplit, GetSelectedStripNum(eSplit) + iAmount);
}

void CMackieControlC4::LimitAndSetSelectedTrack(C4SplitSection eSplit, int iSelectedStripNum)
{
	int iNumStrips = GetStripCount(GetMixerStrip(eSplit));

	if (iSelectedStripNum >= iNumStrips)
		iSelectedStripNum = iNumStrips - 1;

	if (iSelectedStripNum < 0)
		iSelectedStripNum = 0;

	SetSelectedStripNum(eSplit, iSelectedStripNum, m_pMixer);
}

void CMackieControlC4::SetSelectedStripNum(C4SplitSection eSplit, DWORD dwSelectedStripNum, ISonarMixer *pMixer)
{
	if (m_bLockMode[eSplit])
	{
		if (m_dwSelectedStripNum[eSplit] != dwSelectedStripNum)
		{
			m_dwSelectedStripNum[eSplit] = dwSelectedStripNum;

			m_cState.BumpToolbarUpdateCount();
			m_dwC4UpdateCount++;
		}
	}
	else
	{
		m_cState.SetSelectedStripNum(dwSelectedStripNum, pMixer);
	}
}

/////////////////////////////////////////////////////////////////////////////

// Meters

void CMackieControlC4::ToggleLevelMeters(C4SplitSection eSplit)
{
	if (!m_cState.HaveMeters())
		return;

	m_bEnableMeters[eSplit] = !m_bEnableMeters[eSplit];
	m_dwC4UpdateCount++;
}

bool CMackieControlC4::GetDisplayLevelMeters(C4SplitSection eSplit)
{
	if (!m_cState.HaveMeters())
		return false;

	return m_bEnableMeters[eSplit];
}

void CMackieControlC4::SetDisplayLevelMeters(C4SplitSection eSplit, bool bOn)
{
	if (!m_cState.HaveMeters())
		return;

	m_bEnableMeters[eSplit] = bOn;
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::ToggleDisplayValues(C4SplitSection eSplit)
{
	m_bDisplayValues[eSplit] = !m_bDisplayValues[eSplit];
}

bool CMackieControlC4::GetDisplayValues(C4SplitSection eSplit)
{
	return m_bDisplayValues[eSplit];
}

void CMackieControlC4::SetDisplayValues(C4SplitSection eSplit, bool bOn)
{
	m_bDisplayValues[eSplit] = bOn;
}

/////////////////////////////////////////////////////////////////////////////
// Modifier keys
//
// Linked to the main unit if m_bLinkModifiers is true

DWORD CMackieControlC4::GetModifiers(DWORD dwMask)
{
	if (m_bLinkModifiers)
		return m_cState.GetModifiers(dwMask);
	else	
		return m_dwModifiers & dwMask;
}

void CMackieControlC4::EnableModifier(DWORD dwModifier)
{
	if (m_bLinkModifiers)
	{
		m_cState.EnableModifier(dwModifier);
	}
	else
	{
		m_dwModifiers |= dwModifier;

		m_dwC4UpdateCount++;
	}
}

void CMackieControlC4::DisableModifier(DWORD dwModifier)
{
	if (m_bLinkModifiers)
	{
		m_cState.DisableModifier(dwModifier);
	}
	else
	{
		m_dwModifiers &= ~dwModifier;

		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlC4::GetFunctionKey(BYTE bN)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_C4_USER_FUNCTION_KEYS)
		return m_cUserFunctionKeys[bN].GetCommand();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::SetFunctionKey(BYTE bN, DWORD dwCmdId)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_C4_USER_FUNCTION_KEYS)
		m_cUserFunctionKeys[bN].SetCommand(dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

CString CMackieControlC4::GetFunctionKeyName(BYTE bN)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_C4_USER_FUNCTION_KEYS)
		return m_cUserFunctionKeys[bN].GetName();

	return "";
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::SetFunctionKeyName(BYTE bN, CString strName)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_C4_USER_FUNCTION_KEYS)
		m_cUserFunctionKeys[bN].SetName(strName);
}

/////////////////////////////////////////////////////////////////////////////
