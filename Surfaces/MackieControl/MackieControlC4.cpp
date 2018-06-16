// MackieControlC4.cpp : Defines the basic functionality of the "MackieControlC4".
//

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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

// Version informatin for save/load
#define PERSISTENCE_VERSION				1

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlC4
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CMackieControlC4::CMackieControlC4()
{ 
//	TRACE("CMackieControlC4::CMackieControlC4()\n");

	// Override base class settings
	m_bExpectedDeviceType = 0x17;
	m_bBindMuteSoloArm = true;

	// Wire everything up together
	for (int m = 0; m < NUM_ROWS; m++)
	{
		m_HwLCDDisplay[m].Setup(this);
		m_HwLCDDisplay[m].SetLCDId(0x30 + m);

		m_szTempDisplayText[m][0] = 0;
		m_dwTempDisplayTextCounter[m] = 0;

		for (int n = 0; n < NUM_COLS; n++)
		{
			m_HwVPotDisplay[m][n].Setup(this, 0x20 + (m * NUM_COLS) + n);
			m_dwTempDisplayValuesCounter[m][n] = 0;
		}
	}

	m_dwC4UpdateCount = 0;
	m_dwC4PreviousUpdateCount = 0;

	m_bLinkModifiers = true;
	m_eSplitMode = C4_SPLIT_NONE;
	m_eSplit = C4_UPPER;
	m_eC4Assignment = C4_ASSIGNMENT_NORMAL;
	m_eOldC4Assignment = C4_ASSIGNMENT_NORMAL;
	m_dwModifiers = 0;
	m_bMarkersMode = false;

	::memset(m_dwStripNumOffset, 0, sizeof(m_dwStripNumOffset));
	::memset(m_dwPluginNumOffset, 0, sizeof(m_dwPluginNumOffset));
	::memset(m_dwParamNumOffset, 0, sizeof(m_dwParamNumOffset));

	// Variations
	for (int s = 0; s < NUM_SPLITS; s++)
	{
		m_bLockMode[s] = false;

		for (int n = 0; n < NUM_MIXER_STRIP_TYPES; n++)
		{
			// The first send parameter is send enable,
			// so change the starting point to 1 (send level)
			m_dwParamNumOffset[s][n][MCS_ASSIGNMENT_SEND][MCS_ASSIGNMENT_MUTLI_CHANNEL] = 1;
		}

		m_dwSelectedStripNum[s] = 0;

		m_eMixerStrip[s] = MIX_STRIP_TRACK;
		m_eAssignment[s] = MCS_ASSIGNMENT_PARAMETER;
		m_eAssignmentMode[s] = MCS_ASSIGNMENT_CHANNEL_STRIP;
		m_ePreviousAssignment[s] = m_eAssignment[s];

		m_bEnableMeters[s] = false;
		m_bDisplayValues[s] = false;

		m_dwPreviousStripCount[s] = 0;
		m_dwPreviousPluginCount[s] = 0;
	}

	m_bLockMode[C4_LOWER] = true;

	SetFunctionKey(0, CMD_GOTO_FROM);					SetFunctionKeyName(0, "GoFrom");
	SetFunctionKey(1, CMD_GOTO_THRU);					SetFunctionKeyName(1, "GoThru");
	SetFunctionKey(2, CMD_FROM_EQ_NOW);					SetFunctionKeyName(2, "FromNw");
	SetFunctionKey(3, CMD_THRU_EQ_NOW);					SetFunctionKeyName(3, "ThruNw");
	SetFunctionKey(4, CMD_SET_LOOP_FROM_SELECTION);		SetFunctionKeyName(4, "LpSltn");
	SetFunctionKey(5, CMD_SET_PUNCH_FROM_SELECTION);	SetFunctionKeyName(5, "PnSltn");
	SetFunctionKey(6, NEW_CMD_AUTO_PUNCH_TOGGLE);		SetFunctionKeyName(6, "AuPnch");
	SetFunctionKey(7, NEW_CMD_STOP_WITH_NOW_MARKER);	SetFunctionKeyName(7, "Pause");
}

/////////////////////////////////////////////////////////////////////////////

