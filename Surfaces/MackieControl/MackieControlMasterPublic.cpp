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

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetConfigureLayoutMode(bool bEnable)
{
//	TRACE("CMackieControlMaster::SetConfigureLayoutMode() %d\n", bEnable);

	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetConfigureLayoutMode(bEnable);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetConfigureLayoutMode()
{
//	TRACE("CMackieControlMaster::GetConfigureLayoutMode()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetConfigureLayoutMode();
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlMaster::GetFunctionKey(BYTE bN)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_USER_FUNCTION_KEYS)
		return m_cState.GetUserFunctionKey(bN);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetFunctionKey(BYTE bN, DWORD dwCmdId)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_USER_FUNCTION_KEYS)
		m_cState.SetUserFunctionKey(bN, dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlMaster::GetFootSwitch(BYTE bN)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_USER_FOOT_SWITCHES)
		return m_cState.GetUserFootSwitch(bN);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetFootSwitch(BYTE bN, DWORD dwCmdId)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	if (bN < NUM_USER_FOOT_SWITCHES)
		m_cState.SetUserFootSwitch(bN, dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlMaster::GetMasterFaderOffset()
{
	TRACE("CMackieControlMaster::GetMasterFaderOffset()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetMasterFaderOffset(m_cState.GetMasterFaderType());
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetMasterFaderOffset(DWORD dwN)
{
	TRACE("CMackieControlMaster::SetMasterFaderOffset(%d)\n", dwN);

	CCriticalSectionAuto csa(m_cState.GetCS());

	SafeSetMasterFaderOffset(m_cState.GetMasterFaderType(), dwN);
}

/////////////////////////////////////////////////////////////////////////////

SONAR_MIXER_STRIP CMackieControlMaster::GetMasterFaderType()
{
	TRACE("CMackieControlMaster::GetMasterFaderType()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetMasterFaderType();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetMasterFaderType(SONAR_MIXER_STRIP eMixerStrip)
{
	TRACE("CMackieControlMaster::SetMasterFaderType(%d)\n", eMixerStrip);

	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetMasterFaderType(eMixerStrip);
}

/////////////////////////////////////////////////////////////////////////////

JogResolution CMackieControlMaster::GetJogResolution()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetJogResolution();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetJogResolution(JogResolution eRes)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetJogResolution(eRes);
}

/////////////////////////////////////////////////////////////////////////////

JogResolution CMackieControlMaster::GetTransportResolution()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetTransportResolution();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetTransportResolution(JogResolution eRes)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetTransportResolution(eRes);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetDisplaySMPTE()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisplaySMPTE();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisplaySMPTE(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisplaySMPTE(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetDisableFaders()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisableFaders();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisableFaders(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisableFaders(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetDisableRelayClick()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisableRelayClick();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisableRelayClick(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisableRelayClick(bVal);

	SetRelayClick(!bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetDisableLCDUpdates()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisableLCDUpdates();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisableLCDUpdates(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisableLCDUpdates(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetSoloSelectsChannel()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetSoloSelectsChannel();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetSoloSelectsChannel(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetSoloSelectsChannel(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetFaderTouchSelectsChannel()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetFaderTouchSelectsChannel();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetFaderTouchSelectsChannel(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetFaderTouchSelectsChannel(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetSelectHighlightsTrack()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetSelectHighlightsTrack();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetSelectHighlightsTrack(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetSelectHighlightsTrack(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetSelectDoubleClick()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetSelectDoubleClick();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetSelectDoubleClick(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetSelectDoubleClick(bVal);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::HaveLevelMeters()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.HaveMeters();
}

/////////////////////////////////////////////////////////////////////////////

LevelMeters CMackieControlMaster::GetDisplayLevelMeters()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisplayLevelMeters();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisplayLevelMeters(LevelMeters eDisplayLevelMeters)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisplayLevelMeters(eDisplayLevelMeters);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlMaster::GetDisableHandshake()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetDisableHandshake();
}

void CMackieControlMaster::SetExcludeFiltersFromPlugins(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetExcludeFiltersFromPlugins(bVal);
}

bool CMackieControlMaster::GetExcludeFiltersFromPlugins()
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	return m_cState.GetExcludeFiletersFromPlugins();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlMaster::SetDisableHandshake(bool bVal)
{
	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.SetDisableHandshake(bVal);
}

/////////////////////////////////////////////////////////////////////////////
