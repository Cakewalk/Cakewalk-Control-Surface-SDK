// MackieControlMaster.h : main header file for the MackieControlMaster DLL
//

#if !defined(AFX_MackieControlMaster_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlMaster_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// GUID's

extern const GUID CLSID_MackieControlMaster;
extern const GUID CLSID_MackieControlMasterPropPage;
extern const GUID LIBID_MackieControlMaster;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szMackieControlMasterFriendlyName[] = "Mackie Control (DEBUG)";
static const char s_szMackieControlMasterFriendlyNamePropPage[] = "Mackie Control Property Page (DEBUG)";
#else
#ifdef _MACKIECONTROLC1
static const char s_szMackieControlMasterFriendlyName[] = "MMcL Mackie Control #1";
static const char s_szMackieControlMasterFriendlyNamePropPage[] = "MMcL Mackie Control #1 Property Page";
#endif
#ifdef _MACKIECONTROLC2
static const char s_szMackieControlMasterFriendlyName[] = "MMcL Mackie Control #2";
static const char s_szMackieControlMasterFriendlyNamePropPage[] = "MMcL Mackie Control #2 Property Page";
#endif
#ifdef _MACKIECONTROLC3
static const char s_szMackieControlMasterFriendlyName[] = "MMcL Mackie Control #3";
static const char s_szMackieControlMasterFriendlyNamePropPage[] = "MMcL Mackie Control #3 Property Page";
#endif
#ifndef _MACKIECONTROLMMCL
static const char s_szMackieControlMasterFriendlyName[] = "Mackie Control";
static const char s_szMackieControlMasterFriendlyNamePropPage[] = "Mackie Control Property Page";
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

// Number of digits in the VSelect display
#define NUM_VSELECT_DISPLAY_CELLS		2

// Number of digits in the time code display
#define NUM_TIME_CODE_DISPLAY_CELLS		10

/////////////////////////////////////////////////////////////////////////////
// CMackieControlMaster

class CMackieControlMaster : public CMackieControlXT
{
public:
	// Ctors
	CMackieControlMaster();
	virtual ~CMackieControlMaster();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "MackieControlMaster"
	// Note: Skeleton methods are provided in MackieControlMaster.cpp.
	// Basic dev caps
	virtual STDMETHODIMP GetStatusText( LPSTR pszStatus, DWORD* pdwLen );

	// ISurfaceParamMapping
	virtual STDMETHODIMP GetStripRangeCount( DWORD *pdwCount);
	virtual STDMETHODIMP GetStripRange( DWORD dwIndex, DWORD *pdwLowStrip, DWORD *pdwHighStrip, SONAR_MIXER_STRIP *pmixerStrip);
	virtual STDMETHODIMP SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip);

	// *** IPersistStream methods ***
	virtual STDMETHODIMP Load( IStream* pStm );
	virtual STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty );
	virtual STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize );
	// (MackieControlMasterGen.cpp)
	virtual STDMETHODIMP GetClassID( CLSID* pClsid );

	// *** ISpecifyPropertyPages methods (MackieControlMasterGen.cpp) ***
	virtual STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods (MackieControlMasterGen.cpp) ***
	virtual STDMETHODIMP GetTypeInfoCount( UINT* );
	virtual STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	virtual STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	virtual STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );
	
// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.

	// MackieControlMasterPublic.cpp
	void SetConfigureLayoutMode(bool bEnable);
	bool GetConfigureLayoutMode();
	DWORD GetFunctionKey(BYTE bN);
	void SetFunctionKey(BYTE bN, DWORD dwCmdId);
	DWORD GetFootSwitch(BYTE bN);
	void SetFootSwitch(BYTE bN, DWORD dwCmdId);
	DWORD GetMasterFaderOffset();
	void SetMasterFaderOffset(DWORD dwN);
	SONAR_MIXER_STRIP GetMasterFaderType();
	void SetMasterFaderType(SONAR_MIXER_STRIP eMixerStrip);
	JogResolution GetJogResolution();
	void SetJogResolution(JogResolution eRes);
	JogResolution GetTransportResolution();
	void SetTransportResolution(JogResolution eRes);
	bool GetDisplaySMPTE();
	void SetDisplaySMPTE(bool bVal);
	bool GetDisableFaders();
	void SetDisableFaders(bool bVal);
	bool GetDisableRelayClick();
	void SetDisableRelayClick(bool bVal);
	bool GetDisableLCDUpdates();
	void SetDisableLCDUpdates(bool bVal);
	bool GetSoloSelectsChannel();
	void SetSoloSelectsChannel(bool bVal);
	bool GetFaderTouchSelectsChannel();
	void SetFaderTouchSelectsChannel(bool bVal);
	bool GetSelectHighlightsTrack();
	void SetSelectHighlightsTrack(bool bVal);
	bool GetSelectDoubleClick();
	void SetSelectDoubleClick(bool bVal);
	bool HaveLevelMeters();
	LevelMeters GetDisplayLevelMeters();
	void SetDisplayLevelMeters(LevelMeters eDisplayLevelMeters);
	void SetDisableHandshake(bool bVal);
	bool GetDisableHandshake();
	void SetExcludeFiltersFromPlugins(bool bVal);
	bool GetExcludeFiltersFromPlugins();
	bool GetScrubBankSelectsTrackBus();
	void SetScrubBankSelectsTrackBus(bool bVal);

