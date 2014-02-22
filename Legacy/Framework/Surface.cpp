/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// Surface.cpp : Device independent methods in class CControlSurface
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SfkStateDefs.h"


/////////////////////////////////////////////////////////////////////////
// CControlSurface:
/////////////////////////////////////////////////////////////////////////
CControlSurface::CControlSurface():
	m_pSurfaceImp( NULL ),
	m_hwndApp( NULL ),
	m_cRef( 1 ),
	m_bIsConnected( FALSE ),
	m_pSonarMixer( NULL ),
	m_pSonarMixer2( NULL ),
	m_pSonarMidiOut( NULL ),
	m_pSonarTransport( NULL ),
	m_pSonarCommands( NULL ),
	m_pSonarKeyboard( NULL ),
	m_pSonarProject( NULL ),
	m_pSonarProject2( NULL ),
	m_pSonarIdentity( NULL ),
	m_pSonarIdentity2( NULL ),
	m_pSonarVUMeters( NULL ),
	m_pSonarParamMapping( NULL ),
	m_dwInstanceID( 0 ),
	m_dwSupportedRefreshFlags( 0 )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxOleLockApp();

	m_pMidiInputRouter = new CMidiInputRouter( this );
	m_pHostNotifyMulticaster = new CHostNotifyMulticaster( this );
	m_pTimer = new CTimer( this );
	m_pStateMgr = new CStateMgr( this );
	m_pTransport = new CTransport( this  );
	m_pLastParamChange = new CLastParamChange( this  );
	
	m_pSurfaceImp = createSurfaceImp();
}

