#ifndef MackieControlState_h
#define MackieControlState_h

/////////////////////////////////////////////////////////////////////////////

// NOTE: the actual usage of these is as BYTE not DWORD even though some of the functions and member vars are DWORD.
// In otherwords, don't add new flags > 0xFF unless you change the SetLed api or consider the effects of your change.
#define MCS_MODIFIER_NONE		0x00
#define MCS_MODIFIER_M1			0x01
#define MCS_MODIFIER_M2			0x02
#define MCS_MODIFIER_M3			0x04
#define MCS_MODIFIER_M4			0x08
#define MCS_MODIFIER_EDIT		0x10   // FLIP on Original MCU
#define MCS_MODIFIER_NUMERIC	0x20   // GLOBAL VIEW on Original MCU
#define MCS_MODIFIER_ZOOM		0x40

#define M1_TO_M4_MASK			0x0F

// Number of mixer strip types (MIX_STRIP_*)
#define NUM_MIXER_STRIP_TYPES			6

// Number of user configurable function keys and foot switches
#define NUM_USER_FUNCTION_KEYS			8
#define NUM_USER_FOOT_SWITCHES			2

/////////////////////////////////////////////////////////////////////////////

enum Assignment
{
	MCS_ASSIGNMENT_PARAMETER,
	MCS_ASSIGNMENT_PAN,
	MCS_ASSIGNMENT_SEND,
	MCS_ASSIGNMENT_PLUGIN,
	MCS_ASSIGNMENT_EQ,
	MCS_ASSIGNMENT_EQ_FREQ_GAIN,
	MCS_ASSIGNMENT_DYNAMICS,
	NUM_ASSIGNMENT_TYPES
};

enum AssignmentMode
{
	MCS_ASSIGNMENT_MUTLI_CHANNEL,
	MCS_ASSIGNMENT_CHANNEL_STRIP,
	NUM_ASSIGNMENT_MODES
};

enum NavigationMode
{
	MCS_NAVIGATION_NORMAL,
	MCS_NAVIGATION_MARKER,
	MCS_NAVIGATION_LOOP,
	MCS_NAVIGATION_SELECT,
	MCS_NAVIGATION_PUNCH
};

enum FlipMode
{
	MCS_FLIP_NORMAL,
	MCS_FLIP_DUPLICATE,
	MCS_FLIP_FLIP
};

enum JogResolution
{
	JOG_TICKS,
	JOG_BEATS,
	JOG_MEASURES,
	JOG_HOURS,
	JOG_MINUTES,
	JOG_SECONDS,
	JOG_FRAMES
};

enum LevelMeters
{
	METERS_OFF,
	METERS_LEDS,
	METERS_BOTH
};

/////////////////////////////////////////////////////////////////////////////

// Disable identifier name too long for debugging warning
#pragma warning(disable: 4786)

#include <list>
#include <vector>
#include <map>

/////////////////////////////////////////////////////////////////////////////

#define PT_UNKNOWN		0x0001
#define PT_EQ			0x0002
#define PT_DYNAMICS		0x0004
#define PT_ALL			0xFFFF

struct CParameterProperties
{
	CParameterProperties()
	{
		m_dwParamNum = 0;
		m_eDataType = DT_LEVEL;
		m_fDefaultValue = 0.5;
		m_fStepSize = 0.005f;
	};

	DWORD m_dwParamNum;
	DataType m_eDataType;
	float m_fDefaultValue;
	float m_fStepSize;
};

typedef std::map<DWORD, CParameterProperties> mapParameterProperties;

struct CPluginProperties
{
	CPluginProperties()
	{
		m_dwPluginType = 0;
		m_dwNumVPots = 0;
		m_dwNumFreqBands = 0;
	};

	DWORD m_dwPluginType;
	DWORD m_dwNumVPots;
	mapParameterProperties m_mapParamPropsVPot;
	mapParameterProperties m_mapParamPropsM1VPot;
	mapParameterProperties m_mapParamPropsM2VPot;
	mapParameterProperties m_mapParamPropsM3VPot;
	mapParameterProperties m_mapParamPropsM4VPot;

	DWORD m_dwNumFreqBands;
	mapParameterProperties m_mapParamPropsGain;
	mapParameterProperties m_mapParamPropsCourseFreq;
	mapParameterProperties m_mapParamPropsFineFreq;
	mapParameterProperties m_mapParamPropsQ;
	mapParameterProperties m_mapParamPropsBandEnable;
};

