/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// Surface.cpp : Device independent methods in class CControlSurface
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "surface.h"
#include "SfkMidi.h"

/////////////////////////////////////////////////////////////////////////
// CControlSurface:
/////////////////////////////////////////////////////////////////////////
CControlSurface::CControlSurface():
	m_hwndApp( NULL )
	,m_cRef( 1 )
	,m_bIsConnected( FALSE )
	,m_pSonarMixer( NULL )
	,m_pSonarMixer2( NULL )
	,m_pSonarMidiOut( NULL )
	,m_pSonarTransport( NULL )
	,m_pSonarCommands( NULL )
	,m_pSonarKeyboard( NULL )
	,m_pSonarProject( NULL )
	,m_pSonarProject2( NULL )
	,m_pSonarIdentity( NULL )
	,m_pSonarIdentity2( NULL )
	,m_pSonarVUMeters( NULL )
	,m_pSonarUIContext( NULL )
	,m_pSonarParamMapping( NULL )
	,m_dwSurfaceID( 0 )
	,m_dwSupportedRefreshFlags( 0 )
	,m_dwRefreshCount(0)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxOleLockApp();

	// Create the helper objects we need
	m_pMidiInputRouter = new CMidiInputRouter( this );
	m_pTimer = new CTimer( this );
}

/////////////////////////////////////////////////////////////////////////
CControlSurface::~CControlSurface()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_hwndApp = NULL;

	// it is important to deactivate the timer before any clients are
	// destroyed.
	m_pTimer->SetIsActive( FALSE );

	delete m_pTimer;
	m_pTimer = NULL;

	delete m_pMidiInputRouter;
	m_pMidiInputRouter = NULL;

	AfxOleUnlockApp();
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CControlSurface::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface3 == riid)
		*ppv = static_cast<IControlSurface3*>(this);
	else if (IID_ISurfaceParamMapping == riid)
		*ppv = static_cast<ISurfaceParamMapping*>(this);
	else if (IID_ISpecifyPropertyPages == riid)
		*ppv = static_cast<ISpecifyPropertyPages*>(this);
	else if ( IID_ISurfaceParamMapping == riid )
		*ppv = static_cast<ISurfaceParamMapping*>(this);
	else if (IID_IPersistStream == riid)
		*ppv = static_cast<IPersistStream*>(this);
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CControlSurface::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CControlSurface::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

//////////////////////////////////////////////////////////////////////////
// Override this and provide an array of MIDI status Messages that your surface
// is interested in.  The host will ignore those and only pass them thru to us
HRESULT CControlSurface::GetNoEchoStatusMessages(WORD** ppdwMessages, DWORD* pdwCount )
{
	*pdwCount = 0;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// This is the IControlSurface way of obtaining the MIDI mask.  Older versions 
// of SONAR will call this instead of GetNoEchoStatusMessages.   This function
// has much less granularity because it only returns a bitmask for the MIDI
// channels in use.
HRESULT CControlSurface::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	ASSERT(0);  // override
	return E_NOTIMPL;
}




