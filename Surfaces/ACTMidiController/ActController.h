// ACTController.h : main header file for the ACTController DLL
//

#if !defined(AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif



#include "resource.h"		// main symbols

extern const GUID CLSID_ACTController;
extern const GUID CLSID_ACTControllerPropPage;
extern const GUID LIBID_ACTController;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szFriendlyName[] = "ACT MIDI Controller (DEBUG)";
static const char s_szFriendlyNamePropPage[] = "ACT MIDI Controller Property Page (DEBUG)";
#else
static const char s_szFriendlyName[] = "ACT MIDI Controller";
static const char s_szFriendlyNamePropPage[] = "ACT MIDI Controller Property Page";
#endif

/////////////////////////////////////////////////////////////////////////////
extern long g_lComponents;	// Count of active components
extern long g_lServerLocks;	// Count of locks

/////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <set>

#include "MixParam.h"
#include "MidiBinding.h"

#define NUM_STRIP_TYPES 8

#define NUM_KNOBS	8
#define NUM_SLIDERS	8
#define NUM_BUTTONS	8

#define NUM_BANKS	4
#define MAX_BANK	(NUM_BANKS - 1)

enum AssignmentMode
{
	MCS_ASSIGNMENT_MUTLI_CHANNEL,
	MCS_ASSIGNMENT_CHANNEL_STRIP,
	NUM_ASSIGNMENT_MODES
};

struct CDwordCStringPair
{
	DWORD   m_Num;
	CString m_Str;
};

typedef std::vector<CDwordCStringPair> vectorDwordCStringPairs;

#define MASK_COMMAND						0x80000000U

// NOTE: this values are saved in the presets so should not be changed, only added to
#define CMD_NONE							(MASK_COMMAND | 0)
#define CMD_ACT_ENABLE						(MASK_COMMAND | 1)
#define CMD_ACT_LOCK						(MASK_COMMAND | 2)
#define CMD_ROTARIES_MODE					(MASK_COMMAND | 3)
#define CMD_PREV_TRACK						(MASK_COMMAND | 4)
#define CMD_NEXT_TRACK						(MASK_COMMAND | 5)
#define CMD_PREV_TRACK_BANK					(MASK_COMMAND | 6)
#define CMD_NEXT_TRACK_BANK					(MASK_COMMAND | 7)
#define CMD_PREV_STRIP_TYPE					(MASK_COMMAND | 8)
#define CMD_NEXT_STRIP_TYPE					(MASK_COMMAND | 9)
#define CMD_PREV_SEL_TRACK					(MASK_COMMAND | 10)
#define CMD_NEXT_SEL_TRACK					(MASK_COMMAND | 11)
#define CMD_MUTE_SEL_TRACK					(MASK_COMMAND | 12)
#define CMD_SOLO_SEL_TRACK					(MASK_COMMAND | 13)
#define CMD_REC_ARM_SEL_TRACK				(MASK_COMMAND | 14)
#define CMD_ACT_LEARN						(MASK_COMMAND | 15)
#define CMD_PREV_ROTARIES_BANK				(MASK_COMMAND | 16)
#define CMD_NEXT_ROTARIES_BANK				(MASK_COMMAND | 17)
#define CMD_PREV_SLIDERS_BANK				(MASK_COMMAND | 18)
#define CMD_NEXT_SLIDERS_BANK				(MASK_COMMAND | 19)
#define CMD_PREV_BUTTONS_BANK				(MASK_COMMAND | 20)
#define CMD_NEXT_BUTTONS_BANK				(MASK_COMMAND | 21)
#define CMD_PREV_CONTROLLERS_BANK			(MASK_COMMAND | 22)
#define CMD_NEXT_CONTROLLERS_BANK			(MASK_COMMAND | 23)
#define CMD_AUTO_READ_SEL_TRACK				(MASK_COMMAND | 24)
#define CMD_AUTO_WRITE_SEL_TRACK			(MASK_COMMAND | 25)
#define CMD_PREV_ROTARIES_AND_SLIDERS_BANK	(MASK_COMMAND | 26)
#define CMD_NEXT_ROTARIES_AND_SLIDERS_BANK	(MASK_COMMAND | 27)
#define CMD_SURFACE_PROPS_TOGGLE					(MASK_COMMAND | 28)
#define CMD_SURFACE_PROPS_SHOW					(MASK_COMMAND | 29)
#define CMD_SURFACE_PROPS_HIDE					(MASK_COMMAND | 30)
#define CMD_OPEN_CUR_FX								(MASK_COMMAND | 31)
#define CMD_CLOSE_CUR_FX							(MASK_COMMAND | 32)
#define CMD_FOCUS_NEXT_FX							(MASK_COMMAND | 33)
#define CMD_FOCUS_PREV_FX							(MASK_COMMAND | 34)
#define CMD_OPEN_NEXT_FX							(MASK_COMMAND | 35)
#define CMD_OPEN_PREV_FX							(MASK_COMMAND | 36)

