//////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// Surface.h: Definition of the CControlSurface class which
// implements the IControlSurface defined in ControlSurface.idl
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLSURFACE_H__10B580DD_BEAD_42AD_A6B2_9F52912A20CF__INCLUDED_)
#define AFX_CONTROLSURFACE_H__10B580DD_BEAD_42AD_A6B2_9F52912A20CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "sfkUtils.h"
#include "SfkMidi.h"

#define UNDEF_STRIP_NUM ((DWORD)-1)
#define TRUEFLOAT( _f ) _f >= .5f
#define CID_IN( _bid, _bidMin, _bidMax ) (_bid >= _bidMin && _bid <= _bidMax )

class CMidiInputRouter;
class CTimer;
class CMidiMsg;


/////////////////////////////////////////////////////////////////////////////
// CControlSurface

class CControlSurface :
	public IPersistStream,
	public ISpecifyPropertyPages,
	public ISurfaceParamMapping,
	public IControlSurface3
{
public:
	CControlSurface();
	virtual ~CControlSurface();

	// *** IUnknown methods ***
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppv );
  	virtual ULONG STDMETHODCALLTYPE	AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();


	// Idispatch... blah
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo){return E_NOTIMPL;}
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo){return E_NOTIMPL;}
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId){return E_NOTIMPL;}
	virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr){return E_NOTIMPL;}


	//		IControlSurface2 impl
	// BD 2006: No surface I could fine actually implements this.  SONAR never asks for this.  Why the
	// ^%^%$# was it left in the IDL!?   Well it's published so here it stands
	STDMETHODIMP_(HRESULT)	GetVersion(ULONG *,ULONG *,ULONG *,ULONG *) {return E_NOTIMPL;}



//DECLARE_NOT_AGGREGATABLE(CControlSurface) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 