/////////////////////////////////////////////////////////////////////////
// Host is connecting us.  QI for any interfaces we care about and
// cache various host states as needed
HRESULT CControlSurface::Connect( IUnknown* pHostUnk, HWND hwndApp )
{
	CCriticalSectionAuto lock( m_cs );

	HRESULT hr = S_OK;

	m_hwndApp = hwndApp;

	_ASSERT( m_pSonarCommands == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarCommands, (void**)&m_pSonarCommands );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarMixer == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarMixer, (void**)&m_pSonarMixer );
	if (hr != S_OK)
		return hr;
	// optional ISonarMixer2
	_ASSERT( m_pSonarMixer2 == NULL );
	if ( FAILED( pHostUnk->QueryInterface( IID_ISonarMixer2, (void**)&m_pSonarMixer2 ) ) )
		m_pSonarMixer2 = NULL;

	_ASSERT( m_pSonarMidiOut == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarMidiOut, (void**)&m_pSonarMidiOut );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarTransport == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarTransport, (void**)&m_pSonarTransport );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarKeyboard == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarKeyboard, (void**)&m_pSonarKeyboard );
	if (hr != S_OK)
		return hr;

	_ASSERT( m_pSonarProject == NULL );
	hr = pHostUnk->QueryInterface( IID_ISonarProject, (void**)&m_pSonarProject );
	if (hr != S_OK)
		return hr;
	// optional ISonarProject2
	_ASSERT( m_pSonarProject2 == NULL );
	if ( FAILED( pHostUnk->QueryInterface( IID_ISonarProject2, (void**)&m_pSonarProject2 ) ) )
		m_pSonarProject2 = NULL;

	_ASSERT( m_pSonarIdentity == NULL );
	if ( SUCCEEDED( pHostUnk->QueryInterface( IID_ISonarIdentity, (void**)&m_pSonarIdentity ) ))
	{
		// obtain our instance ID
		hr = m_pSonarIdentity->GetUniqueSurfaceId( static_cast<IControlSurface*>(this), &m_dwSurfaceID );
		if ( hr != S_OK )
			return hr;
	}
	// optional ISonarIdentity2
	_ASSERT( NULL == m_pSonarIdentity2 );
	if ( FAILED( pHostUnk->QueryInterface( IID_ISonarIdentity2, (void**)&m_pSonarIdentity2 ) ) )
		m_pSonarIdentity2 = NULL;
	else
		m_pSonarIdentity2->GetSupportedRefreshFlags( &m_dwSupportedRefreshFlags );


	// optional Sonar Param Mapping
	_ASSERT( NULL == m_pSonarParamMapping );
	if ( FAILED( pHostUnk->QueryInterface( IID_ISonarParamMapping, (void**)&m_pSonarParamMapping ) ) )
		m_pSonarParamMapping = NULL;

	// optional ISonarVUMeters
	_ASSERT( m_pSonarVUMeters == NULL );
	if ( FAILED( pHostUnk->QueryInterface( IID_ISonarVUMeters, (void**)&m_pSonarVUMeters ) ))
		m_pSonarVUMeters = NULL;

	_ASSERT( NULL == m_pSonarUIContext );
	if ( FAILED ( pHostUnk->QueryInterface( IID_ISonarUIContext2, (void**)&m_pSonarUIContext ) ))
	{
		m_pSonarUIContext = NULL;
	}

	if (hr != S_OK)
	{
		Disconnect();
		return E_FAIL;
	}

	onConnect();	// let derived classes do something useful

	m_bIsConnected = TRUE;

	return hr;
}

