// TacomaSurface.h : main header file for the TacomaSurface DLL
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
#include "IOBoxInterface.h"
#include <deque>


extern const GUID CLSID_TacomaSurface;
extern const GUID CLSID_TacomaSurfacePropPage;
extern const GUID LIBID_TacomaSurface;

#define WND_MOVE_INERTIA	0.10f

class CMixParam;
class CIOBoxInterface;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const TCHAR s_szFriendlyName[] = _T("VS-700 (DEBUG)");
static const TCHAR s_szFriendlyNamePropPage[] = _T("VS-700 Property Page (DEBUG)");
#else
static const TCHAR s_szFriendlyName[] = _T("VS-700");
static const TCHAR s_szFriendlyNamePropPage[] = _T("VS-700 Property Page");
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

// More host commands not included in CommandIDs.h
enum {
	CMD_CLIP_MUTE = 299,

	CMD_TRACK_FREEZE= 302,
	CMD_TRACK_UNFREEZE = 304,
	CMD_TRACK_QUICK_UNFREEZE = 303,

	CMD_SYNTH_FREEZE = 305,
	CMD_SYNTH_UNFREEZE = 307,
	CMD_SYNTH_QUICK_UNFREEZE = 306,

	CMD_TRACK_FREEZE_OPTIONS = 689,

	CMD_VIEW_SURROUND_PANNER = 309,


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

	CMD_AUTOPUNCH_TOGGLE = 544,

	CMD_TRACK_VIEW	= 635,
	CMD_DIM_SOLO = 730,
	CMD_EXC_SOLO = 785,


	CMD_AUDIOSNAP_PALETTE = 666,

};


enum JogAmount	{ JA_1 = 0, JA_2, JA_3, JA_4 };
enum JogDirection { JD_CCW, JD_CW };

struct STRIP_INFO
{
   SONAR_MIXER_STRIP stripType;
   DWORD             stripIndex;
   DWORD             stripIndexPhysical; // Hardware index (0 thru 8)
   DWORD             stripOffset;
   BOOL              isStripLocked;

   STRIP_INFO()
   {
      clear();
   };

   void clear()
   {
      stripType          = MIX_STRIP_TRACK;
      stripIndex         = 0;
      stripIndexPhysical = 0;
      stripOffset        = 0;
      isStripLocked      = FALSE;
   };
};

#define BID_IN( _bid, _bidMin, _bidMax ) (_bid >= _bidMin && _bid <= _bidMax )

class CTimerClientWithID : public CTimerClient
{
public:
	// CTimerClient
	virtual void	Tick();

	CTimerClientWithID( CTacomaSurface* pSurface, DWORD dwID ) : m_pSurface( pSurface ),m_id(dwID),m_bActive(false) {}
	void SetActive( bool b ) { m_bActive = b; }
	bool IsActive() const { return m_bActive; }

protected:
	CTacomaSurface*	m_pSurface;

private:
	DWORD m_id;
	bool m_bActive;
};

//class CDisplayEE;

class CKeyboardRepTimerClient : public CTimerClientWithID
{
public:
	CKeyboardRepTimerClient( CTacomaSurface* pSurface, DWORD dwID ) : 
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


//--------------------------------------------------------
// A timer client to cause repeated values from shuttle ring
class CShuttleRingTimer : public CTimerClientWithID
{
public:
	CShuttleRingTimer( CTacomaSurface* p ): CTimerClientWithID(p, 0),m_f01Magnitude(0.f),m_jd(JD_CW) {}
	virtual void	Tick();
	void Set( float f01, JogDirection jd ){ m_f01Magnitude = f01; m_jd = jd; }

private:
	float		m_f01Magnitude;
	JogDirection	m_jd;
};


#define TBAR_DYN_INDEX			16
#define ROTARY_DYN_INDEX		12
#define ROTARY_DYN_COUNT		12
#define SWITCH_DYN_COUNT		4
#define TOTAL_DYN_COUNT			( ROTARY_DYN_COUNT + SWITCH_DYN_COUNT + 1 ) // +1 = Tbar



/////////////////////////////////////////////////////////////////////////////
// CTacomaSurface

class CTacomaSurface :
	public CControlSurface, 
	public ITacomaIOBox
{
public:
	
	friend class CTimerClientWithID;
	friend class CKeyboardRepTimerClient;
	friend class CShuttleRingTimer;

	// Ctors
	CTacomaSurface();
	virtual ~CTacomaSurface();

	// *** IUnknown methods ***
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppv );


	// IControlSurface
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );

	virtual HRESULT STDMETHODCALLTYPE GetStatusText(LPSTR pszStatus,DWORD* pdwLen);

	// IControlSurface3
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoStatusMessages(WORD**, DWORD* );

	// ISurfaceParamMapping overrides
	virtual HRESULT STDMETHODCALLTYPE GetStripRangeCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetStripRange(DWORD,DWORD *,DWORD *,SONAR_MIXER_STRIP *);
	virtual HRESULT STDMETHODCALLTYPE SetStripRange(DWORD,SONAR_MIXER_STRIP);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *);
	virtual HRESULT STDMETHODCALLTYPE SetLearnState(BOOL);

	// ISpecifyPropertyPages overrides
	virtual HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages );

	// IPersistStream overrides
	virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );
	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(_ULARGE_INTEGER * pcbSize);


	// ITacomaIOBox (impl in IOBoxInterface.cpp)
	HRESULT	STDMETHODCALLTYPE GetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float* pf01 );
	HRESULT STDMETHODCALLTYPE SetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float f01 );
	HRESULT STDMETHODCALLTYPE GetIOBoxParamName( TacomaIOBoxParam p, CString* pstr );
	HRESULT STDMETHODCALLTYPE GetIOBoxParamValueText( TacomaIOBoxParam p, DWORD dwIxChan, float f01, CString* pstr );
	HRESULT STDMETHODCALLTYPE RefreshFromHardware( );

	////////////////////////////////////////////
	// Callbacks for CMidiMessage
	HRESULT	OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue, CMidiMsg::ValueChange vc );
	////////////////////////////////////////////


	// CTimerClient
	void	Tick( DWORD dwID );

	void SetTimeFormat( MFX_TIME_FORMAT fmt );
	MFX_TIME_FORMAT GetTimeFormat() { return m_mfxfPrimary; }

	virtual HRESULT SetHostContextSwitch( );

	// SONAR X1d addition (maybe move to framework later)
	DWORD GetSelectedBus();
	void  SetSelectedBus( DWORD dwStripNum );
	SONAR_MIXER_STRIP GetStripFocus();
	void  SetStripFocus( SONAR_MIXER_STRIP type );

	// Various ENUMS for surface state
	enum ModifierKey
	{
		SMK_SHIFT		= 0x1,
		SMK_CTRL			= 0x2,
		SMK_ALT			= 0x4,
		SMK_COMMAND		= 0x8,
		SMK_LEFTMARKER	= 0x10, // hardly a modifier, but used as such to clear selection
	};

	enum KnobSectionMode
	{
		KSM_EQ = 0,
		KSM_SEND,
		KSM_ACT,
		KSM_PC_COMP,
		KSM_PC_EQ,
		KSM_PC_EQ_P2,
		KSM_PC_SAT,
		KSM_FLEXIBLE_PC // Resorderable
	};

	void					AssignRotaries( SONAR_MIXER_PARAM smp, DWORD dwParam, DWORD dwIndex );
	void					AssignChannelBranchRotaries();
	bool					IsFlipped() { return m_bFlipped; };
	void					OnFlip();
	void 					VZoom( int n );
	void 					HZoom( int n );

	// UI Accesors for the Prop page
	bool 					IsFirstLoaded() { bool b = m_bLoaded; m_bLoaded = false; return b; }
	KnobSectionMode			GetKnobSectionMode() const { return m_eActSectionMode; }
	void 					SetKnobSectionMode(KnobSectionMode e) { m_eActSectionMode = e; }
	bool					GetIOMode() const { return m_bIOMode; }
	void					SetIOMode( bool b );
	SONAR_MIXER_STRIP	Get8StripType() const { return m_e8StripType; }
	void					Set8StripType( SONAR_MIXER_STRIP eStrip );

	CString				GetACTParamLabel( DWORD id );
	CString				GetACTContextName();

	void					Jog( JogDirection jd, JogAmount ja );

	bool              ProChanLocked() const { return m_bProChanLock; }
	void              SetProChanLock( bool bLock = true ) { m_bProChanLock = bLock; }

	// access to IO box interface
	CIOBoxInterface*	GetIOBoxInterface() { return m_pIOBoxInterface; }

	enum RefreshMod 
	{
		RM_EVERY	= 0,	// every refresh
		RM_EVEN,			// only even refreshes
		RM_ODD,			// only odd refreshes
		RM_NEVER,		// don't
	};