// std::string doesn't work here for some reason...
typedef std::map<CString, CPluginProperties> mapPluginProperties;

/////////////////////////////////////////////////////////////////////////////

// Forward ref
class CMackieControlBase;

// Lists of (pointers to) units
typedef std::list<CMackieControlBase *> listMackieControlUnits;

// Vector of unit information
typedef std::vector<CMackieControlInformation> vectorMackieControlInformation;

/////////////////////////////////////////////////////////////////////////////

class CMackieControlState
{
public:
	CMackieControlState();
	virtual ~CMackieControlState();

	CRITICAL_SECTION *GetCS()			{ return &m_cs; };

	void BumpToolbarUpdateCount()		{ m_dwToolbarUpdateCount++; };

	DWORD GetToolbarUpdateCount()		{ return m_dwToolbarUpdateCount; };
	DWORD GetMasterUpdateCount()		{ return m_dwMasterUpdateCount; };
	DWORD GetXTUpdateCount()			{ return m_dwXTUpdateCount; };
	DWORD GetC4UpdateCount()			{ return m_dwC4UpdateCount; };

	bool GetConfigureLayoutMode()		{ return m_bConfigureLayoutMode; };

	// Getters
	DWORD GetStripNumOffset()
	{
		return m_dwStripNumOffset[m_eMixerStrip];
	};
	DWORD GetPluginNumOffset()
	{
		return m_dwPluginNumOffset[m_eMixerStrip][m_eAssignment];
	};
	DWORD GetParamNumOffset()
	{
		return m_dwParamNumOffset[m_eMixerStrip][m_eAssignment][m_eAssignmentMode];
	};
	DWORD GetMasterFaderOffset(SONAR_MIXER_STRIP eMixerStrip)
	{
		return m_dwMasterFaderOffset[eMixerStrip];
	};
	SONAR_MIXER_STRIP GetMasterFaderType()
	{
		return m_eMasterFaderType;
	};
	DWORD GetSelectedStripNum()			{ return m_dwSelectedStripNum; };
	SONAR_MIXER_STRIP GetMixerStrip()	{ return m_eMixerStrip; };
	Assignment GetAssignment()			{ return m_eAssignment; };
	AssignmentMode GetAssignmentMode()	{ return m_eAssignmentMode; };
	FlipMode GetFlipMode()				{ return m_eFlipMode; };
	DWORD GetModifiers(DWORD dwMask)	{ return m_dwModifiers & dwMask; };
	NavigationMode GetNavigationMode()	{ return m_eNavigationMode; };
	bool GetDisplayFlip()				{ return m_bDisplayFlip; };
	bool GetDisplayValues()				{ return m_bDisplayValues; };
	bool GetDisplayTrackNumbers()		{ return m_bDisplayTrackNumbers; };
	LevelMeters GetDisplayLevelMeters()
	{
		if (!m_bHaveMeters)
			return METERS_OFF;

		return m_eDisplayLevelMeters;
	};
	int GetMetersUpdatePeriod()			{ return m_iMetersUpdatePeriod; };
	bool GetDisplaySMPTE()				{ return m_bDisplaySMPTE; };
	bool GetDisableFaders()				{ return m_bDisableFaders; };
	bool GetDisableRelayClick()			{ return m_bDisableRelayClick; };
	bool GetDisableLCDUpdates()			{ return m_bDisableLCDUpdates; };
	bool GetJogParamMode()				{ return m_bJogParamMode; };
	bool GetSoloSelectsChannel()		{ return m_bSoloSelectsChannel; };
	bool GetSelectHighlightsTrack()		{ return m_bSelectHighlightsTrack; };
	bool GetSelectDoubleClick()			{ return m_bSelectDoubleClick; }
	bool GetFaderTouchSelectsChannel()	{ return m_bFaderTouchSelectsChannel; };
	DWORD GetUserFunctionKey(BYTE bN)
	{
		return m_cUserFunctionKeys[bN].GetCommand();
	};
	DWORD GetUserFootSwitch(BYTE bN)
	{
		return m_cUserFootSwitch[bN].GetCommand();
	};
	JogResolution GetJogResolution()		{ return m_eJogResolution; };
	JogResolution GetTransportResolution()	{ return m_eTransportResolution; };
	double GetLastDisplayTime()			{ return m_dLastDisplayTime; };