protected:
	enum MASTER_IDs
	{
		// Generic names:
		MC_PARAM = 0x28,	// MC_ASSIGN_1
		MC_SEND,			// MC_ASSIGN_2
		MC_PAN,				MC_PLUG_INS,
		MC_EQ,				MC_DYNAMICS,		MC_BANK_DOWN,		MC_BANK_UP,
		MC_CHANNEL_DOWN,	MC_CHANNEL_UP,		MC_FLIP,			MC_EDIT,
		MC_NAME_VALUE,		MC_SMPTE_BEATS,
		MC_F1,				MC_F2,				MC_F3,				MC_F4,
		MC_F5,				MC_F6,				MC_F7,				MC_F8,
		MC_NEW_AUDIO,		// MC_F9
		MC_NEW_MIDI,		// MC_F10
		MC_FIT_TRACKS,		// MC_F11
		MC_FIT_PROJECT,		// MC_F12
		MC_OK_ENTER,		// MC_F13
		MC_CANCEL,			// MC_F14,
		MC_NEXT_WIN,		// MC_F15
		MC_CLOSE_WIN,		// MC_F16
		MC_M1,				// MC_CTRL
		MC_M2,				// MC_OPTION
		MC_M3,				// MC_SNAPSHOT
		MC_M4,				// MC_SHIFT
		MC_READ_OFF,		// READ_OFF
		MC_SNAPSHOT,		// MC_WRITE
		MC_TRACK,			// MC_UNDO
		MC_DISARM,			// MC_SAVE
		MC_OFFSET,			// MC_TOUCH
		MC_SAVE,			// MC_REDO
		MC_AUX,				// MC_FDR_GROUP
		MC_MAIN,			// MC_CLR_SOLO
		MC_UNDO,			// MC_MRKR
		MC_REDO,			// MC_MIXR
		MC_MARKER,			// MC_PREV_FROM
		MC_LOOP,			// MC_NEXT_FROM
		MC_SELECT,			// MC_END
		MC_PUNCH,			// MC_PI
		MC_JOG_PRM,			// MC_PO
		MC_LOOP_ON_OFF,		// MC_LOOP
		MC_HOME,			// MC_HOME
		MC_REWIND,			MC_FAST_FORWARD,	MC_STOP,			MC_PLAY,
		MC_RECORD,			MC_CURSOR_UP,		MC_CURSOR_DOWN,		MC_CURSOR_LEFT,
		MC_CURSOR_RIGHT,	MC_CURSOR_ZOOM,		MC_SCRUB,			MC_USER_A,
		MC_USER_B,			MC_FADER_MASTER = 0x70,
		MC_SMPTE,			MC_BEATS,			MC_RUDE
	};

	HRESULT SafeWrite(IStream *pStm, void const *pv, ULONG cb);
	HRESULT SafeRead(IStream *pStm, void *pv, ULONG cb);
	static void CALLBACK EXPORT _TransportTimerCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	void SetTransportCallbackTimer(float fAlpha, UINT uMax, UINT uMin);
	void ClearTransportCallbackTimer();
	void KillTransportCallbackTimer();
	static void CALLBACK EXPORT _KeyRepeatTimerCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	void SetKeyRepeatCallbackTimer(float fAlpha, UINT uMax, UINT uMin);
	void ClearKeyRepeatCallbackTimer();
	void KillKeyRepeatCallbackTimer();
	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual bool OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2);
	virtual bool OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg);
	virtual void OnReceivedSerialNumber();
	virtual void QuerySerialNumber(BYTE bDeviceType);
	void ReconfigureMaster(bool bForce);
	void ShiftStripNumOffset(int iAmount);
	void ShiftPluginNumOffset(int iAmount);
	void ShiftParamNumOffset(int iAmount);
	void SafeSetMasterFaderOffset(SONAR_MIXER_STRIP eMixerStrip, DWORD dwOffset);
	void TempDisplayMasterFader();
	void TempDisplayMasterFaderPan();

	// MackieControlMasterRx.cpp
	bool OnFader(BYTE bD1, BYTE bD2);
	bool OnSwitch(BYTE bD1, BYTE bD2);
	bool OnExternalController(BYTE bD2);
	bool OnJog(BYTE bD2);

	void OnSelectAssignment(Assignment eAssignment);
	void OnSwitchBankDown();
	void OnSwitchBankUp();
	void OnSwitchChannelDown();
	void OnSwitchChannelUp();
	void OnSwitchFlip();
	void OnSwitchEdit();
	void OnSwitchNameValue();
	void OnSwitchSMPTEBeats();
	void OnSwitchF1();
	void OnSwitchF2();
	void OnSwitchF3();
	void OnSwitchF4();
	void OnSwitchF5();
	void OnSwitchF6();
	void OnSwitchF7();
	void OnSwitchF8();
	void OnSwitchNewAudio();
	void OnSwitchNewMidi();
	void OnSwitchFitTracks();
	void OnSwitchFitProject();
	void OnSwitchOKEnter();
	void OnSwitchCancel();
	void OnSwitchNextWin();
	void OnSwitchCloseWin();
	void OnSwitchModifier(BYTE bN, bool bDown);
	void OnSwitchReadOff();
	void OnSwitchSnapshot();
	void OnSelectMixerStrip(SONAR_MIXER_STRIP eMixerStrip);
	void OnSwitchDisarm();
	void OnSwitchOffset();
	void OnSwitchSave();
	void OnSwitchUndo();
	void OnSwitchRedo();
	void OnSwitchMarker();
	void OnSwitchLoop();
	void OnSwitchSelect();
	void OnSwitchPunch();
	void OnSelectNavigationMode(NavigationMode eNavigationMode);
	void OnSwitchJogPrm();
	void OnSwitchLoopOnOff();
	void OnSwitchHome();
	void OnSwitchRewind(bool bDown);
	void OnSwitchFastForward(bool bDown);
	void OnSwitchStop();
	void OnSwitchPlay();
	void OnSwitchRecord();
	void OnSwitchCursorUp(bool bDown);
	void OnSwitchCursorDown(bool bDown);
	void OnSwitchCursorLeft(bool bDown);
	void OnSwitchCursorRight(bool bDown);
	void OnSwitchCursorZoom();
	void OnSwitchScrub();
	void OnSwitchUserA();
	void OnSwitchUserB();
	void OnSwitchMasterFader(bool bDown);
	void OnSwitchTrack();
	void TransportTimerCallback();
	void MoveRewindOrFastForward(Direction eDir);
	void SetRewindOrFastForward(Direction eDir);
	void KeyRepeatTimerCallback();
	void DoCursorKey(BOOL bKey);
	void DoCursorKeyUp();
	void DoCursorKeyDown();
	void DoCursorKeyLeft();
	void DoCursorKeyRight();
	void OnHandleScrubButton(bool bDown);
	void OnHandleBankDownButton();
	void OnHandleBankUpButton();
	void DoExportTrackTemplate();
	void DoImportTrackTemplate();

	// MackieControlMasterTx.cpp
	virtual void OnRefreshSurface(DWORD fdwRefresh, bool bForceSend);
	void CheckForTempDisplayText();
	void UpdateLEDs(bool bForceSend);
	void UpdateTransportLEDs(bool bForceSend);
	void UpdateVSelectDisplay(bool bForceSend);
	void UpdateTimeCodeDisplay(bool bForceSend);
	void UpdateMasterFader(bool bForceSend);

	bool m_bWasInEQFreqGainMode;

	Direction m_eCurrentTransportDir;
	float m_fTransportTimeout;

	BYTE m_bKeyRepeatKey;
	float m_fKeyRepeatTimeout;

	DWORD m_dwMasterUpdateCount;

	CMackieControl7SegmentDisplay m_cVSelectDisplay[NUM_VSELECT_DISPLAY_CELLS];
	CMackieControl7SegmentDisplay m_cTimeCodeDisplay[NUM_TIME_CODE_DISPLAY_CELLS];

	CMackieControlFader m_HwMasterFader;

	CMixParam m_SwMasterFader;
	CMixParam m_SwMasterFaderPan;

	WORD				m_wTransportTimerPeriod;
	UINT				m_uiTransportTimerID;
	bool				m_bTransportTimerActive;
	WORD				m_wKeyRepeatTimerPeriod;
	UINT				m_uiKeyRepeatTimerID;
	bool				m_bKeyRepeatTimerActive;
	bool				m_bScrubKeyDown;
	BYTE				m_bLastButtonPressed;

	CString	m_cbTempTrackTemplateFilename;
	void getTempTrackTemplateFilename();

	virtual void ZeroAllFaders();
};

/////////////////////////////////////////////////////////////////////////////

class CMackieControlMasterFactory : public IClassFactory
{
public:
	// IUnknown
	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// Interface IClassFactory
	STDMETHODIMP_(HRESULT) CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP_(HRESULT) LockServer( BOOL bLock ); 

	// Constructor
	CMackieControlMasterFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlMaster_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