protected:
   void				onProjectClose( void );
	void				onProjectOpen( void );

	bool				IsLegacyEQ() const { return m_bLegacyEq; }
	bool				IsFlexibleProChan() const { return m_bFlexiblePC; }

	HRESULT			onChannelBranchMode( bool bOn );
	HRESULT 			onRefreshSurface(DWORD fdwRefresh, DWORD dwCookie );
	HRESULT			refreshMarkers();
	HRESULT			refreshLayers();
	HRESULT			refreshPlugins( bool bForce = false );
	HRESULT 			persist(IStream* pStm, bool bSave);
	void				onConnect();
	void				onDisconnect();
	HRESULT			updateParamBindings();
	HRESULT			updateActSectionBindings( bool bUpdateDisplay );
   HRESULT			updateActContextName( bool bForceUpdate );
   HRESULT			updatePCContextName( bool bForceUpdate ); // [GP]
	HRESULT			updatePluginLEDs( void );

	BOOL				toggleStepCapture( void );
	void				toggleCapture( CMixParam *pParam );

	HRESULT			buildBindings();
	HRESULT			buildStripBindings();
	HRESULT			buildMiscBindings();
	HRESULT			buildTransportBindings();
	HRESULT			buildActSectionBindings();
	HRESULT			buildCommandBindings();
	HRESULT			buildKeyboardBindings();

	CMixParam*		createMixParam();
	CMidiMsg*		createMidiMsg( LPCTSTR sz = 0, DWORD dwId = (DWORD)-1 );
	void				destroyMidiMsg( CMidiMsg*& pmsg );

   void           setBankOffset(SONAR_MIXER_STRIP type, DWORD offset);
   DWORD          getBankOffset(SONAR_MIXER_STRIP type);
   DWORD          getCurrentBankOffset();

public:
	bool				m_bSwitchUI;

