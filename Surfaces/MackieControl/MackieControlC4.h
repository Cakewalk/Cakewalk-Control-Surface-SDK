// MackieControlC4.h : main header file for the MackieControlC4 DLL
//

#if !defined(AFX_MackieControlC4_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlC4_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// GUID's

extern const GUID CLSID_MackieControlC4;
extern const GUID CLSID_MackieControlC4PropPage;
extern const GUID LIBID_MackieControlC4;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szMackieControlC4FriendlyName[] = "Mackie Control C4 (DEBUG)";
static const char s_szMackieControlC4FriendlyNamePropPage[] = "Mackie Control C4 Property Page (DEBUG)";
#else
#ifdef _MACKIECONTROLC1
static const char s_szMackieControlC4FriendlyName[] = "MMcL Mackie Control C4 #1";
static const char s_szMackieControlC4FriendlyNamePropPage[] = "MMcL Mackie Control C4 #1 Property Page";
#endif
#ifdef _MACKIECONTROLC2
static const char s_szMackieControlC4FriendlyName[] = "MMcL Mackie Control C4 #2";
static const char s_szMackieControlC4FriendlyNamePropPage[] = "MMcL Mackie Control C4 #2 Property Page";
#endif
#ifdef _MACKIECONTROLC3
static const char s_szMackieControlC4FriendlyName[] = "MMcL Mackie Control C4 #3";
static const char s_szMackieControlC4FriendlyNamePropPage[] = "MMcL Mackie Control C4 #3 Property Page";
#endif
#ifndef _MACKIECONTROLMMCL
static const char s_szMackieControlC4FriendlyName[] = "Mackie Control C4";
static const char s_szMackieControlC4FriendlyNamePropPage[] = "Mackie Control C4 Property Page";
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

// Number of channels
#define NUM_ROWS	4
#define NUM_COLS	8

#define NUM_C4_USER_FUNCTION_KEYS	8

/////////////////////////////////////////////////////////////////////////////
// CMackieControlC4

class CMackieControlC4 : public CMackieControlBase
{
public:
	// Ctors
	CMackieControlC4();
	virtual ~CMackieControlC4();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "MackieControlC4"
	// Note: Skeleton methods are provided in MackieControlC4.cpp.
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
	// (MackieControlC4Gen.cpp)
	virtual STDMETHODIMP GetClassID( CLSID* pClsid );

	// *** ISpecifyPropertyPages methods (MackieControlC4Gen.cpp) ***
	virtual STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods (MackieControlC4Gen.cpp) ***
	virtual STDMETHODIMP GetTypeInfoCount( UINT* );
	virtual STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	virtual STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	virtual STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );
	
// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.
	
