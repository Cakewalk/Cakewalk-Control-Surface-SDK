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

#include "resource.h"       // main symbols


#define MM_CHECKHR( _fn ) do { HRESULT __hr = _fn; if (FAILED(__hr)) return __hr; }while(0)



// forwards
class CSurfaceImp;

// motor mix forwards
class CMMStrip;
class CMMChoiceLedUpdater;
class CMMBankShifter;
class CMMTransport;
class CMMLocator;
class CMMChooseInsert;
class CMMContainerMode;
class CMMContainerPicker;
class CMMFineTweakMode;

/////////////////////////////////////////////////////////////////////////////
// CControlSurface

class CControlSurface :
	public IDispatchImpl<IControlSurface, &IID_IControlSurface, &LIBID_SURFACEFRAMEWORKLib>, 
	public ICSProperties,
	public IPersistStream,
	public ISpecifyPropertyPages,
	public ISurfaceParamMapping,
	public CComObjectRoot,
	public CComCoClass<CControlSurface,&CLSID_ControlSurface>
	
{
public:
	CControlSurface();
	~CControlSurface();
BEGIN_COM_MAP(CControlSurface)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IControlSurface)
	COM_INTERFACE_ENTRY(ISurfaceParamMapping)
	COM_INTERFACE_ENTRY(ICSProperties)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
END_COM_MAP()

//DECLARE_NOT_AGGREGATABLE(CControlSurface) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_ControlSurface)

// IControlSurface
public:

	// IControlSurface implementation:
	// Basic dev caps
	HRESULT STDMETHODCALLTYPE GetStatusText(
		LPSTR pszStatus,
		DWORD* pdwLen
	);

	// Hooks to allow the DLL to process MIDI messages sent from the hardware
	HRESULT STDMETHODCALLTYPE MidiInShortMsg(
		DWORD dwShortMsg
	);
	HRESULT STDMETHODCALLTYPE MidiInLongMsg(
		DWORD cbLongMsg,
		const BYTE* pbLongMsg
	);

	// SONAR grants access to its interfaces
	HRESULT STDMETHODCALLTYPE Connect(
		IUnknown* pUnknown,
		HWND hwndApp
	);

	// SONAR revokes access.to its interfaces
	HRESULT STDMETHODCALLTYPE Disconnect();

	// Called by SONAR when the surface needs to updated based on a change in
	// project state.  fdwRefresh is a combination of bits from REFRESH_F_xxx.
	// dwCookie is user supplied data
	HRESULT STDMETHODCALLTYPE RefreshSurface(
		DWORD fdwRefresh,
		DWORD dwCookie
	);


	HRESULT STDMETHODCALLTYPE GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
	{
		if (pwMask == NULL || pbNoEchoSysx == NULL)
			return E_POINTER;
		
		*pwMask = 0x0001; // no echo on channel 1
		*pbNoEchoSysx = 0;
		return S_OK;	
	}

	// ISurfaceparamMapping
	HRESULT STDMETHODCALLTYPE GetStripRangeCount(DWORD *);
	HRESULT STDMETHODCALLTYPE GetStripRange(DWORD,DWORD *,DWORD *,SONAR_MIXER_STRIP *);
	HRESULT STDMETHODCALLTYPE SetStripRange(DWORD,SONAR_MIXER_STRIP);
	HRESULT STDMETHODCALLTYPE GetDynamicControlCount(DWORD *);
	HRESULT STDMETHODCALLTYPE GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *);
	HRESULT STDMETHODCALLTYPE SetLearnState(BOOL);


	// IPersistStream
	HRESULT STDMETHODCALLTYPE GetClassID( CLSID *pClassID );
	HRESULT STDMETHODCALLTYPE IsDirty();
	HRESULT STDMETHODCALLTYPE Load(IStream * pstm);
	HRESULT STDMETHODCALLTYPE Save(IStream * pstm, BOOL fClearDirty);
	HRESULT STDMETHODCALLTYPE GetSizeMax(_ULARGE_INTEGER * pcbSize);

	// ISpecifyPropertyPages
	HRESULT STDMETHODCALLTYPE GetPages( CAUUID *pPages );

	