protected:
	typedef std::map<SONAR_MIXER_STRIP,DWORD>	BankOffsetMap;
	typedef BankOffsetMap::iterator		      BankOffsetIterator;
   BankOffsetMap  m_mapBankOffsets;

	SONAR_MIXER_STRIP	m_e8StripType; // This corresponds to "Fader View" on the surface
	DWORD				   m_dwMasterOrg;
	DWORD					m_dw8EncoderParam;

	// a struct for a Param and a Midi output message
	struct PMBINDING
	{
		PMBINDING() : pParam(0),pMsgOut(0),nRefreshMod(RM_EVERY){}
		CMixParam*	pParam;
		CMidiMsg*	pMsgOut;	// use if there is a unique output message
      int			nRefreshMod;
	};


	typedef std::map<CMidiMsg*,PMBINDING>	InputBindingMap;
	typedef InputBindingMap::iterator		InputBindingIterator;

	InputBindingMap					m_mapShiftMsgs;
	InputBindingMap					m_mapAltMsgs;
	InputBindingMap					m_mapStripMsgs;
	InputBindingMap					m_mapACTControls;
	InputBindingMap					m_mapSendControls;
	InputBindingMap					m_mapEQControls;
	InputBindingMap					m_mapPCEQControls;
	InputBindingMap					m_mapPCEQP2Controls;
	InputBindingMap					m_mapPCCompControls;
	InputBindingMap					m_mapPCSatControls;
	InputBindingMap					m_mapMiscMsgs;	// master fader, joystick, etc etc

	CMixParam							*m_pEqTypeParams[4];
	typedef std::set<CMixParam*>	MixParamSet;
	typedef MixParamSet::iterator	MixParamSetIterator;
	MixParamSet						m_setEveryMixparam;	// used for memory management/cleanup
	MixParamSet						m_setSonitusEQ;
	MixParamSet						m_setGlossEQ;

	typedef std::set<CMidiMsg*>	MidiMsgSet;
	typedef MidiMsgSet::iterator	MidiMsgSetIterator;
	MidiMsgSet						m_setEveryMidiMsg;	// Used for memory management / cleanup

	DWORD							m_dwMessageIdSeed;

	WORD							m_wModifierKey;		// the modifier key bits

	STRIP_INFO				   m_PCStrip;				// ProChannel Strip index for the context for the PC section
	STRIP_INFO				   m_selectedStrip;		// Strip index for the context for the ACT/EQ/Send section
	STRIP_INFO				   m_prevSelectedStrip;	// Strip index for the context for the ACT/EQ/Send section
   //STRIP_INFO info = getStripInfo( m_dwPhysicalStripSel );

	bool							m_bIOMode;			// in IO control mode
	KnobSectionMode			m_eActSectionMode;	// Which mode is the act/eq/send section in?
	WORD							m_wACTDisplayRow;	// which row in the act/eq/send section do we show params for?
	bool							m_bSidewaysChanStrip;
	WORD							m_wACTPage;			// which param page in act/eq/send section
   bool                    m_bProjectClosed;

	bool							m_bFlipped;
	enum {ACT_PAGE_NUM = 4};

	bool							m_bFlexiblePC;		// Whether pro-channel can be reordered
	bool							m_bLegacyEq;      // Non Producer or Expanded

