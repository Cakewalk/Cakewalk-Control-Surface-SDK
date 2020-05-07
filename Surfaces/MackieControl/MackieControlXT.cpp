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
	m_cState.AddUnit(this);

	// Load the plugin mappings here so that we can locate the .ini file correctly
	m_cState.LoadPluginMappings();

	UpdateToolbarDisplay(true);

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

void CMackieControlXT::ClearLCDDisplay( bool bTurnMetersOff, bool bTurnMetersBackOn )
{
	// turn metering off
	if ( bTurnMetersOff )
		for ( int i = 0; i < NUM_MAIN_CHANNELS; i++ )
		{
			m_HwLCDDisplay.SetMeterMode( i, true, false, false, true );
		}

	// write spaces to the lcd strips
	m_HwLCDDisplay.WriteCentered( 0, " ", true );
	m_HwLCDDisplay.WriteCentered( 1, " ", true );

	// force the lower LCD to update, which will turn metering back on if
	// necessary.
	if ( bTurnMetersBackOn )
		UpdateLowerLCD( true );
}

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

	ClearLCDDisplay(true, true);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::OnDisconnect()
{
//	TRACE("CMackieControlXT::OnDisconnect()\n");

	if (m_bConnected)
	{
		ZeroAllFaders();

		ClearLCDDisplay( true, false );
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlXT::OnMidiInShort()\n");

	if (UsingHUIProtocol())
		return OnHUIMidiInShort(bStatus, bD1, bD2);

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
		case MIX_STRIP_RACK:	snprintf(szTrackType, sizeof(szTrackType), "Synth Rack");					break;
		default:
			return;
	}

	char szLine[1024];

	if (IsAPluginMode(m_cState.GetAssignment()))
	{
		char szPluginName[64];
		dwLen = sizeof(szPluginName);

		if ( GetCurrentPluginName( m_cState.GetAssignment(), m_cState.GetMixerStrip(),
			m_cState.GetSelectedStripNum(), m_cState.GetPluginNumOffset(),
			szPluginName, &dwLen ) )
		{
			if ( eMixerStrip == MIX_STRIP_RACK )
				snprintf( szLine, sizeof( szLine ), "Rack Synth %d: %s", m_cState.GetPluginNumOffset() + 1, szPluginName );
			else
				snprintf( szLine, sizeof( szLine ), "%s: \"%s\", Plugin %d: \"%s\"", szTrackType, szStripName, m_cState.GetPluginNumOffset() + 1, szPluginName );
		}
		else
		{
			if ( eMixerStrip == MIX_STRIP_RACK )
				snprintf( szLine, sizeof( szLine ), "Rack Synth %d: --None--", m_cState.GetPluginNumOffset() + 1 );
			else
				snprintf( szLine, sizeof( szLine ), "%s: \"%s\", Plugin %d: --None--", szTrackType, szStripName, m_cState.GetPluginNumOffset() + 1 );
		}
	}
	else
	{
		if ( eMixerStrip == MIX_STRIP_RACK )
			snprintf( szLine, sizeof( szLine ), "%s", szTrackType );
		else
			snprintf(szLine, sizeof(szLine), "%s: \"%s\"", szTrackType, szStripName);
	}

	szLine[LCD_WIDTH] = 0;

	char szBuf[LCD_WIDTH];
	snprintf(szBuf, sizeof(szBuf), "%-56s", szLine);

	m_cState.SetTempDisplayText(szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::SetProjectLoadedState(bool bProjectLoadedState)
{
	m_bProjectLoadedState = bProjectLoadedState;

	if (!bProjectLoadedState)
		ZeroAllFaders();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlXT::ZeroAllFaders()
{
	for (int i = 0; i < NUM_MAIN_CHANNELS; i++)
	{
		// set fader to minimum
		m_HwFader[i].SetVal(0.0f, true);

		// move fader up slightly - can help with slightly sticky faders during calibration
		m_HwFader[i].SetVal(0.01f, true);
	}

}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::OnHUIMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
	if (bStatus == 0x90) // ping
		return true;

	if (bStatus != 0xB0)
		return false;

	// Zone Select
	if (bD1 == 0x0F)
	{
		m_bCurrentHUIZone = bD2;
		return true;
	}

	if (bD1 < 0x08) // fader msb
	{
		m_bHUIFaderHi[bD1] = bD2;
		return true;
	}

	if ((bD1 >= 0x20) && (bD1 <= 0x27)) // fader lsb
	{
		return OnFader((bD1 - 0x20), bD2, m_bHUIFaderHi[(bD1 - 0x20)]);
	}

	if ((bD1 == 0x2F) || (bD1 == 0x2C)) // switch
	{
		BYTE b_D1 = 0;
		BYTE b_D2 = 0;

		bool translated = TranslateHUIButtons(m_bCurrentHUIZone, (bD2 & 0x0F), ((bD2 & 0x40) > 0), b_D1, b_D2);

		if (translated)
			return OnSwitch(b_D1, b_D2);
		else
			return false;
	}

	if ((bD1 >= 0x40) && (bD1 <= 0x47)) // VPot
		return OnVPot((bD1 - 0x30), bD2);

	return false;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::TranslateHUIButtons(BYTE bCurrentZone, BYTE bPort, bool bOn, BYTE &bD1, BYTE &bD2)
{
	bD1 = 0xFF;
	bD2 = bOn ? 0x7F : 0x00;

	switch (bCurrentZone)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_FADER_1 + bCurrentZone;
					break;

				case 0x01:
					bD1 = MC_SELECT_1 + bCurrentZone;
					break;

				case 0x02:
					bD1 = MC_MUTE_1 + bCurrentZone;
					break;

				case 0x03:
					bD1 = MC_SOLO_1 + bCurrentZone;
					break;

				case 0x04:
					bD1 = MC_VPOT_1 + bCurrentZone;
					break;

				case 0x07:
					bD1 = MC_REC_ARM_1 + bCurrentZone;
					break;
			}
			break;
	}
	return (bD1 != 0xFF);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlXT::SetHuiLED(BYTE bID, BYTE bVal, bool bForceSend)
{
	switch (bID)
	{
		case MC_REC_ARM_1:
		case MC_REC_ARM_2:
		case MC_REC_ARM_3:
		case MC_REC_ARM_4:
		case MC_REC_ARM_5:
		case MC_REC_ARM_6:
		case MC_REC_ARM_7:
		case MC_REC_ARM_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_REC_ARM_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x47 : 0x07);
			return true;

		case MC_SOLO_1:
		case MC_SOLO_2:
		case MC_SOLO_3:
		case MC_SOLO_4:
		case MC_SOLO_5:
		case MC_SOLO_6:
		case MC_SOLO_7:
		case MC_SOLO_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_SOLO_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x43 : 0x03);
			return true;

		case MC_MUTE_1:
		case MC_MUTE_2:
		case MC_MUTE_3:
		case MC_MUTE_4:
		case MC_MUTE_5:
		case MC_MUTE_6:
		case MC_MUTE_7:
		case MC_MUTE_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_MUTE_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);
			return true;

		case MC_SELECT_1:
		case MC_SELECT_2:
		case MC_SELECT_3:
		case MC_SELECT_4:
		case MC_SELECT_5:
		case MC_SELECT_6:
		case MC_SELECT_7:
		case MC_SELECT_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_SELECT_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x41 : 0x01);
			return true;

		case MC_VPOT_1:
		case MC_VPOT_2:
		case MC_VPOT_3:
		case MC_VPOT_4:
		case MC_VPOT_5:
		case MC_VPOT_6:
		case MC_VPOT_7:
		case MC_VPOT_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_VPOT_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x45 : 0x05);
			return true;

		case MC_FADER_1:
		case MC_FADER_2:
		case MC_FADER_3:
		case MC_FADER_4:
		case MC_FADER_5:
		case MC_FADER_6:
		case MC_FADER_7:
		case MC_FADER_8:
			SendMidiShort(0xB0, 0x0C, (bID - MC_FADER_1));
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x40 : 0x00);
			return true;

		default: return false;
	}
}

/////////////////////////////////////////////////////////////////////////////

MackieSurfaceType CMackieControlXT::GetSurfaceType()
{
	return SURFACE_TYPE_XT;
}

/////////////////////////////////////////////////////////////////////////////
