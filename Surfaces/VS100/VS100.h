// VS100.h : main header file for the VS100 DLL
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
#include "surface.h"			// surface base class
#include "sfkMidi.h"

extern const GUID CLSID_VS100;
extern const GUID CLSID_VS100PropPage;
extern const GUID LICID_VS100;


class CMixParam;
class CLCDTextWriter;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const TCHAR s_szFriendlyName[] = _T("VS-100 (DEBUG)");
static const TCHAR s_szFriendlyNamePropPage[] = _T("VS-100 Property Page (DEBUG)");
#else
static const TCHAR s_szFriendlyName[] = _T("VS-100");
static const TCHAR s_szFriendlyNamePropPage[] = _T("VS-100 Property Page");
#endif

/////////////////////////////////////////////////////////////////////////////
extern long g_lComponents;	// Count of active components
extern long g_lServerLocks;	// Count of locks

/////////////////////////////////////////////////////////////////////////////

#define MASK_COMMAND						0x80000000U

// NOTE: this values are saved in the presets so should not be changed, only added to
#define CMD_NONE							(MASK_COMMAND | 0)
#define CMD_ACT_ENABLE					(MASK_COMMAND | 1)
#define CMD_ACT_LOCK						(MASK_COMMAND | 2)
#define CMD_ACT_LEARN					(MASK_COMMAND | 15)
#define CMD_SURFACE_PROPS_TOGGLE		(MASK_COMMAND | 28)
#define CMD_SURFACE_PROPS_SHOW		(MASK_COMMAND | 29)
#define CMD_SURFACE_PROPS_HIDE		(MASK_COMMAND | 30)
#define CMD_OPEN_CUR_FX					(MASK_COMMAND | 31)
#define CMD_CLOSE_CUR_FX				(MASK_COMMAND | 32)
#define CMD_FOCUS_NEXT_FX				(MASK_COMMAND | 33)
#define CMD_FOCUS_PREV_FX				(MASK_COMMAND | 34)
#define CMD_OPEN_NEXT_FX				(MASK_COMMAND | 35)
#define CMD_OPEN_PREV_FX				(MASK_COMMAND | 36)

#if 0 // Now in commandids.h

// More host commands not included in CommandIDs.h
enum {
	CMD_CLIP_MUTE = 299,
	CMD_TRACK_FREEZE= 302,
	CMD_TRACK_UNFREEZE = 304,
	CMD_TRACK_QUICK_UNFREEZE = 303,
	CMD_TRACK_FREEZE_OPTIONS = 689,

	CMD_VIEW_SURROUND_PANNER = 309,


	CMD_TRACK_SHOW_LAYERS = 310,
	CMD_TRACK_COMPACT_LAYERS = 629,
	CMD_TRACK_REBUILD_LAYERS = 628,

	CMD_PROCESS_FADE_SELECTED = 562,

	CMD_TRACK_INLINE_PRV = 563,

	CMD_NUDGE_LEFT1 = 546,
	CMD_NUDGE_RIGHT1 = 548,
	CMD_NUDGE_LEFT2 = 549,
	CMD_NUDGE_RIGHT2 = 551,
	CMD_NUDGE_LEFT3 = 552,
	CMD_NUDGE_RIGHT3 = 554,
	CMD_NUDGE_UP = 553,
	CMD_NUDGE_DOWN = 547,
	CMD_NUDGE_SETTINGS = 550,

	CMD_AUTOPUNCH_TOGGLE = 544,

	CMD_TRACK_VIEW	= 635,
	CMD_DIM_SOLO = 730,
	CMD_EXC_SOLO = 785,

	CMD_ZOOM_UNDO = 508,
	CMD_ZOOM_REDO = 509,

	CMD_FIT_PROJECT = 494,

	CMD_SNAP_TIME = 501,

	CMD_AUDIOSNAP_PALETTE = 666,
};

#endif

class CVS100;



class CTimerClientWithID : public CTimerClient
{
public:
	// CTimerClient
	virtual void	Tick();

