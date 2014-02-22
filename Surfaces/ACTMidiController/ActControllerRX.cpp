#include "stdafx.h"

#include "ACTController.h"

/////////////////////////////////////////////////////////////////////////////

void CACTController::OnShortMidiIn(BYTE bStatus, BYTE bD1, BYTE bD2)
{
//	TRACE("CACTController::OnShortMidiIn(0x%02X, 0x%02X, 0x%02X)\n", bStatus, bD1, bD2);

	if (m_pMidiLearnTarget)
	{
		m_pMidiLearnTarget->SetMessage(bStatus, bD1, bD2);

		if (m_pMidiLearnTarget == &m_cMidiModifierDown)
			m_pMidiLearnTarget = &m_cMidiModifierUp;
		else
		{
			if ( CMidiBinding::MI_Increment == m_pMidiLearnTarget->m_eMessageInterpretation )
			{
				// for Increment/Decrement controls (rotary encoders), we just collect data
				// from the control and the user exits learn when he clicks the cell again.
				// At that point, we set the hinge value to be the median of all data collected
				m_setLastReceivedLearnValue.insert( bD2 );
			}
			else if ( CMidiBinding::MI_Literal == m_pMidiLearnTarget->m_eMessageInterpretation )
			{
				// we're done
				m_pMidiLearnTarget = NULL;
			}
		}
		OnContextSwitch();

		return;
	}

	int n;

	for (n = 0; n < NUM_KNOBS; n++)
	{
		if (m_cMidiKnob[n].IsMatch(bStatus, bD1, bD2))
			OnKnob(n, bD2);
	}

	for (n = 0; n < NUM_SLIDERS; n++)
	{
		if (m_cMidiSlider[n].IsMatch(bStatus, bD1, bD2))
			OnSlider(n, bD2);
	}

	for (n = 0; n < NUM_BUTTONS; n++)
	{
		if (m_cMidiButton[n].IsMatch(bStatus, bD1, bD2))
			OnButton(n, bD2);
	}

	if (m_cMidiModifierDown.IsMatch(bStatus, bD1, bD2))
	{
		m_bModifierIsDown = true;
	}

	if (m_cMidiModifierUp.IsMatch(bStatus, bD1, bD2))
	{
		m_bModifierIsDown = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::OnKnob(BYTE bKnob, BYTE bVal)
{
//	TRACE("CACTController::OnKnob(%d, %d)\n", bKnob, bVal);

	if (bKnob >= NUM_KNOBS)
		return;

	if ( CMidiBinding::MI_Literal == m_cMidiKnob[bKnob].m_eMessageInterpretation )
	{
		float fVal = (float)bVal / 127.0f;
		m_SwKnob[m_iRotaryBank][bKnob].SetVal(fVal);
	}
	else if ( CMidiBinding::MI_Increment == m_cMidiKnob[bKnob].m_eMessageInterpretation )
	{
		float fFactor = 1.0f;
		DWORD dwIncrementHinge = m_cMidiKnob[bKnob].m_dwHinge;
		if ( m_cMidiKnob[bKnob].m_bUseAccel )
			fFactor += .5f * ((float)abs((int)(bVal - dwIncrementHinge)) - 1.0f);
		float fDelta = fFactor * 1.0f/127.0f;
		if ( bVal <= dwIncrementHinge )
			fDelta *= -1.0f;

		m_SwKnob[m_iRotaryBank][bKnob].Adjust( fDelta );
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::OnSlider(BYTE bSlider, BYTE bVal)
{
//	TRACE("CACTController::OnSlider(%d, %d)\n", bSlider, bVal);

	if (bSlider >= NUM_SLIDERS)
		return;

	if ( CMidiBinding::MI_Literal == m_cMidiSlider[bSlider].m_eMessageInterpretation )
	{
		float fVal = (float)bVal / 127.0f;

		m_SwSlider[m_iSliderBank][bSlider].SetVal(fVal);
	}
	else if ( CMidiBinding::MI_Increment == m_cMidiSlider[bSlider].m_eMessageInterpretation )
	{
		float fFactor = 1.0f;
		DWORD dwIncrementHinge = m_cMidiSlider[bSlider].m_dwHinge;
		if ( m_cMidiSlider[bSlider].m_bUseAccel )
			fFactor += .5f * ((float)abs((int)(bVal - dwIncrementHinge)) - 1.0f);
		float fDelta = fFactor * 1.0f/127.0f;
		if ( bVal <= dwIncrementHinge )
			fDelta *= -1.0f;

		m_SwSlider[m_iSliderBank][bSlider].Adjust( fDelta );
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::OnButton(BYTE bButton, BYTE bVal)
{
//	TRACE("CACTController::OnButton(%d, %d)\n", bButton, bVal);

	if (bButton >= NUM_BUTTONS)
		return;

	// All done if it's not a button down event
	if (bVal == 0)
		return;

	// Figure out the virtual button index
	int iVirtualButton = bButton + (m_bModifierIsDown ? 8 : 0);

	if (GetButtonACTMode(m_iButtonBank, (VirtualButton)iVirtualButton))
	{
		m_SwButton[m_iButtonBank][iVirtualButton].ToggleBooleanParam();
	}
	else // Not ACT...
	{
		DWORD dwAction = m_dwButtonAction[m_iButtonBank][iVirtualButton];

		switch (dwAction)
		{
			case CMD_NONE:
				break;

			case CMD_ACT_ENABLE:
				SetUseDynamicMappings(!GetUseDynamicMappings());
				break;

			case CMD_ACT_LOCK:
				SetLockDynamicMappings(!GetLockDynamicMappings());
				break;

			case CMD_ACT_LEARN:
				SetLearnDynamicMappings(!GetLearnDynamicMappings());
				break;

			case CMD_ROTARIES_MODE:
				if (m_eRotariesMode == MCS_ASSIGNMENT_MUTLI_CHANNEL)
					m_eRotariesMode = MCS_ASSIGNMENT_CHANNEL_STRIP;
				else
					m_eRotariesMode = MCS_ASSIGNMENT_MUTLI_CHANNEL;

				UpdateBindings();
				break;

			case CMD_PREV_TRACK:
				ShiftStripNum(-1);
				break;

			case CMD_NEXT_TRACK:
				ShiftStripNum(+1);
				break;

			case CMD_PREV_TRACK_BANK:
				ShiftStripNum(-NUM_SLIDERS);
				break;

			case CMD_NEXT_TRACK_BANK:
				ShiftStripNum(+NUM_SLIDERS);
				break;

			case CMD_PREV_STRIP_TYPE:
				ShiftStripType(-1);
				break;

			case CMD_NEXT_STRIP_TYPE:
				ShiftStripType(+1);
				break;

			case CMD_PREV_SEL_TRACK:
				ShiftSelectedTrack(-1);
				break;

			case CMD_NEXT_SEL_TRACK:
				ShiftSelectedTrack(+1);
				break;

			case CMD_MUTE_SEL_TRACK:
				ToggleSelectedTrackParam(MIX_PARAM_MUTE);
				break;

			case CMD_SOLO_SEL_TRACK:
				ToggleSelectedTrackParam(MIX_PARAM_SOLO);
				break;

			case CMD_REC_ARM_SEL_TRACK:
				ToggleSelectedTrackParam(MIX_PARAM_RECORD_ARM);
				break;

#define PREV_BANK(v) { if (v > 0) v--; else v = MAX_BANK; }
#define NEXT_BANK(v) { if (v < MAX_BANK) v++; else v = 0; }

			case CMD_PREV_ROTARIES_BANK:
				PREV_BANK(m_iRotaryBank);
				UpdateBindings();
				break;

			case CMD_NEXT_ROTARIES_BANK:
				NEXT_BANK(m_iRotaryBank);
				UpdateBindings();
				break;

			case CMD_PREV_SLIDERS_BANK:
				PREV_BANK(m_iSliderBank);
				UpdateBindings();
				break;

			case CMD_NEXT_SLIDERS_BANK:
				NEXT_BANK(m_iSliderBank);
				UpdateBindings();
				break;

			case CMD_PREV_BUTTONS_BANK:
				PREV_BANK(m_iButtonBank);
				UpdateBindings();
				break;

			case CMD_NEXT_BUTTONS_BANK:
				NEXT_BANK(m_iButtonBank);
				UpdateBindings();
				break;

			case CMD_PREV_CONTROLLERS_BANK:
				PREV_BANK(m_iButtonBank);
				m_iRotaryBank = m_iButtonBank;
				m_iSliderBank = m_iButtonBank;
				UpdateBindings();
				break;

			case CMD_NEXT_CONTROLLERS_BANK:
				NEXT_BANK(m_iButtonBank);
				m_iRotaryBank = m_iButtonBank;
				m_iSliderBank = m_iButtonBank;
				UpdateBindings();
				break;

			case CMD_PREV_ROTARIES_AND_SLIDERS_BANK:
				PREV_BANK(m_iRotaryBank);
				m_iSliderBank = m_iRotaryBank;
				UpdateBindings();
				break;

			case CMD_NEXT_ROTARIES_AND_SLIDERS_BANK:
				NEXT_BANK(m_iRotaryBank);
				m_iSliderBank = m_iRotaryBank;
				UpdateBindings();
				break;

			case CMD_AUTO_READ_SEL_TRACK:
				ToggleSelectedTrackAutomationRead();
				break;

			case CMD_AUTO_WRITE_SEL_TRACK:
				ToggleSelectedTrackAutomationWrite();
				break;

			case CMD_SURFACE_PROPS_TOGGLE:
				ToggleProps();
				break;
			case CMD_SURFACE_PROPS_SHOW:
				ActivateProps( TRUE );
				break;
			case CMD_SURFACE_PROPS_HIDE:
				ActivateProps( FALSE );
				break;
			case CMD_OPEN_CUR_FX:
				ActivateCurrentFx( UIA_OPEN );
				break;
			case CMD_CLOSE_CUR_FX:
				ActivateCurrentFx( UIA_CLOSE );
				break;
			case CMD_FOCUS_NEXT_FX:
				ActivateNextFx( UIA_FOCUS );
				break;
			case CMD_FOCUS_PREV_FX:
				ActivatePrevFx( UIA_FOCUS );
				break;
			case CMD_OPEN_NEXT_FX:
				ActivateNextFx( UIA_OPEN );
				break;
			case CMD_OPEN_PREV_FX:
				ActivatePrevFx( UIA_OPEN );
				break;

			default:
				if ((dwAction & MASK_COMMAND) == 0) // SONAR Command?
					DoCommand(dwAction);
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
