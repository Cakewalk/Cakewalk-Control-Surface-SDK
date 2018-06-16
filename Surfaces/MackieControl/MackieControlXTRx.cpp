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

#define ONE_OVER_1023		0.0009775171065493646f

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnFader(BYTE bChan, BYTE bD1, BYTE bD2)
{
	// One of ours?
	if (bChan > 7)
		return false;

	WORD wVal = (((WORD)bD2 << 7) | (WORD)bD1) >> 4;

	float fVal = (float)wVal * ONE_OVER_1023;

	HRESULT hr = m_SwFader[bChan].SetNormalizedVal(fVal, MIX_TOUCH_MANUAL);

	if (SUCCEEDED(hr) && MCS_FLIP_DUPLICATE == m_cState.GetFlipMode())
		m_dwTempDisplayValuesCounter[bChan] = TEMP_DISPLAY_TIMEOUT;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnVPot(BYTE bD1, BYTE bD2)
{
	// One of ours?
	if (bD1 < 0x10 || bD1 > 0x17)
		return false;

	BYTE bChan = bD1 & 0x0F;
	bool bLeft = (bD2 & 0x40) != 0;

	CCriticalSectionAuto csa(m_cState.GetCS());

	if (m_cState.GetConfigureLayoutMode())
	{
		if (0 == bChan)
		{
			if (bLeft)
			{
				if (m_dwUnitStripNumOffset >= 8)
					m_dwUnitStripNumOffset -= 8;
			}
			else
			{
				if (m_dwUnitStripNumOffset < 120)
					m_dwUnitStripNumOffset += 8;
			}
		}
	}
	else
	{
		if (m_SwVPot[bChan].HasBinding())
		{
			BYTE bVelocity = (bD2 & 0x0F);
			float fVelocity = (float)(((bVelocity - 1) * 3) + 1);

			float fStepSize = m_SwVPot[bChan].GetStepSize() * fVelocity;

			if (m_cState.GetModifiers(MCS_MODIFIER_M1))
			{
				if (m_SwVPot[bChan].GetAllowFineResolution())
					fStepSize *= 0.1f;
			}

			HRESULT hr = m_SwVPot[bChan].Adjust(bLeft ? -fStepSize : fStepSize);

			// The SetVal() always reports a failure, even though it works, so
			// we can't do "if (SUCCEEDED(hr))" here

			m_dwTempDisplayValuesCounter[bChan] = TEMP_DISPLAY_TIMEOUT;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnSwitch(BYTE bD1, BYTE bD2)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	bool bDown = (bD2 == 0x7F);

	switch (bD1)
	{
		case MC_REC_ARM_1:	case MC_REC_ARM_2:
		case MC_REC_ARM_3:	case MC_REC_ARM_4:
		case MC_REC_ARM_5:	case MC_REC_ARM_6:
		case MC_REC_ARM_7:	case MC_REC_ARM_8:
			if (bDown)
				OnSwitchRecArm(bD1 - MC_REC_ARM_1);
			break;

		case MC_SOLO_1:		case MC_SOLO_2:
		case MC_SOLO_3:		case MC_SOLO_4:
		case MC_SOLO_5:		case MC_SOLO_6:
		case MC_SOLO_7:		case MC_SOLO_8:
			if (bDown)
				OnSwitchSolo(bD1 - MC_SOLO_1);
			break;

		case MC_MUTE_1:		case MC_MUTE_2:
		case MC_MUTE_3:		case MC_MUTE_4:
		case MC_MUTE_5:		case MC_MUTE_6:
		case MC_MUTE_7:		case MC_MUTE_8:
			if (bDown)
				OnSwitchMute(bD1 - MC_MUTE_1);
			break;

		case MC_SELECT_1:	case MC_SELECT_2:
		case MC_SELECT_3:	case MC_SELECT_4:
		case MC_SELECT_5:	case MC_SELECT_6:
		case MC_SELECT_7:	case MC_SELECT_8:
			if (bDown)
				OnSwitchSelect(bD1 - MC_SELECT_1);
			break;

		case MC_VPOT_1:		case MC_VPOT_2:
		case MC_VPOT_3:		case MC_VPOT_4:
		case MC_VPOT_5:		case MC_VPOT_6:
		case MC_VPOT_7:		case MC_VPOT_8:
			if (bDown)
				OnSwitchVPot(bD1 - MC_VPOT_1);
			break;

		case MC_FADER_1:	case MC_FADER_2:
		case MC_FADER_3:	case MC_FADER_4:
		case MC_FADER_5:	case MC_FADER_6:
		case MC_FADER_7:	case MC_FADER_8:
			OnSwitchFader(bD1 - MC_FADER_1, bDown);
			break;

		default:
			return false;
	}

	// It's one of ours, so record if it's currently pressed
	m_bSwitches[bD1] = bDown;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchVPot(BYTE bChan)
{
	if (m_cState.GetModifiers(MCS_MODIFIER_M1))
	{
		m_SwVPot[bChan].ToggleArm();
	}
	else
	{
		HRESULT hr = m_SwVPot[bChan].ToggleOrSetToDefault();

		if (SUCCEEDED(hr))
			m_dwTempDisplayValuesCounter[bChan] = TEMP_DISPLAY_TIMEOUT;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchRecArm(BYTE bChan)
{
	switch (m_cState.GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:
			m_SwRec[bChan].ToggleBooleanParam();
			break;

		case MCS_MODIFIER_M1:
			m_SwFader[bChan].ToggleArm();
			break;

		case MCS_MODIFIER_M2:
			// m_SwSolo is bound to the strip for tracks and buses, but 
			// not masters, which is what we want
			m_SwSolo[bChan].ToggleArmAll();
			break;

		case MCS_MODIFIER_M4:
			DisarmAllTracks();
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchSolo(BYTE bChan)
{
	switch (m_cState.GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:
			{
				float fVal;

				HRESULT hr = m_SwSolo[bChan].GetVal(&fVal);

				if (SUCCEEDED(hr))
				{
					fVal = (fVal >= 0.5f) ? 0.0f : 1.0f;

					hr = m_SwSolo[bChan].SetVal(fVal);

					// Only Select if enabling solo
					if (fVal >= 0.5 && m_cState.GetSoloSelectsChannel() && SUCCEEDED(hr))
						OnSwitchSelect(bChan);
				}
			}
			break;

		case MCS_MODIFIER_M4:
			ClearAllSolos();
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchMute(BYTE bChan)
{
	switch (m_cState.GetModifiers(M1_TO_M4_MASK))
	{
		case MCS_MODIFIER_NONE:
			m_SwMute[bChan].ToggleBooleanParam();
			break;

		case MCS_MODIFIER_M1:
			m_SwMute[bChan].ToggleArm();
			break;

		case MCS_MODIFIER_M2:
			m_SwArchive[bChan].ToggleBooleanParam();
			break;

		case MCS_MODIFIER_M3:
			m_SwInputEcho[bChan].ToggleBooleanParam();
			break;

		case MCS_MODIFIER_M4:
			ClearAllMutes();
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchSelect(BYTE bChan)
{
	if (m_SwStrip[bChan].StripExists())
	{
		m_cState.SetSelectedStripNum(m_SwStrip[bChan].GetStripNum(),
			(m_cState.GetSelectHighlightsTrack()) ? m_pMixer : NULL);

		TempDisplaySelectedTrackName();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnSwitchFader(BYTE bChan, bool bDown)
{
	if (bDown)
	{
		m_SwFader[bChan].TouchCapture();

		if (m_cState.GetFaderTouchSelectsChannel() && 
			m_cState.GetAssignment() != MCS_ASSIGNMENT_EQ_FREQ_GAIN &&
			m_cState.GetFlipMode() == MCS_FLIP_NORMAL)
		{
			m_cState.SetSelectedStripNum(m_SwFader[bChan].GetStripNum(),
				(m_cState.GetSelectHighlightsTrack()) ? m_pMixer : NULL);
		}
	}
	else
	{
		m_SwFader[bChan].TouchRelease();

		UpdateFader(bChan, true);
	}
}

/////////////////////////////////////////////////////////////////////////////