// IControlSurface
public:

	// IControlSurface implementation:
	// Basic dev caps
	virtual HRESULT STDMETHODCALLTYPE GetStatusText(LPSTR pszStatus,DWORD* pdwLen);

	// Hooks to allow the DLL to process MIDI messages sent from the hardware
	HRESULT STDMETHODCALLTYPE MidiInShortMsg(DWORD dwShortMsg);
	HRESULT STDMETHODCALLTYPE MidiInLongMsg(DWORD cbLongMsg,const BYTE* pbLongMsg);

	// SONAR grants access to its interfaces
	HRESULT STDMETHODCALLTYPE Connect(IUnknown* pUnknown,HWND hwndApp);

	// SONAR revokes access.to its interfaces
	HRESULT STDMETHODCALLTYPE Disconnect();

	// Called by the host when the surface needs to updated based on a change in
	// project state.  fdwRefresh is a combination of bits from REFRESH_F_xxx.
	// dwCookie is user supplied data
	HRESULT STDMETHODCALLTYPE RefreshSurface(DWORD fdwRefresh,DWORD dwCookie);

	virtual HRESULT STDMETHODCALLTYPE GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );


	// IControlSurface3
	virtual HRESULT STDMETHODCALLTYPE GetNoEchoStatusMessages(WORD**, DWORD* );


	// ISurfaceparamMapping
	virtual HRESULT STDMETHODCALLTYPE GetStripRangeCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetStripRange(DWORD,DWORD *,DWORD *,SONAR_MIXER_STRIP *);
	virtual HRESULT STDMETHODCALLTYPE SetStripRange(DWORD,SONAR_MIXER_STRIP);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlCount(DWORD *);
	virtual HRESULT STDMETHODCALLTYPE GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *);
	virtual HRESULT STDMETHODCALLTYPE SetLearnState(BOOL);

	// IPersistStream
	virtual HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID ){return S_OK; };
	virtual HRESULT STDMETHODCALLTYPE IsDirty();
	virtual HRESULT STDMETHODCALLTYPE Load(IStream * pstm);
	virtual HRESULT STDMETHODCALLTYPE Save(IStream * pstm, BOOL fClearDirty);
	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(_ULARGE_INTEGER * pcbSize);

	// ISpecifyPropertyPages
	virtual HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages ){ASSERT(0); return E_NOTIMPL;}	// override


	// Callbacks for CMidiMessage
	virtual HRESULT	OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue, CMidiMsg::ValueChange vtRet ) {ASSERT(0); return S_OK;}	// override

	// host supports dynamic mapping?
	HRESULT GetSupportsDynamicMapping( BOOL* pb );

	HRESULT GetMappingName( LPSTR pszText,DWORD* pdwLen );
	HRESULT GetMappedParamName(DWORD dwKey,LPSTR pszText,DWORD* pdwLen);

	// ACT switching
	HRESULT GetACTEnable( BOOL* pbState );
	HRESULT SetACTEnable( BOOL bState );
	HRESULT GetACTExclusive( BOOL* pbState );
	HRESULT SetACTExclusive( BOOL bState );
	HRESULT GetACTLockContext( BOOL* pbState );
	HRESULT SetACTLockContext( BOOL bState );

	HRESULT SetLearnEnable( BOOL bState );
	HRESULT GetLearnEnable( BOOL* pbState );
	virtual HRESULT SetHostContextSwitch( );

	bool				SupportsDynamicMappings();
	bool				GetLockDynamicMappings();
	void				SetLockDynamicMappings(bool b);
	bool				GetLearnDynamicMappings();
	void				SetLearnDynamicMappings(bool b);
	void				GetDynamicMappingName(CString *strName);

	SONAR_UI_CONTEXT	GetCurrentUIContext();
	void				ActivateProps( BOOL b );
	void				ToggleProps();
	void				ActivateCurrentFx( SONAR_UI_ACTION uia );
	void				ActivateNextFx( SONAR_UI_ACTION uia );
	void				ActivatePrevFx( SONAR_UI_ACTION uia );
	void				ActivateStripFx( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, SONAR_UI_ACTION uia );

	bool				IsFilterEnabled( SONAR_MIXER_FILTER filter, SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum );
	void				SetFilterEnabled( SONAR_MIXER_FILTER filter, SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, bool b );

	// non add-refed interface access
	ISonarTransport*	GetTransportInterface() { return m_pSonarTransport; }
	ISonarMidiOut*		GetMidiOutInterface() { return m_pSonarMidiOut; }
	ISonarCommands*	GetCommandInterface() { return m_pSonarCommands; }

	// Host state helpers
	DWORD					GetStripCount(SONAR_MIXER_STRIP eMixerStrip);
	bool					GetTransportState(SONAR_TRANSPORT_STATE eState);
	bool					GetIsStripMidiTrack( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum );
	DWORD					GetSelectedTrack();
	void					SetSelectedTrack(DWORD dwStripNum);
	bool					IsSurround(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum );
   bool              IsSendSurround(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, DWORD dwSendNum );
   void					GetStripFormatString( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetInputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetOutputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetMeterValues( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, std::vector<float>* pv, DWORD dwPhysicalMeters );

	bool					GetMuteDefeat();
	void					ClearMuteDefeat();
	bool					GetRudeSolo();
	void					ClearSolo();
	bool					GetRudeMute();
	void					ClearMute();
	bool					GetRudeArm();
	void					ClearArm();

	void 					Stop();
	void 					Play( );
	bool 					IsPlaying() { return GetTransportState( TRANSPORT_STATE_PLAY ); }
	void 					Record( bool bToggle );
	bool 					IsRecording() { return GetTransportState( TRANSPORT_STATE_REC ); }
	void					SetNowTime( MFX_TIME& time, bool bOnTheFly, bool bAllowScrub );
	void					GotoEnd( bool bOnTheFly );
	void					GotoStart( bool bOnTheFly );
	void					GotoMarker( DWORD dwIxMarker, bool bOnTheFly );
	void					Rewind( int );
	bool					IsRewind();
	void					FastForward( int );
	bool					IsFastForward();
	void					Pause( bool );
	bool					IsPause();
	void					Audition( );
	bool					IsAudition();
	void					Scrub( bool );
	bool					IsScrub();

	void					AllRead( SONAR_MIXER_STRIP eStrip, DWORD dwStrip, bool bIncludeSynth = false );
	void					AllWrite( SONAR_MIXER_STRIP eStrip, DWORD dwStrip, bool bIncludeSynth = false );
	void					SetOffsetMode();


	DWORD					IndexForCommandID( DWORD dwID );

	void					RequestStatusQuery();

	CMidiInputRouter*	GetMidiInputRouter() { return m_pMidiInputRouter; }
	CTimer*				GetTimer() { return m_pTimer; }

	DWORD GetSurfaceID() const { return m_dwSurfaceID; }

	// ISonarMidiOut pass thru's
	HRESULT MidiOutShortMsg( DWORD dwShortMsg );
	HRESULT MidiOutLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );

	// Plug-in Access
	typedef std::basic_string<char>	AnsiString;
	HRESULT GetPluginListForStrip( SONAR_MIXER_STRIP eStrip, DWORD dwStrip, std::vector<AnsiString>* pV, size_t cMaxFx = 8 );

	// automation modes
	HRESULT SetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b );
	bool GetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip );
	HRESULT SetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b );
	bool GetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip );

protected:
	virtual HRESULT persist( IStream* pStm, bool bSave ) = 0;	// must override this
	virtual HRESULT onRefreshSurface( DWORD fdwRefresh, DWORD dwCookie ){ return S_OK; }
	virtual HRESULT onFirstRefresh(){ return S_OK; }
	virtual void	onConnect(){}
	virtual void	onDisconnect(){}

	virtual HRESULT	buildBindings();

	DWORD					m_dwRefreshCount;
	DWORD					m_dwSurfaceID;
	DWORD					m_dwSupportedRefreshFlags;

public:
	// Host interfaces we will QI for.  Your surface should handle situations
	// where some of these are not implemented.
	ISonarMixer*					m_pSonarMixer;
	ISonarMixer2*					m_pSonarMixer2;
	ISonarMidiOut*					m_pSonarMidiOut;
	ISonarTransport*				m_pSonarTransport;
	ISonarTransport2*				m_pSonarTransport2;
	ISonarCommands*				m_pSonarCommands;
	ISonarKeyboard*				m_pSonarKeyboard;
	ISonarProject*					m_pSonarProject;
	ISonarProject2*				m_pSonarProject2;
	ISonarProject3*				m_pSonarProject3;
	ISonarIdentity*				m_pSonarIdentity;
	ISonarVUMeters*				m_pSonarVUMeters;
	ISonarParamMapping*			m_pSonarParamMapping;
	ISonarUIContext3*				m_pSonarUIContext;
	IHostDataEdit*					m_pHostDataEdit;
	IHostWindow*					m_pHostWindow;
	IHostStripInfo*				m_pHostStripinfo;
	IHostLockStrip*				m_pHostLockStrip;

protected:

#define ACTKEY_BASE (0xB0B)

	struct STRIPRANGE
	{
		STRIPRANGE() : dwL(UNDEF_STRIP_NUM), dwH(UNDEF_STRIP_NUM){}
		DWORD dwL;
		DWORD dwH;
	};
	typedef std::map<SONAR_MIXER_STRIP,STRIPRANGE>	StripRangeMap;
	typedef StripRangeMap::iterator						StripRangeMapIterator;
	StripRangeMap					m_mapStripRanges;

	CMidiInputRouter*				m_pMidiInputRouter;
	CTimer*							m_pTimer;
	bool								m_bConnected;
	CSFKCriticalSection			m_cs;

protected:

	HWND								m_hwndApp;

	LONG								m_cRef;

	bool								m_bLoaded;
	BOOL								m_bActLearnActive;
	float								m_afMeter[32];	// room for largest possible interleave
};


#endif // !defined(AFX_CONTROLSURFACE_H__10B580DD_BEAD_42AD_A6B2_9F52912A20CF__INCLUDED_)