enum VirtualButton
{
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_8,
	BUTTON_SHIFT_1,
	BUTTON_SHIFT_2,
	BUTTON_SHIFT_3,
	BUTTON_SHIFT_4,
	BUTTON_SHIFT_5,
	BUTTON_SHIFT_6,
	BUTTON_SHIFT_7,
	BUTTON_SHIFT_8,
	NUM_VIRTUAL_BUTTONS
};

#define MASK_KNOB(n)	(0x1000 | (n))
#define MASK_SLIDER(n)	(0x2000 | (n))
#define MASK_BUTTON(n)	(0x4000 | (n))

void FillComboBox(CComboBox *pBox, vectorDwordCStringPairs *pData);
void SelectByItemData(CComboBox *pBox, DWORD dwData);

#define ACTBgColour()  RGB(193, 205, 227)

/////////////////////////////////////////////////////////////////////////////
// CACTController

class CACTController :
	public IControlSurface3,
	public ISurfaceParamMapping,
	public IPersistStream,
	public ISpecifyPropertyPages
{
public:
	// Ctors
	CACTController();
	virtual ~CACTController();

	// *** IUnknown methods ***
	// Handled in ACTControllerGen.cpp
	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "ACTController"
	// Note: Skeleton methods are provided in ACTController.cpp.
	// Basic dev caps
	STDMETHODIMP GetStatusText( LPSTR pszStatus, DWORD* pdwLen );
	STDMETHODIMP MidiInShortMsg( DWORD dwShortMsg );
	STDMETHODIMP MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );
	STDMETHODIMP RefreshSurface( DWORD fdwRefresh, DWORD dwCookie );
	STDMETHODIMP GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );
	// (ACTControllerGen.cpp)
	STDMETHODIMP Connect( IUnknown* pUnknown, HWND hwndApp );
	STDMETHODIMP Disconnect();

	//		IControlSurface2 impl
	// BD 2006: No surface I could fine actually implements this.  SONAR never asks for this.  Why the
	// ^%^%$# was it left in the IDL!?   Well it's published so here it stands
	STDMETHODIMP_(HRESULT)	GetVersion(ULONG *,ULONG *,ULONG *,ULONG *) {return E_NOTIMPL;}

	// IControlSurface3 impl
	STDMETHODIMP_(HRESULT)	GetNoEchoStatusMessages(WORD **,DWORD *);

	// *** ISurfaceParamMapping ***
	STDMETHODIMP GetStripRangeCount( DWORD* pdwCount );
	STDMETHODIMP GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip );
	STDMETHODIMP SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip );
	// access to mapped controls
	STDMETHODIMP GetDynamicControlCount( DWORD* pdwCount );
	STDMETHODIMP GetDynamicControlInfo( DWORD dwIndex, DWORD* pdwKey, SURFACE_CONTROL_TYPE* pcontrolType );
	STDMETHODIMP SetLearnState( BOOL bLearning );

	// *** IPersistStream methods ***
	STDMETHODIMP Load( IStream* pStm );
	STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty );
	STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize );
	// (ACTControllerGen.cpp)
	STDMETHODIMP GetClassID( CLSID* pClsid );
	STDMETHODIMP IsDirty();

	// *** ISpecifyPropertyPages methods (ACTControllerGen.cpp) ***
	STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods (ACTControllerGen.cpp) ***
	STDMETHODIMP GetTypeInfoCount( UINT* );
	STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );

	// ACTControllerPublic.cpp
	void				RestoreDefaultBindings(bool bUpdateBindings =false);
	void				ResetMidiLearn();

	SONAR_MIXER_STRIP	GetStripType();
	void				SetStripType(SONAR_MIXER_STRIP eStripType);

	vectorDwordCStringPairs *GetKnobBindings();
	vectorDwordCStringPairs *GetSliderBindings();

	DWORD				GetKnobsBinding(int iBank);
	void				SetKnobsBinding(int iBank, DWORD dwBinding);

	DWORD				GetSlidersBinding(int iBank);
	void				SetSlidersBinding(int iBank, DWORD dwBinding);
	
	vectorDwordCStringPairs *GetButtonNames();
	vectorDwordCStringPairs	*GetButtonActions();
	vectorDwordCStringPairs	*GetBankNames();

	DWORD				GetButtonAction(int iBank, VirtualButton bButton);
	void				SetButtonAction(int iBank, VirtualButton bButton, DWORD dwAction);

	bool				GetButtonExcludeACT(int iBank, VirtualButton bButton);
	void				SetButtonExcludeACT(int iBank, VirtualButton bButton, bool bExclude);

	void				SetButtonLabel(VirtualButton bButton, CString strText);
	void				GetButtonLabel(VirtualButton bButton, CString *strText);
	void				GetButtonName(VirtualButton bButton, CString *strText);
	void				GetButtonValue(VirtualButton bButton, CString *strText);

	void				SetRotaryLabel(BYTE bKnob, CString strText);
	void				GetRotaryLabel(BYTE bKnob, CString *strText);
	void				GetRotaryName(BYTE bKnob, CString *strText);
	void				GetRotaryValue(BYTE bKnob, CString *strText);

	void				SetSliderLabel(BYTE bSlider, CString strText);
	void				GetSliderLabel(BYTE bSlider, CString *strText);
	void				GetSliderName(BYTE bSlider, CString *strText);
	void				GetSliderValue(BYTE bSlider, CString *strText);

	bool				SupportsDynamicMappings();
	bool				GetUseDynamicMappings();
	void				SetUseDynamicMappings(bool b);
	bool				GetLockDynamicMappings();
	void				SetLockDynamicMappings(bool b);
	bool				GetLearnDynamicMappings();
	void				SetLearnDynamicMappings(bool b);
	void				GetDynamicMappingName(CString *strName);

	DWORD				GetUpdateCount();

	bool				GetButtonACTMode(int iBank, VirtualButton bButton);
	bool				GetRotariesACTMode(int iBank);
	bool				GetSlidersACTMode(int iBank);

	bool				GetExcludeRotariesACT(int iBank);
	void				SetExcludeRotariesACT(int iBank, bool b);
	bool				GetExcludeSlidersACT(int iBank);
	void				SetExcludeSlidersACT(int iBank, bool b);

	CMixParam::CaptureType GetRotaryCaptureType( int iBank ) const { return m_aCTRotary[iBank];}
	CMixParam::CaptureType GetSliderCaptureType(int iBank) const { return m_aCTSlider[iBank];}
	void SetRotaryCaptureType(int iBank, CMixParam::CaptureType ct);
	void SetSliderCaptureType(int iBank, CMixParam::CaptureType ct);

	void				ActivateProps( BOOL b );
	void				ToggleProps();
	void				ActivateCurrentFx( SONAR_UI_ACTION uia );
	void				ActivateNextFx( SONAR_UI_ACTION uia );
	void				ActivatePrevFx( SONAR_UI_ACTION uia );

	AssignmentMode		GetRotariesMode();
	void				SetRotariesMode(AssignmentMode eMode);

	bool				GetSelectHighlightsTrack();
	void				SetSelectHighlightsTrack(bool b);

	bool				GetACTFollowsContext();
	void				SetACTFollowsContext(bool b);

	void				MidiLearnRotary(int n);
	void				MidiLearnSlider(int n);
	void				MidiLearnButton(int n);
	void				MidiLearnShift();

	int					GetRotaryBank();
	void				SetRotaryBank(int iBank);
	int					GetSliderBank();
	void				SetSliderBank(int iBank);
	int					GetButtonBank();
	void				SetButtonBank(int iBank);

	void				GetComments(CString *strComments);
	void				SetComments(CString strComments);
	void				EndMidiLearn();

	// MIDI Binding attributes
	CMidiBinding::MessageInterpretation GetRotaryMessageInterpretation( int n );
	CMidiBinding::MessageInterpretation GetSliderMessageInterpretation( int n );
	void				SetRotaryMidiInterpretation( int n, CMidiBinding::MessageInterpretation mi );
	void				SetSliderMidiInterpretation( int n, CMidiBinding::MessageInterpretation mi );
	void				SetRotaryIncrementHingeValue( int n, DWORD dwHinge );
	void				SetSliderIncrementHingeValue( int n, DWORD dwHinge );
	DWORD				GetRotaryIncrementHingeValue( int n );
	DWORD				GetSliderIncrementHingeValue( int n );
	void				SetRotaryIncrementUseAccel( int n, bool b );
	void				SetSliderIncrementUseAccel( int n, bool b );
	bool				GetRotaryIncrementUseAccel( int n );
	bool				GetSliderIncrementUseAccel( int n );
	void				SetRotaryMessage( int n, WORD w );
	void				SetSliderMessage( int n, WORD w );
	WORD				GetRotaryMessage( int n );
	WORD				GetSliderMessage( int n );

	void				SetButtonMessage( int n, WORD w );
	WORD				GetButtonMessage( int n );
	void				GetButtonSysex( int n, std::vector<BYTE>* pv );
	void				SetButtonSysex( int n, std::vector<BYTE>& v );

	void				SendInitMessages();
	bool				InitSent() const { return m_bInitSent; }
	void				XFerInitShortMsgs( std::vector<DWORD>* pv, bool bFromUI );
	void				XferInitLongMsgs( std::vector<BYTE>* pv, bool bFromUI );
	bool				HasInitMessage() const { return !m_vdwInitShortMsg.empty() || !m_vdwInitSysexMsg.empty(); } 

