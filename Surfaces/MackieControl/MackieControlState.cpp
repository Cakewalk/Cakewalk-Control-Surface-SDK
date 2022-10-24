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
#include <string>

/////////////////////////////////////////////////////////////////////////////

// m_dwMasterUpdateCount is incremented whenever a change is made that
// will require the Master Fader to be rebound to a new set of parameters
//
// m_dwXTUpdateCount is incremented whenever a change is made that
// will require the XT (or XT section) faders, switches or VPots to be
// rebound to a new set of parameters

/////////////////////////////////////////////////////////////////////////////

#define streq(a, b)		(strcmp((a), (b)) == 0)

/////////////////////////////////////////////////////////////////////////////

CMackieControlState::CMackieControlState()
{
	TRACE("DBWin32_ClearBuffer");

//	TRACE("CMackieControlState::CMackieControlState()\n");

	::InitializeCriticalSection(&m_cs);

	m_dwToolbarUpdateCount = 0;
	m_dwMasterUpdateCount = 0;
	m_dwXTUpdateCount = 0;
	m_dwC4UpdateCount = 0;

	m_bConfigureLayoutMode = false;
	
	::memset(m_dwStripNumOffset, 0, sizeof(m_dwStripNumOffset));
	::memset(m_dwPluginNumOffset, 0, sizeof(m_dwPluginNumOffset));
	::memset(m_dwParamNumOffset, 0, sizeof(m_dwParamNumOffset));
	::memset(m_dwMasterFaderOffset, 0, sizeof(m_dwMasterFaderOffset));
	m_eMasterFaderType = MIX_STRIP_MASTER;

	// Variations
	for (int n = 0; n < NUM_MIXER_STRIP_TYPES; n++)
	{
		// In paramter mode the faders are in position 0, so
		// set the inital offset for multi-channel mode to 1 (pan)
		m_dwParamNumOffset[n][MCS_ASSIGNMENT_PARAMETER][MCS_ASSIGNMENT_MULTI_CHANNEL] = 1;

		// Similarly, the first send parameter is send enable,
		// so change the starting point to 1 (send level)
		m_dwParamNumOffset[n][MCS_ASSIGNMENT_SEND][MCS_ASSIGNMENT_MULTI_CHANNEL] = 1;
	}

	m_dwSelectedStripNum = 0;

	m_eMixerStrip = MIX_STRIP_TRACK;
	m_eAssignment = MCS_ASSIGNMENT_PARAMETER;
	m_eAssignmentMode = MCS_ASSIGNMENT_MULTI_CHANNEL; 

	m_ePreSynthRackAssignment = MCS_ASSIGNMENT_PARAMETER;
	m_ePreSynthRackAssignmentMode = MCS_ASSIGNMENT_MULTI_CHANNEL;
	m_bPreSynthRackEditMode = false;

	m_eFlipMode = MCS_FLIP_NORMAL;

	m_dwModifiers = 0;

	m_eNavigationMode = MCS_NAVIGATION_NORMAL;

	m_bDisplayFlip = false;
	m_bDisplayValues = false;
	m_bDisplayTrackNumbers = false;
	m_eDisplayLevelMeters = METERS_OFF;
	m_bDisplaySMPTE = false;

	m_bDisableFaders = false;
	m_bDisableRelayClick = false;
	m_bDisableLCDUpdates = false;
	m_bJogParamMode = false;
	m_bSoloSelectsChannel = false;
	m_bSelectHighlightsTrack = false;
	m_bSelectDoubleClick = false;
	m_bFaderTouchSelectsChannel = false;
	m_bDisableHandshake = true;
	m_bScrubBankSelectsTrackBus = false;

	m_eJogResolution = JOG_MEASURES;
	m_eTransportResolution = JOG_MEASURES;
	m_dLastDisplayTime = -1.0;

	m_szTempDisplayText[0] = 0;

	// Start from 1, so that an Id of 0 means unknown
	m_dwUniqueIdCount = 1;

	m_dwFirstFaderNumber = 0;
	m_dwLastFaderNumber = 7;

	m_bHaveCheckedCapabilities = false;
	m_bHaveBuses = true;
	m_bHaveMeters = false;
	m_bHaveAudioMeters = false;
	m_bHaveMidiMeters = false;
	m_bHaveInputEcho = true;
	m_bHaveStripAny = false;
	m_bHaveStripFilter = true;
	m_iMetersUpdatePeriod = 1;
	m_eBusType = MIX_STRIP_BUS;
	m_eMasterType = MIX_STRIP_MASTER;
	m_bUseHUIProtocol = false;
	m_bHUIKeyPadControlsKeyPad = false;
	m_bUseUniversalProtocol = false;
	m_bUseCubaseProtocol = false;
}

