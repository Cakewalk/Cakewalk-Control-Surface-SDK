// MackieControlMaster.cpp : Defines the basic functionality of the "MackieControlMaster".
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

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

// Version informatin for save/load
#define PERSISTENCE_VERSION				10

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlMaster
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CMackieControlMaster::CMackieControlMaster()
{
//	TRACE("CMackieControlMaster::CMackieControlMaster()\n");

	m_bExpectedDeviceType = 0x14;

	m_bWasInEQFreqGainMode = false;

	m_eCurrentTransportDir = DIR_FORWARD;
	m_fTransportTimeout = 0;

	m_bKeyRepeatKey = 0;
	m_fKeyRepeatTimeout = 0;
	
	int n = 0;

	// Wire everything up together
	for (n = 0; n < NUM_VSELECT_DISPLAY_CELLS; n++)
		m_cVSelectDisplay[n].Setup(this, 0x0A + NUM_VSELECT_DISPLAY_CELLS - n - 1);

	for (n = 0; n < NUM_TIME_CODE_DISPLAY_CELLS; n++)
		m_cTimeCodeDisplay[n].Setup(this, NUM_TIME_CODE_DISPLAY_CELLS - n - 1);

	m_HwMasterFader.Setup(this, 8);

	CCriticalSectionAuto csa(m_cState.GetCS());

	// Default user key and foot bindings
	m_cState.SetUserFunctionKey(0, CMD_VIEW_NEW_EXPLORER);
	m_cState.SetUserFunctionKey(1, CMD_VIEW_NEW_LOOP_CONSTR);
	m_cState.SetUserFunctionKey(2, CMD_VIEW_CONSOLE);
	m_cState.SetUserFunctionKey(3, CMD_VIEW_NEW_EVENT_LIST);
	m_cState.SetUserFunctionKey(4, CMD_VIEW_NEW_PIANO_ROLL);
	m_cState.SetUserFunctionKey(5, CMD_VIEW_VIDEO);
	m_cState.SetUserFunctionKey(6, CMD_VIEW_NEW_STAFF);
	m_cState.SetUserFunctionKey(7, CMD_VIEW_NEW_LYRICS);
	m_cState.SetUserFootSwitch(0, CMD_REALTIME_PLAY);
	m_cState.SetUserFootSwitch(1, CMD_REALTIME_RECORD);

	TIMECAPS timeDevCaps;
	timeGetDevCaps(&timeDevCaps, sizeof(timeDevCaps));
	TRACE("timeDevCaps.wPeriodMin = %d\n", timeDevCaps.wPeriodMin);
	m_wTransportTimerPeriod = max(timeDevCaps.wPeriodMin, 10);
	m_wKeyRepeatTimerPeriod = max(timeDevCaps.wPeriodMin, 10);
	m_bTransportTimerActive = false;
	m_bKeyRepeatTimerActive = false;
	m_bLastButtonPressed = 0;
	m_bScrubKeyDown = false;

	getTempTrackTemplateFilename();
}

/////////////////////////////////////////////////////////////////////////////