protected:
	enum C4_IDs
	{
		MC_SPLIT = 0x00,
		MC_SPLIT_1_3 = 0x00,	MC_SPLIT_2_2,	MC_SPLIT_3_1,
		MC_LOCK = 0x03,			MC_SPOT_ERASE,	MC_MARKER,		MC_TRACK,
		MC_CHANNEL_STRIP,		MC_FUNCTION,	MC_BANK_LEFT,	MC_BANK_RIGHT,
		MC_PARAM_LEFT,			MC_PARAM_RIGHT,	MC_SHIFT,		MC_OPTION,
		MC_CONTROL,				MC_ALT,			MC_SLOT_UP,		MC_SLOT_DOWN,
		MC_TRACK_LEFT,			MC_TRACK_RIGHT,
		MC_VPOT_1_1 = 0x20,		MC_VPOT_1_2,	MC_VPOT_1_3,	MC_VPOT_1_4,
		MC_VPOT_1_5,			MC_VPOT_1_6,	MC_VPOT_1_7,	MC_VPOT_1_8,
		MC_VPOT_2_1,			MC_VPOT_2_2,	MC_VPOT_2_3,	MC_VPOT_2_4,
		MC_VPOT_2_5,			MC_VPOT_2_6,	MC_VPOT_2_7,	MC_VPOT_2_8,
		MC_VPOT_3_1,			MC_VPOT_3_2,	MC_VPOT_3_3,	MC_VPOT_3_4,
		MC_VPOT_3_5,			MC_VPOT_3_6,	MC_VPOT_3_7,	MC_VPOT_3_8,
		MC_VPOT_4_1,			MC_VPOT_4_2,	MC_VPOT_4_3,	MC_VPOT_4_4,
		MC_VPOT_5_5,			MC_VPOT_4_6,	MC_VPOT_5_7,	MC_VPOT_4_8,
	};

	enum C4SplitMode
	{
		C4_SPLIT_NONE,
		C4_SPLIT_1_3,
		C4_SPLIT_2_2,
		C4_SPLIT_3_1,
	};

	enum C4SplitSection
	{
		C4_UPPER,
		C4_LOWER,
		NUM_SPLITS,
	};

	enum C4Assignment
	{
		C4_ASSIGNMENT_NORMAL,
		C4_ASSIGNMENT_TRACK,
		C4_ASSIGNMENT_FUNCTION,
		NUM_C4_ASSIGNMENT_TYPES
	};

	HRESULT SafeWrite(IStream *pStm, void const *pv, ULONG cb);
	HRESULT SafeRead(IStream *pStm, void *pv, ULONG cb);
	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual bool OnMidiInShort(BYTE bStatus, BYTE bD1, BYTE bD2);
	virtual bool OnMidiInLong(DWORD cbLongMsg, const BYTE* pbLongMsg);
	virtual void OnReceivedSerialNumber();
	virtual void SetAllFadersToDefault();
	virtual void SetAllVPotsToDefault();
	void GetStatusText(C4SplitSection eSplit, CString *str);
	void ShiftTrack(int iAmount);
	void TempDisplaySelectedTrackName(C4SplitSection eSplit);
	C4SplitSection GetSplitForRow(BYTE row);
	int GetNumVPotsInSplit(C4SplitSection eSplit);

	// MackieControlC4State.cpp
	SONAR_MIXER_STRIP GetMixerStrip(C4SplitSection eSplit);
	void SetMixerStrip(C4SplitSection eSplit, SONAR_MIXER_STRIP eMixerStrip);
	Assignment GetAssignment(C4SplitSection eSplit);
	void SetAssignment(C4SplitSection eSplit, Assignment eAssignment);
	Assignment GetPreSynthRackAssignment(C4SplitSection eSplit);
	void SetPreSynthRackAssignment( C4SplitSection eSplit, Assignment eAssignment );
	AssignmentMode GetAssignmentMode(C4SplitSection eSplit);
	DWORD GetPluginNumOffset(C4SplitSection eSplit);
	void ShiftPluginNumOffset(C4SplitSection eSplit, int iAmount);
	DWORD GetParamNumOffset(C4SplitSection eSplit);
	void ShiftParamNumOffset(C4SplitSection eSplit, int iAmount);
	DWORD GetStripNumOffset(C4SplitSection eSplit);
	void ShiftStripNumOffset(C4SplitSection eSplit, int iAmount);
	void LimitAndSetStripNumOffset(C4SplitSection eSplit, int iStripNumOffset);
	DWORD GetSelectedStripNum(C4SplitSection eSplit);
	void ShiftSelectedTrack(C4SplitSection eSplit, int iAmount);
	void LimitAndSetSelectedTrack(C4SplitSection eSplit, int iSelectedStripNum);
	void SetSelectedStripNum(C4SplitSection eSplit, DWORD dwSelectedStripNum, ISonarMixer *pMixer =NULL);
	void ToggleLevelMeters(C4SplitSection eSplit);
	bool GetDisplayLevelMeters(C4SplitSection eSplit);
	void SetDisplayLevelMeters(C4SplitSection eSplit, bool bOn);
	void ToggleDisplayValues(C4SplitSection eSplit);
	bool GetDisplayValues(C4SplitSection eSplit);
	void SetDisplayValues(C4SplitSection eSplit, bool bOn);
	DWORD GetModifiers(DWORD dwMask);
	void EnableModifier(DWORD dwModifier);
	void DisableModifier(DWORD dwModifier);
public:
	DWORD GetFunctionKey(BYTE bN);
	void SetFunctionKey(BYTE bN, DWORD dwCmdId);
	CString GetFunctionKeyName(BYTE bN);
	void SetFunctionKeyName(BYTE bN, CString strName);
	virtual bool UsingHUIProtocol() { return false; };
	virtual MackieSurfaceType GetSurfaceType();
