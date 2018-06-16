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

void CMackieControlC4::OnRefreshSurface(DWORD fdwRefresh, bool bForceSend)
{
//	TRACE("CMackieControlC4::OnRefreshSurface()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());

	ReconfigureC4(bForceSend);

	UpdateLCDDisplays(bForceSend);
	UpdateLevelMeters(bForceSend);
	UpdateVPotDisplays(bForceSend);
	UpdateLEDs(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateVPotDisplays(bool bForceSend)
{
	switch (m_eC4Assignment)
	{
		case C4_ASSIGNMENT_TRACK:
			UpdateVPotForTrackMixerStrip(0, C4_UPPER, bForceSend);
			UpdateVPotForTrackAssignment(1, C4_UPPER, bForceSend);
			UpdateVPotForTrackMixerStrip(2, C4_LOWER, bForceSend);
			UpdateVPotForTrackAssignment(3, C4_LOWER, bForceSend);
			return;

		case C4_ASSIGNMENT_FUNCTION:
			{
				bool bPlaying = GetTransportState(TRANSPORT_STATE_PLAY);
				bool bRecording = GetTransportState(TRANSPORT_STATE_REC);
				bool bRecAutomation = GetTransportState(TRANSPORT_STATE_REC_AUTOMATION);
				bool bAudioEngine = GetTransportState(TRANSPORT_STATE_AUDIO);
				bool bLoopOnOff = GetTransportState(TRANSPORT_STATE_LOOP);

				int n = 0;
				m_HwVPotDisplay[n][0].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][1].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][2].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][3].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][4].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][5].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][6].SetOnOff(false,			bForceSend);	//
				m_HwVPotDisplay[n][7].SetOnOff(false,			bForceSend);	//
				n++;
				m_HwVPotDisplay[n][0].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][1].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][2].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][3].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][4].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][5].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][6].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][7].SetOnOff(false,			bForceSend);	// 
				n++;
				m_HwVPotDisplay[n][0].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][1].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][2].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][3].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][4].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][5].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][6].SetOnOff(false,			bForceSend);	// 
				m_HwVPotDisplay[n][7].SetOnOff(bLoopOnOff,		bForceSend);	// Loop On/Off
				n++;
				m_HwVPotDisplay[n][0].SetOnOff(false,			bForceSend);	// Rewind
				m_HwVPotDisplay[n][1].SetOnOff(false,			bForceSend);	// Goto End
				m_HwVPotDisplay[n][2].SetOnOff(!bPlaying,		bForceSend);	// Stop
				m_HwVPotDisplay[n][3].SetOnOff(bPlaying,		bForceSend);	// Play
				m_HwVPotDisplay[n][4].SetOnOff(bRecording,		bForceSend);	// Record
				m_HwVPotDisplay[n][5].SetOnOff(bRecAutomation,	bForceSend);	// Record Automation
				m_HwVPotDisplay[n][6].SetOnOff(bAudioEngine,	bForceSend);	// Audio Engine
				m_HwVPotDisplay[n][7].SetOnOff(false,			bForceSend);	// Reset
			}
			return;

		case C4_ASSIGNMENT_NORMAL:
		default:
			break;
	}

	for (int m = 0; m < NUM_ROWS; m++)
	{
		for (int n = 0; n < NUM_COLS; n++)
		{
			if (m_SwVPot[m][n].StripExists())
			{
				if (m_SwVPot[m][n].IsArchived())
					return;

				float fVal;
				bool bDot;

				HRESULT hr = m_SwVPot[m][n].GetNormalizedVal(&fVal, &bDot);

				if (FAILED(hr))
				{
					fVal = 0.0f;
					bDot = false;
				}

				if (GetModifiers(MCS_MODIFIER_M1))
				{
					if (FAILED(m_SwVPot[m][n].GetArm(&bDot)))
						bDot = false;
				}

				m_HwVPotDisplay[m][n].SetVal(fVal, bDot, m_SwVPot[m][n].GetDataType(), bForceSend);
			}
			else
			{
				m_HwVPotDisplay[m][n].SetAllOff(bForceSend);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateVPotForTrackMixerStrip(int row, C4SplitSection eSplit, bool bForceSend)
{
	SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);

	switch (eMixerStrip)
	{
		case MIX_STRIP_AUX:		eMixerStrip = MIX_STRIP_BUS;	break;
		case MIX_STRIP_MAIN:	eMixerStrip = MIX_STRIP_MASTER;	break;
		default: break;
	}

	m_HwVPotDisplay[row][0].SetOnOff(eMixerStrip == MIX_STRIP_TRACK,	bForceSend);
	m_HwVPotDisplay[row][1].SetOnOff(eMixerStrip == MIX_STRIP_BUS,		bForceSend);
	m_HwVPotDisplay[row][2].SetOnOff(eMixerStrip == MIX_STRIP_MASTER,	bForceSend);
	m_HwVPotDisplay[row][3].SetAllOff(bForceSend);

	if (m_cState.HaveMeters())
	{
		m_HwVPotDisplay[row][4].SetOnOff(GetDisplayLevelMeters(eSplit), bForceSend);
	}
	else
	{
		m_HwVPotDisplay[row][4].SetAllOff(bForceSend);
	}

	for (int col = 5; col < NUM_COLS; col++)
		m_HwVPotDisplay[row][col].SetAllOff(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateVPotForTrackAssignment(int row, C4SplitSection eSplit, bool bForceSend)
{
	Assignment eAssignment = GetAssignment(eSplit);

	if (eAssignment == MCS_ASSIGNMENT_EQ_FREQ_GAIN)
		eAssignment = MCS_ASSIGNMENT_EQ;

	m_HwVPotDisplay[row][0].SetOnOff(eAssignment == MCS_ASSIGNMENT_PARAMETER,	bForceSend);
	m_HwVPotDisplay[row][1].SetOnOff(eAssignment == MCS_ASSIGNMENT_SEND,		bForceSend);
	m_HwVPotDisplay[row][2].SetOnOff(eAssignment == MCS_ASSIGNMENT_PAN,			bForceSend);
	m_HwVPotDisplay[row][3].SetOnOff(eAssignment == MCS_ASSIGNMENT_PLUGIN,		bForceSend);
	m_HwVPotDisplay[row][4].SetOnOff(eAssignment == MCS_ASSIGNMENT_EQ,			bForceSend);
	m_HwVPotDisplay[row][5].SetOnOff(eAssignment == MCS_ASSIGNMENT_DYNAMICS,	bForceSend);

	for (int col = 6; col < NUM_COLS; col++)
		m_HwVPotDisplay[row][col].SetAllOff(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateLEDs(bool bForceSend)
{
	SetLED(MC_SPLIT_1_3,		m_eSplitMode == C4_SPLIT_1_3,									bForceSend);
	SetLED(MC_SPLIT_2_2,		m_eSplitMode == C4_SPLIT_2_2,									bForceSend);
	SetLED(MC_SPLIT_3_1,		m_eSplitMode == C4_SPLIT_3_1,									bForceSend);
	SetLED(MC_SPOT_ERASE,		m_eSplit == C4_LOWER,											bForceSend);
	SetLED(MC_LOCK,				m_bLockMode[m_eSplit],											bForceSend);
	SetLED(MC_MARKER,			m_bMarkersMode,													bForceSend);
	SetLED(MC_TRACK,			m_eC4Assignment == C4_ASSIGNMENT_TRACK,							bForceSend);
	SetLED(MC_CHANNEL_STRIP,	m_eAssignmentMode[m_eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP,	bForceSend);
	SetLED(MC_FUNCTION,			m_eC4Assignment == C4_ASSIGNMENT_FUNCTION,						bForceSend);

}

/////////////////////////////////////////////////////////////////////////////