public:

	// enums for all the parameter-less state buttons.
	// Do not change order of this list.  Only add to the end please
	// This is because we're persisting the user-assigned command bindings
	// which are keyed by ControlId
	enum ControlId
	{
		// "view"
		BID_EZr0c0, BID_EZr0c1, BID_EZr0c2, BID_EZr0c3, BID_EZr1c0, BID_EZr1c1, BID_EZr1c2, BID_EZr1c3,
		// "utility"
		BID_EZr2c0, BID_EZr2c1, BID_EZr2c2, BID_EZr2c3, BID_EZr3c0, BID_EZr3c1, BID_EZr3c2, BID_EZr3c3,

		BID_BankL, BID_BankR,
		BID_Flip,
		BID_RotaryAssign,

		// Rude MSR
		BID_RudeMute, BID_RudeSolo, BID_RudeArm,

		// transport
		BID_Play, BID_Stop, BID_Record, BID_FF, BID_Rew, BID_RTZ, BID_RTE,

		// cursor keys
		BID_KeyUp, BID_KeyDn, BID_KeyL, BID_KeyR,

		// Record/Edit section
		BID_LoopOn, BID_PunchOn, BID_MarkerIns, BID_Snap, BID_SetLoopPunch,

		// Project section
		BID_Save, BID_UndoRedo, BID_OK, BID_Cancel,

		// Edit Modes
		BID_Scroll, BID_DataSelect, BID_DataEdit,

		BID_TimeCodeMode,

		// modifiers
		BID_Shift, BID_Control, BID_Alt, BID_Command,

		// ACT / Eq / Send section
		BID_ACTDisplay, BID_Eq, BID_Send, BID_Act,
		BID_PageL, BID_PageR,
		// the Shape/Pre-post/Act buttons
		BID_Btn0, BID_Btn1, BID_Btn2, BID_Btn3,

		// strip sel buttons
		BID_Sel0, BID_Sel1, BID_Sel2, BID_Sel3, BID_Sel4, BID_Sel5, BID_Sel6, BID_Sel7,

		// Faders
		BID_Fader0, BID_Fader1, BID_Fader2, BID_Fader3, BID_Fader4, BID_Fader5, BID_Fader6, BID_Fader7, BID_FaderMstr,
		// fader touch
		BID_FaderTouch0, BID_FaderTouch1, BID_FaderTouch2, BID_FaderTouch3, BID_FaderTouch4, BID_FaderTouch5, BID_FaderTouch6, BID_FaderTouch7, BID_FaderTouchMstr,

		// Encoders
		BID_Encoder0, BID_Encoder1, BID_Encoder2, BID_Encoder3, BID_Encoder4, BID_Encoder5, BID_Encoder6, BID_Encoder7,
		// encoder push
		BID_EncoderPush0, BID_EncoderPush1, BID_EncoderPush2, BID_EncoderPush3, BID_EncoderPush4, BID_EncoderPush5, BID_EncoderPush6, BID_EncoderPush7,

		// Channel strip encoders
		BID_EncoderActR0C0, BID_EncoderActR0C1, BID_EncoderActR0C2, BID_EncoderActR0C3,
		BID_EncoderPushActR0C0, BID_EncoderPushActR0C1, BID_EncoderPushActR0C2, BID_EncoderPushActR0C3,

		BID_EncoderActR1C0, BID_EncoderActR1C1, BID_EncoderActR1C2, BID_EncoderActR1C3,
		BID_EncoderPushActR1C0, BID_EncoderPushActR1C1, BID_EncoderPushActR1C2, BID_EncoderPushActR1C3,

		BID_EncoderActR2C0, BID_EncoderActR2C1, BID_EncoderActR2C2, BID_EncoderActR2C3,
		BID_EncoderPushActR2C0, BID_EncoderPushActR2C1, BID_EncoderPushActR2C2, BID_EncoderPushActR2C3,

		BID_SwitchActC0, BID_SwitchActC1, BID_SwitchActC2, BID_SwitchActC3,

		// Channel Mutes
		BID_Mute0, BID_Mute1, BID_Mute2, BID_Mute3, BID_Mute4, BID_Mute5, BID_Mute6, BID_Mute7,
		// Channel Solos
		BID_Solo0, BID_Solo1, BID_Solo2, BID_Solo3, BID_Solo4, BID_Solo5,	BID_Solo6, BID_Solo7,
		// Channel Arms
		BID_Arm0, BID_Arm1, BID_Arm2, BID_Arm3, BID_Arm4, BID_Arm5,	BID_Arm6, BID_Arm7,

		// Joystick
		BID_JoyX, BID_JoyY,
		BID_LFESend,
		BID_SurroundView,

		BID_IOCtrl,	// switch to IO control
		BID_Tracks, BID_Buses, BID_Masters,

		BID_Automation,	// master write enable
		BID_Snapshot,

		BID_Footswitch1, BID_Footswitch2,

		BID_LeftMarker, BID_RightMarker,

		BID_Jog,
		BID_Shuttle,
		
		BID_Tbar,
		BID_TbarFRBal, BID_TbarACT, BID_TbarXRay,
	};

protected:

	typedef std::map<ControlId,CMidiMsg*>	ControlMessageMap;
	typedef ControlMessageMap::iterator		ControlMessageMapIterator;

	ControlMessageMap	m_mapControlMsgs;		// param-less button messages

	void	addControlMsg( CMidiMsg* );

	bool	handleControlMsg( CMidiMsg*, float );
	bool	handleBankButton( CMidiMsg*, float );
	bool	handleTransportControl( CMidiMsg* pmsg, float );
	bool	handleCommandOrKey( CMidiMsg* pmsg, float );
	bool	handleModeButton( CMidiMsg*, float );
	bool	handleRecEditTools( CMidiMsg*, float );
	bool	handleModifierButton( CMidiMsg*, float );
	bool	handleStripSelButton( CMidiMsg*pmsg, float fValue );
	bool	doSelectStrip( DWORD ixStrip );
	bool	handlePushMarkerSelect( CMidiMsg* pmsg, float );
	bool	handleJoyStick( CMidiMsg* pmsg, float );
	bool	handleTbar( CMidiMsg* pmsg, float );

	void	doCursorKeyEdit( ControlId cid );

	void	onMarkersDisplay( bool bState );
	void	onLayersMode( bool bState );

	void	updateLCD() {}
	void	updateParamStateOnLCD();

	void	initLEDs();
	void	initOverUnderLeds();

	void  resetChannelStrip();

	CMixParam* paramFromId( ControlId cid );