	char *GetTempDisplayText()			{ return m_szTempDisplayText; };

	mapPluginProperties *GetMapPluginProperties()	{ return &m_mapPluginProperties; };

	DWORD GetNextUniqueId()				{ return m_dwUniqueIdCount++; };
	DWORD GetFirstFaderNumber()			{ return m_dwFirstFaderNumber; };
	DWORD GetLastFaderNumber()			{ return m_dwLastFaderNumber; };
	bool GetDisableHandshake()			{ return m_bDisableHandshake; };
	bool GetExcludeFiletersFromPlugins(){ return m_bExcludeFiltersFromPlugins; }
	bool GetScrubBankSelectsTrackBus()	{ return m_bScrubBankSelectsTrackBus; }
	bool GetUseHUIProtocol()			{ return m_bUseHUIProtocol;  }
	bool GetHUIKeyPadControlsKeyPad()	{ return m_bHUIKeyPadControlsKeyPad; }

	// Setters
	void SetConfigureLayoutMode(bool bConfigureLayoutMode);
	void SetStripNumOffset(int iStripNumOffset);
	void SetPluginNumOffset(int iPluginNumOffset);
	void SetParamNumOffset(int iParamNumOffset);
	void SetMasterFaderOffset(SONAR_MIXER_STRIP eMixerStrip, DWORD dwOffset);
	void SetMasterFaderType(SONAR_MIXER_STRIP eMixerStrip);
	void SetSelectedStripNum(DWORD dwSelectedStripNum, ISonarMixer *pMixer =NULL);
	void SetMixerStrip(SONAR_MIXER_STRIP eMixerStrip);
	void SetAssignment(Assignment eAssignment);
	void SetAssignmentMode(AssignmentMode eAssignmentMode);
	void CycleFlipMode();
	void SetFlipMode(FlipMode eFlipMode);
	void EnableModifier(DWORD dwModifier);
	void DisableModifier(DWORD dwModifier);
	void SetNavigationMode(NavigationMode eNavigationMode);
	void SetDisplayFlip(bool bDisplayFlip);
	void SetDisplayValues(bool bDisplayValues);
	void SetDisplayTrackNumbers(bool bDisplayTrackNumbers);
	void SetDisplayLevelMeters(LevelMeters eDisplayLevelMeters);
	void SetDisplaySMPTE(bool bDisplaySMPTE);
	void SetDisableFaders(bool bDisableFaders);
	void SetDisableRelayClick(bool bDisableRelayClick);
	void SetDisableLCDUpdates(bool bDisableLCDUpdates);
	void SetJogParamMode(bool bJogParamMode);
	void SetSoloSelectsChannel(bool bSoloSelectsChannel);
	void SetSelectHighlightsTrack(bool bSelectHighlightsTrack);
	void SetSelectDoubleClick(bool bSelectDoubleClick);
	void SetFaderTouchSelectsChannel(bool bFaderTouchSelectsChannel);
	void SetUserFunctionKey(BYTE bN, DWORD dwCmdId);
	void SetUserFootSwitch(BYTE bN, DWORD dwCmdId);
	void SetJogResolution(JogResolution eJogResolution);
	void SetTransportResolution(JogResolution eTransportResolution);
	void SetLastDisplayTime(double dLastDisplayTime);
	void SetDisableHandshake(bool bDisableHandshake);
	void SetExcludeFiltersFromPlugins(bool bExcludeFiltersFromPlugins);
	void SetScrubBankSelectsTrackBus(bool bScrubBankSelectsTrackBus);
	void SetUseHUIProtocol(bool bHUIProtocol);
	void SetHUIKeyPadControlsKeyPad(bool bHUIKeyPadControlsKeyPad);

	void SetTempDisplayText(const char *szText);

	void LoadPluginMappings();
	void LoadParameterProperties(const char *szFile, const char *szPluginName, const char *szParamName, int iIndex, mapParameterProperties *mapParamProps);
	bool ParseParameterPropertiesLine(char *szLine, CParameterProperties *pPluginProps);
	void DumpPluginProperties();

	void AddUnit(CMackieControlBase *pUnit);
	void RemoveUnit(CMackieControlBase *pUnit);
	void RestoreUnitOffset(CMackieControlBase *pUnit);

