// $$Safe_root$$.cpp : Defines the basic functionality of the "$$Safe_root$$".
//

#include "stdafx.h"
#include "$$Safe_root$$.h"
#include "$$Safe_root$$PropPage.h"
#include "ControlSurface_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// C$$Safe_root$$
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

C$$Safe_root$$::C$$Safe_root$$() :
	m_cRef( 1 ),
	m_bDirty( FALSE ),
	m_pMidiOut( NULL ),
	m_pKeyboard( NULL ),
	m_pCommands( NULL ),
	m_pProject( NULL ),
	m_pMixer( NULL ),
	m_pTransport( NULL ),
	m_pSonarIdentity( NULL ),
	m_dwSurfaceId( 0 ),
	m_hwndApp( NULL )
{ 
	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );
}

/////////////////////////////////////////////////////////////////////////////

void C$$Safe_root$$::releaseSonarInterfaces()
{
	if (m_pMidiOut)
		m_pMidiOut->Release(), m_pMidiOut = NULL;
	if (m_pKeyboard)
		m_pKeyboard->Release(), m_pKeyboard = NULL;
	if (m_pCommands)
		m_pCommands->Release(), m_pCommands = NULL;
	if (m_pProject)
		m_pProject->Release(), m_pProject = NULL;
	if (m_pMixer)
		m_pMixer->Release(), m_pMixer = NULL;
	if (m_pTransport)
		m_pTransport->Release(), m_pTransport = NULL;
	if ( m_pSonarIdentity )
		m_pSonarIdentity->Release(), m_pSonarIdentity = NULL;
}

/////////////////////////////////////////////////////////////////////////////

C$$Safe_root$$::~C$$Safe_root$$() 
{ 
	releaseSonarInterfaces();
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT C$$Safe_root$$::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (NULL == pdwLen)
		return E_POINTER;

	// TODO: Fill strStatus with a string describing the current
	// status of the surface, e.g., "Tracks 9-16".
	CString strStatus( "$$Safe_root$$ OK" );

	// Return results to caller
	if (NULL == pszStatus)
		*pdwLen = strlen( strStatus ) + 1;
	else
	{
		strncpy( pszStatus, strStatus, *pdwLen );
		pszStatus[ *pdwLen - 1 ] = 0;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::MidiInShortMsg( DWORD dwShortMsg )
{
	// TODO: Handle a MIDI short message which has just been sent
	// *from* the surface.  In other words, the user has just moved
	// a fader or pressed a button; the surface sent a MIDI message;
	// SONAR received the message, and routed back into this DLL.

	// A typical implementation of this would be:

	// [1] "Crack" the MIDI message to determine its type (note, CC, etc.)

	// [2] Confirm that the message did actually come from the surface.
	// (The user may have daisy-chained gear, and you may have been
	//	given data sent from some other device.)

	// [3] Based on the type of message and the state of the surface
	// module, update a SONAR mixer or transport parameter, i.e., make
	// a call to m_pMixer->SetMixParam or m_pTransport->SetTransportState.
	// Or, particularly if your surface has buttons, you might respond by
	// setting some internal state, i.e.,
	//		m_bEffectSelected = TRUE;

	CCriticalSectionAuto csa( &m_cs );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
	if (NULL == pbLongMsg)
		return E_POINTER;
	if (0 == cbLongMsg)
		return S_OK; // nothing to do

	CCriticalSectionAuto csa( &m_cs );

	// TODO: Handle a MIDI long (SysX) message which has just been sent
	// *from* the surface.  In other words, the user has just moved
	// a fader or pressed a button; the surface sent a MIDI message;
	// SONAR received the message, and routed back into this DLL.

	// See MidiInShortMsg for notes on how this method might be implemented.

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto csa( &m_cs );

	// TODO: Update all motorized faders, LED "scribble strips", etc, to
	// reflect the state of the SONAR mixer and transport, etc.

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	if (NULL == pwMask || NULL == pbNoEchoSysx)
		return E_POINTER;

	CCriticalSectionAuto csa( &m_cs );

	// TODO: Tell SONAR about any channels (or sysx) that do not require echo.
	
	*pwMask = 0;
	*pbNoEchoSysx = FALSE;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_$$Safe_root$$
// and put that in your $$Safe_root$$.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT C$$Safe_root$$::Save( IStream* pStm, BOOL bClearDirty )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should write any data to pStm which you wish to store.
	// Typically you will write values for all of your properties.
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.

	// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
	// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::Load( IStream* pStm )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should read the data using the format you used in Save().
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.
	
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
	// E_FAIL. Don't simply return whatever IStream::Read() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT C$$Safe_root$$::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( &m_cs );

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 0;
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