protected:
	// ACTControllerRefresh.cpp
	void OnRefreshSurface(DWORD fdwRefresh);
	void UpdateToolbarText();
	void UpdateBindings();

	// ACTControllerRx.cpp
	void OnShortMidiIn(BYTE bStatus, BYTE bD1, BYTE bD2);
	void OnKnob(BYTE bKnob, BYTE bVal);
	void OnSlider(BYTE bSlider, BYTE bVal);
	void OnButton(BYTE bButton, BYTE bVal);

	// ACTControllerPersist.cpp
	HRESULT Persist(IStream* pStm, bool bSave);
	HRESULT PersistBank(IStream* pStm, bool bSave, int iBank);
	HRESULT Persist(IStream* pStm, bool bSave, CString *pStr);
	HRESULT Persist(IStream* pStm, bool bSave, void *pData, ULONG ulCount);

	ISonarMidiOut*			m_pMidiOut;
	ISonarKeyboard*		m_pKeyboard;
	ISonarCommands*		m_pCommands;
	ISonarProject*			m_pProject;
	ISonarMixer*			m_pMixer;
	ISonarTransport*		m_pTransport;
	ISonarIdentity*		m_pSonarIdentity;
	ISonarParamMapping*	m_pSonarParamMapping;
	ISonarMixer2*			m_pSonarMixer2;
	ISonarUIContext2*		m_pSonarUIContext;
	
	HWND				m_hwndApp;
	
	void				onCaptureModeChange();
	void				releaseSonarInterfaces();
	void				OnConnect();
	void				OnDisconnect();

	void				OnContextSwitch();
	SONAR_UI_CONTEXT	GetCurrentContext();
	void				DoCommand(DWORD dwCmdID);
	DWORD				GetStripCount(SONAR_MIXER_STRIP eMixerStrip);
	void				BindNthParam(CMixParam *pParam, SONAR_MIXER_STRIP eMixerStrip,
									DWORD dwStripNum, int n);
	void				BindParamToBinding(CMixParam *pParam, SONAR_MIXER_STRIP eMixerStrip,
									DWORD dwStripNum, DWORD dwBinding);
	void				BindToACT(CMixParam *pParam, DWORD dwStripNum); 
	void				BuildDynControlsList();
	void				AddItem(vectorDwordCStringPairs *vPair, UINT nID, DWORD num);
	void				AddItem(vectorDwordCStringPairs *vPair, const char *str, DWORD num);

	void				ShiftStripNum(int iShift, bool bForce =false);
	void				LimitAndSetStripNum(int iStripNum, bool bForce =false);
	DWORD				GetStripNum();
	void				SetStripNum(DWORD dwStripNum);

	void				ShiftStripType(int iShift);

	void				ShiftSelectedTrack(int iShift, bool bForce =false);
	void				LimitAndSetSelectedTrack(int iSelectedTrack, bool bForce =false);
	DWORD				GetSelectedTrack();
	void				SetSelectedTrack(DWORD dwStripNum);
	void				ToggleSelectedTrackParam(SONAR_MIXER_PARAM eMixerParam);
	bool				GetSelectedTrackParam(SONAR_MIXER_PARAM eMixerParam);
	void				ToggleSelectedTrackAutomationRead();
	bool				GetSelectedTrackAutomationRead();
	void				ToggleSelectedTrackAutomationWrite();
	bool				GetSelectedTrackAutomationWrite();

	void				GetStripNameAndParamLabel(CMixParam *pParam, CString *strText);
	void				UpdateButtonActionStrings();
	void				UpdateButtonActionString(int iBank, VirtualButton bButton);

	DWORD				GetParamSelected();
	bool				IsMIDI(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum);
	bool				GetTransportState(SONAR_TRANSPORT_STATE eState);