	CTimerClientWithID( CVS100* pSurface, DWORD dwID ) : m_pSurface( pSurface ),m_id(dwID),m_bActive(false) {}
	void SetActive( bool b ) { m_bActive = b; }
	bool IsActive() const { return m_bActive; }

protected:
	CVS100*	m_pSurface;

private:
	DWORD m_id;
	bool m_bActive;
};

class CKeyboardRepTimerClient : public CTimerClientWithID
{
public:
	CKeyboardRepTimerClient( CVS100* pSurface, DWORD dwID ) : 
      CTimerClientWithID(pSurface,dwID),
      m_cTicks(0),
      m_dwKey( 0 )
   {
   }
	void SetButton( DWORD dwKey ) {m_dwKey = dwKey; m_cTicks = 0; }
	DWORD GetButton() const { return m_dwKey; }
	unsigned GetTicks() const { return ++m_cTicks; }
private:
	mutable unsigned	m_cTicks;
	DWORD m_dwKey;
};


/////////////////////////////////////////////////////////////////////////////
// CVS100



class CVS100 :
	public CControlSurface
{
public:
	// Ctors
	CVS100();
	virtual ~CVS100();

	// IControlSurface
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );
	virtual HRESULT STDMETHODCALLTYPE GetStatusText( LPSTR pszStatus, DWORD* pdwLen );

	// IControlSurface3
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoStatusMessages(WORD**, DWORD* );

	// ISurfaceParamMapping overrides
	virtual HRESULT STDMETHODCALLTYPE SetStripRange(DWORD,SONAR_MIXER_STRIP);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *);
	virtual HRESULT STDMETHODCALLTYPE SetLearnState(BOOL);
	virtual HRESULT STDMETHODCALLTYPE  GetStripRangeCount( DWORD* pdwCount );
	virtual HRESULT STDMETHODCALLTYPE GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip );

	// ISpecifyPropertyPages overrides
	virtual HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages );

	// IPersistStream overrides
	virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );
	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(_ULARGE_INTEGER * pcbSize);


	////////////////////////////////////////////
	// Callbacks for CMidiMessage
	HRESULT	OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue, CMidiMsg::ValueChange vtRet );
	////////////////////////////////////////////

	// CTimerClient
	void	Tick( DWORD dwID );

	void SetTimeFormat( MFX_TIME_FORMAT fmt );
	MFX_TIME_FORMAT GetTimeFormat() { return m_mfxfPrimary; }

	void 					Stop();
	void 					Play();
	void					Pause( bool b );
	bool 					IsPlaying() { return GetTransportState( TRANSPORT_STATE_PLAY ); }
	void 					Record( bool bToggle );
	bool 					IsRecording() { return GetTransportState( TRANSPORT_STATE_REC ); }
	void					GotoNextMarker();
	void					GotoPrevMarker();



	struct EncoderParam
	{
		SONAR_MIXER_PARAM param;
		DWORD					dwParam;
	};

	std::vector<EncoderParam>	GetRotaryAssignParam() const { return m_vEncoderParam; }
	void					AssignRotaries( size_t ixEncoderParam );

	void					OnFlip();
	bool					IsFlipped();

	void 					VZoom( int n );
	void 					HZoom( int n );

	// UI Accesors for the Prop page
	bool 					IsFirstLoaded() { bool b = m_bLoaded; m_bLoaded = false; return b; }
	bool 					GetACTMode() const { return m_bACTMode; }
	void 					SetACTMode(bool b) { m_bACTMode = b; }
	CString				GetACTContextName();
	CString				GetACTParamLabel( DWORD id );
	bool					GetMotorEnabled() const { return m_bEnableMotors; }
	void					SetMotorEnabled( bool b) { m_bEnableMotors = b; }
	bool					GetFullAssignMode() const { return m_bFullAssign; }
	
	enum RefreshMod 
	{
		RM_EVERY	= 0,	// every refresh
		RM_EVEN,			// only even refreshes
		RM_ODD,			// only odd refreshes
	};


	// enums for all the parameter-less state buttons
	enum ControlId
	{
		CID_Prev,
		CID_Next,
		CID_Shift,
		CID_RotaryAssign,
		CID_Play,
		CID_Stop,
		CID_Record,
		CID_RTZ,
		CID_GTE,
		CID_Rew,
		CID_FF,
		CID_ACT,		// the dynamics button.  We'll use for ACT mode
		CID_View,
		CID_Marker,
		CID_Loop,
		CID_Fader,
		CID_StripRotary,
		CID_Mute,
		CID_Solo,
		CID_Arm,
		CID_Feetswitch1,
		CID_Feetswitch2,
		CID_Rotary0,	// eq/action section encoders
		CID_Rotary1,
		CID_Rotary2,
		CID_Comp,
		CID_ValueEnc,
		CID_ValueEncPush,
		CID_MixerPan0,
		CID_MixerPan1,
		CID_MixerVol0,
		CID_MixerVol1,
		CID_MixerVol2,
		CID_MixerVol3,
		CID_MixerVol4,
		CID_MixerVol5,
		CID_MixerSw0,
		CID_MixerSw1,
		CID_MixerSw2,
		CID_MixerSw3,
		CID_FullAssign,
	};


	enum ModifierKey
	{
		SMK_SHIFT		= 0x1,
		SMK_CTRL			= 0x2,
		SMK_ALT			= 0x4,
	};

	struct ButtonActionDefinition
	{
		ButtonActionDefinition( ) : 
				dwCommandOrKey(0), 
				transportState(TRANSPORT_STATE_PLAY),
				eActionType(AT_Command),
				wModKeys(0){}
		DWORD		dwCommandOrKey;
		SONAR_TRANSPORT_STATE transportState;
		enum ActionType {AT_Command, AT_Key, AT_Transport } eActionType;
		WORD		wModKeys;	// use ModifierKey enums
	};

	typedef std::map<ControlId,ButtonActionDefinition>	MsgIdToBADMap;
	typedef MsgIdToBADMap::iterator	MsgIdToBADMapIterator;

	MsgIdToBADMap& GetButtonCommandMap() { return m_mapButton2BAD; }


