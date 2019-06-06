// MackieControlXT.h : main header file for the MackieControlXT DLL
//

#if !defined(AFX_MackieControlXT_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlXT_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// GUID's

extern const GUID CLSID_MackieControlXT;
extern const GUID CLSID_MackieControlXTPropPage;
extern const GUID LIBID_MackieControlXT;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szMackieControlXTFriendlyName[] = "Mackie Control XT (DEBUG)";
static const char s_szMackieControlXTFriendlyNamePropPage[] = "Mackie Control XT Property Page (DEBUG)";
#else
#ifdef _MACKIECONTROLC1
static const char s_szMackieControlXTFriendlyName[] = "MMcL Mackie Control XT #1";
static const char s_szMackieControlXTFriendlyNamePropPage[] = "MMcL Mackie Control XT #1 Property Page";
#endif
#ifdef _MACKIECONTROLC2
static const char s_szMackieControlXTFriendlyName[] = "MMcL Mackie Control XT #2";
static const char s_szMackieControlXTFriendlyNamePropPage[] = "MMcL Mackie Control XT #2 Property Page";
#endif
#ifdef _MACKIECONTROLC3
static const char s_szMackieControlXTFriendlyName[] = "MMcL Mackie Control XT #3";
static const char s_szMackieControlXTFriendlyNamePropPage[] = "MMcL Mackie Control XT #3 Property Page";
#endif
#ifndef _MACKIECONTROLMMCL
static const char s_szMackieControlXTFriendlyName[] = "Mackie Control XT";
static const char s_szMackieControlXTFriendlyNamePropPage[] = "Mackie Control XT Property Page";
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

// Number of fader channels
#define NUM_MAIN_CHANNELS			8

// Number of refreshes before a ping is sent to the HUI
#define HUI_PING_COUNTDOWN 100

/////////////////////////////////////////////////////////////////////////////
// CMackieControlXT

class CMackieControlXT : public CMackieControlBase
{
public:
	// Ctors
	CMackieControlXT();
	virtual ~CMackieControlXT();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "MackieControlXT"
	// Note: Skeleton methods are provided in MackieControlXT.cpp.
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
	// (MackieControlXTGen.cpp)
	virtual STDMETHODIMP GetClassID( CLSID* pClsid );

	// *** ISpecifyPropertyPages methods (MackieControlXTGen.cpp) ***
	virtual STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods (MackieControlXTGen.cpp) ***
	virtual STDMETHODIMP GetTypeInfoCount( UINT* );
	virtual STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	virtual STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	virtual STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );
	
// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.
	