/////////////////////////////////////////////////////////////////////////
// Host is done with us.  Release everything we QI'd for in Connect()
HRESULT CControlSurface::Disconnect()
{
	CCriticalSectionAuto lock( m_cs );

	m_bIsConnected = FALSE;

	onDisconnect();	// let derived classes do something useful

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

	if ( m_pSonarUIContext )
		m_pSonarUIContext->Release();
	m_pSonarUIContext = NULL;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
// We got a long (sysx) message from the host
HRESULT CControlSurface::MidiInLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg )
{
	if (!m_pMidiInputRouter )
		return E_UNEXPECTED;

	m_pMidiInputRouter->OnLongMsg( cbLongMsg, pbLongMsg );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
// we got a short MIDI message from the host
HRESULT CControlSurface::MidiInShortMsg( DWORD dwShortMsg )
{
	if (!m_pMidiInputRouter)
		return E_UNEXPECTED;

	m_pMidiInputRouter->OnShortMsg( dwShortMsg );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiOutShortMsg( DWORD dwShortMsg )
{
	if ( !m_pSonarMidiOut )
		return E_UNEXPECTED;

	return m_pSonarMidiOut->MidiOutShortMsg( static_cast<IControlSurface*>(this), dwShortMsg );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::MidiOutLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
	if ( !m_pSonarMidiOut )
		return E_UNEXPECTED;

	return m_pSonarMidiOut->MidiOutLongMsg( static_cast<IControlSurface*>(this), cbLongMsg, pbLongMsg );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CControlSurface::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (pdwLen == NULL)
		return E_POINTER;

	LPCTSTR sz = "override me";	// a short message describing state ie: "tracks 1-8"

	// Return results to caller
	if (NULL == pszStatus)
	{
		*pdwLen = (DWORD)::strlen(sz) + 1;
	}
	else
	{
		::strncpy(pszStatus, sz, *pdwLen);
	}

	return S_OK;
}



/////////////////////////////////////////////////////////////////////////
// Periodically called from the host
HRESULT CControlSurface::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto lock( m_cs );

	// up our refresh count.  this can be used to thin out updates.  For example,
	// motors and meters should be updated as often as possible.  However, switches,
	// and text names can be updated less often.  You can also use this to interlace
	// updates (even/odd strips get updated every other refresh)
	m_dwRefreshCount++;	

	// call the refresh handler.  Your derived surface should override this,
	// query the host for values and update states
	return onRefreshSurface( fdwRefresh, dwCookie );
}


/////////////////////////////////////////////////////////////////////////////
// ISurfaceParamMapping
// Return a count of strip ranges (ie: if you do tracks and buses, you have 2 ranges).
HRESULT CControlSurface::GetStripRangeCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = (DWORD)m_mapStripRanges.size();
	
	return S_OK;
}