public:
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

	// command / key map
	MsgIdToBADMap	m_mapButton2BAD;
	void		doShuttleRing( float f01, JogDirection jd );

	// window access
	BOOL		moveActiveWindow( int nX, int nY );
	POINT		m_ptGrabbed, m_ptTarget, m_ptCurrent;

private:
	BOOL						m_bDirty;
	CIOBoxInterface*		m_pIOBoxInterface;
	bool						m_bOnMackie;
	bool						m_bStopPressed;
	bool						m_bProChanLock;

	CMidiMsg::ValueRange	m_vrCW;
	CMidiMsg::ValueRange m_vrCCW;


	//////////////////////////////////////////////////////
	// Timer stuff
	enum TimerState
	{
		TS_ParamChanged,	// param has changed.
		TS_Blink,
		TS_KeyRep,
		TS_Revert,
		TS_WindowMove,
		TS_None,
	};

	enum BlinkState{
		BS_None,
		BS_On,
		BS_Off }
	m_eBlinkState;

	void setTimer( CTimerClientWithID&, WORD dwMs );
	void killTimer( CTimerClientWithID& );

	CTimerClientWithID	m_tcbWindowMove;
	CTimerClientWithID	m_tcbParamChange;
	CTimerClientWithID	m_tctBlink;
	CTimerClientWithID	m_tctRevert;
	CKeyboardRepTimerClient	m_tctKeyRep;
	CShuttleRingTimer		m_tctShuttleRing;

	///////////////////////////////////////////////////////
	// 7-segment LED methods
	void	outputTime( MFX_TIME& time );

	///////////////////////////////////////////////////////
	// LCD methods
	enum ParamShowMode { PSM_Refresh, PSM_Touch, PSM_Revert };

	void initLCDs();
	void showParam( CMixParam* pParam, int ixLcd, ParamShowMode eShowMode, bool bForceUpdate = false );
	void showParam( CMixParam* pParam, ParamShowMode eShowMode );
	void showEQParams( ParamShowMode eShowMode );
	void showText( LPCSTR psz, int ixLcd, int ixRow, size_t cChars = 8, bool bForce = false );
	bool getLCDIndex( CMixParam* pParam, bool bFlipped, int* pixLcd );

	STRIP_INFO getStripInfo( DWORD dwPhysicalStrip );
	STRIP_INFO MapStripInfo( DWORD dwCurHostStrip );

	void onKeyRep();

	void onBlink();
	void startBlink( ControlId id );
	void stopBlink( ControlId id );
	void stopBlink( );

	void lightNthSEL( DWORD n, int nOnBlink ); // 0 = off, 1 = on, 2 = blink
	void endChannelBranchInserting( void );

	void requestRevert( CMixParam* p );
	void onRevert();

	std::set<ControlId>	m_setBlinkControls;
	MixParamSet				m_setRevertParams;

	std::set<int>	m_setLCDColsToClear;

	typedef std::map<CMixParam*,DWORD>	ParamDisplayTimeStampMap;
	ParamDisplayTimeStampMap		m_mapParamDisplayTimeStamp;

	BYTE m_aaLCDChars[2][13*8];
   char m_szACTContext[32];

	////////////////////////////////////////////////////////
	// VU Meter methods
	void setMeterLevel( float f01, DWORD dwStrip );
	void refreshMeters( );

	typedef std::map<DWORD,BYTE>		MapVUSendByte;
	typedef MapVUSendByte::iterator	MapVUSendByteIterator;
	MapVUSendByte	m_mapVUSendByte;

	CMidiMsg		m_VUMidiMsg;
	CMidiMsg		m_msgSevenSegLed;
	CMidiMsg		m_ACTRowIndicator;
	CMidiMsg		m_msgMiscLed;

	//
	// Editing 
	//
	enum JogMode
	{
		JM_Standard,	// regular ol Jog/Shuttle/Scrub
		JM_SelByTime,	// move and select XY point by time
		JM_SelByClips,	// move and select XY point by clips
		JM_Edit,			// crop/fade/?
		JM_Scroll,		// v/h scroll
		JM_Zoom,			// v/h zoom
		JM_Loop,			// moving loop markers
		JM_Punch,		// moving punch markers
	} m_eJogMode;

	BOOL					m_bSelectingOrEditing;
	ClipEditFunction	getEditMode( void );
	union
	{
		// these aren't interchangeable, but they can not be used at the same time, and they are used for a similar purpose, so might as well conserve memory
		ClipEditFunction	m_cefCurrent;
		BOOL 					m_bEditRight;
	};

	void		onMarker( bool bLeft, float fVal );
	void		onJogMode( JogMode e );
	void		onJogWheel( float f );
	void		onShuttleRing( float f );

	void		endEdit( BOOL bEnd = TRUE );

	// // // 
	struct Surround
	{
		Surround() : pmsgJoyX(0),pmsgJoyY(0),pParamAngle(0),pParamFocus(0),fValX(0.f), fValY(0.f),
						pParamLFE(0),pParamFRBal(0),pmsgLFE(0){}
		CMidiMsg*	pmsgJoyX;
		CMidiMsg*	pmsgJoyY;
		CMidiMsg*	pmsgLFE;
		float			fValX;
		float			fValY;
		CMixParam*	pParamAngle;
		CMixParam*	pParamFocus;
		CMixParam*	pParamLFE;
		CMixParam*	pParamFRBal;
		CMixParam*	pParamWidth;
	};

	Surround			m_Surround;

	float				m_fTbarRcvd;

	// t-bar
	enum TBarMode
	{
		TBM_FRBalance,
		TBM_ACT,
		TBM_XRay,
	}	m_eTBM;


	// LCD Display Modes