	void GetUnitOffsets(vectorMackieControlInformation *pInfo);
	void SetUnitOffsets(vectorMackieControlInformation *pInfo);

	void DetermineCapabilities(ISonarMixer *pMixer, ISonarIdentity *pSonarIdentity);
	bool HaveBuses()				{ return m_bHaveBuses; };
	bool HaveMeters()				{ return m_bHaveMeters; };
	bool HaveAudioMeters()			{ return m_bHaveAudioMeters; };
	bool HaveMidiMeters()			{ return m_bHaveMidiMeters; };
	bool HaveInputEcho()			{ return m_bHaveInputEcho; };
	bool HaveStripAny()				{ return m_bHaveStripAny; };
	bool HaveStripFilter()			{ return m_bHaveStripFilter; };
	SONAR_MIXER_STRIP BusType()		{ return m_eBusType; };
	SONAR_MIXER_STRIP MasterType()	{ return m_eMasterType; };

protected:
	void SetModifiers(DWORD dwModifiers);
	void CalculateFirstAndLastFader();
	int HasCapability(ISonarIdentity *pSonarIdentity, HOST_CAPABILITY cap);

	DWORD m_dwToolbarUpdateCount;
	DWORD m_dwMasterUpdateCount;
	DWORD m_dwXTUpdateCount;
	DWORD m_dwC4UpdateCount;

	bool m_bConfigureLayoutMode;

	DWORD m_dwStripNumOffset[NUM_MIXER_STRIP_TYPES];
	DWORD m_dwPluginNumOffset[NUM_MIXER_STRIP_TYPES][NUM_ASSIGNMENT_TYPES];
	DWORD m_dwParamNumOffset[NUM_MIXER_STRIP_TYPES][NUM_ASSIGNMENT_TYPES][NUM_ASSIGNMENT_MODES];
	DWORD m_dwMasterFaderOffset[NUM_MIXER_STRIP_TYPES];
	SONAR_MIXER_STRIP m_eMasterFaderType;

	DWORD m_dwSelectedStripNum;
	SONAR_MIXER_STRIP m_eMixerStrip;
	Assignment m_eAssignment;
	AssignmentMode m_eAssignmentMode;
	FlipMode m_eFlipMode;
	DWORD m_dwModifiers;
	NavigationMode m_eNavigationMode;
	bool m_bDisplayFlip;
	bool m_bDisplayValues;
	bool m_bDisplayTrackNumbers;
	LevelMeters m_eDisplayLevelMeters;
	bool m_bDisplaySMPTE;
	bool m_bDisableFaders;
	bool m_bDisableRelayClick;
	bool m_bDisableLCDUpdates;
	bool m_bJogParamMode;
	bool m_bSoloSelectsChannel;
	bool m_bSelectHighlightsTrack;
	bool m_bSelectDoubleClick;
	bool m_bFaderTouchSelectsChannel;
	bool m_bDisableHandshake;
	bool m_bExcludeFiltersFromPlugins;
	bool m_bScrubBankSelectsTrackBus;
	bool m_bUseHUIProtocol;
	bool m_bHUIKeyPadControlsKeyPad;
	CKeyBinding m_cUserFunctionKeys[NUM_USER_FUNCTION_KEYS];
	CKeyBinding m_cUserFootSwitch[NUM_USER_FOOT_SWITCHES];
	JogResolution m_eJogResolution;
	JogResolution m_eTransportResolution;
	double m_dLastDisplayTime;

	char m_szTempDisplayText[64];

	mapPluginProperties m_mapPluginProperties;

	DWORD m_dwUniqueIdCount;

	listMackieControlUnits m_ListOfUnits;
	vectorMackieControlInformation m_SetupInformation;

	DWORD m_dwFirstFaderNumber;
	DWORD m_dwLastFaderNumber;

	bool m_bHaveCheckedCapabilities;
	bool m_bHaveBuses;
	bool m_bHaveMeters;
	bool m_bHaveAudioMeters;
	bool m_bHaveMidiMeters;
	bool m_bHaveInputEcho;
	bool m_bHaveStripAny;
	bool m_bHaveStripFilter;
	int m_iMetersUpdatePeriod;
	SONAR_MIXER_STRIP m_eBusType;
	SONAR_MIXER_STRIP m_eMasterType;

	CRITICAL_SECTION m_cs;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControlState_h
