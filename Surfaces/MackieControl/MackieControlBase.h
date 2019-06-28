// MackieControlBase.h : main header file for the MackieControlBase DLL
//

#if !defined(AFX_MackieControlBase_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlBase_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////

// Number of switch and LED IDs
#define NUM_SWITCH_AND_LED_IDS		0x74

// Volume and pan default values
#define VOL_AUDIO_DEFAULT			(101.0f / 127.0f)
#define VOL_MIDI_DEFAULT			(100.0f / 127.0f)
#define PAN_CENTER					0.5f

// Volume and pan step sizes
#define VOL_AUDIO_STEP_SIZE			0.005f
#define VOL_MIDI_STEP_SIZE			(1.0f / 127.0f)
#define PAN_STEP_SIZE				0.005f

#define IDX_DEFAULT_FADER		-1

// How long (number of refreshes) to display any temporary text
#define TEMP_DISPLAY_TIMEOUT		20

// New command ID's
#define NEW_CMD_ENVELOPE_OFFSET_MODE					267
#define NEW_CMD_SNAPSHOT								283
#define NEW_CMD_DISARM_ALL_AUTOMATION_CONTROLS			284
#define NEW_CMD_ENABLE_DISABLE_AUTOMATION_PLAYBACK		285
#define NEW_CMD_METRONOME_DURING_PLAYBACK				275
#define NEW_CMD_METRONOME_DURING_RECORD					276
#define NEW_CMD_STOP_WITH_NOW_MARKER					293
#define NEW_CMD_AUTO_PUNCH_TOGGLE						544

enum MackieSurfaceType
{
	SURFACE_TYPE_UNKNOWN,
	SURFACE_TYPE_C4,
	SURFACE_TYPE_XT,
	SURFACE_TYPE_MASTER
};

/////////////////////////////////////////////////////////////////////////////

#define snprintf	_snprintf

/////////////////////////////////////////////////////////////////////////////
// CMackieControlBase

class CMackieControlBase :
	public IControlSurface,
	public ISurfaceParamMapping,
	public IPersistStream,
	public ISpecifyPropertyPages
{
public:
	// Ctors
	CMackieControlBase();
	virtual ~CMackieControlBase();

	// *** IUnknown methods ***
	// Handled in MackieControlBaseGen.cpp
	virtual STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
	virtual STDMETHODIMP_(ULONG)	AddRef();
	virtual STDMETHODIMP_(ULONG)	Release();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "MackieControlBase"
	// Note: Skeleton methods are provided in MackieControlBase.cpp.
	// Basic dev caps
	virtual STDMETHODIMP GetStatusText( LPSTR pszStatus, DWORD* pdwLen ) =0;
	virtual STDMETHODIMP MidiInShortMsg( DWORD dwShortMsg );
	virtual STDMETHODIMP MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );
	virtual STDMETHODIMP RefreshSurface( DWORD fdwRefresh, DWORD dwCookie );
	virtual STDMETHODIMP GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );

	// ISurfaceParamMapping
	virtual STDMETHODIMP GetStripRangeCount( DWORD *pdwCount) =0;
	virtual STDMETHODIMP GetStripRange( DWORD dwIndex, DWORD *pdwLowStrip, DWORD *pdwHighStrip, SONAR_MIXER_STRIP *pmixerStrip) =0;
	virtual STDMETHODIMP SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip) =0;
	virtual STDMETHODIMP GetDynamicControlCount( DWORD *pdwCount);
	virtual STDMETHODIMP GetDynamicControlInfo( DWORD dwIndex, DWORD *pdwKey, SURFACE_CONTROL_TYPE *pcontrolType);
	virtual HRESULT STDMETHODCALLTYPE SetLearnState(BOOL) { return E_NOTIMPL; }

	// (MackieControlBaseGen.cpp)
	virtual STDMETHODIMP Connect( IUnknown* pUnknown, HWND hwndApp );
	virtual STDMETHODIMP Disconnect();

	// *** IPersistStream methods ***
	virtual STDMETHODIMP Load( IStream* pStm ) =0;
	virtual STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty ) =0;
	virtual STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize ) =0;
	// (MackieControlBaseGen.cpp)
	virtual STDMETHODIMP GetClassID( CLSID* pClsid ) =0;
	virtual STDMETHODIMP IsDirty();

	// *** ISpecifyPropertyPages methods (MackieControlBaseGen.cpp) ***
	virtual STDMETHODIMP GetPages( CAUUID* pPages ) =0;

	// *** IDispatch methods (MackieControlBaseGen.cpp) ***
	virtual STDMETHODIMP GetTypeInfoCount( UINT* ) =0;
	virtual STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** ) =0;
	virtual STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* ) =0;
	virtual STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* ) =0;

// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.

	BYTE GetDeviceType()						{ return m_bDeviceType; };
	DWORD GetUnitStripNumOffset()				{ return m_dwUnitStripNumOffset; };
	DWORD GetUniqueId()							{ return m_dwUniqueId; };
	ISonarCommands *GetSonarCommands()			{ return m_pCommands; };
	void SetUnitStripNumOffset(DWORD dwOffset);
	void SendMidiShort(BYTE bStatus, BYTE bD1, BYTE bD2);
	void SendMidiLong(DWORD cbLongMsg, const BYTE* pbLongMsg);

protected:
	void releaseSonarInterfaces();

	virtual void OnConnect() =0;
	virtual void OnDisconnect() =0;
	virtual bool OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2) =0;
	virtual bool OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg) =0;
	virtual void OnRefreshSurface(DWORD fdwRefresh, bool bForceSend) =0;
	virtual void OnReceivedSerialNumber() =0;
	virtual void SetAllFadersToDefault() =0;
	virtual void SetAllVPotsToDefault() =0;

	void DoCommand(DWORD dwCmdId)				{ m_pCommands->DoCommand(dwCmdId); };

	bool GetHaveSerialNumber()					{ return m_bHaveSerialNumber; };
	bool GetSerialNumber(BYTE *pSerialNumber);
	bool IsSerialNumber(BYTE *pSerialNumber);

	void ClearRefreshFlags();
	void DispatchRefreshRequest();
	void RequestRefresh(bool bForce);
	void RequestRefreshAll(bool bForce);
	void RequestSetAllFadersToDefault();
	void RequestSetAllVPotsToDefault();

	void SendUniversalDeviceQuery();
	void SendWakeUp(BYTE bDeviceType);
	virtual void QuerySerialNumber(BYTE bDeviceType);

	void SetLED(BYTE bID, BYTE bVal, bool bForceSend);
	void UpdateToolbarDisplay(bool bForce);

	// MackieControlBaseUtils.cpp
	enum Direction
	{
		DIR_FORWARD,
		DIR_BACKWARD
	};

public:
	DWORD GetStripCount(SONAR_MIXER_STRIP eMixerStrip);
	SONAR_MIXER_STRIP GetMasterStripType()		{ return m_cState.MasterType(); };
	virtual bool UsingHUIProtocol();
	virtual MackieSurfaceType GetSurfaceType();