CMackieControlMaster::~CMackieControlMaster()
{
//	TRACE("CMackieControlMaster::~CMackieControlMaster()\n");

	KillTransportCallbackTimer();
	KillKeyRepeatCallbackTimer();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CMackieControlMaster::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
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

		switch (m_cState.GetMixerStrip())
		{
			case MIX_STRIP_TRACK:
				strStatus = _T("Trks");
				break;

			case MIX_STRIP_AUX:
				strStatus = _T("Auxs");
				break;

			case MIX_STRIP_MAIN:
				strStatus = _T("VMns");
				break;

			case MIX_STRIP_BUS:
				strStatus = _T("Bus");
				break;

			case MIX_STRIP_MASTER:
				strStatus =_T( "Mstr");
				break;

			default:
				strStatus = _T("Err0");
				break;
		}

		DWORD dwOffset = m_cState.GetStripNumOffset();
		DWORD dwFirst = m_cState.GetFirstFaderNumber() + dwOffset + 1;
		DWORD dwLast = m_cState.GetLastFaderNumber() + dwOffset + 1;

		CString strBuf;
		strBuf.Format(_T(" %d-%d, "), dwFirst, dwLast);
		strStatus += strBuf;

		switch (m_SwMasterFader.GetMixerStrip())
		{
			case MIX_STRIP_TRACK:
				strBuf.Format(_T("Trk %d"), m_SwMasterFader.GetStripNum() + 1);
				break;

			case MIX_STRIP_AUX:
				strBuf.Format(_T("Aux %d"), m_SwMasterFader.GetStripNum() + 1);
				break;

			case MIX_STRIP_MAIN:
				strBuf.Format(_T("VMn %c"), m_SwMasterFader.GetStripNum() + 'A');
				break;

			case MIX_STRIP_BUS:
				strBuf.Format(_T("Bus %d"), m_SwMasterFader.GetStripNum() + 1);
				break;

			case MIX_STRIP_MASTER:
				strBuf.Format(_T("Mstr %d"), m_SwMasterFader.GetStripNum() + 1);
				break;

			default:
				strBuf = _T("Err1");
				break;
		}
		strStatus += strBuf;

		strStatus += _T(", Jog: ");

		switch (m_cState.GetJogResolution())
		{
			case JOG_MEASURES:
				strStatus += _T("Meas.");
				break;

			case JOG_BEATS:
				strStatus += _T("Beats");
				break;

			case JOG_TICKS:
				strStatus += _T("Ticks");
				break;

			default:
				strStatus += _T("Err2");
				break;
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

HRESULT CMackieControlMaster::GetStripRangeCount( DWORD *pdwCount)
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = 2;		// Strips and Master

	return S_OK;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::GetStripRange( DWORD dwIndex, DWORD *pdwLowStrip, DWORD *pdwHighStrip, SONAR_MIXER_STRIP *pmixerStrip)
{
	if (dwIndex > 1)
		return E_INVALIDARG;

	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	// Strips
	if (0 == dwIndex)
		return CMackieControlXT::GetStripRange(dwIndex, pdwLowStrip, pdwHighStrip, pmixerStrip);

	// Master
	*pdwLowStrip = *pdwHighStrip = m_SwMasterFader.GetStripNum();
	*pmixerStrip = m_SwMasterFader.GetMixerStrip();

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip)
{
	TRACE("CMackieControlMaster::SetStripRange(%u, %d) %d\n", dwLowStrip, mixerStrip, m_SwMasterFader.GetMixerStrip());

	if (m_SwMasterFader.GetMixerStrip() == mixerStrip)
	{
		SafeSetMasterFaderOffset(mixerStrip, dwLowStrip);

		return S_OK;
	}

	return CMackieControlXT::SetStripRange(dwLowStrip, mixerStrip);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_MackieControlMaster
// and put that in your MackieControlMaster.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CMackieControlMaster::Save( IStream* pStm, BOOL bClearDirty )
{
	TRACE("CMackieControlMaster::Save()\n");

	CCriticalSectionAuto csa1(&m_cs);
	CCriticalSectionAuto csa2(m_cState.GetCS());

	// Version information
	DWORD dwVer = PERSISTENCE_VERSION;
	if (FAILED(SafeWrite(pStm, &dwVer, sizeof(dwVer))))
	{
		TRACE("CMackieControlMaster::Save(): dwVer failed\n");
		return E_FAIL;
	}

	int n = 0;

	// Function key bindings
	for (n = 0; n < NUM_USER_FUNCTION_KEYS; n++)
	{
		DWORD dwCmdId = m_cState.GetUserFunctionKey(n);

		if (FAILED(SafeWrite(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlMaster::Save(): key dwCmdId %d failed\n", n);
			return E_FAIL;
		}
	}

	// Foot switch bindings
	for (n = 0; n < NUM_USER_FOOT_SWITCHES; n++)
	{
		DWORD dwCmdId = m_cState.GetUserFootSwitch(n);

		if (FAILED(SafeWrite(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlMaster::Save(): foot dwCmdId %d failed\n", n);
			return E_FAIL;
		}
	}

	// Master fader offset
	DWORD dwOffset = m_cState.GetMasterFaderOffset(m_cState.GetMasterFaderType());
	if (FAILED(SafeWrite(pStm, &dwOffset, sizeof(dwOffset))))
	{
		TRACE("CMackieControlMaster::Save(): master fader offset failed\n");
		return E_FAIL;
	}

	// Jog resolution
	JogResolution eJogResolution = m_cState.GetJogResolution(); 
	if (FAILED(SafeWrite(pStm, &eJogResolution, sizeof(eJogResolution))))
	{
		TRACE("CMackieControlMaster::Save(): m_eJogResolution failed\n");
		return E_FAIL;
	}

	// Transport resolution
	JogResolution eTransportResolution = m_cState.GetTransportResolution(); 
	if (FAILED(SafeWrite(pStm, &eTransportResolution, sizeof(eTransportResolution))))
	{
		TRACE("CMackieControlMaster::Save(): m_eTransportResolution failed\n");
		return E_FAIL;
	}

	// Time display format
	bool bDisplaySMPTE = m_cState.GetDisplaySMPTE();
	if (FAILED(SafeWrite(pStm, &bDisplaySMPTE, sizeof(bDisplaySMPTE))))
	{
		TRACE("CMackieControlMaster::Save(): m_bDisplaySMPTE failed\n");
		return E_FAIL;
	}

	// Disable faders
	bool bDisableFaders = m_cState.GetDisableFaders();
	if (FAILED(SafeWrite(pStm, &bDisableFaders, sizeof(bDisableFaders))))
	{
		TRACE("CMackieControlMaster::Save(): bDisableFaders failed\n");
		return E_FAIL;
	}

	// Disable relay click
	bool bDisableRelayClick = m_cState.GetDisableRelayClick();
	if (FAILED(SafeWrite(pStm, &bDisableRelayClick, sizeof(bDisableRelayClick))))
	{
		TRACE("CMackieControlMaster::Save(): bDisableRelayClick failed\n");
		return E_FAIL;
	}

	// Disable LCD updates
	bool bDisableLCDUpdates = m_cState.GetDisableLCDUpdates();
	if (FAILED(SafeWrite(pStm, &bDisableLCDUpdates, sizeof(bDisableLCDUpdates))))
	{
		TRACE("CMackieControlMaster::Save(): bDisableLCDUpdates failed\n");
		return E_FAIL;
	}

	// Solo selects channel
	bool bSoloSelectsChannel = m_cState.GetSoloSelectsChannel();
	if (FAILED(SafeWrite(pStm, &bSoloSelectsChannel, sizeof(bSoloSelectsChannel))))
	{
		TRACE("CMackieControlMaster::Save(): m_bSoloSelectsChannel failed\n");
		return E_FAIL;
	}

	// Unit information (offsets)
	{
		vectorMackieControlInformation vInfo;

		m_cState.GetUnitOffsets(&vInfo);

		DWORD dwNumUnits = (DWORD)vInfo.size();
		if (FAILED(SafeWrite(pStm, &dwNumUnits, sizeof(dwNumUnits))))
		{
			TRACE("CMackieControlMaster::Save(): vectorMackieControlInformation dwNumUnits failed\n");
			return E_FAIL;
		}

		for (DWORD n = 0; n < dwNumUnits; n++)
		{
			if (!vInfo[n].Save(pStm))
				return E_FAIL;
		}
	}

	// Added in PERSISTENCE_VERSION = 3

	// Fader touch selects channel
	bool bFaderTouchSelectsChannel = m_cState.GetFaderTouchSelectsChannel();
	if (FAILED(SafeWrite(pStm, &bFaderTouchSelectsChannel, sizeof(bFaderTouchSelectsChannel))))
	{
		TRACE("CMackieControlMaster::Save(): bFaderTouchSelectsChannel failed\n");
		return E_FAIL;
	}

	// Select highlights track
	bool bSelectHighlightsTrack = m_cState.GetSelectHighlightsTrack();
	if (FAILED(SafeWrite(pStm, &bSelectHighlightsTrack, sizeof(bSelectHighlightsTrack))))
	{
		TRACE("CMackieControlMaster::Save(): bSelectHighlightsTrack failed\n");
		return E_FAIL;
	}

	// Added in PERSISTENCE_VERSION = 4

	// Master fader type
	SONAR_MIXER_STRIP eMixerStrip = m_cState.GetMasterFaderType();
	if (FAILED(SafeWrite(pStm, &eMixerStrip, sizeof(eMixerStrip))))
	{
		TRACE("CMackieControlMaster::Save(): eMixerStrip failed\n");
		return E_FAIL;
	}

	// Added in PERSISTENCE_VERSION = 5
	LevelMeters eLevelMeters = m_cState.GetDisplayLevelMeters();
	if (FAILED(SafeWrite(pStm, &eLevelMeters, sizeof(eLevelMeters))))
	{
		TRACE("CMackieControlMaster::Save(): eLevelMeters failed\n");
		return E_FAIL;
	}

	// Disable handshake PERSISTENcE_VERSION = 6
	bool bDisableHandshake = m_cState.GetDisableHandshake();
	if (FAILED(SafeWrite(pStm, &bDisableHandshake, sizeof(bDisableHandshake))))
	{
		TRACE("CMackieControlMaster::Save(): bDisableHandshake failed\n");
		return E_FAIL;
	}

	//  PERSISTENcE_VERSION = 7
	bool bExcludeFiltersFromPlugins = m_cState.GetExcludeFiletersFromPlugins();
	if (FAILED(SafeWrite(pStm, &bExcludeFiltersFromPlugins, sizeof(bExcludeFiltersFromPlugins))))
	{
		TRACE("CMackieControlMaster::Save(): bExcludeFiltersFromPlugins failed\n");
		return E_FAIL;
	}

	//  PERSISTENcE_VERSION = 8
	// Double click for select
	bool bSelectDoubleClick = m_cState.GetSelectDoubleClick();
	if (FAILED(SafeWrite(pStm, &bSelectDoubleClick, sizeof(bSelectDoubleClick))))
	{
		TRACE("CMackieControlMaster::Save(): bSelectDoubleClick failed\n");
		return E_FAIL;
	}

	// PERSISTENcE_VERSION = 9
	// Scrub+Bank Down/Up selects Tracks / Buses
	bool bScrubBankSelectsTrackBus = m_cState.GetScrubBankSelectsTrackBus();
	if (FAILED(SafeWrite(pStm, &bScrubBankSelectsTrackBus, sizeof(bScrubBankSelectsTrackBus))))
	{
		TRACE("CMackieControlMaster::Save(): bScrubBankSelectsTrackBus failed\n");
		return E_FAIL;
	}

	// PERSISTENCE_VERSION = 10
	bool bUseHUIProtocol = m_cState.GetUseHUIProtocol();
	if (FAILED(SafeWrite(pStm, &bUseHUIProtocol, sizeof(bUseHUIProtocol))))
	{
		TRACE("CMackieControlMaster::Save(): bUseHUIProtocol failed\n");
		return E_FAIL;
	}

	bool bHUIKeypad = m_cState.GetHUIKeyPadControlsKeyPad();
	if (FAILED(SafeWrite(pStm, &bHUIKeypad, sizeof(bHUIKeypad))))
	{
		TRACE("CMackieControlMaster::Save(): bHUIKeypad failed\n");
		return E_FAIL;
	}

	if (bClearDirty)
		m_bDirty = FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::Load( IStream* pStm )
{
	TRACE("CMackieControlMaster::Load()\n");

	CCriticalSectionAuto csa1(&m_cs);
	CCriticalSectionAuto csa2(m_cState.GetCS());

	// Version information
	DWORD dwVer;
	if (FAILED(SafeRead(pStm, &dwVer, sizeof(dwVer))))
	{
		TRACE("CMackieControlMaster::Load(): dwVer failed\n");
		return E_FAIL;
	}

	if (dwVer > PERSISTENCE_VERSION)
	{
		TRACE("CMackieControlMaster::Load(): version mismatch\n");
		return E_FAIL;
	}

	int n = 0; 

	// Function key bindings
	for (n = 0; n < NUM_USER_FUNCTION_KEYS; n++)
	{
		DWORD dwCmdId;

		if (FAILED(SafeRead(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlMaster::Load(): key dwCmdId %d failed\n", n);
			return E_FAIL;
		}

		m_cState.SetUserFunctionKey(n, dwCmdId);
	}

	// Foot switch bindings
	for (n = 0; n < NUM_USER_FOOT_SWITCHES; n++)
	{
		DWORD dwCmdId;

		if (FAILED(SafeRead(pStm, &dwCmdId, sizeof(dwCmdId))))
		{
			TRACE("CMackieControlMaster::Load(): foot dwCmdId %d failed\n", n);
			return E_FAIL;
		}

		m_cState.SetUserFootSwitch(n, dwCmdId);
	}

	// Master fader offset
	DWORD dwMasterFaderOffset;
	if (FAILED(SafeRead(pStm, &dwMasterFaderOffset, sizeof(dwMasterFaderOffset))))
	{
		TRACE("CMackieControlMaster::Load(): master fader offset failed\n");
		return E_FAIL;
	}
	if (dwVer < 4)
		SafeSetMasterFaderOffset(m_cState.GetMasterFaderType(), dwMasterFaderOffset);

	// Jog resolution
	JogResolution eJogResolution;
	if (FAILED(SafeRead(pStm, &eJogResolution, sizeof(eJogResolution))))
	{
		TRACE("CMackieControlMaster::Load(): m_eJogResolution failed\n");
		return E_FAIL;
	}
	m_cState.SetJogResolution(eJogResolution);

	// Transport resolution
	JogResolution eTransportResolution;
	if (FAILED(SafeRead(pStm, &eTransportResolution, sizeof(eTransportResolution))))
	{
		TRACE("CMackieControlMaster::Load(): m_eTransportResolution failed\n");
		return E_FAIL;
	}
	m_cState.SetTransportResolution(eTransportResolution);

	// Time display format
	bool bDisplaySMPTE;
	if (FAILED(SafeRead(pStm, &bDisplaySMPTE, sizeof(bDisplaySMPTE))))
	{
		TRACE("CMackieControlMaster::Load(): m_bDisplaySMPTE failed\n");
		return E_FAIL;
	}
	m_cState.SetDisplaySMPTE(bDisplaySMPTE);

	// Disable faders
	bool bDisableFaders;
	if (FAILED(SafeRead(pStm, &bDisableFaders, sizeof(bDisableFaders))))
	{
		TRACE("CMackieControlMaster::Load(): bDisableFaders failed\n");
		return E_FAIL;
	}
	m_cState.SetDisableFaders(bDisableFaders);

	// Disable relay click
	bool bDisableRelayClick;
	if (FAILED(SafeRead(pStm, &bDisableRelayClick, sizeof(bDisableRelayClick))))
	{
		TRACE("CMackieControlMaster::Load(): bDisableRelayClick failed\n");
		return E_FAIL;
	}
	m_cState.SetDisableRelayClick(bDisableRelayClick);
	SetRelayClick(!bDisableRelayClick);

	// Disable LCD updates
	bool bDisableLCDUpdates;
	if (FAILED(SafeRead(pStm, &bDisableLCDUpdates, sizeof(bDisableLCDUpdates))))
	{
		TRACE("CMackieControlMaster::Load(): bDisableLCDUpdates failed\n");
		return E_FAIL;
	}
	m_cState.SetDisableLCDUpdates(bDisableLCDUpdates);

	// Solo selects channel
	bool bSoloSelectsChannel;
	if (FAILED(SafeRead(pStm, &bSoloSelectsChannel, sizeof(bSoloSelectsChannel))))
	{
		TRACE("CMackieControlMaster::Load(): m_bSoloSelectsChannel failed\n");
		return E_FAIL;
	}
	m_cState.SetSoloSelectsChannel(bSoloSelectsChannel);

	// Unit information (offsets)
	{
		DWORD dwNumUnits;
		if (FAILED(SafeRead(pStm, &dwNumUnits, sizeof(dwNumUnits))))
		{
			TRACE("CMackieControlMaster::Load(): vectorMackieControlInformation dwNumUnits failed\n");
			return E_FAIL;
		}

		vectorMackieControlInformation vInfo;

		for (n = 0; n < (int)dwNumUnits; n++)
		{
			CMackieControlInformation cInfo;

			if (!cInfo.Load(pStm))
				return E_FAIL;

			vInfo.push_back(cInfo);
		}

		m_cState.SetUnitOffsets(&vInfo);
	}

	if (dwVer >= 3)
	{
		// Fader touch selects channel
		bool bFaderTouchSelectsChannel;
		if (FAILED(SafeRead(pStm, &bFaderTouchSelectsChannel, sizeof(bFaderTouchSelectsChannel))))
		{
			TRACE("CMackieControlMaster::Load(): bFaderTouchSelectsChannel failed\n");
			return E_FAIL;
		}
		m_cState.SetFaderTouchSelectsChannel(bFaderTouchSelectsChannel);

		// Select highlights track
		bool bSelectHighlightsTrack;
		if (FAILED(SafeRead(pStm, &bSelectHighlightsTrack, sizeof(bSelectHighlightsTrack))))
		{
			TRACE("CMackieControlMaster::Load(): bSelectHighlightsTrack failed\n");
			return E_FAIL;
		}
		m_cState.SetSelectHighlightsTrack(bSelectHighlightsTrack);
	}

	if (dwVer >= 4)
	{
		// Master fader type
		SONAR_MIXER_STRIP eMixerStrip;
		if (FAILED(SafeRead(pStm, &eMixerStrip, sizeof(eMixerStrip))))
		{
			TRACE("CMackieControlMaster::Load(): eMixerStrip failed\n");
			return E_FAIL;
		}
		m_cState.SetMasterFaderType(eMixerStrip);

		SafeSetMasterFaderOffset(eMixerStrip, dwMasterFaderOffset);
	}

	if (dwVer >= 5)
	{
		// Level meters
		LevelMeters eLevelMeters;
		if (FAILED(SafeRead(pStm, &eLevelMeters, sizeof(eLevelMeters))))
		{
			TRACE("CMackieControlMaster::Load(): eLevelMeters failed\n");
			return E_FAIL;
		}
		m_cState.SetDisplayLevelMeters(eLevelMeters);
	}

	if (dwVer >= 6)
	{
		// Disable handshake
		bool bDisableHandshake;
		if (FAILED(SafeRead(pStm, &bDisableHandshake, sizeof(bDisableHandshake))))
		{
			TRACE("CMackieControlMaster::Load(): bDisableHandshake failed\n");
			return E_FAIL;
		}
		m_cState.SetDisableHandshake(bDisableHandshake);
	}
	if (dwVer >= 7)
	{
		// Disable handshake
		bool bExcludeFiletersFromPlugins;
		if (FAILED(SafeRead(pStm, &bExcludeFiletersFromPlugins, sizeof(bExcludeFiletersFromPlugins))))
		{
			TRACE("CMackieControlMaster::Load(): bExcludeFiltersFromPlugins failed\n");
			return E_FAIL;
		}
		m_cState.SetExcludeFiltersFromPlugins(bExcludeFiletersFromPlugins);
	}
	if (dwVer >= 8)
	{
		// Double click select
		bool bSelectDoubleClick;
		if (FAILED(SafeRead(pStm, &bSelectDoubleClick, sizeof(bSelectDoubleClick))))
		{
			TRACE("CMackieControlMaster::Load(): bSelectDoubleClick failed\n");
			return E_FAIL;
		}
		m_cState.SetSelectDoubleClick(bSelectDoubleClick);
	}
	if (dwVer >= 9)
	{
		// Scrub + Bank Down/Up selects Tracks / Buses
		bool bScrubBankSelectsTrackBus;
		if (FAILED(SafeRead(pStm, &bScrubBankSelectsTrackBus, sizeof(bScrubBankSelectsTrackBus))))
		{
			TRACE("CMackieControlMaster::Load(): bScrubBankSelectsTrackBus failed\n");
			return E_FAIL;
		}
		m_cState.SetScrubBankSelectsTrackBus(bScrubBankSelectsTrackBus);
	}
	if (dwVer >= 10)
	{
		bool bUseHUIProtocol;
		if (FAILED(SafeRead(pStm, &bUseHUIProtocol, sizeof(bUseHUIProtocol))))
		{
			TRACE("CMackieControlMaster::Load(): bUseHUIProtocol failed\n");
			return E_FAIL;
		}
		m_cState.SetUseHUIProtocol(bUseHUIProtocol);

		// HUI Keypad controls keypad keys rather than standard numeric keys above QUERTY
		bool bHUIKeypad;
		if (FAILED(SafeRead(pStm, &bHUIKeypad, sizeof(bHUIKeypad))))
		{
			TRACE("CMackieControlMaster::Load(): bHUIKeypad failed\n");
			return E_FAIL;
		}
		m_cState.SetHUIKeyPadControlsKeyPad(bHUIKeypad);
	}

	m_bDirty = FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa(&m_cs);

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 4096;	// Arbitrary
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::SafeWrite(IStream *pStm, void const *pv, ULONG cb)
{
	ULONG ulWritten;

	if (pStm->Write(pv, cb, &ulWritten) != S_OK || ulWritten != cb)
	{
		TRACE("CMackieControlMaster::SafeWrite(): failed\n");
		return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::SafeRead(IStream *pStm, void *pv, ULONG cb)
{
	ULONG ulRead;

	if (pStm->Read(pv, cb, &ulRead) != S_OK || ulRead != cb)
	{
		TRACE("CMackieControlMaster::SafeRead: failed\n");
		return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

void CALLBACK EXPORT CMackieControlMaster::_TransportTimerCallback(UINT uID, UINT uMsg,
																	DWORD_PTR dwUser, DWORD_PTR dw1,
																	DWORD_PTR dw2)
{
	CMackieControlMaster *pMC = (CMackieControlMaster *)dwUser;

	if (pMC)
	{
		pMC->KillTransportCallbackTimer();
		pMC->TransportTimerCallback();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetTransportCallbackTimer(float fAlpha, UINT uMax, UINT uMin)
{
	float fMax = (float)uMax;
	float fMin = (float)uMin;

	m_fTransportTimeout = ((1.0f - fAlpha) * m_fTransportTimeout) + (fAlpha);

	UINT uElapse = (UINT)(fMax - m_fTransportTimeout * (fMax - fMin));

//	TRACE("CMackieControlMaster::SetTransportCallbackTimer(): %d\n", uElapse);

	// Just in case
	KillTransportCallbackTimer();

	// Setup the timer
	timeBeginPeriod(m_wTransportTimerPeriod);

	m_uiTransportTimerID = timeSetEvent(uElapse,
										m_wTransportTimerPeriod,
										(LPTIMECALLBACK)_TransportTimerCallback,
										(DWORD_PTR)this,
										TIME_ONESHOT | TIME_CALLBACK_FUNCTION);

	if (!m_uiTransportTimerID)
		TRACE("Transport timeSetEvent failed!\n");

	m_bTransportTimerActive = true;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ClearTransportCallbackTimer()
{
	KillTransportCallbackTimer();
	m_fTransportTimeout = 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::KillTransportCallbackTimer()
{
	if (m_bTransportTimerActive)
	{
		if (m_uiTransportTimerID)
			timeKillEvent(m_uiTransportTimerID);

		timeEndPeriod(m_wTransportTimerPeriod);

		m_bTransportTimerActive = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CALLBACK EXPORT CMackieControlMaster::_KeyRepeatTimerCallback(UINT uID, UINT uMsg,
																	DWORD_PTR dwUser, DWORD_PTR dw1,
																	DWORD_PTR dw2)
{
	CMackieControlMaster *pMC = (CMackieControlMaster *)dwUser;
	
	if (pMC)
	{
		pMC->KillKeyRepeatCallbackTimer();
		pMC->KeyRepeatTimerCallback();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetKeyRepeatCallbackTimer(float fAlpha, UINT uMax, UINT uMin)
{
	float fMax = (float)uMax;
	float fMin = (float)uMin;

	m_fKeyRepeatTimeout = ((1.0f - fAlpha) * m_fKeyRepeatTimeout) + (fAlpha);

	UINT uElapse = (UINT)(fMax - m_fKeyRepeatTimeout * (fMax - fMin));

//	TRACE("CMackieControlMaster::SetKeyRepeatCallbackTimer(): %d\n", uElapse);

	// Just in case
	KillKeyRepeatCallbackTimer();

	// Setup the timer
	timeBeginPeriod(m_wKeyRepeatTimerPeriod);

	m_uiKeyRepeatTimerID = timeSetEvent(uElapse,
										m_wKeyRepeatTimerPeriod,
										(LPTIMECALLBACK)_KeyRepeatTimerCallback,
										(DWORD_PTR)this,
										TIME_ONESHOT | TIME_CALLBACK_FUNCTION);

	if (!m_uiKeyRepeatTimerID)
		TRACE("KeyRepeat timeSetEvent failed!\n");

	m_bKeyRepeatTimerActive = true;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ClearKeyRepeatCallbackTimer()
{
	KillKeyRepeatCallbackTimer();
	m_fKeyRepeatTimeout = 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::KillKeyRepeatCallbackTimer()
{
	if (m_bKeyRepeatTimerActive)
	{
		if (m_uiKeyRepeatTimerID)
			timeKillEvent(m_uiKeyRepeatTimerID);

		timeEndPeriod(m_wKeyRepeatTimerPeriod);

		m_bKeyRepeatTimerActive = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::OnConnect()
{
//	TRACE("CMackieControlMaster::OnConnect()\n");

	// Tell the XT section too
	CMackieControlXT::OnConnect();

	// Wire these up now that we have a valid m_pMixer
	m_SwMasterFader.Setup(m_pMixer, m_pTransport, &m_FilterLocator);
	m_SwMasterFaderPan.Setup(m_pMixer, m_pTransport, &m_FilterLocator);

	CCriticalSectionAuto csa(m_cState.GetCS());
	ReconfigureMaster(true);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::OnDisconnect()
{
//	TRACE("CMackieControlMaster::OnDisconnect()\n");

	// Tell the XT section too
	CMackieControlXT::OnDisconnect();
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
//	TRACE("CMackieControlMaster::OnMidiInShort()\n");

	// See if it's for the XT section
	if (CMackieControlXT::OnMidiInShort(bStatus, bD1, bD2))
		return true;

	if (UsingHUIProtocol())
	{
		if (bD1 == 0x0D)
			return OnJog((bD2 & 0x40) ? (bD2 - 0x40) : (bD2 + 0x40)); // HUI goes reverse direction to MCU

		if ((bD1 == 0x2F) || (bD1 == 0x2C)) // switch
		{
			BYTE b_D1 = 0;
			BYTE b_D2 = 0;

			bool translated = TranslateHUIButtons(m_bCurrentHUIZone, (bD2 & 0x0F), ((bD2 & 0x40) > 0), b_D1, b_D2);

			if (translated)
			{
				if (b_D1 >= HUI_KEYPAD_0)
					return OnHuiSwitch(b_D1, b_D2);
				else
					return OnSwitch(b_D1, b_D2);
			}
			else
				return false;
		}
	}


	switch (bStatus)
	{
		case 0xE8:
			return OnFader(bD1, bD2);
			break;

		case 0x90:
			return OnSwitch(bD1, bD2);
			break;

		case 0xB0:
			if (0x2E == bD1)
			{
				return OnExternalController(bD2);
			}
			else if (0x3C == bD1)
			{
				return OnJog(bD2);
			}
			break;

		default:
			break;
	}

	return false;	// Wasn't for us
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg)
{
//	TRACE("CMackieControlMaster::OnMidiInLong()\n");

	// See if it's for the XT section
	if (CMackieControlXT::OnMidiInLong(cbLongMsg, pbLongMsg))
		return true;

	return false;	// Wasn't for us
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::OnReceivedSerialNumber()
{
	TRACE("CMackieControlMaster::OnReceivedSerialNumber()\n");

	CMackieControlXT::OnReceivedSerialNumber();

	SetRelayClick(!m_cState.GetDisableRelayClick());
}

void CMackieControlMaster::QuerySerialNumber(BYTE bDeviceType)
{
	TRACE("CMackieControlMaster::QuerySerialNumber(): 0x%02X\n", bDeviceType);

	if (!GetDisableHandshake()) {
		CMackieControlBase::QuerySerialNumber(bDeviceType);
		return;
	}
	// don't start till project is loaded.  The ISonarProject
	// APIs return E_FAIL if no project is loaded.
	if (m_pProject && !SUCCEEDED(m_pProject->GetProjectModified()))
		return;

	if (!m_bHaveSerialNumber)
	{
		if (GetUseHUIProtocol())
			m_bDeviceType = 0x05; // HUI
		else
			m_bDeviceType = m_bExpectedDeviceType;

		m_bHaveSerialNumber = true;

		m_bForceRefreshWhenDone = true;

		CCriticalSectionAuto csa(m_cState.GetCS());
		m_cState.RestoreUnitOffset(this);

		OnReceivedSerialNumber();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ReconfigureMaster(bool bForce)
{
	if (!bForce && m_cState.GetMasterUpdateCount() == m_dwMasterUpdateCount)
		return;

	TRACE("CMackieControlMaster::ReconfigureMaster(): [0x%08X] Reconfiguring, %d\n", this, m_dwUnitStripNumOffset);

	SONAR_MIXER_STRIP eBusType    = m_cState.BusType();
	SONAR_MIXER_STRIP eMasterType = m_cState.MasterType();

	switch (m_cState.GetMixerStrip())
	{
		default:
		case MIX_STRIP_TRACK:
			{
				SONAR_MIXER_STRIP eType = m_cState.GetMasterFaderType();

				m_SwMasterFader.SetParams(eType, m_cState.GetMasterFaderOffset(eType), MIX_PARAM_VOL, 0);

				if (eType == MIX_STRIP_MASTER)
					m_SwMasterFaderPan.ClearBinding();
				else
					m_SwMasterFaderPan.SetParams(eType, m_cState.GetMasterFaderOffset(eType), MIX_PARAM_PAN, 0);
			}
			break;

		case MIX_STRIP_AUX:		// SONAR 2
			m_SwMasterFader.SetParams(   eMasterType, m_cState.GetMasterFaderOffset(eMasterType), MIX_PARAM_VOL, 0);
			m_SwMasterFaderPan.SetParams(eMasterType, m_cState.GetMasterFaderOffset(eMasterType), MIX_PARAM_PAN, 0);
			break;

		case MIX_STRIP_MAIN:	// SONAR 2
			m_SwMasterFader.SetParams(   eBusType, m_cState.GetMasterFaderOffset(eBusType), MIX_PARAM_SEND_VOL, 0);
			m_SwMasterFaderPan.SetParams(eBusType, m_cState.GetMasterFaderOffset(eBusType), MIX_PARAM_PAN, 0);
			break;

		case MIX_STRIP_BUS:		// SONAR 3+
			m_SwMasterFader.SetParams(   eMasterType, m_cState.GetMasterFaderOffset(eMasterType), MIX_PARAM_VOL, 0);
			m_SwMasterFaderPan.ClearBinding();
			break;

		case MIX_STRIP_MASTER:	// SONAR 3
			m_SwMasterFader.SetParams(   eBusType, m_cState.GetMasterFaderOffset(eBusType), MIX_PARAM_VOL, 0);
			m_SwMasterFaderPan.SetParams(eBusType, m_cState.GetMasterFaderOffset(eBusType), MIX_PARAM_PAN, 0);
			break;
	}

	m_SwMasterFader.SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
	m_SwMasterFaderPan.SetAttribs(DT_PAN, PAN_CENTER, 0.025f);

	OnContextSwitch();

	m_dwMasterUpdateCount = m_cState.GetMasterUpdateCount();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ShiftStripNumOffset(int iAmount)
{
	LimitAndSetStripNumOffset(m_cState.GetStripNumOffset() + iAmount);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ShiftPluginNumOffset(int iAmount)
{
	if (IsAPluginMode(m_cState.GetAssignment()))
	{
		int iPluginNumOffset = m_cState.GetPluginNumOffset() + iAmount;

		if (iPluginNumOffset < 0)
			iPluginNumOffset = 0;
		else if (iPluginNumOffset > 9)
			iPluginNumOffset = 9;

		m_cState.SetPluginNumOffset(iPluginNumOffset);

		TempDisplaySelectedTrackName();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ShiftParamNumOffset(int iAmount)
{
	int iParamNumOffset = m_cState.GetParamNumOffset() + iAmount;

	int iNumParams = GetNumParams(m_cState.GetMixerStrip(), m_cState.GetSelectedStripNum(),
									m_cState.GetPluginNumOffset(), m_cState.GetAssignment());

	if (MCS_ASSIGNMENT_EQ_FREQ_GAIN == m_cState.GetAssignment() ||
		MCS_ASSIGNMENT_CHANNEL_STRIP == m_cState.GetAssignmentMode())
	{
		int iLastFader = m_cState.GetLastFaderNumber();

		if (iParamNumOffset + iLastFader >= iNumParams)
			iParamNumOffset = iNumParams - iLastFader - 1;
	}
	else	// MCS_ASSIGNMENT_MUTLI_CHANNEL
	{
		if (iParamNumOffset >= iNumParams)
			iParamNumOffset = iNumParams - 1;
	}

	if (iParamNumOffset < 0)
		iParamNumOffset = 0;

	m_cState.SetParamNumOffset(iParamNumOffset);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SafeSetMasterFaderOffset(SONAR_MIXER_STRIP eMixerStrip, DWORD dwOffset)
{
	if (dwOffset < GetStripCount(eMixerStrip))
		m_cState.SetMasterFaderOffset(eMixerStrip, dwOffset);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::TempDisplayMasterFader()
{
	char szBuf[LCD_WIDTH];

	SONAR_MIXER_STRIP eMixerStrip = m_SwMasterFader.GetMixerStrip();

	// Note: don't use m_SwMasterFader.GetStripNum() as it won't be
	// updated until the next ReconfigureMaster()
	// Well, it doesn't work, anyway

	DWORD dwOffset = m_cState.GetMasterFaderOffset(eMixerStrip);

	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:
			snprintf(szBuf, sizeof(szBuf), "Master Fader = Track %d", dwOffset + 1);
			break;

		case MIX_STRIP_AUX:
			snprintf(szBuf, sizeof(szBuf), "Master Fader = Aux %d", dwOffset + 1);
			break;

		case MIX_STRIP_MAIN:
			snprintf(szBuf, sizeof(szBuf), "Master Fader = VMain %c", dwOffset + 'A');
			break;

		case MIX_STRIP_BUS:
			snprintf(szBuf, sizeof(szBuf), "Master Fader = Bus %d", dwOffset + 1);
			break;

		case MIX_STRIP_MASTER:
			snprintf(szBuf, sizeof(szBuf), "Master Fader = Master %d", dwOffset + 1);
			break;

		default:
			::strlcpy(szBuf, "Bad mixer strip type", sizeof(szBuf));
			break;
	}

	TempDisplay(TEMP_DISPLAY_TIMEOUT, szBuf);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::TempDisplayMasterFaderPan()
{
	if (!m_SwMasterFaderPan.HasBinding())
		return;

	char szBuf[16];
	DWORD dwLen = 12;

	m_SwMasterFaderPan.GetValueText(szBuf, &dwLen);
	szBuf[12] = 0;

	char szLine[LCD_WIDTH];

	switch (m_SwMasterFaderPan.GetMixerStrip())
	{
		case MIX_STRIP_TRACK:
			snprintf(szLine, sizeof(szLine), "Track %d Pan: %-6s", m_SwMasterFaderPan.GetStripNum() + 1, szBuf);
			break;

		case MIX_STRIP_AUX:
			snprintf(szLine, sizeof(szLine), "Aux %d Pan: %-6s", m_SwMasterFaderPan.GetStripNum() + 1, szBuf);
			break;

		case MIX_STRIP_MAIN:
			snprintf(szLine, sizeof(szLine), "VMain %c Pan: %-6s", m_SwMasterFaderPan.GetStripNum() + 'A', szBuf);
			break;

		case MIX_STRIP_BUS:
			snprintf(szLine, sizeof(szLine), "Bus %d Pan: %-6s", m_SwMasterFaderPan.GetStripNum() + 1, szBuf);
			break;

		case MIX_STRIP_MASTER:
			snprintf(szLine, sizeof(szLine), "Master %d Pan: %-6s", m_SwMasterFaderPan.GetStripNum() + 1, szBuf);
			break;

		default:
			::strlcpy(szLine, "Bad mixer strip type", sizeof(szLine));
			break;
	}

	TempDisplay(TEMP_DISPLAY_TIMEOUT, szLine);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::getTempTrackTemplateFilename()
{
	WCHAR szRegValueBuffer[1024];
	size_t dwRegValueBufferSize = 1024;
	auto cbData = static_cast<DWORD>(dwRegValueBufferSize);
	auto rc = RegGetValueW(HKEY_CURRENT_USER, L"SOFTWARE\\Cakewalk Music Software\\Cakewalk\\Core\\UserPaths\\", L"TrackTplFolder", RRF_RT_REG_SZ, nullptr, static_cast<void*>(&szRegValueBuffer[0]), &cbData);
	if (rc != ERROR_SUCCESS)
		GetTempPathW(1024, static_cast<LPWSTR>(szRegValueBuffer));

	m_cbTempTrackTemplateFilename = szRegValueBuffer;
	m_cbTempTrackTemplateFilename.Append(L"\\__mcu-temp-track-template__.cwx");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::ZeroAllFaders()
{
	CMackieControlXT::ZeroAllFaders();
	m_HwMasterFader.SetVal(0.0f, true);
	m_HwMasterFader.SetVal(0.01f, true);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::TranslateHUIButtons(BYTE bCurrentZone, BYTE bPort, bool bOn, BYTE &bD1, BYTE &bD2)
{
	if (CMackieControlXT::TranslateHUIButtons(bCurrentZone, bPort, bOn, bD1, bD2))
		return true;

	bD1 = 0xFF;
	bD2 = bOn ? 0x7F : 0x00;

	switch (bCurrentZone)
	{
		case 0x08:
			switch (bPort)
			{
				case 0x07:
					bD1 = MC_SAVE;
					break;
			}
			break;

		case 0x09:
			switch (bPort)
			{
				case 0x03:
					bD1 = MC_MARKER;
					break;
			}
			break;

		case 0x13:
			switch (bPort)
			{
				case 0x00:
					bD1 = HUI_KEYPAD_0;
					break;

				case 0x01:
					bD1 = HUI_KEYPAD_1;
					break;

				case 0x02:
					bD1 = HUI_KEYPAD_4;
					break;

				case 0x03:
					bD1 = HUI_KEYPAD_2;
					break;

				case 0x04:
					bD1 = HUI_KEYPAD_5;
					break;

				case 0x05:
					bD1 = HUI_KEYPAD_DOT;
					break;

				case 0x06:
					bD1 = HUI_KEYPAD_3;
					break;

				case 0x07:
					bD1 = HUI_KEYPAD_6;
					break;
			}
			break;

		case 0x14:
			switch (bPort)
			{
				case 0x00:
					bD1 = HUI_KEYPAD_ENTER;
					break;

				case 0x01:
					bD1 = HUI_KEYPAD_PLUS;
					break;
			}
			break;

		case 0x15:
			switch (bPort)
			{
				case 0x00:
					bD1 = HUI_KEYPAD_7;
					break;

				case 0x01:
					bD1 = HUI_KEYPAD_8;
					break;

				case 0x02:
					bD1 = HUI_KEYPAD_9;
					break;

				case 0x03:
					bD1 = HUI_KEYPAD_MINUS;
					break;

				case 0x04:
					bD1 = HUI_KEYPAD_CLR;
					break;

				case 0x05:
					bD1 = HUI_KEYPAD_EQUALS;
					break;

				case 0x06:
					bD1 = HUI_KEYPAD_FWDSLASH;
					break;

				case 0x07:
					bD1 = HUI_KEYPAD_ASTERISK;
					break;
			}
			break;

		case 0x0A:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_CHANNEL_DOWN;
					break;

				case 0x01:
					bD1 = MC_BANK_DOWN;
					break;

				case 0x02:
					bD1 = MC_CHANNEL_UP;
					break;

				case 0x03:
					bD1 = MC_BANK_UP;
					break;
			}
			break;

		case 0x0D:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_CURSOR_DOWN;
					break;

				case 0x01:
					bD1 = MC_CURSOR_LEFT;
					break;

				case 0x02:
					bD1 = MC_CURSOR_ZOOM;
					break;

				case 0x03:
					bD1 = MC_CURSOR_RIGHT;
					break;

				case 0x04:
					bD1 = MC_CURSOR_UP;
					break;

				case 0x05:
					bD1 = MC_SCRUB;
					break;
			}
			break;

		case 0x0E:
			switch (bPort)
			{
				case 0x01:
					bD1 = MC_REWIND;
					break;

				case 0x02:
					bD1 = MC_FAST_FORWARD;
					break;

				case 0x03:
					bD1 = MC_STOP;
					break;

				case 0x04:
					bD1 = MC_PLAY;
					break;

				case 0x05:
					bD1 = MC_RECORD;
					break;
			}
			break;

		case 0x0F:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_HOME;
					break;

				case 0x03:
					bD1 = MC_LOOP_ON_OFF;
					break;

				case 0x04:
					bD1 = MC_PUNCH;
					break;
			}
			break;

		case 0x16:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_SMPTE;
					break;

				case 0x02:
					bD1 = MC_BEATS;
					break;

				case 0x03:
					bD1 = MC_RUDE;
					break;
			}
			break;

		case 0x17:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_PLUG_INS;
					break;

				case 0x01:
					bD1 = MC_PAN;
					break;

				case 0x04:
					bD1 = MC_SEND;
					break;
			}
			break;

		case 0x18:
			switch (bPort)
			{
				case 0x01:
					bD1 = MC_OFFSET;
					break;

				case 0x02:
					bD1 = MC_READ_OFF;
					break;

				case 0x03:
					bD1 = MC_DISARM;
					break;

				case 0x04:
					bD1 = MC_SNAPSHOT;
					break;

				case 0x05:
					bD1 = MC_DISARM;
					break;

				case 0x06:
					bD1 = MC_JOG_PRM;
					break;
			}
			break;

		case 0x1A:
			switch (bPort)
			{
				case 0x02:
					bD1 = MC_SNAPSHOT;
					break;
			}
			break;

		case 0x1B:
			bD1 = MC_F1 + bPort;
			break;

		case 0x1C:
			switch (bPort)
			{
				case 0x00:
					bD1 = MC_PARAM;
					break;

				case 0x02:
					bD1 = MC_M1;
					break;

				case 0x03:
					bD1 = MC_M2;
					break;

				case 0x04:
					bD1 = MC_M3;
					break;

				case 0x05:
					bD1 = MC_M4;
					break;
			}
			break;
	}

	return (bD1 != 0xFF);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::SetHuiLED(BYTE bID, BYTE bVal, bool bForceSend)
{
	if (bForceSend); // TODO

	if (CMackieControlXT::SetHuiLED(bID, bVal, bForceSend))
		return true;

	switch (bID)
	{
		case MC_LOOP_ON_OFF:
			SendMidiShort(0xB0, 0x0C, 0x0F);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x43 : 0x03);
			return true;

		case MC_REWIND:
			SendMidiShort(0xB0, 0x0C, 0x0E);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x41 : 0x01);
			return true;

		case MC_FAST_FORWARD:
			SendMidiShort(0xB0, 0x0C, 0x0E);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);
			return true;

		case MC_STOP:
			SendMidiShort(0xB0, 0x0C, 0x0E);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x43 : 0x03);
			return true;

		case MC_PLAY:
			SendMidiShort(0xB0, 0x0C, 0x0E);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x44 : 0x04);
			return true;

		case MC_RECORD:
			SendMidiShort(0xB0, 0x0C, 0x0E);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x45 : 0x05);
			return true;

		case MC_SCRUB:
			SendMidiShort(0xB0, 0x0C, 0x0D);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x45 : 0x05);
			return true;

		case MC_PARAM:
			SendMidiShort(0xB0, 0x0C, 0x1C);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x40 : 0x00);
			return true;

		case MC_SEND:
			SendMidiShort(0xB0, 0x0C, 0x17);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x44 : 0x04);
			return true;

		case MC_PAN:
			SendMidiShort(0xB0, 0x0C, 0x17);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x41 : 0x01);
			return true;

		case MC_PLUG_INS:  // not sure if this is right... this should mean bypass FX on a HUI
			SendMidiShort(0xB0, 0x0C, 0x17);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x40 : 0x00);
			return true;

		/*
		case MC_EQ:
		case MC_DYNAMICS:
		case MC_FLIP:
		case MC_EDIT:
			return true;
		*/

		case MC_READ_OFF:
			SendMidiShort(0xB0, 0x0C, 0x18);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);
			return true;

		case MC_SNAPSHOT:  // use "write" button - the equivalent button on MCU overlay
			SendMidiShort(0xB0, 0x0C, 0x18);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x44 : 0x04);

			SendMidiShort(0xB0, 0x0C, 0x1A);  // also use "capture" button
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);

			return true;

		case MC_DISARM:  // use "touch" button - the equivalent button on MCU overlay
			SendMidiShort(0xB0, 0x0C, 0x18);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x45 : 0x05);

			SendMidiShort(0xB0, 0x0C, 0x18); // also use "off" button
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x43 : 0x03);
			return true;

		case MC_OFFSET:  // use "latch" button
			SendMidiShort(0xB0, 0x0C, 0x18);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x41 : 0x01);
			return true;

		/*
		case MC_SELECT:
		case MC_TRACK:
		case MC_AUX:
		case MC_MAIN:
		case MC_LOOP:
			return true;
		*/

		case MC_SAVE:
			SendMidiShort(0xB0, 0x0C, 0x08);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x47 : 0x07);
			return true;

		case MC_MARKER:
			SendMidiShort(0xB0, 0x0C, 0x09);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x48 : 0x03);
			return true;

		case MC_PUNCH:
			SendMidiShort(0xB0, 0x0C, 0x0F);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x44 : 0x04);
			return true;

		case MC_SMPTE:
			SendMidiShort(0xB0, 0x0C, 0x16);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x40 : 0x00);
			return true;

		case MC_BEATS:
			SendMidiShort(0xB0, 0x0C, 0x16);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);
			return true;

		case MC_RUDE:
			SendMidiShort(0xB0, 0x0C, 0x16);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x43 : 0x03);
			return true;

		case MC_CURSOR_ZOOM:
			SendMidiShort(0xB0, 0x0C, 0x0D);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x42 : 0x02);
			return true;

		case MC_JOG_PRM:
			SendMidiShort(0xB0, 0x0C, 0x0D);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x46 : 0x06);
			return true;

		case MC_HOME:
			SendMidiShort(0xB0, 0x0C, 0x0F);
			SendMidiShort(0xB0, 0x2C, (bVal != 0) ? 0x40 : 0x00);
			return true;

		default: return false;
	}
}

/////////////////////////////////////////////////////////////////////////////

MackieSurfaceType CMackieControlMaster::GetSurfaceType()
{
	return SURFACE_TYPE_MASTER;
}

/////////////////////////////////////////////////////////////////////////////