protected:

	// MackieControlC4Reconfigure.cpp
	void ReconfigureC4(bool bForce);
	bool ReconfigureC4Half(C4SplitSection eSplit, int first, int last, bool bForce);

	// MackieControlC4Rx.cpp
	bool OnVPot(BYTE bD1, BYTE bD2);
	bool OnSwitch(BYTE bD1, BYTE bD2);

	void OnSwitchSplit();
	void OnSwitchLock();
	void OnSwitchSpotErase();
	void OnSwitchMarker(bool bDown);
	void OnSwitchTrack(bool bDown);
	void OnSwitchChanStrip();
	void OnSwitchFunction(bool bDown);
	void OnSwitchModifier(bool bDown, DWORD dwMask);
	void OnSwitchBankLeft();
	void OnSwitchBankRight();
	void OnSwitchParamLeft();
	void OnSwitchParamRight();
	void OnSwitchSlotUp();
	void OnSwitchSlotDown();
	void OnSwitchTrackLeft();
	void OnSwitchTrackRight();
	void OnSwitchVPot(BYTE row, BYTE col);

	// MackieControlC4Tx.cpp
	virtual void OnRefreshSurface(DWORD fdwRefresh, bool bForceSend);
	void UpdateVPotDisplays(bool bForceSend);
	void UpdateVPotForTrackMixerStrip(int row, C4SplitSection eSplit, bool bForceSend);
	void UpdateVPotForTrackAssignment(int row, C4SplitSection eSplit, bool bForceSend);
	void UpdateLEDs(bool bForceSend);

	// MackieControlC4TxDisplay.cpp
	void UpdateLCDDisplays(bool bForceSend);
	void UpdateUpperLCD(bool bForceSend);
	void UpdateLowerLCD(bool bForceSend);
	BYTE ConfigureMeters(int row, bool bForceSend);
	void UpdateLevelMeters(bool bForceSend);
	void FormatTrackNameOrNumber(C4SplitSection eSplit, BYTE row, BYTE col, char *szTrack, int len);
	void FormatParamNameOrValue(C4SplitSection eSplit, BYTE row, BYTE col, char *szParam, int len);
	void TempDisplay(BYTE row, DWORD dwCount, char *szText);
	void DisplayTrackSettings(C4SplitSection eSplit, BYTE row, bool bForceSend);

	CMackieControlLCDDisplay m_HwLCDDisplay[NUM_ROWS];
	CMackieControlVPotDisplay m_HwVPotDisplay[NUM_ROWS][NUM_COLS];

	CMixParam m_SwStrip[NUM_ROWS][NUM_COLS];
	CMixParam m_SwVPot[NUM_ROWS][NUM_COLS];

	DWORD m_dwC4UpdateCount;
	DWORD m_dwC4PreviousUpdateCount;
	DWORD m_dwPreviousStripCount[NUM_SPLITS];
	DWORD m_dwPreviousPluginCount[NUM_SPLITS];

	bool m_bLinkModifiers;
	C4SplitMode m_eSplitMode;
	C4SplitSection m_eSplit;
	C4Assignment m_eC4Assignment;
	C4Assignment m_eOldC4Assignment;
	DWORD m_dwModifiers;
	bool m_bMarkersMode;

	bool m_bLockMode[NUM_SPLITS];
	SONAR_MIXER_STRIP m_eMixerStrip[NUM_SPLITS];
	Assignment m_eAssignment[NUM_SPLITS];
	Assignment m_ePreSynthRackAssignment[NUM_SPLITS];
	Assignment m_ePreviousAssignment[NUM_SPLITS];
	AssignmentMode m_eAssignmentMode[NUM_SPLITS];
	bool m_bEnableMeters[NUM_SPLITS];
	bool m_bDisplayValues[NUM_SPLITS];

	DWORD m_dwSelectedStripNum[NUM_SPLITS];
	DWORD m_dwStripNumOffset[NUM_SPLITS][NUM_MIXER_STRIP_TYPES];
	DWORD m_dwPluginNumOffset[NUM_SPLITS][NUM_MIXER_STRIP_TYPES][NUM_ASSIGNMENT_TYPES];
	DWORD m_dwParamNumOffset[NUM_SPLITS][NUM_MIXER_STRIP_TYPES][NUM_ASSIGNMENT_TYPES][NUM_ASSIGNMENT_MODES];

	char m_szTempDisplayText[NUM_ROWS][LCD_WIDTH];
	DWORD m_dwTempDisplayTextCounter[NUM_ROWS];
	DWORD m_dwTempDisplayValuesCounter[NUM_ROWS][NUM_COLS];

	CKeyBinding m_cUserFunctionKeys[NUM_C4_USER_FUNCTION_KEYS];
	virtual bool TranslateHUIButtons(BYTE bCurrentZone, BYTE bPort, bool bOn, BYTE &bD1, BYTE &bD2) { return false; };
	virtual bool SetHuiLED(BYTE bID, BYTE bVal, bool bForceSend) { return false; };
};

/////////////////////////////////////////////////////////////////////////////

class CMackieControlC4Factory : public IClassFactory
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
	CMackieControlC4Factory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlC4_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
