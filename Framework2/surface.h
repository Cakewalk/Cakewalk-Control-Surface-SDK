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
#include <map>
#include <set>
#include <vector>

#define UNDEF_STRIP_NUM ((DWORD)-1)

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
	~CControlSurface();

	// *** IUnknown methods ***
	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppv );
  	ULONG STDMETHODCALLTYPE	AddRef();
	ULONG STDMETHODCALLTYPE Release();


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
	HRESULT STDMETHODCALLTYPE GetStatusText(LPSTR pszStatus,DWORD* pdwLen);

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
	virtual HRESULT	OnMIDIMessageDelivered( CMidiMsg* pObj, float fValue ) {ASSERT(0); return S_OK;}	// override

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
	void					GetStripFormatString( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetInputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetOutputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr );
	void					GetMeterValues( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, std::vector<float>* pv, DWORD dwPhysicalMeters );

	CMidiInputRouter*	GetMidiInputRouter() { return m_pMidiInputRouter; }
	CTimer*				GetTimer() { return m_pTimer; }

	DWORD GetSurfaceID() const { return m_dwSurfaceID; }


	// ISonarMidiOut pass thru's
	HRESULT MidiOutShortMsg( DWORD dwShortMsg );
	HRESULT MidiOutLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );

	HRESULT GetStripName( SONAR_MIXER_STRIP eStrip, DWORD dwStrip, CString* pstr );
	
	// automation modes
	HRESULT SetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b );
	bool GetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip );
	HRESULT SetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b );
	bool GetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip );

protected:
	virtual HRESULT persist( IStream* pStm, bool bSave ) = 0;	// must override this
	virtual HRESULT onRefreshSurface( DWORD fdwRefresh, DWORD dwCookie ){ return S_OK; }
	virtual void	onConnect(){}
	virtual void	onDisconnect(){}

	virtual HRESULT	buildBindings();

	DWORD					m_dwRefreshCount;
	DWORD					m_dwSurfaceID;
	DWORD					m_dwSupportedRefreshFlags;

	// Host interfaces we will QI for.  Your surface should handle situations
	// where some of these are not implemented.
	ISonarMixer*					m_pSonarMixer;
	ISonarMixer2*					m_pSonarMixer2;
	ISonarMidiOut*					m_pSonarMidiOut;
	ISonarTransport*				m_pSonarTransport;
	ISonarCommands*				m_pSonarCommands;
	ISonarKeyboard*				m_pSonarKeyboard;
	ISonarProject*					m_pSonarProject;
	ISonarProject2*				m_pSonarProject2;
	ISonarIdentity*				m_pSonarIdentity;
	ISonarIdentity2*				m_pSonarIdentity2;
	ISonarVUMeters*				m_pSonarVUMeters;
	ISonarParamMapping*			m_pSonarParamMapping;
	ISonarUIContext2*				m_pSonarUIContext;


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

private:

	HWND								m_hwndApp;

	LONG								m_cRef;
	BOOL								m_bIsConnected;
	CCriticalSection				m_cs;

	bool								m_bLoaded;
	bool								m_bConnected;
	BOOL								m_bActLearnActive;
	float								m_afMeter[32];	// room for largest possible interleave
};


#endif // !defined(AFX_CONTROLSURFACE_H__10B580DD_BEAD_42AD_A6B2_9F52912A20CF__INCLUDED_)
