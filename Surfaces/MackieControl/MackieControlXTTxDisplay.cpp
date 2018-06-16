#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

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

void CMackieControlXT::UpdateLCDDisplay(bool bForceSend)
{
	// Layout mode?
	if (m_cState.GetConfigureLayoutMode())
	{
		for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
			m_HwLCDDisplay.SetMeterMode(n, false, false, false, bForceSend);

		// Top line
		m_HwLCDDisplay.WriteCentered(0, "Use VPot 1 to assign channel numbers", bForceSend);

		// Bottom line
		char szLine[128];
		FormatTrackNumbers(szLine, sizeof(szLine), m_dwUnitStripNumOffset);
		m_HwLCDDisplay.Write(0, 1, szLine, bForceSend);

		return;
	}

	if (!IsProjectLoaded())
	{
		m_HwLCDDisplay.WriteCentered(0, "No SONAR Project Loaded", bForceSend);
		m_HwLCDDisplay.WriteCentered(1, "", bForceSend);

		return;
	}

	// Refresh the upper line
	UpdateUpperLCD(bForceSend);

	// Refresh the lower line
	UpdateLowerLCD(bForceSend);

	// Update the timeout counters
	if (m_dwTempDisplayTextCounter > 0)
		m_dwTempDisplayTextCounter--;

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		if (m_dwTempDisplayValuesCounter[n] > 0)
			m_dwTempDisplayValuesCounter[n]--;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateUpperLCD(bool bForceSend)
{
	if (m_cState.GetDisableLCDUpdates())
	{
		m_HwLCDDisplay.WriteCentered(0, "Display Updates Disabled", bForceSend);

		return;
	}

	if (m_dwTempDisplayTextCounter > 0)
	{
		m_HwLCDDisplay.WriteCentered(0, m_szTempDisplayText, bForceSend);
	}
	else
	{
		char szLine[128];
		szLine[0] = 0;

		bool bDisplayFlip = m_cState.GetDisplayFlip();

		for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		{
			char szBuf[16];

			if (bDisplayFlip)
				FormatParamNameOrValue(n, szBuf, sizeof(szBuf));
			else
				FormatTrackNameOrNumber(n, szBuf, sizeof(szBuf));

			::strlcat(szLine, szBuf, sizeof(szLine));
		}

		m_HwLCDDisplay.Write(0, 0, szLine, bForceSend);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateLowerLCD(bool bForceSend)
{
	char szLine[128];
	szLine[0] = 0;
	BYTE bX = 0;

	bool bDisplayLevelMeters = (m_cState.GetAssignmentMode() == MCS_ASSIGNMENT_MUTLI_CHANNEL &&
								m_cState.GetDisplayLevelMeters() == METERS_BOTH);

	bool bDisplayFlip = m_cState.GetDisplayFlip();

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		// Turn on meter mode?
		if (m_SwStrip[n].StripExists())
		{
			m_HwLCDDisplay.SetMeterMode(n, true, bDisplayLevelMeters, false, bForceSend);

			if (bDisplayLevelMeters)
				bX += 7;
		}
		else
		{
			m_HwLCDDisplay.SetMeterMode(n, true, false, false, bForceSend);
		}

		char szBuf[16];

		if (bDisplayFlip)
			FormatTrackNameOrNumber(n, szBuf, sizeof(szBuf));
		else
			FormatParamNameOrValue(n, szBuf, sizeof(szBuf));

		::strlcat(szLine, szBuf, sizeof(szLine));
	}

	m_HwLCDDisplay.Write(bX, 1, &szLine[bX], bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::UpdateLevelMeters(bool bForceSend)
{
	if (m_cState.GetDisplayLevelMeters() == METERS_OFF)
		return;

	if ((m_dwRefreshCount % m_cState.GetMetersUpdatePeriod()) != 0)
		return;

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		m_SwStrip[n].SetMetering(true);

		float fVal;
		BYTE bVal;

		HRESULT hr = m_SwStrip[n].ReadMeter(&fVal);

		if (SUCCEEDED(hr))
		{
//			TRACE("Got level %d = %f\n", n, fVal);

			if (fVal > 1.0f)
				fVal = 1.0f;

			bVal = (BYTE)(fVal * 13.0);
		}
		else
		{
			bVal = 0;
		}

		m_HwLCDDisplay.SendMeterLevel(n, bVal);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::FormatTrackNameOrNumber(BYTE bChan, char *szTrack, int len)
{
	AssignmentMode eAssignmentMode = m_cState.GetAssignmentMode();

	bool bDisplayValues = false;

	// Temp display the paramter value instead?
	if (!m_cState.GetDisplayValues() && m_dwTempDisplayValuesCounter[bChan] > 0)
		bDisplayValues = true;

	char szBuf[16];

	if (MCS_ASSIGNMENT_CHANNEL_STRIP == eAssignmentMode)
	{
		m_SwVPot[bChan].GetCrunchedParamLabel(szBuf, 6);
	}
	else
	{
		if (bDisplayValues)
		{
			m_SwVPot[bChan].GetCrunchedValueText(szBuf, 6);
		}
		else if (m_cState.GetDisplayTrackNumbers())
		{
			if (m_SwStrip[bChan].StripExists())
				snprintf(szBuf, sizeof(szBuf), "Trk%3d", m_SwStrip[bChan].GetStripNum() + 1);
			else
				szBuf[0] = 0;
		}
		else
		{
			m_SwStrip[bChan].GetCrunchedStripName(szBuf, 6);
		}
	}

	szBuf[6] = 0;

	snprintf(szTrack, len, "%-6s ", szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::FormatParamNameOrValue(BYTE bChan, char *szParam, int len)
{
	// Display values?
	bool bDisplayValues = m_cState.GetDisplayValues();

	if (m_cState.GetAssignmentMode() == MCS_ASSIGNMENT_CHANNEL_STRIP)
		bDisplayValues = true;

	if (!m_cState.GetDisableLCDUpdates())
	{
		char szBuf[16];

		if (bDisplayValues)
			m_SwVPot[bChan].GetCrunchedValueText(szBuf, 6);
		else
			m_SwVPot[bChan].GetCrunchedParamLabel(szBuf, 6);

		szBuf[6] = 0;

		snprintf(szParam, len, "%-6s ", szBuf);
	}
	else
	{
		::strlcpy(szParam, "       ", len);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::FormatTrackNumbers(char *str, int len, DWORD dwStartNum)
{
	snprintf(str, len, " %3d    %3d    %3d    %3d    %3d    %3d    %3d    %3d  ",
			dwStartNum + 1, dwStartNum + 2, dwStartNum + 3, dwStartNum + 4,
			dwStartNum + 5, dwStartNum + 6, dwStartNum + 7, dwStartNum + 8);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::TempDisplay(DWORD dwCount, char *szText)
{
	::strlcpy(m_szTempDisplayText, szText, sizeof(m_szTempDisplayText));

	m_dwTempDisplayTextCounter = dwCount;
}

/////////////////////////////////////////////////////////////////////////////