public:

	// helpers
	HRESULT GetSonarMixer( ISonarMixer** ppSonarMixer );
	HRESULT GetSonarMidiOut( ISonarMidiOut** ppSonarMidiOut );
	HRESULT GetSonarTransport( ISonarTransport** ppSonarTransport );
	HRESULT GetSonarCommands( ISonarCommands** ppSonarCommands );
	HRESULT GetSonarKeyboard( ISonarKeyboard** ppSonarKeyboard );
	HRESULT GetSonarProject( ISonarProject** ppSonarProject );
	HRESULT GetSonarIdentity( ISonarIdentity** ppSonarIdentity );
	HRESULT GetSonarParamMapping( ISonarParamMapping** ppSonarParamMapping );
	BOOL IsConnected();
	HRESULT UpdateHostContext();
	HRESULT ParamInContext( SONAR_MIXER_PARAM mixerParam ) { return S_OK; }

	CMidiInputRouter*				GetMidiInputRouter() { return m_pMidiInputRouter; }
	CHostNotifyMulticaster*		GetHostNotifyMulticaster() { return m_pHostNotifyMulticaster; }
	CTimer*							GetTimer() { return m_pTimer; }
	CStateMgr*						GetStateMgr() { return m_pStateMgr; }
	CTransport*						GetTransport() { return m_pTransport; }
	CLastParamChange*				GetLastParamChange() { return m_pLastParamChange; }

	// returns the current strip kind according to stContainerClass
	SONAR_MIXER_STRIP GetCurrentStripKind( WORD wBankID = 0 );

	// returns the base strip according to stContainerClass
	int GetBaseStrip( WORD wBankID = 0 );

	// returns the current strip according to stContainerClass
	int GetCurrentStrip( WORD wBankID = 0 );

	BOOL GetIsStripMidiTrack( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum );

	// ISonarMidiOut pass thru's
	HRESULT MidiOutShortMsg( DWORD dwShortMsg );
	HRESULT MidiOutLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );

	DWORD	GetInstanceID() const { return m_dwInstanceID; }

private:

	CSurfaceImp*					createSurfaceImp();

	HWND								m_hwndApp;

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

	DWORD								m_dwInstanceID;
	DWORD								m_dwSupportedRefreshFlags;

	CMidiInputRouter*				m_pMidiInputRouter;
	CHostNotifyMulticaster*		m_pHostNotifyMulticaster;
	CTimer*							m_pTimer;
	CStateMgr*						m_pStateMgr;
	CTransport*						m_pTransport;
	CLastParamChange*				m_pLastParamChange;
	LONG								m_cRef;
	BOOL								m_bIsConnected;
	CCriticalSection				m_cs;
	CSurfaceImp*					m_pSurfaceImp;
};

/////////////////////////////////////////////////////////////////////////
// CSurfaceImp:
//
// Base class for implementation of the control surface
/////////////////////////////////////////////////////////////////////////

class CSurfaceImp
{
public:
	CSurfaceImp( CControlSurface* pSurface ):
		m_pSurface( pSurface )
	{
	}

	virtual ~CSurfaceImp() {}

	virtual HRESULT Initialize() = 0;

	virtual HRESULT OnConnect()
	{
		return S_FALSE;
	}

	virtual HRESULT OnDisconnect()
	{
		return S_FALSE;
	}

	virtual void MakeStatusString( char *pStatus ) = 0;

	virtual HRESULT OnMidiInShortMsg( DWORD dwShortMsg )
	{
		return S_FALSE;
	}

	virtual HRESULT OnMidiInLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg )
	{
		return S_FALSE;
	}

	virtual HRESULT IsDirty()
	{
		return S_FALSE;
	}

	virtual HRESULT Load( IStream * pstm )
	{
		return S_OK;
	}

	virtual HRESULT Save( IStream * pstm, BOOL fClearDirty )
	{
		return S_OK;
	}

protected:
	CControlSurface* m_pSurface;
};

#endif // !defined(AFX_CONTROLSURFACE_H__10B580DD_BEAD_42AD_A6B2_9F52912A20CF__INCLUDED_)
