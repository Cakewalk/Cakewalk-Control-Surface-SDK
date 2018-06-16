// MackieControlXT.cpp : Defines the basic functionality of the "MackieControlXT".
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
#include "MackieControlFader.h"
#include "MackieControlXT.h"
#include "MackieControlXTPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlXT
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CMackieControlXT::CMackieControlXT()
{
//	TRACE("CMackieControlXT::CMackieControlXT()\n");

	m_bExpectedDeviceType = 0x15;

	// Wire everything up together
	m_HwLCDDisplay.Setup(this);

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		m_HwVPotDisplay[n].Setup(this, 0x30 + n);
		m_HwFader[n].Setup(this, n);

		m_dwTempDisplayValuesCounter[n] = 0;
	}

	m_dwXTUpdateCount = 0;
	m_dwPreviousStripCount = 0;
	m_dwPreviousPluginCount = 0;

	m_szTempDisplayText[0] = 0;
	m_dwTempDisplayTextCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////

CMackieControlXT::~CMackieControlXT()
{
//	TRACE("CMackieControlXT::~CMackieControlXT()\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CMackieControlXT::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (NULL == pdwLen)
		return E_POINTER;

	CCriticalSectionAuto csa1(&m_cs);

	CString strStatus;

	if (0x00 == m_bDeviceType)
	{
		strStatus = "Connecting...";
	}
	else
	{
		strStatus = "Mackie Control XT OK";
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

HRESULT CMackieControlXT::GetStripRangeCount( DWORD *pdwCount)
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = 1;		// Strips

	return S_OK;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::GetStripRange( DWORD dwIndex, DWORD *pdwLowStrip, DWORD *pdwHighStrip, SONAR_MIXER_STRIP *pmixerStrip)
{
	if (dwIndex > 0)
		return E_INVALIDARG;

	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	*pdwLowStrip = m_dwUnitStripNumOffset + m_cState.GetStripNumOffset();
	*pdwHighStrip = *pdwLowStrip + NUM_MAIN_CHANNELS - 1;
	*pmixerStrip = m_cState.GetMixerStrip();

	return S_OK;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip)
{
	// Set the strip type first, so that...
	m_cState.SetMixerStrip(mixerStrip);

	// ...this reads back the correct strip type
	LimitAndSetStripNumOffset(dwLowStrip - m_dwUnitStripNumOffset);

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_MackieControlXT
// and put that in your MackieControlXT.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CMackieControlXT::Save( IStream* pStm, BOOL bClearDirty )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should write any data to pStm which you wish to store.
	// Typically you will write values for all of your properties.
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.

	// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
	// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::Load( IStream* pStm )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should read the data using the format you used in Save().
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.
	
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
	// E_FAIL. Don't simply return whatever IStream::Read() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( &m_cs );

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 0;
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnConnect()
{
//	TRACE("CMackieControlXT::OnConnect()\n");

	// Wire these up now that we have a valid m_pMixer
	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		m_SwStrip[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator, m_dwUniqueId);
		m_SwVPot[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwRec[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwSolo[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwMute[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwArchive[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwInputEcho[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
		m_SwFader[n].Setup(m_pMixer, m_pTransport, &m_FilterLocator);
	}

	CCriticalSectionAuto csa(m_cState.GetCS());
	ReconfigureXT(true);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnDisconnect()
{
//	TRACE("CMackieControlXT::OnDisconnect()\n");

	if (m_bConnected)
	{
		m_HwLCDDisplay.WriteCentered(0, "", true);
		m_HwLCDDisplay.WriteCentered(1, "", true);
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlXT::OnMidiInShort()\n");

	switch (bStatus)
	{
		case 0xE0:
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
			return OnFader(bStatus & 0x0F, bD1, bD2);
			break;

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

bool CMackieControlXT::OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg)
{
//	TRACE("CMackieControlXT::OnMidiInLong()\n");

	return false;	// Wasn't for us
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnReceivedSerialNumber()
{
	TRACE("CMackieControlXT::OnReceivedSerialNumber()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::SetAllFadersToDefault()
{
//	TRACE("CMackieControlXT::SetAllFadersToDefault()\n");

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
		m_SwFader[n].SetToDefaultValue();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::SetAllVPotsToDefault()
{
//	TRACE("CMackieControlXT::SetAllVPotsToDefault()\n");

	for (int n = 0; n < NUM_MAIN_CHANNELS; n++)
	{
		HRESULT hr = m_SwVPot[n].SetToDefaultValue();

		if (SUCCEEDED(hr))
			m_dwTempDisplayValuesCounter[n] = TEMP_DISPLAY_TIMEOUT;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::TempDisplaySelectedTrackName()
{
	SONAR_MIXER_STRIP eMixerStrip = m_cState.GetMixerStrip();
	DWORD dwStripNum = m_cState.GetSelectedStripNum();

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

	if (IsAPluginMode(m_cState.GetAssignment()))
	{
		char szPluginName[64];
		dwLen = sizeof(szPluginName);

		if (GetCurrentPluginName(m_cState.GetAssignment(), m_cState.GetMixerStrip(),
								m_cState.GetSelectedStripNum(), m_cState.GetPluginNumOffset(),
								szPluginName, &dwLen))
			snprintf(szLine, sizeof(szLine), "%s: \"%s\", Plugin: \"%s\"", szTrackType, szStripName, szPluginName);
		else
			snprintf(szLine, sizeof(szLine), "%s: \"%s\", Plugin: --None--", szTrackType, szStripName);
	}
	else
	{
		snprintf(szLine, sizeof(szLine), "%s: \"%s\"", szTrackType, szStripName);
	}

	szLine[LCD_WIDTH] = 0;

	char szBuf[LCD_WIDTH];
	snprintf(szBuf, sizeof(szBuf), "%-56s", szLine);

	m_cState.SetTempDisplayText(szBuf);
}

/////////////////////////////////////////////////////////////////////////////
