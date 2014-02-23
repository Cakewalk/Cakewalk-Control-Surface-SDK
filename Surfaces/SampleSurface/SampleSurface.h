// SampleSurface.h : main header file for the SampleSurface DLL
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

extern const GUID CLSID_SampleSurface;
extern const GUID CLSID_SampleSurfacePropPage;
extern const GUID LIBID_SampleSurface;


class CMixParam;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szFriendlyName[] = "Sample SDK Surface(DEBUG)";
static const char s_szFriendlyNamePropPage[] = "Sample SDK Surface Property Page (DEBUG)";
#else
static const char s_szFriendlyName[] = "Sample SDK Surface";
static const char s_szFriendlyNamePropPage[] = "Sample SDK Surface Property Page";
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


#define EU_CHECKHR( __terr ) \
	do{	if(__terr!=kERR_OK) return E_FAIL;} \
	while(0)

#define CHECK_RET(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			return __hr; } } while (0)


/////////////////////////////////////////////////////////////////////////////
// CSampleSurface

class CSampleSurface :
	public CControlSurface
{
public:
	// Ctors
	CSampleSurface();
	virtual ~CSampleSurface();

	// IControlSurface
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );

	// IControlSurface3
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoStatusMessages(WORD**, DWORD* );

	// ISurfaceParamMapping overrides
	virtual HRESULT STDMETHODCALLTYPE SetStripRange(DWORD,SONAR_MIXER_STRIP);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *);
	virtual HRESULT STDMETHODCALLTYPE SetLearnState(BOOL);

	// ISpecifyPropertyPages overrides
	virtual HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages );

	// IPersistStream overrides
	virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );
	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(_ULARGE_INTEGER * pcbSize);


	////////////////////////////////////////////
	// Callbacks for CMidiMessage
	HRESULT	OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue );
	////////////////////////////////////////////


	void SetTimeFormat( MFX_TIME_FORMAT fmt );
	MFX_TIME_FORMAT GetTimeFormat() { return m_mfxfPrimary; }

	virtual HRESULT SetHostContextSwitch( );


	void Stop();
	void Play( bool bToggle );
	bool IsPlaying() { return GetTransportState( TRANSPORT_STATE_PLAY ); }
	void Record( bool bToggle );
	bool IsRecording() { return GetTransportState( TRANSPORT_STATE_REC ); }

	void VZoom( int n );
	void HZoom( int n );

	bool IsFirstLoaded() { bool b = m_bLoaded; m_bLoaded = false; return b; }

	enum RefreshMod 
	{
		RM_EVERY	= 0,	// every refresh
		RM_EVEN,			// only even refreshes
		RM_ODD,			// only odd refreshes
	};

protected:
	HRESULT 			onRefreshSurface(DWORD fdwRefresh, DWORD dwCookie );
	HRESULT 			persist(IStream* pStm, bool bSave);
	void				onConnect();
	void				onDisconnect();
	HRESULT			updateParamBindings();


	HRESULT			buildBindings();
	HRESULT			buildStripBindings();
	HRESULT			buildMiscBindings();
	HRESULT			buildTransportBindings();

	CMixParam*		createMixParam();
	void				destroyMidiMsg( CMidiMsg*& pmsg );

	bool				m_bLoaded;

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

	void deleteBinding( InputBindingIterator it );

	InputBindingMap	m_mapStripContinuous;
	InputBindingMap	m_mapStripTrigger;
	InputBindingMap	m_mapFaderTouch;

	typedef std::set<CMixParam*>				MixParamSet;
	typedef MixParamSet::iterator				MixParamSetIterator;
	MixParamSet			m_setEveryMixparam;	// used for memory management/cleanup

	CMidiMsg*		m_pmsgBankL;
	CMidiMsg*		m_pmsgBankR;
	CMidiMsg*		m_pmsgTrkL;
	CMidiMsg*		m_pmsgTrkR;
	std::set<CMidiMsg*>	m_setBankSwitches;

	CMidiMsg*		m_pmsgPlay;
	CMidiMsg*		m_pmsgStop;
	CMidiMsg*		m_pmsgRec;
	std::set<CMidiMsg*>	m_setTransportSwitches;

private:
	BOOL				m_bDirty;

	CSFKCriticalSection	m_cs;		// critical section to guard properties

	MFX_TIME_FORMAT				m_mfxfPrimary;		// TimeFormat for Cursor / In/Out times

};

/////////////////////////////////////////////////////////////////////////////
// CSampleSurfaceApp
// See ACTControllerGen.cpp for the implementation of this class
// This class should need to be modified when creating your plugin
//

class CSampleSurfaceApp : public CWinApp
{
public:
	CSampleSurfaceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSampleSurfaceApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSampleSurfaceApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSampleSurfaceApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTController_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
