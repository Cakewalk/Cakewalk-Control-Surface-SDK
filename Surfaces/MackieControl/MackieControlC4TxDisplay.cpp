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
#include "MackieControlC4.h"
#include "MackieControlC4PropPage.h"

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateLCDDisplays(bool bForceSend)
{
	if (!IsProjectLoaded())
	{
		m_HwLCDDisplay[0].WriteCentered(0, "No SONAR Project Loaded", bForceSend);
		m_HwLCDDisplay[0].WriteCentered(1, "", bForceSend);

		for (int n = 1; n < NUM_ROWS; n++)
		{
			m_HwLCDDisplay[n].WriteCentered(0, "", bForceSend);
			m_HwLCDDisplay[n].WriteCentered(1, "", bForceSend);
		}
		return;
	}

	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
	{
		for (int m = 0; m < NUM_ROWS; m++)
			ConfigureMeters(m, bForceSend);
	}

	switch (m_eC4Assignment)
	{
		default:
		case C4_ASSIGNMENT_NORMAL:
			UpdateUpperLCD(bForceSend);
			UpdateLowerLCD(bForceSend);
			break;

		case C4_ASSIGNMENT_TRACK:
			DisplayTrackSettings(C4_UPPER, 0, bForceSend);
			DisplayTrackSettings(C4_LOWER, 2, bForceSend);
			break;

		case C4_ASSIGNMENT_FUNCTION:
			//                               123456|123456|123456|123456|123456|123456|123456|123456
			m_HwLCDDisplay[0].WriteToEOL(0, "  F1     F2     F3     F4     F5     F6     F7     F8  ", bForceSend);

			char szLine[128];
			szLine[0] = 0;
			for (int n = 0; n < NUM_C4_USER_FUNCTION_KEYS; n++)
			{
				char szBuf[32];

				snprintf(szBuf, sizeof(szBuf), "%- 6s", (LPCSTR)CStringA(GetFunctionKeyName(n)));
				szBuf[6] = ' ';
				szBuf[7] = 0;

				strlcat(szLine, szBuf, sizeof(szLine));
			}
			m_HwLCDDisplay[0].WriteToEOL(1, szLine, bForceSend);

			//                               123456|123456|123456|123456|123456|123456|123456|123456
			m_HwLCDDisplay[1].WriteToEOL(0, "  Metronome  | Automation  |   Disarm    |     Fit     ", bForceSend);
			m_HwLCDDisplay[1].WriteToEOL(1, "Playbk Record| Snap  Playbk|Tracks Params|Tracks  Proj ", bForceSend);
			//                               123456|123456|123456|123456|123456|123456|123456|123456
			m_HwLCDDisplay[2].WriteToEOL(0, "           Track           |                    | Loop ", bForceSend);
			m_HwLCDDisplay[2].WriteToEOL(1, "NewAud NewMid Clone  Delete| Save   Undo   Redo |On/Off", bForceSend);
			//                               123456|123456|123456|123456|123456|123456|123456|123456
			m_HwLCDDisplay[3].WriteToEOL(0, "                Transport                |Audio Engine ", bForceSend);
			m_HwLCDDisplay[3].WriteToEOL(1, "Rewind  End    Stop   Play  Record RecAtm|On/Off Reset ", bForceSend);
			//                               123456|123456|123456|123456|123456|123456|123456|123456
			break;
	}

	// Update the timeout counters
	for (int m = 0; m < NUM_ROWS; m++)
	{
		if (m_dwTempDisplayTextCounter[m] > 0)
			m_dwTempDisplayTextCounter[m]--;

		for (int n = 0; n < NUM_COLS; n++)
		{
			if (m_dwTempDisplayValuesCounter[m][n] > 0)
				m_dwTempDisplayValuesCounter[m][n]--;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateUpperLCD(bool bForceSend)
{
	if (m_cState.GetDisableLCDUpdates())
	{
		for (int m = 0; m < NUM_ROWS; m++)
			m_HwLCDDisplay[m].WriteCentered(0, "Display Updates Disabled", bForceSend);

		return;
	}

	for (int m = 0; m < NUM_ROWS; m++)
	{
		C4SplitSection eSplit = GetSplitForRow(m);

		if (m_dwTempDisplayTextCounter[m] > 0)
		{
			m_HwLCDDisplay[m].WriteCentered(0, m_szTempDisplayText[m], bForceSend);
		}
		else
		{
			char szLine[128];
			szLine[0] = 0;

			bool bDisplayFlip = m_cState.GetDisplayFlip();

			for (int n = 0; n < NUM_COLS; n++)
			{
				char szBuf[16];

				if (bDisplayFlip)
					FormatParamNameOrValue(eSplit, m, n, szBuf, sizeof(szBuf));
				else
					FormatTrackNameOrNumber(eSplit, m, n, szBuf, sizeof(szBuf));

				::strlcat(szLine, szBuf, sizeof(szLine));
			}

			m_HwLCDDisplay[m].Write(0, 0, szLine, bForceSend);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateLowerLCD(bool bForceSend)
{
	bool bDisplayFlip = m_cState.GetDisplayFlip();

	for (int m = 0; m < NUM_ROWS; m++)
	{
		char szLine[128];
		szLine[0] = 0;

		BYTE bX = ConfigureMeters(m, bForceSend);

		C4SplitSection eSplit = GetSplitForRow(m);

		for (int n = 0; n < NUM_COLS; n++)
		{
			char szBuf[16];

			if (bDisplayFlip)
				FormatTrackNameOrNumber(eSplit, m, n, szBuf, sizeof(szBuf));
			else
				FormatParamNameOrValue(eSplit, m, n, szBuf, sizeof(szBuf));

			::strlcat(szLine, szBuf, sizeof(szLine));
		}

		m_HwLCDDisplay[m].Write(bX, 1, &szLine[bX], bForceSend);
	}
}

/////////////////////////////////////////////////////////////////////////////

BYTE CMackieControlC4::ConfigureMeters(int row, bool bForceSend)
{
	BYTE bX = 0;

	C4SplitSection eSplit = GetSplitForRow(row);

	bool bDisplayLevelMeters = (m_eC4Assignment == C4_ASSIGNMENT_NORMAL &&
								GetAssignmentMode(eSplit) == MCS_ASSIGNMENT_MUTLI_CHANNEL &&
								GetDisplayLevelMeters(eSplit));

	for (int n = 0; n < NUM_COLS; n++)
	{
		if (m_SwStrip[row][n].StripExists())
		{
			m_HwLCDDisplay[row].SetMeterMode(n, true, bDisplayLevelMeters, false, bForceSend);

			if (bDisplayLevelMeters)
				bX += 7;
		}
		else
		{
			m_HwLCDDisplay[row].SetMeterMode(n, true, false, false, bForceSend);
		}
	}

	return bX;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::UpdateLevelMeters(bool bForceSend)
{
	if (m_eC4Assignment != C4_ASSIGNMENT_NORMAL)
		return;

	if ((m_dwRefreshCount % m_cState.GetMetersUpdatePeriod()) != 0)
		return;

	for (int m = 0; m < NUM_ROWS; m++)
	{
		C4SplitSection eSplit = GetSplitForRow(m);

		if (!GetDisplayLevelMeters(eSplit))
			continue;

		if (GetAssignmentMode(eSplit) != MCS_ASSIGNMENT_MUTLI_CHANNEL)
			continue;

		for (int n = 0; n < NUM_COLS; n++)
		{
			m_SwStrip[m][n].SetMetering(true);
	
			float fVal;
			BYTE bVal;

			HRESULT hr = m_SwStrip[m][n].ReadMeter(&fVal);

			if (SUCCEEDED(hr))
			{
//				TRACE("Got level %d, %d = %f\n", m, n, fVal);

				if (fVal > 1.0f)
					fVal = 1.0f;

				bVal = (BYTE)(fVal * 12.0);
			}
			else
			{
				bVal = 0;
			}

			m_HwLCDDisplay[m].SendMeterLevel(n, bVal, m);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::FormatTrackNameOrNumber(C4SplitSection eSplit, BYTE row, BYTE col, char *szTrack, int len)
{
	AssignmentMode eAssignmentMode = GetAssignmentMode(eSplit);

	bool bDisplayValues = false;

	// Temp display the paramter value instead?
	if (!GetDisplayValues(eSplit) && m_dwTempDisplayValuesCounter[row][col] > 0)
		bDisplayValues = true;

	char szBuf[16];

	if (MCS_ASSIGNMENT_CHANNEL_STRIP == eAssignmentMode)
	{
		m_SwVPot[row][col].GetCrunchedParamLabel(szBuf, 6);
	}
	else
	{
		if (bDisplayValues)
		{
			m_SwVPot[row][col].GetCrunchedValueText(szBuf, 6);
		}
		else if (m_cState.GetDisplayTrackNumbers())
		{
			if (m_SwStrip[row][col].StripExists())
				snprintf(szBuf, sizeof(szBuf), "Trk%3d", m_SwStrip[row][col].GetStripNum() + 1);
			else
				szBuf[0] = 0;
		}
		else
		{
			m_SwStrip[row][col].GetCrunchedStripName(szBuf, 6);
		}
	}

	szBuf[6] = 0;

	snprintf(szTrack, len, "%-6s ", szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::FormatParamNameOrValue(C4SplitSection eSplit, BYTE row, BYTE col, char *szParam, int len)
{
	// Display values?
	bool bDisplayValues = GetDisplayValues(eSplit);

	if (GetAssignmentMode(eSplit) == MCS_ASSIGNMENT_CHANNEL_STRIP)
		bDisplayValues = true;

	if (!m_cState.GetDisableLCDUpdates())
	{
		char szBuf[16];

		if (bDisplayValues)
			m_SwVPot[row][col].GetCrunchedValueText(szBuf, 6);
		else
			m_SwVPot[row][col].GetCrunchedParamLabel(szBuf, 6);

		szBuf[6] = 0;

		snprintf(szParam, len, "%-6s ", szBuf);
	}
	else
	{
		::strlcpy(szParam, "       ", len);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::TempDisplay(BYTE row, DWORD dwCount, char *szText)
{
//	TRACE("sizeof: %d\n", sizeof(m_szTempDisplayText[row]));
	::strlcpy(m_szTempDisplayText[row], szText, sizeof(m_szTempDisplayText[row]));

	m_dwTempDisplayTextCounter[row] = dwCount;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::DisplayTrackSettings(C4SplitSection eSplit, BYTE row, bool bForceSend)
{
	CString strUpper, strLower, strTemp;

	//              123456|123456|123456|123456|123456|123456|123456|123456

	strTemp = (eSplit == C4_UPPER) ? _T("Upper") : _T("Lower");
	strTemp += _T(" Control Group");

	strUpper.Format(_T("%-27s "), strTemp);
	strUpper += (m_cState.HaveMeters()) ? _T("Meters ") : _T("       ");
	strUpper += _T("Disply");

	strTemp = _T("Track   Bus   Main");
	strLower.Format(_T("%-27s "), strTemp);
	if (m_cState.HaveMeters())
		strLower += GetDisplayLevelMeters(eSplit) ? _T("On     ") : _T("Off    ");
	else
		strLower += _T("       ");
	
	strLower += GetDisplayValues(eSplit) ? _T("Values") : _T("Names ");

	char szBuf[ 256 ] = {0};
	int const cb = _countof( szBuf );

	TCHAR2Char( szBuf, strUpper, cb );
	m_HwLCDDisplay[row].WriteToEOL(0, szBuf, bForceSend);

	TCHAR2Char( szBuf, strLower, cb );
	m_HwLCDDisplay[row].WriteToEOL(1, szBuf, bForceSend);


	strUpper = (eSplit == C4_UPPER) ?_T("Upper") : _T("Lower");
	strUpper += _T(" Assignment");
	strLower = _T("Params Sends   Pan   Plugin   EQ    Dyn");

	TCHAR2Char( szBuf, strUpper, cb );
	m_HwLCDDisplay[row + 1].WriteToEOL(0, szBuf, bForceSend);

	TCHAR2Char( szBuf, strLower, cb );
	m_HwLCDDisplay[row + 1].WriteToEOL(1, szBuf, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////