protected:
	enum XT_IDs
	{
		MC_REC_ARM_1 = 0x00,	MC_REC_ARM_2,	MC_REC_ARM_3,	MC_REC_ARM_4,
		MC_REC_ARM_5,			MC_REC_ARM_6,	MC_REC_ARM_7,	MC_REC_ARM_8,
		MC_SOLO_1,				MC_SOLO_2,		MC_SOLO_3,		MC_SOLO_4,
		MC_SOLO_5,				MC_SOLO_6,		MC_SOLO_7,		MC_SOLO_8,
		MC_MUTE_1,				MC_MUTE_2,		MC_MUTE_3,		MC_MUTE_4,
		MC_MUTE_5,				MC_MUTE_6,		MC_MUTE_7,		MC_MUTE_8,
		MC_SELECT_1,			MC_SELECT_2,	MC_SELECT_3,	MC_SELECT_4,
		MC_SELECT_5,			MC_SELECT_6,	MC_SELECT_7,	MC_SELECT_8,
		MC_VPOT_1,				MC_VPOT_2,		MC_VPOT_3,		MC_VPOT_4,
		MC_VPOT_5,				MC_VPOT_6,		MC_VPOT_7,		MC_VPOT_8,
		MC_FADER_1 = 0x68,		MC_FADER_2,		MC_FADER_3,		MC_FADER_4,
		MC_FADER_5,				MC_FADER_6,		MC_FADER_7,		MC_FADER_8,
	};

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual bool OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2);
	virtual bool OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg);
	virtual bool OnHUIMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2);
	virtual void OnReceivedSerialNumber();
	virtual void SetAllFadersToDefault();
	virtual void SetAllVPotsToDefault();
	void TempDisplaySelectedTrackName();

	// MackieControlXTReconfigure.cpp
	void ReconfigureXT(bool bForce);
	void ConfEQGainFreq(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwPluginNum, DWORD dwIndex, CMixParam *pFader, CMixParam *pVPot);

	// MackieControlXTRx.cpp
	bool OnFader(BYTE bChan, BYTE bD1, BYTE bD2);
	bool OnSwitch(BYTE bD1, BYTE bD2);		// *Not* virtual
	bool OnVPot(BYTE bD1, BYTE bD2);
	void OnSwitchVPot(BYTE bChan);
	void OnSwitchRecArm(BYTE bChan);
	void OnSwitchSolo(BYTE bChan);
	void OnSwitchMute(BYTE bChan);
	void OnSwitchSelect(BYTE bChan);
	void OnSwitchFader(BYTE bChan, bool bDown);

	// MackieControlXTTx.cpp
	virtual void OnRefreshSurface(DWORD fdwRefresh, bool bForceSend);
	void UpdateVPotDisplays(bool bForceSend);
	void UpdateLEDs(bool bForceSend);
	void UpdateFaders(bool bForceSend);
	void UpdateFader(BYTE bChan, bool bForceSend);

	// MackieControlXTTxDisplay.cpp
	void UpdateLCDDisplay(bool bForceSend);
	void UpdateUpperLCD(bool bForceSend);
	void UpdateLowerLCD(bool bForceSend);
	void UpdateLevelMeters(bool bForceSend);
	void FormatTrackNameOrNumber(BYTE bChan, char *szTrack, int len);
	void FormatParamNameOrValue(BYTE bChan, char *szParam, int len);
	void FormatTrackNumbers(char *str, int len, DWORD dwStartNum);
	void TempDisplay(DWORD dwCount, char *szText);
	virtual void SetProjectLoadedState(bool bProjectLoadedState);

	DWORD m_dwXTUpdateCount;
	DWORD m_dwPreviousStripCount;
	DWORD m_dwPreviousPluginCount;

	CMackieControlLCDDisplay m_HwLCDDisplay;
	CMackieControlVPotDisplay m_HwVPotDisplay[NUM_MAIN_CHANNELS];
	CMackieControlFader m_HwFader[NUM_MAIN_CHANNELS];

	CMixParam m_SwStrip[NUM_MAIN_CHANNELS];
	CMixParam m_SwVPot[NUM_MAIN_CHANNELS];
	CMixParam m_SwRec[NUM_MAIN_CHANNELS];
	CMixParam m_SwSolo[NUM_MAIN_CHANNELS];
	CMixParam m_SwMute[NUM_MAIN_CHANNELS];
	CMixParam m_SwArchive[NUM_MAIN_CHANNELS];
	CMixParam m_SwInputEcho[NUM_MAIN_CHANNELS];
	CMixParam m_SwFader[NUM_MAIN_CHANNELS];

	char m_szTempDisplayText[LCD_WIDTH];
	DWORD m_dwTempDisplayTextCounter;
	DWORD m_dwTempDisplayValuesCounter[NUM_MAIN_CHANNELS];

	virtual void ZeroAllFaders();

	// HUI specific stuff

	int m_huiPingCounter = HUI_PING_COUNTDOWN;
	virtual bool TranslateHUIButtons(BYTE bCurrentZone, BYTE bPort, bool bOn, BYTE &bD1, BYTE &bD2);
	virtual bool SetHuiLED(BYTE bID, BYTE bVal, bool bForceSend);
	void PingHuiIfRequired();
};

/////////////////////////////////////////////////////////////////////////////

class CMackieControlXTFactory : public IClassFactory
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
	CMackieControlXTFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlXT_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