public:
	enum SpecialDisplayModes
	{
		SDM_Normal,
		SDM_ExistingFX,
		SDM_FXTree,
		SDM_Markers,
		SDM_Layers,
		SDM_ChannelBranch,
	};

	SpecialDisplayModes	GetSpecialDisplayMode() const { return m_eDisplayMode; }

	void						SetFaderMotorEnable( bool b ) { m_bEnableFaderMotors = b; }
	bool						GetFaderMotorEnable() const { return m_bEnableFaderMotors; }
	void						SetSoloSelectsChannel( bool b ) { m_bSoloSelectsChannel = b; }
	bool						GetSoloSelectsChannel( ) const  { return m_bSoloSelectsChannel; }
	void						SetFaderTouchSelectsChannel( bool b ) { m_bFaderTouchSelectsChannel = b; }
	bool						GetFaderTouchSelectsChannel( ) const  { return m_bFaderTouchSelectsChannel; }

	typedef enum EncoderOptions { Enc_Track, Enc_Bus, Enc_IO, Enc_Main };
   typedef struct SEncoderParams
   {
      SEncoderParams::SEncoderParams() :
         stripType( Enc_Track ),
         dwCmd( 0 ),
         dwParam( 0 ),
         dwIndex( 0 )
      {
      };

      EncoderOptions stripType;
      union
      {
			DWORD dwCmd;
			TacomaIOBoxParam ioParam;
         SONAR_MIXER_PARAM mixParam;
         // IO param
      };

      DWORD dwParam; // the dwParam field when calling into the host
      DWORD dwIndex;
   } SEncoderParams, *PSEncoderParams;

	void showIoCtrlText();
	CString getIoParamString( TacomaIOBoxParam ioParam, BOOL bShort );
	HRESULT refreshIoMode( PMBINDING &pmb, CMidiMsg *pmsgIn, PSEncoderParams pEnc );