protected:
	bool GetStripName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, char *pszText, DWORD *pdwLen);
	DWORD GetPluginCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	bool GetPluginName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwParamNum, char *pszText, DWORD *pwdLen);
	DWORD GetPluginParamCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum);
	bool GetFilterExists(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwFilterNum);
	bool GetFilterName(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwFilterNum, char *pszText, DWORD *pwdLen);
	DWORD GetFilterParamCount(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwFilterNum);
	DWORD GetNumInputs(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	DWORD GetNumOutputs(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	DWORD GetNumSends(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	DWORD GetMaxNumSends(SONAR_MIXER_STRIP eMixerStrip);
	DWORD GetSelected();
	bool IsProjectLoaded();
	bool HaveMixerStrips();
	bool IsAPluginMode(Assignment eAssignment);
	bool IsMIDI(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	bool GetRudeSoloStatus();
	void ClearAllMutes();
	void ClearAllSolos();
	void DisarmAllTracks();
	void ToggleLoopMode();
	void ToggleScrubMode();
	bool GetTransportState(SONAR_TRANSPORT_STATE eState);
	void SetTransportState(SONAR_TRANSPORT_STATE eState, bool bVal);
	bool IsRecording();
	void SetTimeCursor(SONAR_TRANSPORT_TIME eTime);
	void SetMarker(SONAR_TRANSPORT_TIME eTime);
	void FakeKeyPress(bool bShift, bool bCtrl, bool bAlt, int nVirtKey);
	void SetRelayClick(bool bOn);
	void NudgeTimeCursor(JogResolution eJogResolution, Direction eDir);
	void OnPlayPressed();
	void OnContextSwitch();
	void LimitAndSetStripNumOffset(int iStripNumOffset);

	// MackieControlBaseBinder.cpp
	void ConfParameterTrack(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfParameterAux(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfParameterMain(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfParameterBus(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfParameterMaster(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfPan(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfSend(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfPlugin(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfEQ(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfDynamics(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfNoBindings(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam);
	void ConfigureVolume(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, bool bIsMIDI, CMixParam *pParam);
	void ConfigureParamInput(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, bool bAllowNone, CMixParam *pParam);
	void ConfigureParamOutput(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, bool bAllowNone, CMixParam *pParam);
	void ConfigureAuxSends(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwIndex, CMixParam *pParam);
	void ConfigureAuxSendPans(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwIndex, CMixParam *pParam);
	void ConfigurePlugins(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwIndex, DWORD dwPluginNum, DWORD dwFilterMask, DWORD dwModifiers, CMixParam *pParam);
	void SetParamToProperty(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwIndex, DWORD dwPluginNum, mapParameterProperties *pParamProps, CMixParam *pParam);
	bool GetPluginProperties(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD *dwPluginNum, DWORD dwFilterMask, CPluginProperties *pProps, char *pszText =NULL, DWORD *pdwLen =NULL);
	DWORD GetNumParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, Assignment eAssignment);
	DWORD GetNumPluginParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwFilterMask);
	bool GetCurrentPluginName(Assignment eAssignment, SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, char *pszText, DWORD *pdwLen);

	ISonarMidiOut*		m_pMidiOut;
	ISonarKeyboard*		m_pKeyboard;
	ISonarCommands*		m_pCommands;
	ISonarProject*		m_pProject;
	ISonarMixer*		m_pMixer;
	ISonarMixer2*		m_pMixer2;
	ISonarTransport*	m_pTransport;
	ISonarIdentity*		m_pSonarIdentity;
	ISonarIdentity2*	m_pSonarIdentity2;
	ISonarParamMapping*	m_pParamMapping;

	FilterLocator		m_FilterLocator;

	DWORD				m_dwSupportedRefreshFlags;
	HWND				m_hwndApp;

	LONG				m_cRef;
	BOOL				m_bDirty;
	CRITICAL_SECTION	m_cs;		// critical section to guard properties

	// A shared single CMackieControlState for communication between modules
	static CMackieControlState	m_cState;

	bool m_bConnected;
	DWORD m_dwRefreshCount;
	BYTE m_bExpectedDeviceType;
	BYTE m_bDeviceType;
	bool m_bHaveSerialNumber;
	BYTE m_bSerialNumber[LEN_SERIAL_NUMBER];
	DWORD m_dwUniqueId;
	DWORD m_dwUnitStripNumOffset;
	DWORD m_dwToolbarUpdateCount;
	bool m_bBindMuteSoloArm;

	bool m_bSwitches[NUM_SWITCH_AND_LED_IDS];
	BYTE m_bLEDs[NUM_SWITCH_AND_LED_IDS];

	bool m_bRefreshWhenDone;
	bool m_bForceRefreshWhenDone;
	bool m_bRefreshAllWhenDone;
	bool m_bForceRefreshAllWhenDone;

	// Messages to pass into RefreshSurface()
	// We pass in the *address* of these messages to signal what we want to happen
	// The last four are static, so that all instances will respond
	bool m_bMsgRefresh;
	bool m_bMsgForceRefresh;
	static bool m_bMsgRefreshAll;
	static bool m_bMsgForceRefreshAll;
	static bool m_bMsgSetAllFadersToDefault;
	static bool m_bMsgSetAllVPotsToDefault;
	BYTE m_lastMidiOnNote;
	bool m_currentPluginIsTrackCompressor;
	bool m_bProjectLoadedState;
	virtual void SetProjectLoadedState(bool bProjectLoadedState);

	// HUI specific stuff

	BYTE m_bHuiScribble[13] = { 0xF0, 0x00, 0x00, 0x66, 0x05, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 };
	BYTE m_bCurrentHUIZone;
	BYTE m_bHUIFaderHi[8];
	virtual bool SetHuiLED(BYTE bID, BYTE bVal, bool bForceSend) = 0;
	virtual bool TranslateHUIButtons(BYTE bCurrentZone, BYTE bPort, bool bOn, BYTE &bD1, BYTE &bD2) = 0;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlBase_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
