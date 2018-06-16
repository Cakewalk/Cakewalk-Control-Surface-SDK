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

void CMackieControlXT::OnRefreshSurface(DWORD fdwRefresh, bool bForceSend)
{
//	TRACE("CMackieControlXT::OnRefreshSurface()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());

	ReconfigureXT(bForceSend);

	// Do the reconfigure before this so that the setup is up to date
	UpdateToolbarDisplay(bForceSend);

	// Now update the various sections
	UpdateLCDDisplay(bForceSend);
	UpdateLevelMeters(bForceSend);
	UpdateVPotDisplays(bForceSend);
	UpdateLEDs(bForceSend);

	if (!m_cState.GetDisableFaders())
		UpdateFaders(bForceSend);

	// Send this last so that it doesn't corrupt the display
	m_HwLCDDisplay.SetGlobalMeterMode(false, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateVPotDisplays(bool bForceSend)
{
	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		if (m_SwVPot[n].StripExists())
		{
			if (m_SwVPot[n].IsArchived())
				return;

			float fVal;
			bool bDot;

			HRESULT hr = m_SwVPot[n].GetNormalizedVal(&fVal, &bDot);

			if (FAILED(hr))
			{
				fVal = 0.0f;
				bDot = false;
			}

			if (m_cState.GetModifiers(MCS_MODIFIER_M1))
			{
				if (FAILED(m_SwVPot[n].GetArm(&bDot)))
					bDot = false;
			}

			m_HwVPotDisplay[n].SetVal(fVal, bDot, m_SwVPot[n].GetDataType(), bForceSend);
		}
		else
		{
			m_HwVPotDisplay[n].SetAllOff(bForceSend);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateLEDs(bool bForceSend)
{
	DWORD dwOffset = m_cState.GetStripNumOffset() + m_dwUnitStripNumOffset;
	DWORD dwSelected = m_cState.GetSelectedStripNum();
	DWORD dwModifiers = m_cState.GetModifiers(M1_TO_M4_MASK);

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		float fVal;

		bool bValRec;
		switch (dwModifiers)
		{
			case MCS_MODIFIER_NONE:
			case MCS_MODIFIER_M4:
				bValRec = SUCCEEDED(m_SwRec[n].GetVal(&fVal)) ? (fVal >= 0.5f) : false;
				break;

			case MCS_MODIFIER_M1:
				if (FAILED(m_SwFader[n].GetArm(&bValRec)))
					bValRec = false;
				break;

			case MCS_MODIFIER_M2:
				if (FAILED(m_SwSolo[n].GetArmAll(&bValRec)))
					bValRec = false;
				break;

			default:
				bValRec = false;
				break;
		}
		SetLED(MC_REC_ARM_1 + n, bValRec, bForceSend);

		bool bValSolo = SUCCEEDED(m_SwSolo[n].GetVal(&fVal)) ? (fVal >= 0.5f) : false;
		SetLED(MC_SOLO_1 + n, bValSolo, bForceSend);

		bool bValMute;
		switch (dwModifiers)
		{
			case MCS_MODIFIER_NONE:
			case MCS_MODIFIER_M4:
				bValMute = SUCCEEDED(m_SwMute[n].GetVal(&fVal)) ? (fVal >= 0.5f) : false;
				break;

			case MCS_MODIFIER_M1:
				if (FAILED(m_SwMute[n].GetArm(&bValMute)))
					bValMute = false;
				break;

			case MCS_MODIFIER_M2:
				bValMute = SUCCEEDED(m_SwArchive[n].GetVal(&fVal)) ? (fVal >= 0.5f) : false;
				break;

			case MCS_MODIFIER_M3:
				bValMute = SUCCEEDED(m_SwInputEcho[n].GetVal(&fVal)) ? (fVal >= 0.5f) : false;
				break;

			default:
				bValMute = false;
				break;
		}
		SetLED(MC_MUTE_1 + n, bValMute, bForceSend);

		bool bValSelect = (dwOffset + n) == dwSelected;
		SetLED(MC_SELECT_1 + n, bValSelect, bForceSend);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateFaders(bool bForceSend)
{
	if (!HaveMixerStrips())
		return;

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		UpdateFader(n, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateFader(BYTE bChan, bool bForceSend)
{
	if (m_SwFader[bChan].IsArchived())
		return;

	float fVal;

	HRESULT hr = m_SwFader[bChan].GetNormalizedVal(&fVal);

	if (FAILED(hr))
		fVal = 0.0f;

	m_HwFader[bChan].SetVal(fVal, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////