// Implementation
private:
	LONG				m_cRef;
	BOOL				m_bDirty;
	DWORD				m_dwSurfaceId;

	CRITICAL_SECTION	m_cs;		// critical section to guard properties
	CRITICAL_SECTION	m_dyn_cs;

	bool				m_bConnected;

	CMixParam		m_cParamUtil;		// re-use to get/set params


	SONAR_MIXER_STRIP	m_eStripType;
	DWORD				m_dwStripNum[NUM_STRIP_TYPES];
	DWORD				m_dwSelectedTrack[NUM_STRIP_TYPES];
	AssignmentMode		m_eRotariesMode;
	bool				m_bUseDynamicMappings;
	bool				m_bSelectHighlightsTrack;
	bool				m_bACTFollowsContext;
	bool				m_bExcludeRotariesACT[NUM_BANKS];
	bool				m_bExcludeSlidersACT[NUM_BANKS];

	DWORD				m_dwUpdateCount;

	DWORD				m_dwKnobsBinding[NUM_BANKS];
	DWORD				m_dwSlidersBinding[NUM_BANKS];

	CString				m_strToolbarText;

	DWORD				m_dwNumTracks;
	DWORD				m_dwNumBuses;
	DWORD				m_dwNumMains;
	SONAR_UI_CONTEXT	m_uiContext;

	CString				m_strRotaryLabel[NUM_KNOBS];
	CString				m_strSliderLabel[NUM_SLIDERS];
	CString				m_strButtonLabel[NUM_VIRTUAL_BUTTONS];

	CMidiBinding		m_cMidiKnob[NUM_KNOBS];
	CMidiBinding		m_cMidiSlider[NUM_SLIDERS];
	CMidiBinding		m_cMidiButton[NUM_BUTTONS];
	CMidiBinding		m_cMidiModifierDown;
	CMidiBinding		m_cMidiModifierUp;
	bool				m_bModifierIsDown;
	CMidiBinding		*m_pMidiLearnTarget;
	std::set<BYTE>		m_setLastReceivedLearnValue;

	CMixParam			m_SwKnob[NUM_BANKS][NUM_KNOBS];
	CMixParam			m_SwSlider[NUM_BANKS][NUM_SLIDERS];
	CMixParam			m_SwButton[NUM_BANKS][NUM_VIRTUAL_BUTTONS];

	DWORD				m_dwButtonAction[NUM_BANKS][NUM_VIRTUAL_BUTTONS];
	CString				m_strButtonAction[NUM_BANKS][NUM_VIRTUAL_BUTTONS];
	bool				m_bButtonExcludeACT[NUM_BANKS][NUM_VIRTUAL_BUTTONS];

	int					m_iRotaryBank;
	int					m_iSliderBank;
	int					m_iButtonBank;

	CString				m_strComments;

	vectorDwordCStringPairs m_vKnobBindings;
	vectorDwordCStringPairs m_vSliderBindings;
	vectorDwordCStringPairs m_vButtonNames;
	vectorDwordCStringPairs m_vButtonActions;
	vectorDwordCStringPairs m_vBankNames;

	struct DYNCONTROL
	{
		DYNCONTROL(DWORD dw, SURFACE_CONTROL_TYPE sct) : dwKey(dw), sctType(sct){}
		DWORD dwKey;
		SURFACE_CONTROL_TYPE sctType;
	};

	std::vector<DYNCONTROL>	m_vDynControls;

	CString				m_strMidiLearn;
	CString				m_strOn;
	CString				m_strOff;
	CString				m_strMultiChannel;
	CString				m_strChannelStrip;
	CString				m_strStripParameters;

	CMixParam::CaptureType	m_aCTRotary[NUM_BANKS];
	CMixParam::CaptureType	m_aCTSlider[NUM_BANKS];

	std::vector<DWORD>	m_vdwInitShortMsg;
	std::vector<BYTE>		m_vdwInitSysexMsg;
	bool						m_bInitSent;
};


/////////////////////////////////////////////////////////////////////////////
// CCriticalSectionAuto

class CCriticalSectionAuto
{
public:
	CCriticalSectionAuto( CRITICAL_SECTION* pcs ) : m_pcs(pcs)
	{
		::EnterCriticalSection( m_pcs );
	}
	virtual ~CCriticalSectionAuto()
	{
		::LeaveCriticalSection( m_pcs );
	}
private:
	CRITICAL_SECTION* m_pcs;
};

/////////////////////////////////////////////////////////////////////////////
// CACTControllerApp
// See ACTControllerGen.cpp for the implementation of this class
// This class should need to be modified when creating your plugin
//

class CACTControllerApp : public CWinApp
{
public:
	CACTControllerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CACTControllerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CACTControllerApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