/////////////////////////////////////////////////////////////////////////
CControlSurface::~CControlSurface()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_hwndApp = NULL;

	// it is important to deactivate the timer before any clients are
	// destroyed.
	m_pTimer->SetIsActive( FALSE );

	delete m_pTransport;
	m_pTransport = NULL;

	delete m_pSurfaceImp;
	m_pSurfaceImp = NULL;

	delete m_pLastParamChange;
	m_pLastParamChange = NULL;

	delete m_pTransport;
	m_pTransport = NULL;

	delete m_pStateMgr;
	m_pStateMgr = NULL;

	delete m_pTimer;
	m_pTimer = NULL;

	delete m_pHostNotifyMulticaster;
	m_pHostNotifyMulticaster = NULL;

	delete m_pMidiInputRouter;
	m_pMidiInputRouter = NULL;

	AfxOleUnlockApp();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::Connect( IUnknown* pSonarUnk, HWND hwndApp )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hr = S_OK;

	m_hwndApp = hwndApp;

	_ASSERT( m_pSonarCommands == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarCommands, (void**)&m_pSonarCommands );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarMixer == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarMixer, (void**)&m_pSonarMixer );
	if (hr != S_OK)
		return hr;
	// optional ISonarMixer2
	_ASSERT( m_pSonarMixer2 == NULL );
	if ( FAILED( pSonarUnk->QueryInterface( IID_ISonarMixer2, (void**)&m_pSonarMixer2 ) ) )
		m_pSonarMixer2 = NULL;


	_ASSERT( m_pSonarMidiOut == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarMidiOut, (void**)&m_pSonarMidiOut );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarTransport == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarTransport, (void**)&m_pSonarTransport );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarKeyboard == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarKeyboard, (void**)&m_pSonarKeyboard );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarProject == NULL );
	hr = pSonarUnk->QueryInterface( IID_ISonarProject, (void**)&m_pSonarProject );
	if (hr != S_OK)
		return hr;
	// optional ISonarProject2
	_ASSERT( m_pSonarProject2 == NULL );
	if ( FAILED( pSonarUnk->QueryInterface( IID_ISonarProject2, (void**)&m_pSonarProject2 ) ) )
		m_pSonarProject2 = NULL;



	_ASSERT( m_pSonarIdentity == NULL );
	if ( SUCCEEDED( pSonarUnk->QueryInterface( IID_ISonarIdentity, (void**)&m_pSonarIdentity ) ))
	{
		// obtain our instance ID
		hr = m_pSonarIdentity->GetUniqueSurfaceId( static_cast<IControlSurface*>(this), &m_dwInstanceID );
		if ( hr != S_OK )
			return hr;
	}
	// optional ISonarIdentity2
	_ASSERT( NULL == m_pSonarIdentity2 );
	if ( FAILED( pSonarUnk->QueryInterface( IID_ISonarIdentity2, (void**)&m_pSonarIdentity2 ) ) )
		m_pSonarIdentity2 = NULL;
	else
		m_pSonarIdentity2->GetSupportedRefreshFlags( &m_dwSupportedRefreshFlags );


	// optional Sonar Param Mapping
	_ASSERT( NULL == m_pSonarParamMapping );
	if ( FAILED( pSonarUnk->QueryInterface( IID_ISonarParamMapping, (void**)&m_pSonarParamMapping ) ) )
		m_pSonarParamMapping = NULL;

	// optional ISonarVUMeters
	_ASSERT( m_pSonarVUMeters == NULL );
	if ( FAILED( pSonarUnk->QueryInterface( IID_ISonarVUMeters, (void**)&m_pSonarVUMeters ) ))
		m_pSonarVUMeters = NULL;


	if (m_pSurfaceImp)
	{
		hr = m_pSurfaceImp->Initialize();
		m_pSurfaceImp->OnConnect();
	}

	if (hr != S_OK)
	{
		Disconnect();
		return E_FAIL;
	}

	m_bIsConnected = TRUE;

	return hr;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::Disconnect()
{
	CCriticalSectionAuto lock( m_cs );

	m_bIsConnected = FALSE;

	if (m_pSurfaceImp)
	{
		HRESULT hr = m_pSurfaceImp->OnDisconnect();
		_ASSERT( SUCCEEDED( hr ) );
	}

	if (m_pSonarCommands != NULL)
		m_pSonarCommands->Release();
	m_pSonarCommands = NULL;

	if (m_pSonarMixer != NULL)
		m_pSonarMixer->Release();
	m_pSonarMixer = NULL;

	if (m_pSonarMixer2 != NULL)
		m_pSonarMixer2->Release();
	m_pSonarMixer2 = NULL;

	if (m_pSonarMidiOut != NULL)
		m_pSonarMidiOut->Release();
	m_pSonarMidiOut = NULL;

	if (m_pSonarTransport != NULL)
		m_pSonarTransport->Release();
	m_pSonarTransport = NULL;

	if (m_pSonarKeyboard != NULL)
		m_pSonarKeyboard->Release();
	m_pSonarKeyboard = NULL;
	
	if (m_pSonarProject != NULL)
		m_pSonarProject->Release();
	m_pSonarProject = NULL;

	if (m_pSonarProject2 != NULL)
		m_pSonarProject2->Release();
	m_pSonarProject2 = NULL;

	if (m_pSonarIdentity)
		m_pSonarIdentity->Release();
	m_pSonarIdentity = NULL;

	if (m_pSonarIdentity2)
		m_pSonarIdentity2->Release();
	m_pSonarIdentity2 = NULL;

	if (m_pSonarVUMeters)
		m_pSonarVUMeters->Release();
	m_pSonarVUMeters = NULL;

	if ( m_pSonarParamMapping )
		m_pSonarParamMapping->Release();
	m_pSonarParamMapping = NULL;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiInLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg )
{
	if (GetMidiInputRouter() == NULL)
		return E_UNEXPECTED;

	if (m_pSurfaceImp)
		m_pSurfaceImp->OnMidiInLongMsg( cbLongMsg, pbLongMsg );
	
	GetMidiInputRouter()->OnLongMsg( cbLongMsg, pbLongMsg );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiInShortMsg( DWORD dwShortMsg )
{
	if (GetMidiInputRouter() == NULL)
		return E_UNEXPECTED;

	if (m_pSurfaceImp)
		m_pSurfaceImp->OnMidiInShortMsg( dwShortMsg );

	GetMidiInputRouter()->OnShortMsg( dwShortMsg );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiOutShortMsg( DWORD dwShortMsg )
{
	return m_pSonarMidiOut->MidiOutShortMsg( static_cast<IControlSurface*>(this), dwShortMsg );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiOutLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
	return m_pSonarMidiOut->MidiOutLongMsg( static_cast<IControlSurface*>(this), cbLongMsg, pbLongMsg );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (pdwLen == NULL)
		return E_POINTER;

	if (m_pSurfaceImp == NULL)
		return E_UNEXPECTED;

	char string[64];
	DWORD dwAvailable = 64;
	m_pSurfaceImp->MakeStatusString( string );

	if (NULL != pdwLen)
	{
		dwAvailable = *pdwLen;
		*pdwLen = min( (DWORD)strlen( string ) + 1, dwAvailable );
	}

	if (NULL != pszStatus)
	{
		strncpy( pszStatus, string, dwAvailable );
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarMixer( ISonarMixer** ppSonarMixer )
{
	if (ppSonarMixer == NULL)
		return E_POINTER;

	if (m_pSonarMixer == NULL)
		return E_UNEXPECTED;

	*ppSonarMixer = m_pSonarMixer;
	(*ppSonarMixer)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarMidiOut( ISonarMidiOut** ppSonarMidiOut )
{
	if (ppSonarMidiOut == NULL)
		return E_POINTER;

	if (m_pSonarMidiOut == NULL)
		return E_UNEXPECTED;

	*ppSonarMidiOut = m_pSonarMidiOut;
	(*ppSonarMidiOut)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarTransport( ISonarTransport** ppSonarTransport )
{
	if (ppSonarTransport == NULL)
		return E_POINTER;

	if (m_pSonarTransport == NULL)
		return E_UNEXPECTED;

	*ppSonarTransport = m_pSonarTransport;
	(*ppSonarTransport)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarCommands( ISonarCommands** ppSonarCommands )
{
	if (ppSonarCommands == NULL)
		return E_POINTER;

	if (m_pSonarCommands == NULL)
		return E_UNEXPECTED;

	*ppSonarCommands = m_pSonarCommands;
	(*ppSonarCommands)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarKeyboard( ISonarKeyboard** ppSonarKeyboard )
{
	if (ppSonarKeyboard == NULL)
		return E_POINTER;

	if (m_pSonarKeyboard == NULL)
		return E_UNEXPECTED;

	*ppSonarKeyboard = m_pSonarKeyboard;
	(*ppSonarKeyboard)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarProject( ISonarProject** ppSonarProject )
{
	if (ppSonarProject== NULL)
		return E_POINTER;

	if (m_pSonarProject == NULL)
		return E_UNEXPECTED;

	*ppSonarProject = m_pSonarProject;
	(*ppSonarProject)->AddRef();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSonarIdentity( ISonarIdentity** ppSonarIdentity )
{
	if ( !ppSonarIdentity )
		return E_POINTER;

	// don't fail if m_pSonarIdentity is NULL, this is an 
	// optional interface in the host

	*ppSonarIdentity = m_pSonarIdentity;
	if ( m_pSonarIdentity )
	 	(*ppSonarIdentity)->AddRef();

	return S_OK;
}

HRESULT CControlSurface::GetSonarParamMapping( ISonarParamMapping** ppSonarParamMapping )
{
	if ( !ppSonarParamMapping )
		return E_POINTER;

	// don't fail if m_pSonarIdentity is NULL, this is an 
	// optional interface in the host

	*ppSonarParamMapping = m_pSonarParamMapping;
	if ( m_pSonarParamMapping )
	 	(*ppSonarParamMapping)->AddRef();

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto lock( m_cs );

	if (GetHostNotifyMulticaster() == NULL)
		return E_UNEXPECTED;

	if (IsConnected())
	{
		GetHostNotifyMulticaster()->OnHostNotify( fdwRefresh, dwCookie );
		return S_OK;
	}
	else
	{
		return S_FALSE; // should not get refreshed while not connected
	}
}

/////////////////////////////////////////////////////////////////////////
BOOL CControlSurface::IsConnected()
{
	return m_bIsConnected;
}

/////////////////////////////////////////////////////////////////////////
// IPersistStream
/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetClassID( CLSID *pClassID )
{
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::IsDirty()
{
	if (m_pSurfaceImp == NULL)
		return E_UNEXPECTED;

	if (GetStateMgr() == NULL)
		return E_UNEXPECTED;

	if (GetStateMgr()->IsDirty() == S_OK || m_pSurfaceImp->IsDirty() == S_OK)
		return S_OK;
	else
		return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::Load( IStream * pstm )
{
	if (m_pSurfaceImp == NULL)
		return E_UNEXPECTED;

	if (GetStateMgr() == NULL)
		return E_UNEXPECTED;

	HRESULT hr = GetStateMgr()->Load( pstm, &CLSID_ControlSurface );
	if (FAILED( hr ))
		return hr;

	hr = m_pSurfaceImp->Load( pstm );
	if (FAILED( hr ))
		return hr;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::Save( IStream * pstm, BOOL fClearDirty )
{
	if (m_pSurfaceImp == NULL)
		return E_UNEXPECTED;

	if (GetStateMgr() == NULL)
		return E_UNEXPECTED;

	HRESULT hr = GetStateMgr()->Save( pstm, fClearDirty, &CLSID_ControlSurface );
	if (FAILED( hr ))
		return hr;

	hr = m_pSurfaceImp->Save( pstm, fClearDirty );
	if (FAILED( hr ))
		return hr;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetSizeMax( _ULARGE_INTEGER * pcbSize )
{
	if (pcbSize == NULL)
		return E_POINTER;
		
	return 4096; // arbitrary maximum size
}

// ISpecifyPropertyPages
/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetPages( CAUUID *pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_ControlSurfacePropPage;

	return NOERROR;
}

// handy globals
/////////////////////////////////////////////////////////////////////////
SONAR_MIXER_STRIP CControlSurface::GetCurrentStripKind( WORD wBankID /*= 0*/ )
{
	CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stContainerClass, wBankID ) );
	
	if (pShifter == NULL)
	{
		_ASSERT( 0 ); // no state shifter for stContainerClass
		return MIX_STRIP_TRACK;
	}

	switch (pShifter->GetCurrentState())
	{
	case ccTracks:
		return MIX_STRIP_TRACK;
		break;

	case ccBus:
		return MIX_STRIP_BUS;
		break;

	case ccMains:
		return MIX_STRIP_MASTER;
		break;

	default:
		_ASSERT( 0 ); // unknown container class
		return MIX_STRIP_TRACK;
	}	
}

/////////////////////////////////////////////////////////////////////////
int CControlSurface::GetBaseStrip( WORD wBankID /*= 0*/ )
{
	CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stContainerClass, wBankID ) );
	
	if (pShifter == NULL)
	{
		_ASSERT( 0 ); // no state shifter for stContainerClass
		return -1;
	}

	switch (pShifter->GetCurrentState())
	{
	case ccTracks:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stBaseTrack, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	case ccBus:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stBaseBus, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	case ccMains:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stBaseMain, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	default:
		_ASSERT( 0 ); // unknown container class
		return -1;
	}
}

/////////////////////////////////////////////////////////////////////////
int CControlSurface::GetCurrentStrip( WORD wBankID /*= 0*/ )
{
	CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stContainerClass, wBankID ) );
	
	if (pShifter == NULL)
	{
		_ASSERT( 0 ); // no state shifter for stContainerClass
		return -1;
	}

	switch (pShifter->GetCurrentState())
	{
	case ccTracks:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stCurrentTrack, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	case ccBus:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stCurrentBus, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	case ccMains:
		{
			CStateShifter *pShifter = GetStateMgr()->GetShifterFromID( MAKELONG( stCurrentMain, wBankID ) );
			if (pShifter == NULL)
			{
				_ASSERT( 0 ); // no state shifter for stBaseTrack
				return -1;
			}
			return pShifter->GetCurrentState();
		}

	default:
		_ASSERT( 0 ); // unknown container class
		return -1;
	}
}

/////////////////////////////////////////////////////////////////////////
BOOL CControlSurface::GetIsStripMidiTrack( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	if (mixerStrip != MIX_STRIP_TRACK)
		return FALSE;

	// so it is a track...
	ISonarMixer* pSonarMixer = 0;
	HRESULT hr = GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
	{
		return FALSE; // we can't tell if it is a MIDI track, no SONAR mixer to ask
	}

	float fVal = 0;
	pSonarMixer->GetMixParam(
		mixerStrip,
		dwStripNum,
		MIX_PARAM_IS_MIDI,
		0,
		&fVal
	);

	pSonarMixer->Release();
	
	return (fVal > 0.5);
}



HRESULT CControlSurface::UpdateHostContext()
{
	if ( m_pSonarParamMapping )
		return m_pSonarParamMapping->OnContextSwitch( m_dwInstanceID );
	return E_NOTIMPL;
}