private:
	float				m_fXRay;
	float				m_fSendDest;
	int				m_nInserting; // used for Channel Branch when inserting a send. 0 == not inserting, anything else == physical strip where inserting
	SpecialDisplayModes	m_eDisplayMode;
	MFX_TIME_FORMAT		m_mfxfPrimary;		// TimeFormat for Cursor / In/Out times
	bool				m_bShowFaderOnLCD;
	bool				m_bEnableFaderMotors;
	bool				m_bSoloSelectsChannel;
	bool				m_bFaderTouchSelectsChannel;

	std::vector<PSEncoderParams>	m_vEncoderParamList;
	void emptyEncoderParamList( void );

	void showExistingPlugins();
	void showAvailablePlugins();
	void endInsertPlugin();
	void setPluginChild( int ixRotary, CMidiMsg::ValueChange vc );
	void insertPlugin( int ixRotary );

	void setDisplayMode( SpecialDisplayModes eMode );

	enum {NumPluginDesc = 4,		// max depth of the plug-in tree
			NumPluginChar = 16 };	// chars to use per plug-in

   //CDisplayEE  *m_pDisplayEE;

	int			m_aPluginChildIndex[NumPluginDesc];	// Child index for each LCD
	PLUGINS*		m_pPluginTree;				// The host's Plug-in Tree struct
	DWORD			m_dwBinInsertIndex;		// the index to insert in the bin

	//////////////////////////////////////////
	/// Presets
public:
	enum PresetType { PT_Preamp, PT_DM, PT_NONE };
	HRESULT GetPresetsOnDisk( PresetType pt, std::vector<CString>* pvNames );

	HRESULT	SaveIOPreset( PresetType pt, LPCTSTR szName );
	HRESULT	LoadIOPreset( PresetType pt, LPCTSTR szName );
	bool		PresetExists( PresetType pt, LPCTSTR szName );
	HRESULT	DeletePreset( PresetType pt, LPCTSTR szName );

	CString	GetCurrentPresetName( PresetType pt );
	void 		SetCurrentPresetName( PresetType pt, LPCTSTR sxName );

   PSEncoderParams GetEncoderListParam( DWORD dwIndex, DWORD dwCmd, SONAR_MIXER_STRIP mixerStrip );
	PSEncoderParams GetEncoderListParam( DWORD dwIndex, DWORD dwCmd, EncoderOptions strip );

  // void showDisplayEE( void );

private:
   // dwIndex is the zero-based index of the strip. For now, there are 4 Track indexes, 4 bus indexes, and 4 IO indexes
   void addEncoderParam( DWORD dwIndex, EncoderOptions strip, DWORD dwCmd, DWORD dwParam = 0 );
   DWORD getCountByStripType( SONAR_MIXER_STRIP strip );
	DWORD getCountByStripType( EncoderOptions strip );

	EncoderOptions getModeForRotary( void );

	HRESULT getDataPath( CString* pstrPath );

	CString		m_strPreampPreset;
	CString		m_strDMPreset;

// Main Persistence
public:
	class CTacomaSurfacePersist : public CTTSPersistObject
	{
	public:
		CTacomaSurfacePersist( CTacomaSurface* p ):
			CTTSPersistObject( m_wPersistSchema, m_wPersistChunkID )
			,m_pSurface( p )
		{
		}
		// CTTSPersistObject Override ************************************
		HRESULT Persist( WORD wSchema, CPersistDDX& ddx );
		HRESULT NeedTOCEntry() { return S_OK; }

		// CTTSPersistObject schema and chunk id
		static WORD m_wPersistSchema;
		static WORD m_wPersistChunkID;

	private:
		CTacomaSurface* m_pSurface;

	}	m_persist;

};

#if DAISY
class CDisplayEE : public CWnd
{
	DECLARE_DYNAMIC(CDisplayEE)

public:
	CDisplayEE();
	virtual ~CDisplayEE();

	void CreateWnd( float fVal );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnTimer( UINT_PTR nTimer );
	afx_msg void OnKillFocus( CWnd *pWnd );
};
#endif
/////////////////////////////////////////////////////////////////////////////
// CTacomaSurfaceApp
// See ACTControllerGen.cpp for the implementation of this class
// This class should need to be modified when creating your plugin
//

class CTacomaSurfaceApp : public CWinApp
{
public:
	CTacomaSurfaceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTacomaSurfaceApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTacomaSurfaceApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CTacomaSurfaceApp theApp;

#endif // !defined(AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