protected:
	HRESULT 			onRefreshSurface(DWORD fdwRefresh, DWORD dwCookie );
	HRESULT			onFirstRefresh();
	HRESULT			refreshLEDs();
	HRESULT			refreshLCDs();

	void				requestFullAssignState();
	CMidiMsg			m_FullAssignReqMidiMsg;

	enum JogAmount	{ JA_1 = 0, JA_2, JA_3, JA_4 };
	enum JogDirection { JD_CCW, JD_CW };

	HRESULT 			persist(IStream* pStm, bool bSave);
	void				onConnect();
	void				onDisconnect();
	HRESULT			updateParamBindings();
	void				doSelectStrip( int ixStrip );
	void				onJog( CMidiMsg* pObj, float f );
	void				onRotaryTrackSel( CMidiMsg* pObj, float f );
	void				jog( JogDirection jd, JogAmount ja );

	DWORD				getCurrentMarker();

	HRESULT			buildBindings();
	HRESULT			buildStripBindings();
	HRESULT			buildMiscBindings();
	HRESULT			buildTransportBindings();

	CMidiMsg*		createMidiMsg( LPCTSTR sz = 0, DWORD dwId = (DWORD)-1 );
	CMixParam*		createMixParam();
	void				destroyMidiMsg( CMidiMsg*& pmsg );
	void				onActToggle();
	void				onStripType( SONAR_MIXER_STRIP e );
	void				onShift();

	void				onJogMode( bool b );
	bool				m_bLoaded;
	SONAR_MIXER_STRIP	m_eStripType;

	enum SpecialRotaryMode{ SRM_Normal, SRM_Jog, SRM_TrackSel };

	SpecialRotaryMode	m_eRotaryMode;

	bool				m_bACTMode;
	bool				m_bFullAssign;
	bool				m_bShiftPressed;
	DWORD				m_dwMessageIdSeed;
	bool				m_bEnableMotors;

	std::vector<EncoderParam>		m_vEncoderParam;
	size_t			m_ixEncoderParam;

	MsgIdToBADMap	m_mapButton2BAD;

	// a struct for a Param and a Midi output message
	struct PMBINDING
	{
		PMBINDING() : pParam(0),pMsgOut(0),nRefreshMod(RM_EVERY){}
		int			nRefreshMod;
		CMixParam*	pParam;
		CMidiMsg*	pMsgOut;	// use if there is a unique output message
	};


	typedef std::map<CMidiMsg*,PMBINDING>	InputBindingMap;
	typedef InputBindingMap::iterator		InputBindingIterator;

	InputBindingMap	m_mapStripMsgs;
	InputBindingMap	m_mapShiftMsgs;
	InputBindingMap	m_mapFaderTouchMsgs;
	InputBindingMap	m_mapACT;

	CMidiMsg::ValueRange	m_vrCW;
	CMidiMsg::ValueRange m_vrCCW;

	typedef std::set<CMixParam*>				MixParamSet;
	typedef MixParamSet::iterator				MixParamSetIterator;
	MixParamSet			m_setEveryMixparam;	// used for memory management/cleanup

	typedef std::set<CMidiMsg*>	MidiMsgSet;
	typedef MidiMsgSet::iterator	MidiMsgSetIterator;
	MidiMsgSet						m_setEveryMidiMsg;	// Used for memory management / cleanup


	///////////////////////////////////////////////////////
	// LCD methods
	enum ParamShowMode { PSM_Refresh, PSM_Touch, PSM_Revert };

	void initLCDs();
	void showParam( CMixParam* pParam, int ixLcd, ParamShowMode eShowMode );
	void showParam( CMixParam* pParam, ParamShowMode eShowMode );
	void showEQParams( ParamShowMode eShowMode );
	void showACTParamLabel( CMixParam* p );
	void showEncoderParam( CMixParam* p );

	typedef std::map<CMixParam*,DWORD>	ParamDisplayTimeStampMap;
	ParamDisplayTimeStampMap		m_mapParamDisplayTimeStamp;

	CLCDTextWriter*		m_pLCD;


	//////////////////////////////////////////////////////
	// Timer stuff
	enum TimerState
	{
		TS_ParamChanged,	// param has changed.
		TS_Blink,
		TS_KeyRep,
		TS_Revert,
		TS_None,
	};

	enum BlinkState{
		BS_None,
		BS_On,
		BS_Off }
	m_eBlinkState;

	void setTimer( CTimerClientWithID&, WORD dwMs );
	void killTimer( CTimerClientWithID& );

	CTimerClientWithID	m_tcbParamChange;
	CTimerClientWithID	m_tctBlink;
	CTimerClientWithID	m_tctRevert;
	CKeyboardRepTimerClient	m_tctKeyRep;

	void onKeyRep();

	void onBlink();
	void startBlink( ControlId id );
	void stopBlink( ControlId id );
	void stopBlink( );

	void requestRevert( CMixParam* p );
	void onRevert();

	std::set<ControlId>	m_setBlinkControls;
	MixParamSet				m_setFineControls;
	MixParamSet				m_setRevertParams;

	typedef std::map<ControlId,CMidiMsg*>	ControlMessageMap;
	typedef ControlMessageMap::iterator		ControlMessageMapIterator;

	ControlMessageMap	m_mapControlMsgs;		// param-less button messages

	void	addControlMsg( CMidiMsg* );
	bool	handleButtonMsg( CMidiMsg*, float );
	bool	handleBankButton( CMidiMsg*, float );
	bool	handleTransportButton( CMidiMsg* pmsg, float );
	bool	handleCommandOrKey( CMidiMsg* pmsg, float );
	bool	handleModeButton( CMidiMsg*, float );
	bool	handleShiftStripFunctions( CMidiMsg* pmsg, float );

	void	updateParamStateOnLCD(){}

	DWORD	getActiveStrip();


	// command map
	typedef std::map<ControlId,DWORD>			MsgIdToCommandIDMap;
	typedef MsgIdToCommandIDMap::iterator	MsgIdToCommandIDMapIterator;

	MsgIdToCommandIDMap	m_mapButton2Command;

private:
	BOOL				m_bDirty;

	CSFKCriticalSection	m_cs;		// critical section to guard properties

	MFX_TIME_FORMAT				m_mfxfPrimary;		// TimeFormat for Cursor / In/Out times

};

/////////////////////////////////////////////////////////////////
class CVS100App : public CWinApp
{
public:
	CVS100App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVS100App)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CVS100App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CVS100App theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