CMackieControlState::~CMackieControlState()
{
//	TRACE("CMackieControlState::~CMackieControlState()\n");

	::DeleteCriticalSection(&m_cs);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetConfigureLayoutMode(bool bConfigureLayoutMode)
{
	m_bConfigureLayoutMode = bConfigureLayoutMode;

	if (!bConfigureLayoutMode)
	{
		CalculateFirstAndLastFader();

		m_dwMasterUpdateCount++;
		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetStripNumOffset(int iStripNumOffset)
{
	if (iStripNumOffset < 0)
		iStripNumOffset = 0;

	if (m_dwStripNumOffset[m_eMixerStrip] != iStripNumOffset)
	{
		m_dwStripNumOffset[m_eMixerStrip] = iStripNumOffset;

		m_dwToolbarUpdateCount++;
		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetPluginNumOffset(int iPluginNumOffset)
{
	if (iPluginNumOffset < 0)
		iPluginNumOffset = 0;

	if (m_dwPluginNumOffset[m_eMixerStrip][m_eAssignment] != iPluginNumOffset)
	{
		if (MCS_ASSIGNMENT_EQ == m_eAssignment || MCS_ASSIGNMENT_EQ_FREQ_GAIN == m_eAssignment)
		{
			// These two shouldn't have separate offsets
			m_dwPluginNumOffset[m_eMixerStrip][MCS_ASSIGNMENT_EQ] = iPluginNumOffset;
			m_dwPluginNumOffset[m_eMixerStrip][MCS_ASSIGNMENT_EQ_FREQ_GAIN] = iPluginNumOffset;
		}
		else
		{
			m_dwPluginNumOffset[m_eMixerStrip][m_eAssignment] = iPluginNumOffset;
		}

		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetParamNumOffset(int iParamNumOffset)
{
	if (iParamNumOffset < 0)
		iParamNumOffset = 0;

	if (m_dwParamNumOffset[m_eMixerStrip][m_eAssignment][m_eAssignmentMode] != iParamNumOffset)
	{
		m_dwParamNumOffset[m_eMixerStrip][m_eAssignment][m_eAssignmentMode] = iParamNumOffset;

		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetMasterFaderOffset(SONAR_MIXER_STRIP eMixerStrip, DWORD dwOffset)
{
	if (m_dwMasterFaderOffset[eMixerStrip] != dwOffset)
	{
		m_dwMasterFaderOffset[eMixerStrip] = dwOffset;

		m_dwToolbarUpdateCount++;
		m_dwMasterUpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetMasterFaderType(SONAR_MIXER_STRIP eMixerStrip)
{
	if (m_eMasterFaderType != eMixerStrip)
	{
		m_eMasterFaderType = eMixerStrip;

		m_dwToolbarUpdateCount++;
		m_dwMasterUpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetSelectedStripNum(DWORD dwSelectedStripNum, ISonarMixer *pMixer)
{
	if (m_dwSelectedStripNum != dwSelectedStripNum)
	{
		m_dwSelectedStripNum = dwSelectedStripNum;

		if (MCS_ASSIGNMENT_CHANNEL_STRIP == m_eAssignmentMode)
			m_dwXTUpdateCount++;

		m_dwToolbarUpdateCount++;
		m_dwC4UpdateCount++;

		// MIX_PARAM_SELECTED only works for tracks
		if (pMixer && m_eMixerStrip == MIX_STRIP_TRACK)
		{
			pMixer->SetMixParam(MIX_STRIP_TRACK, 0, MIX_PARAM_SELECTED, 0,
								(float)dwSelectedStripNum, MIX_TOUCH_NORMAL);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetMixerStrip(SONAR_MIXER_STRIP eMixerStrip)
{
	if (m_eMixerStrip != eMixerStrip)
	{
		m_eMixerStrip = eMixerStrip;

		m_dwToolbarUpdateCount++;
		m_dwMasterUpdateCount++;
		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetAssignment(Assignment eAssignment)
{
	if (m_eAssignment != eAssignment)
	{
		m_eAssignment = eAssignment;

		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SavePreSynthRackAssignments()
{
	m_ePreSynthRackAssignmentMode = m_eAssignmentMode;
	m_ePreSynthRackAssignment = m_eAssignment;

	DWORD dwMods = GetModifiers( M1_TO_M4_MASK | MCS_MODIFIER_EDIT | MCS_MODIFIER_NUMERIC );
	m_bPreSynthRackEditMode = (dwMods & MCS_MODIFIER_EDIT);
}

/////////////////////////////////////////////////////////////////////////////
void CMackieControlState::SetAssignmentMode(AssignmentMode eAssignmentMode)
{
	if (m_eAssignmentMode != eAssignmentMode)
	{
		m_eAssignmentMode = eAssignmentMode;

		m_dwXTUpdateCount++;
		m_dwC4UpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::CycleFlipMode()
{
	switch (m_eFlipMode)
	{
		case MCS_FLIP_NORMAL:
			SetFlipMode(MCS_FLIP_DUPLICATE);
			break;

		case MCS_FLIP_DUPLICATE:
			// Don't go to MCS_FLIP_FLIP mode when using MIX_STRIP_RACK
			if ( m_eMixerStrip == MIX_STRIP_RACK )
				SetFlipMode( MCS_FLIP_NORMAL );
			else
				SetFlipMode(MCS_FLIP_FLIP);
			break;

		case MCS_FLIP_FLIP:
			SetFlipMode(MCS_FLIP_NORMAL);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetFlipMode(FlipMode eFlipMode)
{
	if (m_eFlipMode != eFlipMode)
	{
		m_eFlipMode = eFlipMode;

		m_dwXTUpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetModifiers(DWORD dwModifiers)
{
//	TRACE("CMackieControlState::SetModifiers(): 0x%08X\n", dwModifiers);

	if ((m_dwModifiers & M1_TO_M4_MASK) != (dwModifiers & M1_TO_M4_MASK))
	{
		m_dwMasterUpdateCount++;

		switch (m_eAssignment)
		{
			case MCS_ASSIGNMENT_PLUGIN:
			case MCS_ASSIGNMENT_EQ:
			case MCS_ASSIGNMENT_EQ_FREQ_GAIN:
			case MCS_ASSIGNMENT_DYNAMICS:
				m_dwXTUpdateCount++;
				m_dwC4UpdateCount++;
				break;

			default:
				break;
		}
	}

	m_dwModifiers = dwModifiers;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::EnableModifier(DWORD dwModifier)
{
	DWORD dwTmp = m_dwModifiers | dwModifier;

	// MCS_MODIFIER_EDIT and MCS_MODIFIER_NUMERIC are mutually exclusive
	if (MCS_MODIFIER_EDIT == dwModifier)
		dwTmp &= ~MCS_MODIFIER_NUMERIC;
	else if (MCS_MODIFIER_NUMERIC == dwModifier)
		dwTmp &= ~MCS_MODIFIER_EDIT;

	SetModifiers(dwTmp);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::DisableModifier(DWORD dwModifier)
{
	DWORD dwTmp = m_dwModifiers & ~dwModifier;

	SetModifiers(dwTmp);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetNavigationMode(NavigationMode eNavigationMode)
{
	m_eNavigationMode = eNavigationMode;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisplayFlip(bool bDisplayFlip)
{
	m_bDisplayFlip = bDisplayFlip;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisplayValues(bool bDisplayValues)
{
	m_bDisplayValues = bDisplayValues;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisplayTrackNumbers(bool bDisplayTrackNumbers)
{
	m_bDisplayTrackNumbers = bDisplayTrackNumbers;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisplayLevelMeters(LevelMeters eDisplayLevelMeters)
{
	m_eDisplayLevelMeters = eDisplayLevelMeters;
	m_dwXTUpdateCount++;
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisplaySMPTE(bool bDisplaySMPTE)
{
	m_bDisplaySMPTE = bDisplaySMPTE;
	m_dLastDisplayTime = -1.0;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisableFaders(bool bDisableFaders)
{
	m_bDisableFaders = bDisableFaders;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisableRelayClick(bool bDisableRelayClick)
{
	m_bDisableRelayClick = bDisableRelayClick;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisableLCDUpdates(bool bDisableLCDUpdates)
{
	m_bDisableLCDUpdates = bDisableLCDUpdates;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetJogParamMode(bool bJogParamMode)
{
	m_bJogParamMode = bJogParamMode;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetSoloSelectsChannel(bool bSoloSelectsChannel)
{
	m_bSoloSelectsChannel = bSoloSelectsChannel;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetSelectHighlightsTrack(bool bSelectHighlightsTrack)
{
	m_bSelectHighlightsTrack = bSelectHighlightsTrack;
	if (!bSelectHighlightsTrack)
		m_bSelectDoubleClick = false;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetSelectDoubleClick(bool bSelectDoubleClick)
{
	m_bSelectDoubleClick = bSelectDoubleClick;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetScrubBankSelectsTrackBus(bool bScrubBankSelectsTrackBus)
{
	m_bScrubBankSelectsTrackBus = bScrubBankSelectsTrackBus;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetFaderTouchSelectsChannel(bool bFaderTouchSelectsChannel)
{
	m_bFaderTouchSelectsChannel = bFaderTouchSelectsChannel;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetUserFunctionKey(BYTE bN, DWORD dwCmdId)
{
	if (bN < NUM_USER_FUNCTION_KEYS)
		m_cUserFunctionKeys[bN].SetCommand(dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetUserFootSwitch(BYTE bN, DWORD dwCmdId)
{
	if (bN < NUM_USER_FOOT_SWITCHES)
		m_cUserFootSwitch[bN].SetCommand(dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetJogResolution(JogResolution eJogResolution)
{
	if (m_eJogResolution != eJogResolution)
	{
		m_eJogResolution = eJogResolution;
	
		m_dwToolbarUpdateCount++;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetTransportResolution(JogResolution eTransportResolution)
{
	m_eTransportResolution = eTransportResolution;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetLastDisplayTime(double dLastDisplayTime)
{
	m_dLastDisplayTime = dLastDisplayTime;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetTempDisplayText(const char *szText)
{
	::strlcpy(m_szTempDisplayText, szText, sizeof(m_szTempDisplayText));
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetDisableHandshake(bool bDisableHandshake)
{
	m_bDisableHandshake = bDisableHandshake;
}

void CMackieControlState::SetExcludeFiltersFromPlugins(bool bExcludeFiltersFromPlugins)
{
	m_bExcludeFiltersFromPlugins = bExcludeFiltersFromPlugins;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::LoadIniSection( const char *szPath, const char *szSection, std::map<std::string, std::string> &paramMap )
{	
	const int bufferSize = 32767;
	char buffer[bufferSize] = "";

	int charsRead = 0;

	charsRead = GetPrivateProfileSectionA( szSection,
		buffer,
		bufferSize,
		szPath );
	// if there isn't enough space, returns bufferSize - 2

	ASSERT( bufferSize );
	// if we got some data...
	if ( (0 < charsRead) && ((bufferSize - 2) > charsRead) )
	{
		// walk the buffer extracting values

		// start at the beginning (const to remind us not to
		// change the contents of the buffer)
		const char* pSubstr = buffer;

		// while we have non-empty substrings...
		while ( '\0' != *pSubstr )
		{
			// length of key-value pair substring
			size_t substrLen = strlen( pSubstr );

			// split substring on '=' char
			const char* pos = strchr( pSubstr, '=' );
			if ( NULL != pos )
			{
				char name[256] = "";
				char value[256] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - (pos - pSubstr) );

				// Remove any comments
				char *s = ::strchr( value, ';' );
				if ( s )
					*s = 0;

				auto rtrim = [&] ( std::string s )
				{
					const std::string WHITESPACE = " \n\r\t\f\v";
					size_t start = s.find_first_not_of( WHITESPACE );
					return ( start == std::string::npos ) ? "" : s.substr( start );
				};

				std::string strName = rtrim( name );
				std::string strValue = rtrim( value );

				if ( !strName.empty() && !strValue.empty() )
					paramMap.emplace( strName.c_str(), strValue.c_str() );
			}

			// jump over the current substring plus its null
			pSubstr += (substrLen + 1);
		}
	}
}

void CMackieControlState::LoadPluginMappings()
{
//	TRACE("CMackieControlState::LoadPluginMappings()\n");

	// Figure out where the .ini file is
	char szFile[_MAX_PATH] = {0};
	if (::GetModuleFileNameA(theApp.m_hInstance, szFile, _MAX_PATH) < 1)
		return;

	char *s = ::strrchr(szFile, '\\');
	if (!s)
		return;

	*++s = 0;
	::strlcat(szFile, "MackieControl.ini", sizeof(szFile));

#if 0 // def _DEBUG
	::strlcpy(szFile, "C:\\Home\\Chris\\Src\\MackieControl\\MackieControl.ini", sizeof(szFile));
#endif

	// Load some misc settings first
	char szPeriod[32];
	::GetPrivateProfileStringA("Meters", "UpdatePeriod", "1", szPeriod, sizeof(szPeriod), szFile);
	m_iMetersUpdatePeriod = atoi(szPeriod);
	if (m_iMetersUpdatePeriod <= 0)
		m_iMetersUpdatePeriod = 1;

	TRACE("Meters Update Period = %d\n", m_iMetersUpdatePeriod);

	// Clear out any existing mappings
	m_mapPluginProperties.clear();

	std::map<std::string, std::string> pluginEntries;
	LoadIniSection( szFile, "Plugins", pluginEntries );

	for ( auto entryPair : pluginEntries )
	{
		std::map<std::string, std::string> pluginParams;
		char szPluginName[256];
		strcpy( szPluginName, entryPair.second.c_str() );

		LoadIniSection( szFile, entryPair.second.c_str(), pluginParams );

		int iPluginType = std::stoi( pluginParams["PluginType"] );
		if ( iPluginType < 0 || iPluginType >= 32 )
			continue;

		int iNumVPots = std::stoi( pluginParams["NumVPots"] );
		if ( iNumVPots < 0 || iNumVPots >= 1024 )
			continue;

		CPluginProperties cPluginProps;

		cPluginProps.m_dwPluginType = 1 << iPluginType;		// Convert to bit mask
		cPluginProps.m_dwNumVPots = iNumVPots;

		for ( int j = 0; j < iNumVPots; j++ )
		{
			LoadParameterProperties( pluginParams, "VPot", j, &cPluginProps.m_mapParamPropsVPot );
			LoadParameterProperties( pluginParams, "M1VPot", j, &cPluginProps.m_mapParamPropsM1VPot );
			LoadParameterProperties( pluginParams, "M2VPot", j, &cPluginProps.m_mapParamPropsM2VPot );
			LoadParameterProperties( pluginParams, "M3VPot", j, &cPluginProps.m_mapParamPropsM3VPot );
			LoadParameterProperties( pluginParams, "M4VPot", j, &cPluginProps.m_mapParamPropsM4VPot );

			// Always load the labels AFTER the main properties, so the properties that were loaded above 
			// are added to and not replaced
			LoadParameterProperties( pluginParams, "VPotLabel", j, &cPluginProps.m_mapParamPropsVPot, true );
			LoadParameterProperties( pluginParams, "M1VPotLabel", j, &cPluginProps.m_mapParamPropsM1VPot, true );
			LoadParameterProperties( pluginParams, "M2VPotLabel", j, &cPluginProps.m_mapParamPropsM2VPot, true );
			LoadParameterProperties( pluginParams, "M3VPotLabel", j, &cPluginProps.m_mapParamPropsM3VPot, true );
			LoadParameterProperties( pluginParams, "M4VPotLabel", j, &cPluginProps.m_mapParamPropsM4VPot, true );			
		}

		// EQ?
		if ( iPluginType == 1 )
		{
			int iNumFreqBands = std::stoi( pluginParams["NumFreqBands"] );
			if ( iNumFreqBands < 0 || iNumFreqBands >= 128 )
				continue;

			// Load Gain/Freq/Q mode mappings
			for ( int k = 0; k < iNumFreqBands; k++ )
			{
				LoadParameterProperties( pluginParams, "Gain", k, &cPluginProps.m_mapParamPropsGain );
				LoadParameterProperties( pluginParams, "CourseFreq", k, &cPluginProps.m_mapParamPropsCourseFreq );
				LoadParameterProperties( pluginParams, "FineFreq", k, &cPluginProps.m_mapParamPropsFineFreq );
				LoadParameterProperties( pluginParams, "Q", k, &cPluginProps.m_mapParamPropsQ );
				LoadParameterProperties( pluginParams, "BandEnable", k, &cPluginProps.m_mapParamPropsBandEnable );
			}
		}

		m_mapPluginProperties[entryPair.second.c_str()] = cPluginProps;
	}


#ifdef _DEBUG
//	DumpPluginProperties();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::LoadParameterProperties( std::map<std::string, std::string> &pluginParams,
	const char *szParamName, int iIndex, mapParameterProperties *mapParamProps, bool isLabelLine )
{
	char szTemp[32];
	snprintf( szTemp, sizeof( szTemp ), "%s%d", szParamName, iIndex );

	CParameterProperties cParamProps;

	bool success = false;

	char szPropsLine[256];
	if ( pluginParams.find( szTemp ) != pluginParams.end() )
	{
		strcpy( szPropsLine, pluginParams[szTemp].c_str() );

		if ( isLabelLine )
		{
			// Change the existing properties, don't add a new one.
			// Note that this relies on having first read the main properties.
			if ( mapParamProps->count( iIndex ) )
			{
				cParamProps = (*mapParamProps)[iIndex];
				success = ParseParameterPropertiesLabelLine( szPropsLine, &cParamProps );
			}
		}
		else
			success = ParseParameterPropertiesLine( szPropsLine, &cParamProps );

		if ( success )
			(*mapParamProps)[iIndex] = cParamProps;

		ASSERT( success );
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlState::ParseParameterPropertiesLine(char *szLine, CParameterProperties *pPluginProps)
{
	if (!szLine)
		return false;

	// Remove any comments
	char *s = ::strchr(szLine, ';');
	if (s)
		*s = 0;

	// Remove trailing white space
	s = szLine + ::strlen(szLine) - 1;
	while (s >= szLine && (*s == ' ' || *s == '\t'))
		*s-- = 0;

	// Anything left?
	if (!*szLine)
		return false;

	if (::strncmp(szLine, "enable", 6) == 0)
		pPluginProps->m_dwParamNum = 0x80000000;
	else
		pPluginProps->m_dwParamNum = ::atoi(szLine);

	s = ::strchr(szLine, ',');
	if (s)
	{
		++s;

		char szDataType[128];

		char *p = szDataType;
		while (*s && *s != ',' && (p - szDataType) < sizeof(szDataType))
			*p++ = *s++;
		*p = 0;

//		TRACE("Type: '%s'\n", szDataType);

		if (streq(szDataType, "level"))				pPluginProps->m_eDataType = DT_LEVEL;
		else if (streq(szDataType, "pan"))			pPluginProps->m_eDataType = DT_PAN;
		else if (streq(szDataType, "freq"))			pPluginProps->m_eDataType = DT_PAN;
		else if (streq(szDataType, "bool"))			pPluginProps->m_eDataType = DT_BOOL;
		else if (streq(szDataType, "switch"))		pPluginProps->m_eDataType = DT_BOOL;
		else if (streq(szDataType, "boost/cut"))	pPluginProps->m_eDataType = DT_BOOST_CUT;
		else if (streq(szDataType, "revspread" ) )	pPluginProps->m_eDataType = DT_REVSPREAD;
		else if (streq(szDataType, "spread"))		pPluginProps->m_eDataType = DT_SPREAD;
		else
		{
			TRACE("CMackieControlState::ParseParameterPropertiesLine(): Unknown DataType\n");
			return false;
		}

		if (pPluginProps->m_eDataType == DT_BOOL)
			pPluginProps->m_fDefaultValue = NO_DEFAULT;

		if (*s)
		{
			s = ::strchr(s, ',');
			if (s)
			{
				float fDefaultValue = (float)::atof(++s);
				if (fDefaultValue < 0.0 || fDefaultValue > 1.0)
				{
					TRACE("CMackieControlState::ParseParameterPropertiesLine(): DefaultValue out of range\n");
					return false;
				}

				pPluginProps->m_fDefaultValue = fDefaultValue;

				s = ::strchr(s, ',');
				if (s)
				{
					float fStepSize = (float)::atof(++s);
					if (fStepSize < 0.0 || fStepSize > 1.0)
					{
						TRACE("CMackieControlState::ParseParameterPropertiesLine(): StepSize out of range\n");
						return false;
					}

					pPluginProps->m_fStepSize = fStepSize;
				}
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlState::ParseParameterPropertiesLabelLine( char *szLine, CParameterProperties *pPluginProps )
{
	if ( !szLine )
		return false;

	// Remove any comments
	char *s = ::strchr( szLine, ';' );
	if ( s )
		*s = 0;

	// Remove trailing white space
	s = szLine + ::strlen( szLine ) - 1;
	while ( s >= szLine && (*s == ' ' || *s == '\t') )
		*s-- = 0;

	// Anything left?
	if ( !*szLine )
		return false;


	if ( ::strncmp( szLine, "enable", 6 ) == 0 )
		pPluginProps->m_dwParamNum = 0x80000000;
	else
		pPluginProps->m_dwParamNum = ::atoi( szLine );

	s = ::strchr( szLine, ',' );
	if ( s )
	{
		++s;

		char szTempLabel[128];

		char *p = szTempLabel;
		while ( *s && *s != ',' && (p - szTempLabel) < sizeof( szTempLabel ) )
			*p++ = *s++;
		*p = 0;
		int labelLength = int( ::strlen( szTempLabel ) );

		if ( labelLength > MAX_LABEL_LEN || labelLength < 1 )
			return false;

		::strncpy( &pPluginProps->m_szAltLabel[0], szTempLabel, labelLength );

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::DumpPluginProperties()
{
#ifdef _DEBUG
	TRACE("Plugins...\n");

	mapPluginProperties::iterator i;

	for (i = m_mapPluginProperties.begin(); i != m_mapPluginProperties.end(); i++)
	{
		TRACE("Name: '%s'\n", (LPCTSTR)i->first);
		TRACE("PluginType: 0x%08X\n", i->second.m_dwPluginType);
		TRACE("NumVPots: %d\n", i->second.m_dwNumVPots);

		mapParameterProperties::iterator j;

		mapParameterProperties *pp = &(i->second.m_mapParamPropsVPot);

		for (j = pp->begin(); j != pp->end(); j++)
		{
			TRACE("%d => %d, %d, %f, %f\n", j->first,
											j->second.m_dwParamNum,
											j->second.m_eDataType,
											j->second.m_fDefaultValue,
											j->second.m_fStepSize);
		}
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////

// Called in the CMackieControlBase constructor to add it to the list
//
void CMackieControlState::AddUnit(CMackieControlBase *pUnit)
{
	RemoveUnit(pUnit);	// Just in case

	if (m_ListOfUnits.size() == 0)
		m_dwNextAutoStripNumOffset = 0;

	if (pUnit->GetSurfaceType() >= SURFACE_TYPE_XT)
	{
		pUnit->SetUnitStripNumOffset(m_dwNextAutoStripNumOffset);
		m_dwNextAutoStripNumOffset += 8;
	}

	m_ListOfUnits.push_back(pUnit);

	CalculateFirstAndLastFader();
}

/////////////////////////////////////////////////////////////////////////////

// Called in the CMackieControlBase destructor to from it from the list
//
void CMackieControlState::RemoveUnit(CMackieControlBase *pUnit)
{
	listMackieControlUnits::iterator i;

	for (i = m_ListOfUnits.begin(); i != m_ListOfUnits.end(); ++i)
	{
		if (*i == pUnit)
		{
			m_ListOfUnits.erase(i);
			break;
		}
	}

	CalculateFirstAndLastFader();
}

/////////////////////////////////////////////////////////////////////////////

// Called once CMackieControlBase has retreived the units serial number
//
void CMackieControlState::RestoreUnitOffset(CMackieControlBase *pUnit)
{
//	TRACE("CMackieControlState::RestoreUnitOffset()\n");

	vectorMackieControlInformation::iterator j;

	for (j = m_SetupInformation.begin(); j != m_SetupInformation.end(); ++j)
	{
		if (pUnit->GetUniqueId() == j->m_dwUniqueId)
		{
//			TRACE("Setting offset to %d\n", j->m_dwOffset);

			pUnit->SetUnitStripNumOffset(j->m_dwOffset);
			break;
		}
	}

	CalculateFirstAndLastFader();

	m_dwXTUpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

// Called by CMackieControlMaster::Save() so that it can save the state
// for *all* of the units
//
void CMackieControlState::GetUnitOffsets(vectorMackieControlInformation *pInfo)
{
//	TRACE("CMackieControlState::GetUnitOffsets()\n");

	pInfo->clear();

	listMackieControlUnits::iterator i;

	for (i = m_ListOfUnits.begin(); i != m_ListOfUnits.end(); ++i)
	{
		if ((*i)->GetUniqueId() > 0)
		{
			CMackieControlInformation sInfo;

			// Copy over the data
			sInfo.m_dwUniqueId = (*i)->GetUniqueId();
			sInfo.m_dwOffset = (*i)->GetUnitStripNumOffset();

			// Add it to the vector
			pInfo->push_back(sInfo);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

// Called by CMackieControlMaster::Load() which is used to restore 
// the previous state
//
void CMackieControlState::SetUnitOffsets(vectorMackieControlInformation *pInfo)
{
//	TRACE("CMackieControlState::SetUnitOffsets()\n");

	listMackieControlUnits::iterator i;

	for (i = m_ListOfUnits.begin(); i != m_ListOfUnits.end(); ++i)
	{
		vectorMackieControlInformation::iterator j;

		for (j = pInfo->begin(); j != pInfo->end(); ++j)
		{
			if ((*i)->GetUniqueId() > 0 && (*i)->GetUniqueId() == j->m_dwUniqueId)
			{
//				TRACE("Setting offset to %d\n", j->m_dwOffset);

				(*i)->SetUnitStripNumOffset(j->m_dwOffset);
				break;
			}
		}
	}

	m_SetupInformation = *pInfo;

	CalculateFirstAndLastFader();

	m_dwXTUpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::CalculateFirstAndLastFader()
{
//	TRACE("CMackieControlState::CalculateFirstAndLastFader()\n");

	m_dwFirstFaderNumber = 0;
	m_dwLastFaderNumber = 0;

	DWORD dwLowest = 0xFFFF;

	listMackieControlUnits::iterator i;

	for (i = m_ListOfUnits.begin(); i != m_ListOfUnits.end(); ++i)
	{
		DWORD dwOffset = (*i)->GetUnitStripNumOffset();

		if (dwOffset < dwLowest)
			dwLowest = dwOffset;

		if (dwOffset > m_dwLastFaderNumber)
			m_dwLastFaderNumber = dwOffset;
	}

	m_dwFirstFaderNumber = dwLowest;
	m_dwLastFaderNumber += 7;

	m_dwToolbarUpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::DetermineCapabilities(ISonarMixer *pMixer, ISonarIdentity *pSonarIdentity)
{
//	TRACE("CMackieControlState::DetermineCapabilities()\n");

	if (m_bHaveCheckedCapabilities || pMixer == NULL)
		return;

	if (pSonarIdentity)
	{
		TRACE("Have ISonarIdentity\n");

		m_bHaveBuses = HasCapability(pSonarIdentity, CAP_FLEX_BUSES) > 0;
		m_bHaveAudioMeters = HasCapability(pSonarIdentity, CAP_AUDIO_METERS) > 0;
		m_bHaveMidiMeters = HasCapability(pSonarIdentity, CAP_MIDI_METERS) > 0;
		m_bHaveInputEcho = HasCapability(pSonarIdentity, CAP_INPUT_ECHO) > 0;
		m_bHaveStripAny = HasCapability(pSonarIdentity, CAP_STRIP_ANY) > 0;
		m_bHaveStripFilter = HasCapability(pSonarIdentity, CAP_STRIP_FILTER) > 0;

		m_bHaveMeters = (m_bHaveAudioMeters || m_bHaveMidiMeters);
	}
	else
	{
		DWORD dwCount;

		HRESULT hr = pMixer->GetMixStripCount(MIX_STRIP_TRACK, &dwCount);

		// If that failed, SONAR isn't ready yet
		if (FAILED(hr))
			return;

		// Now check to see if we can count the number of bus strips
		hr = pMixer->GetMixStripCount(MIX_STRIP_BUS, &dwCount);

		m_bHaveBuses = (FAILED(hr)) ? false : true;
		m_bHaveMeters = false;
		m_bHaveAudioMeters = false;
		m_bHaveMidiMeters = false;
		m_bHaveInputEcho = m_bHaveBuses;
		m_bHaveStripAny = false;
		m_bHaveStripFilter = m_bHaveBuses;
	}

	TRACE("CMackieControlState::DetermineCapabilities() has a result (%d, %d)!!!\n", m_bHaveBuses, m_bHaveMeters);

	if (m_bHaveBuses)
	{
		m_eBusType = MIX_STRIP_BUS;
		m_eMasterType = MIX_STRIP_MASTER;
	}
	else
	{
		m_eBusType = MIX_STRIP_AUX;
		m_eMasterType = MIX_STRIP_MAIN;
	}

	// Don't check this again
	m_bHaveCheckedCapabilities = true;

	// Force a reconfigure
	m_dwToolbarUpdateCount++;
	m_dwMasterUpdateCount++;
	m_dwXTUpdateCount++;
	m_dwC4UpdateCount++;
}

/////////////////////////////////////////////////////////////////////////////

int CMackieControlState::HasCapability(ISonarIdentity *pSonarIdentity, HOST_CAPABILITY cap)
{
	if (!pSonarIdentity)
		return -2;

	HRESULT hr = pSonarIdentity->HasCapability(cap);

	int ret;

	if (FAILED(hr))
		ret = -1;
	else
		ret = (hr == S_OK) ? 1 : 0;

	TRACE("HasCapability() %d = %d, %d\n", cap, hr, ret);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetUseHUIProtocol(bool bUseHUIProtocol)
{
	m_bUseHUIProtocol = bUseHUIProtocol;
	if ( bUseHUIProtocol )
	{
		m_bUseUniversalProtocol = false;
		m_bUseCubaseProtocol = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetUseUniversalProtocol( bool bUseUniversalProtocol )
{
	m_bUseUniversalProtocol = bUseUniversalProtocol;
	if ( bUseUniversalProtocol )
	{
		m_bUseHUIProtocol = false;
		m_bUseCubaseProtocol = false;
		m_bHUIKeyPadControlsKeyPad = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetUseCubaseProtocol( bool bUseCubaseProtocol )
{
	m_bUseCubaseProtocol = bUseCubaseProtocol;
	if ( bUseCubaseProtocol )
	{
		m_bUseHUIProtocol = false;
		m_bUseUniversalProtocol = false;
		m_bHUIKeyPadControlsKeyPad = false;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlState::SetHUIKeyPadControlsKeyPad(bool bHUIKeyPadControlsKeyPad)
{
	m_bHUIKeyPadControlsKeyPad = bHUIKeyPadControlsKeyPad;
}

/////////////////////////////////////////////////////////////////////////////