/////////////////////////////////////////////////////////////////////
// Given an index, return strip range information
HRESULT CControlSurface::GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip )
{
	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	if ( dwIndex >= (DWORD)m_mapStripRanges.size() )
		return E_INVALIDARG;

	// linear lookup for index -> iterator conversion.  Could be optimized but this has at most
	// 3 things in the map
	DWORD dwCount = 0;
	for ( StripRangeMapIterator it = m_mapStripRanges.begin(); it != m_mapStripRanges.end(); ++it )
	{
		if ( dwCount++ == dwIndex )
		{
			const STRIPRANGE& sr = it->second;
			*pdwLowStrip = sr.dwL;
			*pdwHighStrip = sr.dwH;
			*pmixerStrip = it->first;
		}
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////
// Override to set a strip range (from the the WAI interface in the host)
HRESULT CControlSurface::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////////
// Override to return the count or ACT controls
HRESULT CControlSurface::GetDynamicControlCount( DWORD* pdwCount )
{
	return E_NOTIMPL;
}
/////////////////////////////////////////////////////////////////////////////////
// Override to return the nth ACT Control's info
HRESULT CControlSurface::GetDynamicControlInfo(DWORD, DWORD*, SURFACE_CONTROL_TYPE *)
{
	return E_NOTIMPL;
}
/////////////////////////////////////////////////////////////////////////
// The host has entered or left ACT Learn mode
HRESULT CControlSurface::SetLearnState( BOOL bLearning )
{
	m_bActLearnActive = bLearning;
	return S_OK;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_ACTController
// and put that in your ACTController.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CControlSurface::Save( IStream* pStm, BOOL bClearDirty )
{
	CCriticalSectionAuto csa( m_cs );

	// Here you should write any data to pStm which you wish to store.
	// Typically you will write values for all of your properties.
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.

	// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
	// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.


	// OVerride Persist to do the actual save/load
	HRESULT hr = persist(pStm, true);

	if (FAILED(hr))
		return hr;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurface::Load( IStream* pStm )
{
	CCriticalSectionAuto csa( m_cs );

	// Here you should read the data using the format you used in Save().
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.
	
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
	// E_FAIL. Don't simply return whatever IStream::Read() returned.

	HRESULT hr = persist(pStm, false);

	// Optional?   Send init messages?
	// SendInitMessages();

	if (FAILED(hr))
		return hr;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurface::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( m_cs );

	TRACE("CControlSurface::GetSizeMax()\n");

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 4096;
	pcbSize->HighPart = 0;

	return S_OK;
}

HRESULT CControlSurface::IsDirty()
{
	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////
// Override this to build your MIDI and Parameter binding tables
HRESULT	CControlSurface::buildBindings()
{
	return S_OK;
}



/////////////////////////////////////////////////////////////////////////
// Convenient helper to determine if a track is MIDI
bool CControlSurface::GetIsStripMidiTrack( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum )
{
	if (mixerStrip != MIX_STRIP_TRACK)
		return false;

	if ( !m_pSonarMixer )
		return false;

	// so it is a track...

	float fVal = 0;
	if ( FAILED( m_pSonarMixer->GetMixParam( mixerStrip, dwStripNum, MIX_PARAM_IS_MIDI, 0, &fVal ) ) )
		return false;
	
	return (fVal > 0.5);
}

/////////////////////////////////////////////////////////////////////////////
// Convenient helper to determine state of a transport state
bool CControlSurface::GetTransportState(SONAR_TRANSPORT_STATE eState)
{
	BOOL bVal;

	HRESULT hr = m_pSonarTransport->GetTransportState(eState, &bVal);

	if (FAILED(hr))
		return false;

	return bVal ? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// Convenient helper Get the Active track in the host
DWORD CControlSurface::GetSelectedTrack()
{
	float fVal;

	HRESULT hr = m_pSonarMixer->GetMixParam(MIX_STRIP_TRACK, 0,MIX_PARAM_SELECTED, 0,&fVal);

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}


/////////////////////////////////////////////////////////////////////////////
// Convenient helper to set the active track in the host
void CControlSurface::SetSelectedTrack(DWORD dwStripNum)
{
	m_pSonarMixer->SetMixParam(MIX_STRIP_TRACK, 0, MIX_PARAM_SELECTED, 0, (float)dwStripNum, MIX_TOUCH_NORMAL);
}

////////////////////////////////////////////////////////////////////////
// Convenient helper to determine if a strip is surround or not
bool CControlSurface::IsSurround(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum )
{
	bool bSur = false;

	float fVal;

	HRESULT hr = m_pSonarMixer->GetMixParam(eMixerStrip, dwStripNum, MIX_PARAM_ISSURROUND, 0, &fVal);

	if (SUCCEEDED(hr))
		bSur = (fVal >= 0.5f);

	return bSur;
}

/////////////////////////////////////////////////////////////////////////
// Convenient helper helper to get a strip's name as a CString
void	CControlSurface::GetInputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr )
{
	if ( !m_pSonarMixer )
		return;

	float fVal = 0.f;
	m_pSonarMixer->GetMixParam( eMixerStrip, dwStripNum, MIX_PARAM_INPUT, 0, &fVal );

	TCHAR sz[64];
	DWORD csz = _countof(sz)-1;
	m_pSonarMixer->GetMixParamValueText( eMixerStrip, dwStripNum, MIX_PARAM_INPUT, 0, fVal, sz, &csz );
	*pstr = sz;
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to get a strip's output name as a CString
void	CControlSurface::GetOutputName( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr )
{
	if ( !m_pSonarMixer )
		return;

	float fVal = 0.f;
	m_pSonarMixer->GetMixParam( eMixerStrip, dwStripNum, MIX_PARAM_OUTPUT, 0, &fVal );

	TCHAR sz[64];
	DWORD csz = _countof(sz)-1;
	m_pSonarMixer->GetMixParamValueText( eMixerStrip, dwStripNum, MIX_PARAM_OUTPUT, 0, fVal, sz, &csz );
	*pstr = sz;
}


//////////////////////////////////////////////////////////////////////
// Convenient helper to get a strip's format description as a CString
void	CControlSurface::GetStripFormatString( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, CString* pstr )
{
	// is surround?
	float fVal = 0.f;
	m_pSonarMixer->GetMixParam(eMixerStrip, dwStripNum, MIX_PARAM_ISSURROUND, 0, &fVal);
	if ( fVal >= .5f )
		*pstr = "Surnd";
	else if ( GetIsStripMidiTrack( eMixerStrip, dwStripNum ) )
		*pstr = "MIDI";
	else
	{
		m_pSonarMixer->GetMixParam(eMixerStrip, dwStripNum, MIX_PARAM_INTERLEAVE, 0, &fVal);
		if ( fVal >= .5f )
			*pstr = "Stereo";
		else
			*pstr = "Mono";
	}
}


//////////////////////////////////////////////////////////////////////
// Convenient helper to get strip count of a given type
DWORD CControlSurface::GetStripCount(SONAR_MIXER_STRIP eMixerStrip)
{
	if (!m_pSonarMixer)
		return 0;

	DWORD dwCount;

	HRESULT hr = m_pSonarMixer->GetMixStripCount(eMixerStrip, &dwCount);

	// While loading a new project, dwCount is -1
	if (FAILED(hr) || 0xFFFFFFFF == dwCount)
		return 0;

	return dwCount;
}



/////////////////////////////////////////////////////////////////////////////
// the ACT context has changed
HRESULT CControlSurface::SetHostContextSwitch()
{
	if (m_pSonarParamMapping)
		m_pSonarParamMapping->OnContextSwitch(m_dwSurfaceID);

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Get the host's current UI context
SONAR_UI_CONTEXT CControlSurface::GetCurrentUIContext()
{
	if (m_pSonarParamMapping)
	{
		SONAR_UI_CONTEXT uiContext = UIC_TRACKVIEW;

		if (SUCCEEDED(m_pSonarParamMapping->GetCurrentContext(m_dwSurfaceID, &uiContext)))
			return uiContext;
	}

	return UIC_TRACKVIEW;
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to activate the prop page for the current ACT effect
void CControlSurface::ActivateProps( BOOL b )
{
	if ( m_pSonarUIContext )
		m_pSonarUIContext->ActivateSurfaceProperties( m_dwSurfaceID, b );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to toggle the open state of the prop page for the current ACT effect
void CControlSurface::ToggleProps()
{
	if ( m_pSonarUIContext )
		m_pSonarUIContext->ToggleSurfaceProperties( m_dwSurfaceID );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to set a given UI context in the host
void CControlSurface::ActivateCurrentFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	
	SONAR_UI_CONTEXT uic = GetCurrentUIContext();
	if ( UIC_PLUGIN != uic )
		return;

	m_pSonarUIContext->SetUIContext(MIX_STRIP_TRACK, 0xB0B,MIX_PARAM_DYN_MAP,m_dwSurfaceID,uia );
}


//////////////////////////////////////////////////////////////////////
// Convenient helper to navigate to previous fx
void CControlSurface::ActivatePrevFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	m_pSonarUIContext->SetPreviousUIContext( m_dwSurfaceID, uia );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper navigate to next Fx
void CControlSurface::ActivateNextFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	m_pSonarUIContext->SetNextUIContext( m_dwSurfaceID, uia );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to determine if the host supports ACT
bool CControlSurface::SupportsDynamicMappings()
{
	return (m_pSonarParamMapping != NULL);
}

//////////////////////////////////////////////////////////////////////
// Convenient helper determine if ACT lock is enabled
bool CControlSurface::GetLockDynamicMappings()
{
	CCriticalSectionAuto csa( m_cs );

	if (!m_pSonarParamMapping)
		return false;

	BOOL bLock = false;

	HRESULT hr = m_pSonarParamMapping->GetMapContextLock(m_dwSurfaceID, &bLock);

	if (FAILED(hr))
		return false;

	return (bLock != FALSE);
}

//////////////////////////////////////////////////////////////////////
// Convenient helper lock ACT
void CControlSurface::SetLockDynamicMappings(bool b)
{
	CCriticalSectionAuto csa( m_cs );

	if (!m_pSonarParamMapping)
		return;

	if (GetLockDynamicMappings() != b)
	{
		m_pSonarParamMapping->SetMapContextLock(m_dwSurfaceID, b ? TRUE : FALSE);
	}
}


//////////////////////////////////////////////////////////////////////
// Convenient helper to determine if ACT learn is enabled
bool CControlSurface::GetLearnDynamicMappings()
{
	CCriticalSectionAuto csa( m_cs );

	if (!m_pSonarParamMapping)
		return false;

	BOOL bLearn;

	HRESULT hr = m_pSonarParamMapping->GetActLearnEnable(&bLearn);

	if (FAILED(hr))
		return false;

	return (bLearn != FALSE);
}

//////////////////////////////////////////////////////////////////////
// Convenient helper Enable act learn mode
void CControlSurface::SetLearnDynamicMappings(bool b)
{
	CCriticalSectionAuto csa( m_cs );

	if (!m_pSonarParamMapping)
		return;

	m_pSonarParamMapping->EnableACTLearnMode(b ? TRUE : FALSE);
}

//////////////////////////////////////////////////////////////////////
// Convenient helper Return the name of the current ACT context as a CString
void CControlSurface::GetDynamicMappingName(CString *pstr)
{
	if ( !m_pSonarParamMapping )
		*pstr = "Not Available";
	else
	{
		char buf[128];
		DWORD dwLen = sizeof(buf);
		if ( SUCCEEDED( m_pSonarParamMapping->GetMapName( m_dwSurfaceID, buf, &dwLen ) ) )
			*pstr = buf;
		else
			*pstr = "GetMapName FAILED";
	}
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to set a strip's read mode
HRESULT CControlSurface::SetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b )
{
	if ( !m_pSonarMixer2 )
		return E_FAIL;
	return m_pSonarMixer2->SetReadMixParam( eStrip, dwStrip, MIX_PARAM_ANY, 0, b );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to get a strip's read mode
bool CControlSurface::GetReadMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip )
{
	BOOL b;
	if ( !m_pSonarMixer2 )
		return false;
	m_pSonarMixer2->GetReadMixParam( eStrip, dwStrip, MIX_PARAM_ANY, 0, &b );
	return !!b;
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to set a strip's write mode
HRESULT CControlSurface::SetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip, bool b )
{
	return m_pSonarMixer->SetArmMixParam( eStrip, dwStrip, MIX_PARAM_ANY, 0, b );
}

//////////////////////////////////////////////////////////////////////
// Convenient helper to get a strip's write mode
bool CControlSurface::GetWriteMode( DWORD dwStrip, SONAR_MIXER_STRIP eStrip )
{
	BOOL b;
	m_pSonarMixer->GetArmMixParam( eStrip, dwStrip, MIX_PARAM_ANY, 0, &b );
	return !!b;
}

/////////////////////////////////////////////////////////////////////////
// Helper to get meter values from the host.
// Specify the number of actual physical meters on your surface and this function will
// do it's best to collapse the true interleave down to what you have on the metal
void	CControlSurface::GetMeterValues( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, 
												 std::vector<float>* pv, DWORD dwPhysicalMeters )
{
	if ( !pv )
	{
		_ASSERT(0);
		return;
	}

	if ( !m_pSonarVUMeters )
		return;	// one could have this fall back to the the legacy method of using ISonarMixer

	pv->clear();

	DWORD dwCount = _countof( m_afMeter );
	HRESULT hr = m_pSonarVUMeters->GetMeterValues( eMixerStrip, dwStripNum, m_dwSurfaceID, m_afMeter, &dwCount );
	if ( FAILED( hr ) )
		return;

	if ( dwCount != dwPhysicalMeters )
	{
		// just max all channels and put it everywhere (we could do some slick "downmixing" algorithm)
		float f = 0.f;
		for ( DWORD i = 0; i < dwCount; i++ )
			f = max( f, m_afMeter[i] );

		for ( DWORD i = 0; i < dwPhysicalMeters; i++ )
			pv->push_back( f );
	}
	else
	{
		for ( DWORD i = 0; i < dwCount; i++ )
			pv->push_back( m_afMeter[i] );
	}
}




