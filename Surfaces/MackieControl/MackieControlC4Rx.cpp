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

bool CMackieControlC4::OnVPot(BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlC4::OnVPot(%d, %d)\n", bD1, bD2);

	if (bD1 > 31)
		return false;

	if (m_eC4Assignment == C4_ASSIGNMENT_NORMAL)
	{
		int m = bD1 / NUM_COLS;
		int n = bD1 % NUM_COLS;
		bool bLeft = (bD2 & 0x40) != 0;

		CCriticalSectionAuto csa(m_cState.GetCS());

//		TRACE("%d %d %d\n", m, n, bLeft);

		if (m_SwVPot[m][n].HasBinding())
		{
			BYTE bVelocity = (bD2 & 0x0F);
			float fVelocity = (float)(((bVelocity - 1) * 3) + 1);

//			TRACE("C4 VPot %d %f\n", bVelocity, fVelocity);

			float fStepSize = m_SwVPot[m][n].GetStepSize() * fVelocity;

			if (GetModifiers(MCS_MODIFIER_M1))
			{
				if (m_SwVPot[m][n].GetAllowFineResolution())
					fStepSize *= 0.1f;
			}

			HRESULT hr = m_SwVPot[m][n].Adjust(bLeft ? -fStepSize : fStepSize);

			// The SetVal() always reports a failure, even though it works, so
			// we can't do "if (SUCCEEDED(hr))" here

			m_dwTempDisplayValuesCounter[m][n] = TEMP_DISPLAY_TIMEOUT;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlC4::OnSwitch(BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlC4::OnSwitch(%d, %d)\n", bD1, bD2);

	CCriticalSectionAuto csa(m_cState.GetCS());

	bool bDown = (bD2 == 0x7F);

	if (bD1 >= MC_VPOT_1_1 && bD1 <= MC_VPOT_4_8)
	{
		if (bDown)
		{
			int N = bD1 - MC_VPOT_1_1;

			int m = N / NUM_COLS;
			int n = N % NUM_COLS;

			OnSwitchVPot(m, n);
		}
	}
	else
	{
		switch (bD1)
		{
			case MC_SPLIT:			if (bDown) OnSwitchSplit();					break;
			case MC_LOCK:			if (bDown) OnSwitchLock();					break;
			case MC_SPOT_ERASE:		if (bDown) OnSwitchSpotErase();				break;
			case MC_MARKER:			OnSwitchMarker(bDown);						break;
			case MC_TRACK:			OnSwitchTrack(bDown);						break;
			case MC_CHANNEL_STRIP:	if (bDown) OnSwitchChanStrip();				break;
			case MC_FUNCTION:		OnSwitchFunction(bDown);					break;
			case MC_SHIFT:			OnSwitchModifier(bDown, MCS_MODIFIER_M1);	break;
			case MC_OPTION:			OnSwitchModifier(bDown, MCS_MODIFIER_M2);	break;
			case MC_CONTROL:		OnSwitchModifier(bDown, MCS_MODIFIER_M3);	break;
			case MC_ALT:			OnSwitchModifier(bDown, MCS_MODIFIER_M4);	break;
			case MC_BANK_LEFT:		if (bDown) OnSwitchBankLeft();				break;
			case MC_BANK_RIGHT:		if (bDown) OnSwitchBankRight();				break;
			case MC_PARAM_LEFT:		if (bDown) OnSwitchParamLeft();				break;
			case MC_PARAM_RIGHT:	if (bDown) OnSwitchParamRight();			break;
			case MC_SLOT_UP:		if (bDown) OnSwitchSlotUp();				break;
			case MC_SLOT_DOWN:		if (bDown) OnSwitchSlotDown();				break;
			case MC_TRACK_LEFT:		if (bDown) OnSwitchTrackLeft();				break;
			case MC_TRACK_RIGHT:	if (bDown) OnSwitchTrackRight();			break;
			default:
				return false;
		}
	}

	// It's one of ours, so record if it's currently pressed
	m_bSwitches[bD1] = bDown;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchSplit()
{
	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	if (GetModifiers(MCS_MODIFIER_M1))
	{
		switch (m_eSplitMode)
		{
			case C4_SPLIT_NONE:	m_eSplitMode = C4_SPLIT_3_1;	break;
			case C4_SPLIT_1_3:	m_eSplitMode = C4_SPLIT_NONE;	break;
			case C4_SPLIT_2_2:	m_eSplitMode = C4_SPLIT_1_3;	break;
			case C4_SPLIT_3_1:	m_eSplitMode = C4_SPLIT_2_2;	break;
			default: break;
		}
	}
	else
	{
		switch (m_eSplitMode)
		{
			case C4_SPLIT_NONE:	m_eSplitMode = C4_SPLIT_1_3;	break;
			case C4_SPLIT_1_3:	m_eSplitMode = C4_SPLIT_2_2;	break;
			case C4_SPLIT_2_2:	m_eSplitMode = C4_SPLIT_3_1;	break;
			case C4_SPLIT_3_1:	m_eSplitMode = C4_SPLIT_NONE;	break;
			default: break;
		}
	}

	if (m_eSplitMode == C4_SPLIT_NONE)
		m_eSplit = C4_UPPER;

	m_cState.BumpToolbarUpdateCount();
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchLock()
{
	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	m_bLockMode[m_eSplit] = !m_bLockMode[m_eSplit];

	if (m_bLockMode[m_eSplit])
	{
		m_eMixerStrip[m_eSplit] = m_cState.GetMixerStrip();
		m_eAssignment[m_eSplit] = m_cState.GetAssignment();
		m_dwSelectedStripNum[m_eSplit] = m_cState.GetSelectedStripNum();
	}

	m_cState.BumpToolbarUpdateCount();
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchSpotErase()
{
	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	if (m_eSplitMode != C4_SPLIT_NONE)
	{
		m_eSplit = (m_eSplit == C4_UPPER) ? C4_LOWER : C4_UPPER;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchMarker(bool bDown)
{
	if (GetModifiers(MCS_MODIFIER_M1))
	{
		if (bDown)
			m_bMarkersMode = true;
	}
	else
	{
		m_bMarkersMode = bDown;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchTrack(bool bDown)
{
	if (GetModifiers(MCS_MODIFIER_M1))
	{
		if (bDown)
		{
			m_eOldC4Assignment = m_eC4Assignment;
			m_eC4Assignment = C4_ASSIGNMENT_TRACK;
		}
	}
	else
	{
		if (bDown)
		{
			m_eOldC4Assignment = m_eC4Assignment;
			m_eC4Assignment = C4_ASSIGNMENT_TRACK;
		}
		else
		{
			if (m_eOldC4Assignment == C4_ASSIGNMENT_TRACK)
				m_eC4Assignment = C4_ASSIGNMENT_NORMAL;
			else
				m_eC4Assignment = m_eOldC4Assignment;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchChanStrip()
{
	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	if (m_eAssignmentMode[m_eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP)
		m_eAssignmentMode[m_eSplit] = MCS_ASSIGNMENT_MUTLI_CHANNEL;
	else
		m_eAssignmentMode[m_eSplit] = MCS_ASSIGNMENT_CHANNEL_STRIP;

	m_cState.BumpToolbarUpdateCount();
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchFunction(bool bDown)
{
	if (!bDown)
		return;
	
	m_eC4Assignment = (m_eC4Assignment != C4_ASSIGNMENT_FUNCTION) ? C4_ASSIGNMENT_FUNCTION : C4_ASSIGNMENT_NORMAL;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchModifier(bool bDown, DWORD dwMask)
{
	if (bDown)											// Modifier on
		EnableModifier(dwMask);
	else												// Modifier off
		DisableModifier(dwMask);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchBankLeft()
{
	if (m_bMarkersMode)
	{
		NudgeTimeCursor(JOG_MEASURES, DIR_BACKWARD);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go left by 8 tracks
			ShiftParamNumOffset(m_eSplit, -8);
			break;

		case MCS_MODIFIER_M1:							// Go to first track
			ShiftParamNumOffset(m_eSplit, -100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchBankRight()
{
	if (m_bMarkersMode)
	{
		NudgeTimeCursor(JOG_MEASURES, DIR_FORWARD);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go right by 8 tracks
			ShiftParamNumOffset(m_eSplit, 8);
			break;

		case MCS_MODIFIER_M1:							// Go to last track
			ShiftParamNumOffset(m_eSplit, 100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchParamLeft()
{
	if (m_bMarkersMode)
	{
		NudgeTimeCursor(JOG_BEATS, DIR_BACKWARD);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go left by 1 track
			ShiftParamNumOffset(m_eSplit, -1);
			break;

		case MCS_MODIFIER_M1:							// Go to first track
			ShiftParamNumOffset(m_eSplit, -100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchParamRight()
{
	if (m_bMarkersMode)
	{
		NudgeTimeCursor(JOG_BEATS, DIR_FORWARD);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go right by 1 track
			ShiftParamNumOffset(m_eSplit, 1);
			break;

		case MCS_MODIFIER_M1:							// Go to last track
			ShiftParamNumOffset(m_eSplit, 100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchSlotUp()
{
	if (m_bMarkersMode)
	{
		// Do nothing (for now)
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go up by 1 plugin
			ShiftPluginNumOffset(m_eSplit, 1);
			break;

		case MCS_MODIFIER_M1:							// Go to last plugin
			ShiftPluginNumOffset(m_eSplit, 100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchSlotDown()
{
	if (m_bMarkersMode)
	{
		// Do nothing (for now)
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go down by 1 plugin
			ShiftPluginNumOffset(m_eSplit, -1);
			break;

		case MCS_MODIFIER_M1:							// Go to first plugin
			ShiftPluginNumOffset(m_eSplit, -100000);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchTrackLeft()
{
	if (m_bMarkersMode)
	{
		DoCommand(CMD_MARKER_PREVIOUS);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go left by 1 track
			ShiftTrack(-1);
			break;

		case MCS_MODIFIER_M1:							// Go to first track
			ShiftTrack(-100000);
			break;

		case MCS_MODIFIER_M2:							// Go left by 8 tracks
			ShiftTrack(-8);
			break;

		default:
			break;
	}

	if (m_eAssignmentMode[m_eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP)
		TempDisplaySelectedTrackName(m_eSplit);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchTrackRight()
{
	if (m_bMarkersMode)
	{
		DoCommand(CMD_MARKER_NEXT);
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	switch (GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:							// Go right by 1 track
			ShiftTrack(1);
			break;

		case MCS_MODIFIER_M1:							// Go to last track
			ShiftTrack(100000);
			break;

		case MCS_MODIFIER_M2:							// Go right by 8 tracks
			ShiftTrack(8);
			break;

		default:
			break;
	}

	if (m_eAssignmentMode[m_eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP)
		TempDisplaySelectedTrackName(m_eSplit);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnSwitchVPot(BYTE row, BYTE col)
{
	switch (m_eC4Assignment)
	{
		case C4_ASSIGNMENT_NORMAL:
		default:
			if (GetModifiers(MCS_MODIFIER_M1))
			{
				m_SwVPot[row][col].ToggleArm();
			}
			else
			{
				HRESULT hr = m_SwVPot[row][col].ToggleOrSetToDefault();

				if (SUCCEEDED(hr))
					m_dwTempDisplayValuesCounter[row][col] = TEMP_DISPLAY_TIMEOUT;
			}
			break;

		case C4_ASSIGNMENT_TRACK:
			{
				C4SplitSection eSplit = (row < 2) ? C4_UPPER : C4_LOWER;

				if (row == 0 || row == 2)
				{
					switch (col)
					{
						case 0: SetMixerStrip(eSplit, MIX_STRIP_TRACK);		break;
						case 1: SetMixerStrip(eSplit, MIX_STRIP_BUS);		break;
						case 2: SetMixerStrip(eSplit, MIX_STRIP_MASTER);	break;
						case 4: ToggleLevelMeters(eSplit);					break;
						case 5: ToggleDisplayValues(eSplit);				break;
						default: break;
					}
				}
				else if (row == 1 || row == 3)
				{
					switch (col)
					{
						case 0: SetAssignment(eSplit, MCS_ASSIGNMENT_PARAMETER);	break;
						case 1: SetAssignment(eSplit, MCS_ASSIGNMENT_SEND);			break;
						case 2: SetAssignment(eSplit, MCS_ASSIGNMENT_PAN);			break;
						case 3: SetAssignment(eSplit, MCS_ASSIGNMENT_PLUGIN);		break;
						case 4: SetAssignment(eSplit, MCS_ASSIGNMENT_EQ);			break;
						case 5: SetAssignment(eSplit, MCS_ASSIGNMENT_DYNAMICS);		break;
						default: break;
					}
				}
			}
			break;

		case C4_ASSIGNMENT_FUNCTION:
			{
				int idx = (row * NUM_COLS) + col;

				switch (idx)
				{
					case  0: DoCommand(GetFunctionKey(0));							break;
					case  1: DoCommand(GetFunctionKey(1));							break;
					case  2: DoCommand(GetFunctionKey(2));							break;
					case  3: DoCommand(GetFunctionKey(3));							break;
					case  4: DoCommand(GetFunctionKey(4));							break;
					case  5: DoCommand(GetFunctionKey(5));							break;
					case  6: DoCommand(GetFunctionKey(6));							break;
					case  7: DoCommand(GetFunctionKey(7));							break;

					case  8: DoCommand(NEW_CMD_METRONOME_DURING_PLAYBACK);			break;
					case  9: DoCommand(NEW_CMD_METRONOME_DURING_RECORD);			break;
					case 10: DoCommand(NEW_CMD_SNAPSHOT);							break;
					case 11: DoCommand(NEW_CMD_ENABLE_DISABLE_AUTOMATION_PLAYBACK);	break;
					case 12: DisarmAllTracks();										break;
					case 13: DoCommand(NEW_CMD_DISARM_ALL_AUTOMATION_CONTROLS);		break;
					case 14: FakeKeyPress(false, false, false, 'F');				break;
					case 15: FakeKeyPress(true, false, false, 'F');					break;

					case 16: DoCommand(CMD_INSERT_AUDIO_TRACK);						break;
					case 17: DoCommand(CMD_INSERT_MIDI_TRACK);						break;
					case 18: DoCommand(CMD_TRACK_CLONE);							break;
					case 19: DoCommand(CMD_DELETE_TRACK);							break;
					case 20: DoCommand(CMD_FILE_SAVE);								break;
					case 21: DoCommand(CMD_EDIT_UNDO);								break;
					case 22: DoCommand(CMD_EDIT_REDO);								break;
					case 23: ToggleLoopMode();										break;

					case 24: DoCommand(CMD_GOTO_START);								break;
					case 25: DoCommand(CMD_GOTO_END);								break;
					case 26: DoCommand(CMD_REALTIME_STOP);							break;
					case 27: OnPlayPressed();										break;
					case 28: DoCommand(CMD_REALTIME_RECORD);						break;
					case 29: DoCommand(CMD_REALTIME_RECORD_AUTOMATION);				break;
					case 30: DoCommand(CMD_REALTIME_AUDIO_RUNNING);					break;
					case 31: DoCommand(CMD_REALTIME_PANIC);							break;

					default:
						break;
				}
			}
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