CMackieControlC4::~CMackieControlC4() 
{ 
//	TRACE("CMackieControlC4::~CMackieControlC4()\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CMackieControlC4::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (NULL == pdwLen)
		return E_POINTER;

	CCriticalSectionAuto csa1(&m_cs);

	CString strStatus;

	if (0x00 == m_bDeviceType)
	{
		strStatus = _T("Connecting...");
	}
	else
	{
		CCriticalSectionAuto csa2(m_cState.GetCS());

		CString strUpper, strLower;

		GetStatusText(C4_UPPER, &strUpper);

		if (m_eSplitMode == C4_SPLIT_NONE)
		{
			strStatus = strUpper;
		}
		else
		{
			GetStatusText(C4_LOWER, &strLower);

			strStatus.Format(_T("U: %-12s L: %-12s"), strUpper, strLower);
		}
	}

	// Return results to caller
	if (NULL == pszStatus)
	{
		*pdwLen = DWORD(strStatus.GetLength() + 1);
	}
	else
	{
		TCHAR2Char(pszStatus, strStatus, *pdwLen);
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/// ISurfaceParamMapping

HRESULT CMackieControlC4::GetStripRangeCount( DWORD *pdwCount)
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = (m_eSplitMode == C4_SPLIT_NONE) ? 1 : 2;

	return S_OK;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::GetStripRange( DWORD dwIndex, DWORD *pdwLowStrip, DWORD *pdwHighStrip, SONAR_MIXER_STRIP *pmixerStrip)
{
	if (dwIndex > 1)
		return E_INVALIDARG;

	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	C4SplitSection eSplit = (dwIndex == 0) ? C4_UPPER : C4_LOWER;

	int s, n;

	if (MCS_ASSIGNMENT_CHANNEL_STRIP == GetAssignmentMode(eSplit))
	{
		s = GetSelectedStripNum(eSplit);
		n = 1;
	}
	else
	{
		s = GetStripNumOffset(eSplit);
		n = GetNumVPotsInSplit(eSplit);
	}

	*pdwLowStrip = s;
	*pdwHighStrip = s + n - 1;
	*pmixerStrip  = GetMixerStrip(eSplit);

	TRACE("CMackieControlC4::GetStripRange(%d) -> %d %d %d\n",
			dwIndex, *pdwLowStrip, *pdwHighStrip, *pmixerStrip);

	return S_OK;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip)
{
	C4SplitSection eSplit = C4_UPPER;

	// Set the strip type first, so that...
	SetMixerStrip(eSplit, mixerStrip);

	// ...these read back the correct strip type
	if (MCS_ASSIGNMENT_CHANNEL_STRIP == GetAssignmentMode(eSplit))
		LimitAndSetSelectedTrack(eSplit, dwLowStrip);
	else
		LimitAndSetStripNumOffset(eSplit, dwLowStrip);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_MackieControlC4
// and put that in your MackieControlC4.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CMackieControlC4::Save( IStream* pStm, BOOL bClearDirty )
{
	TRACE("CMackieControlC4::Save()\n");

	CCriticalSectionAuto csa(&m_cs);

	// Version information
	DWORD dwVer = PERSISTENCE_VERSION;
	if (FAILED(SafeWrite(pStm, &dwVer, sizeof(dwVer))))
	{
		TRACE("CMackieControlC4::Save(): dwVer failed\n");
		return E_FAIL;
	}

	// Function key bindings
	for (int n = 0; n < NUM_C4_USER_FUNCTION_KEYS; n++)
	{
		DWORD dwCmdId = GetFunctionKey(n);

		if (FAILED(SafeWrite(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlC4::Save(): key dwCmdId %d failed\n", n);
			return E_FAIL;
		}

		char szBuf[16] = {0};
		CString strNm = GetFunctionKeyName(n);

		TCHAR2Char( szBuf, strNm, _countof( szBuf ) );
		if (FAILED(SafeWrite(pStm, &szBuf, sizeof(szBuf))))
		{
			TRACE("CMackieControlC4::Save(): key szBuf %d failed\n", n);
			return E_FAIL;
		}
	}

	if (bClearDirty)
		m_bDirty = FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::Load( IStream* pStm )
{
	TRACE("CMackieControlC4::Load()\n");

	CCriticalSectionAuto csa(&m_cs);

	// Version information
	DWORD dwVer;
	if (FAILED(SafeRead(pStm, &dwVer, sizeof(dwVer))))
	{
		TRACE("CMackieControlC4::Load(): dwVer failed\n");
		return E_FAIL;
	}

	if (dwVer > PERSISTENCE_VERSION)
	{
		TRACE("CMackieControlC4::Load(): version mismatch\n");
		return E_FAIL;
	}

	// Function key bindings
	for (int n = 0; n < NUM_C4_USER_FUNCTION_KEYS; n++)
	{
		DWORD dwCmdId;

		if (FAILED(SafeRead(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlC4::Load(): key dwCmdId %d failed\n", n);
			return E_FAIL;
		}

		SetFunctionKey(n, dwCmdId);

		char szBuf[7];
		::memset(szBuf, 0, sizeof(szBuf));

		if (FAILED(SafeRead(pStm, &szBuf, sizeof(szBuf))))
		{
			TRACE("CMackieControlC4::Load(): key szBuf %d failed\n", n);
			return E_FAIL;
		}
		szBuf[6] = 0;

		SetFunctionKeyName(n, szBuf);
	}

	m_bDirty = FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::SafeWrite(IStream *pStm, void const *pv, ULONG cb)
{
	ULONG ulWritten;

	if (pStm->Write(pv, cb, &ulWritten) != S_OK || ulWritten != cb)
	{
		TRACE("CMackieControlC4::SafeWrite(): failed\n");
		return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::SafeRead(IStream *pStm, void *pv, ULONG cb)
{
	ULONG ulRead;

	if (pStm->Read(pv, cb, &ulRead) != S_OK || ulRead != cb)
	{
		TRACE("CMackieControlC4::SafeRead: failed\n");
		return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( &m_cs );

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 0;
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnConnect()
{
	TRACE("CMackieControlC4::OnConnect()\n");

	// Wire these up now that we have a valid m_pMixer
	for (int m = 0; m < NUM_ROWS; m++)
	{
		for (int n = 0; n < NUM_COLS; n++)
		{
			m_SwStrip[m][n].Setup(m_pMixer, m_pTransport, &m_FilterLocator, m_dwUniqueId);
			m_SwVPot[m][n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnDisconnect()
{
	TRACE("CMackieControlC4::OnDisconnect()\n");

	if (m_bConnected)
	{
		for (int m = 0; m < NUM_ROWS; m++)
		{
			m_HwLCDDisplay[m].WriteCentered(0, "", true);
			m_HwLCDDisplay[m].WriteCentered(1, "", true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlC4::OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlC4::OnMidiInShort()\n");

	switch (bStatus)
	{
		case 0x90:
			return OnSwitch(bD1, bD2);
			break;

		case 0xB0:
			return OnVPot(bD1, bD2);
			break;

		default:
			break;
	}

	return false;	// Wasn't for us
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlC4::OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg)
{
	TRACE("CMackieControlC4::OnMidiInLong()\n");

	return false;	// Wasn't for us
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::OnReceivedSerialNumber()
{
	TRACE("CMackieControlC4::OnReceivedSerialNumber()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::SetAllFadersToDefault()
{
	TRACE("CMackieControlC4::SetAllFadersToDefault()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::SetAllVPotsToDefault()
{
	TRACE("CMackieControlC4::SetAllVPotsToDefault()\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::GetStatusText(C4SplitSection eSplit, CString *str)
{
	CString strStatus;

	if (m_eAssignmentMode[eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP)
	{
		switch (GetMixerStrip(eSplit))
		{
			case MIX_STRIP_TRACK:	strStatus = _T("Trk");	break;
			case MIX_STRIP_AUX:		strStatus = _T("Aux");	break;
			case MIX_STRIP_MAIN:		strStatus = _T("VMn");	break;
			case MIX_STRIP_BUS:		strStatus = _T("Bus");	break;
			case MIX_STRIP_MASTER:	strStatus = _T("Mstr");	break;
			default:						strStatus = _T("Err0");	break;
		}

		DWORD dwStripNum = GetSelectedStripNum(eSplit) + 1;

		CString strBuf;
		strBuf.Format(_T(" %d"), dwStripNum);
		strStatus += strBuf;
	}
	else
	{
		switch (GetMixerStrip(eSplit))
		{
			case MIX_STRIP_TRACK:	strStatus = _T("Trk");	break;
			case MIX_STRIP_AUX:		strStatus = _T("Aux");	break;
			case MIX_STRIP_MAIN:		strStatus = _T("VMns");	break;
			case MIX_STRIP_BUS:		strStatus = _T("Bus");	break;
			case MIX_STRIP_MASTER:	strStatus = _T("Mstr");	break;
			default:						strStatus = _T("Err0");	break;
		}

		DWORD dwOffset = GetStripNumOffset(eSplit);
		DWORD dwFirst = dwOffset + 1;
		DWORD dwLast = dwFirst + GetNumVPotsInSplit(eSplit) - 1;

		CString strBuf;
		strBuf.Format( _T(" %d-%d"), dwFirst, dwLast);
		strStatus += strBuf;
	}

	*str = strStatus;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::ShiftTrack(int iAmount)
{
	if (m_eAssignmentMode[m_eSplit] == MCS_ASSIGNMENT_CHANNEL_STRIP)
		ShiftSelectedTrack(m_eSplit, iAmount);
	else
		ShiftStripNumOffset(m_eSplit, iAmount);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlC4::TempDisplaySelectedTrackName(C4SplitSection eSplit)
{
	SONAR_MIXER_STRIP eMixerStrip = GetMixerStrip(eSplit);
	DWORD dwStripNum = GetSelectedStripNum(eSplit);

	char szStripName[64];
	DWORD dwLen = sizeof(szStripName);

	if (!GetStripName(eMixerStrip, dwStripNum, szStripName, &dwLen))
		return;

	char szTrackType[64];
	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:	snprintf(szTrackType, sizeof(szTrackType), "Track %d", dwStripNum + 1);		break;
		case MIX_STRIP_AUX:		snprintf(szTrackType, sizeof(szTrackType), "Aux %d", dwStripNum + 1);		break;
		case MIX_STRIP_MAIN:	snprintf(szTrackType, sizeof(szTrackType), "VMain %c", dwStripNum + 'A');	break;
		case MIX_STRIP_BUS:		snprintf(szTrackType, sizeof(szTrackType), "Bus %d", dwStripNum + 1);		break;
		case MIX_STRIP_MASTER:	snprintf(szTrackType, sizeof(szTrackType), "Master %d", dwStripNum + 1);	break;
		default:
			return;
	}

	char szLine[1024];

	if (IsAPluginMode(GetAssignment(eSplit)))
	{
		DWORD dwPluginNum = GetPluginNumOffset(eSplit);

		char szPluginName[64];
		dwLen = sizeof(szPluginName);

		if (GetCurrentPluginName(GetAssignment(eSplit), GetMixerStrip(eSplit),
									GetSelectedStripNum(eSplit), dwPluginNum,
									szPluginName, &dwLen))
			snprintf(szLine, sizeof(szLine), "%s: \"%s\", Plugin %d: \"%s\"", szTrackType, szStripName, dwPluginNum + 1, szPluginName);
		else
			snprintf(szLine, sizeof(szLine), "%s: \"%s\", Plugin %d: --None--", szTrackType, szStripName, dwPluginNum + 1);
	}
	else
	{
		snprintf(szLine, sizeof(szLine), "%s: \"%s\"", szTrackType, szStripName);
	}

	szLine[LCD_WIDTH] = 0;

	char szBuf[LCD_WIDTH];
	snprintf(szBuf, sizeof(szBuf), "%-56s", szLine);

	BYTE row = 0;
	switch (m_eSplitMode)
	{
		default:
		case C4_SPLIT_NONE:	row = (eSplit == C4_UPPER) ? 0 : 0; break;
		case C4_SPLIT_1_3:	row = (eSplit == C4_UPPER) ? 0 : 1; break;
		case C4_SPLIT_2_2:	row = (eSplit == C4_UPPER) ? 0 : 2; break;
		case C4_SPLIT_3_1:	row = (eSplit == C4_UPPER) ? 0 : 3; break;
	}

	TempDisplay(row, TEMP_DISPLAY_TIMEOUT, szBuf);
}

/////////////////////////////////////////////////////////////////////////////

CMackieControlC4::C4SplitSection CMackieControlC4::GetSplitForRow(BYTE row)
{
	switch (m_eSplitMode)
	{
		default:
		case C4_SPLIT_NONE:	return C4_UPPER;
		case C4_SPLIT_1_3:	return (row < 1) ? C4_UPPER : C4_LOWER;
		case C4_SPLIT_2_2:	return (row < 2) ? C4_UPPER : C4_LOWER;
		case C4_SPLIT_3_1:	return (row < 3) ? C4_UPPER : C4_LOWER;
	}
}

/////////////////////////////////////////////////////////////////////////////

int CMackieControlC4::GetNumVPotsInSplit(C4SplitSection eSplit)
{
	switch (m_eSplitMode)
	{
		default:
		case C4_SPLIT_NONE:	return (eSplit == C4_UPPER) ? 32 :  0;
		case C4_SPLIT_1_3:	return (eSplit == C4_UPPER) ?  8 : 24;
		case C4_SPLIT_2_2:	return (eSplit == C4_UPPER) ? 16 : 16;
		case C4_SPLIT_3_1:	return (eSplit == C4_UPPER) ? 24 :  8;
	}
}

/////////////////////////////////////////////////////////////////////////////