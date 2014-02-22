// MidiSource.cpp: implementation of IMidiSink and CMidiSource classes
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include <algorithm>
#include "MidiSource.h"
#include "WindowsX.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define NUM_SYSEX_BUFFERS (16)
#define SYSEX_BUF_SIZE (1160)


// Use MAYBE_TRACE() for items we'd like to switch off when not actively
// working on this file.
#if 0
	#define MAYBE_TRACE TRACE
#else
	#define MAYBE_TRACE TRUE ? (void)0 : TRACE
#endif

//////////////////////////////////////////////////////////////////////
// Functors

void const CMidiSource::notifySinks::operator()( IMidiSink* pSink )
{
	pSink->OnMidiShortMsg( m_dwMsg, m_dwTime, m_pPort->nPort );
}


//////////////////////////////////////////////////////////////////////

static BOOL			s_bEnableInput = TRUE;
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////

CMidiSource::CMidiSource( CTimer *pTimer ):
	m_pTimer( pTimer ),
	m_pSysexInput( NULL )
{
	s_bEnableInput = TRUE;
	m_pTimer->SetPeriod( this, 50 ); // only used for sysx
}

void CMidiSource::Tick()
{
	if ( !m_pSysexInput || m_pSysexInput->IsEmpty() )
		return;

	BYTE aby[SYSEX_BUF_SIZE];
	WORD wBytes = 0;

	// pull one off
	
	UINT nPort = 0;
	m_pSysexInput->GetBuffer( aby, &wBytes, TRUE, &nPort );
	for ( MidiSinkIt it = m_setSinks.begin(); it != m_setSinks.end( ); it++ )
	{
		IMidiSink* pSink = *it;
		if ( pSink )
			pSink->OnMidiLongMsg( wBytes, aby, nPort );
	}
}

HRESULT CMidiSource::SetNoEchoMask( UINT nPort, WORD wChanMask, std::set<WORD>& setMessageExclude )
{
	if ( nPort < 0 || nPort >= GetPortCount() )
	{
		ASSERT( 0 );
		return E_FAIL; // nocando
	}

	CPort *pPort = m_vecPorts[nPort];
	if ( !pPort )
		return E_FAIL;

	pPort->setMessageExclude = setMessageExclude;
	pPort->wChanMask = wChanMask;

	return S_OK;
}

HRESULT CMidiSource::ClearNoEchoMask()
{
	for ( size_t ix = 0; ix < m_vecPorts.size(); ix++ )
	{
		CPort* pPort = m_vecPorts[ix];
		pPort->wChanMask = 0;
		pPort->setMessageExclude.clear();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
// static
void CMidiSource::SetEnableInput( BOOL bEnable )
{
	s_bEnableInput = bEnable;
}

//////////////////////////////////////////////////////////////////////

CMidiSource::~CMidiSource()
{
	// unsubscribe from timer
	m_pTimer->SetPeriod( this, 0 );

	HRESULT hr = RemoveAllInPorts();
	ASSERT( SUCCEEDED( hr ) );

	m_pSysexInput->Close();
	delete m_pSysexInput;
}

//////////////////////////////////////////////////////////////////////


HRESULT CMidiSource::OnPortsChanged()
{
	HRESULT hr = m_pSysexInput->Init();
	if ( FAILED( hr ) )
		return hr;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::Initialize()
{
	m_pSysexInput = new CSysxInQueue( this );
	m_pSysexInput->Init();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::GetDeviceCount(UINT *pnCount) const
{
	*pnCount = midiInGetNumDevs();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::GetDeviceName(UINT ix, CString* pName) const
{
	MIDIINCAPS midiCaps;

	if( midiInGetNumDevs() <= ix )
		return E_INVALIDARG;

	if( MMSYSERR_NOERROR != midiInGetDevCaps( ix, &midiCaps, sizeof(MIDIINCAPS) ) )
		return E_UNEXPECTED;

	*pName = midiCaps.szPname;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

bool CMidiSource::IsDeviceOpen( UINT ix ) const
{
	return ( 0 != std::count_if(	m_vecPorts.begin(), m_vecPorts.end(), foIsDeviceAtIx( ix ) ) );
}

//////////////////////////////////////////////////////////////////////

UINT CMidiSource::GetPortCount()
{
	return (UINT)m_vecPorts.size();
}




HMIDIIN	CMidiSource::GetMidiInHandle( UINT nPort )
{
	if ( nPort < 0 || nPort >= GetPortCount() )
	{
		ASSERT( 0 );
		return NULL; // nocando
	}

	CPort *pPort = m_vecPorts[nPort];
	if ( !pPort )
		return NULL;

	return pPort->hmi;
}


//////////////////////////////////////////////////////////////////////

UINT CMidiSource::deviceIdFromPort( UINT nPort )
{
	if ( nPort < 0 || nPort >= GetPortCount() )
	{
		ASSERT( 0 );
		return INT_MAX; // nocando
	}

	CPort *pPort = m_vecPorts[nPort];
	ASSERT( pPort );
	ASSERT( pPort->nPort == nPort );

	return pPort->uID;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::GetPortName( UINT nPort, TCHAR* szPname, UINT nLength )
{
	if (nPort < 0 )
		return E_INVALIDARG;

	CString str;

	if (nPort >= GetPortCount()) // if the argument is too big return "no device""
	{
		CString strNoDevice;
		strNoDevice = "none";
		str.Format( _T("port %d"), nPort + 1, strNoDevice );
	}
	else
	{
		// grab the name that we cached in the CPort class
		if (nPort < m_vecPorts.size() )
			str.Format( _T("port %d"), nPort + 1, m_vecPorts[nPort]->strName );
		else
			return E_FAIL;
	}

	int nNameLen = min( nLength, static_cast<unsigned>(str.GetLength()) + 1 );
	_tcsnccpy( szPname, str, nNameLen );
	szPname[nLength - 1] = _T('\0'); // make sure the STL_STRING is terminated
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::AddInPort( UINT uID )
{
	CPort* pPort = new CPort();
	if (pPort == NULL)
		return E_FAIL;

	pPort->pSource = this;
	pPort->nPort = GetPortCount(); // will be labeled by refresh call
	// although the port label is represented by the port's ordinal position,
	// it is not very practical to "look"" up the ports index in the middle
	// of a MIDI callback. Instead, we keep a duplicate that is much more
	// quickly accessible, at the expense of having to update it
	// during offline configuration changes - ARR

	pPort->uID = uID;

	// cache the device name in case the device has a problem later and we can't 
	// ask it for its name directly.
	MIDIINCAPS mic;
	if (MMSYSERR_NOERROR == midiInGetDevCaps( uID, &mic, sizeof(mic ) ))
	{
		pPort->strName = mic.szPname;
	}
	else
	{
		ASSERT(0);
		return E_FAIL;
	}

	MMRESULT mmRes = midiInOpen( &(pPort->hmi), uID, (DWORD_PTR)callback, (DWORD_PTR)pPort, CALLBACK_FUNCTION );
	if (MMSYSERR_NOERROR != mmRes)
	{
		ASSERT(0);
		delete pPort;
		return E_FAIL;
	}
	mmRes = midiInStart( pPort->hmi );
	if (MMSYSERR_NOERROR != mmRes)
	{
		ASSERT(0);
		delete pPort;
		return E_FAIL;
	}

	CSFKCriticalSectionAuto cs( m_csPortCollection );
	try { m_vecPorts.push_back( pPort ); } 
	catch(...) { return E_FAIL; }

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::RemoveAllInPorts()
{
	CSFKCriticalSectionAuto cs( m_csPortCollection );

	// Stop collecting MIDI input when we are removing devices, so we don't run into 
	// problems when we try and close it (sysx data could be left in queue)
	SetEnableInput( FALSE );

	for (PortVectorIt it = m_vecPorts.begin(); it != m_vecPorts.end(); it++)
	{
		CPort *pPort = *it;
		ASSERT( pPort );
		if (pPort->hmi)
		{
			MMRESULT mmr = midiInStop( pPort->hmi );
			if( mmr != MMSYSERR_NOERROR )
				return( E_FAIL );

			VERIFY( SUCCEEDED ( m_pSysexInput->Flush( pPort->hmi ) ) );

			mmr = midiInClose( pPort->hmi );
			if (mmr != MMSYSERR_NOERROR)
				return( E_FAIL ); // The device could not be closed. Maybe it got unplugged?
				// This scenario can be tested by unplugging a MidiMan Oxygen8 while P5 is running.
		}
		delete pPort;	
	}

	m_vecPorts.clear();

	// Start collecting input again
	SetEnableInput( TRUE );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::ResetAllInPorts()
{
	CSFKCriticalSectionAuto cs( m_csPortCollection );

	std::vector<UINT> uIdVect;
	for (PortVectorIt it = m_vecPorts.begin(); it != m_vecPorts.end(); it++)
	{
		CPort *pPort = *it;
		ASSERT( pPort );
		if (pPort->hmi)
		{
			try { uIdVect.push_back( pPort->uID ); } 
			catch(...) { return E_FAIL; }
		}
	}
	
	if (FAILED ( RemoveAllInPorts() ) )
		return E_FAIL;

	std::vector<UINT>::iterator itr;
	for (itr = uIdVect.begin(); itr != uIdVect.end(); ++itr)
	{
		if (FAILED( AddInPort( (*itr) ) ))
			return E_FAIL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::AddSink( IMidiSink *pSink )
{
	HRESULT hr = E_FAIL;

	if ( 0 == m_setPrimarySinks.count( pSink ) && 0 == m_setSinks.count( pSink ) ) // if it is unique
	{
		m_setSinks.insert(MidiSinkSet::value_type( pSink ) );
		hr = S_OK;
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////
// adds the sink that can "swallow"" midi messages
HRESULT CMidiSource::AddPrimarySink( IMidiSink *pSink )
{
	HRESULT hr = E_FAIL;

	if ( 0 == m_setPrimarySinks.count( pSink ) && 0 == m_setSinks.count( pSink ) ) // if it is unique
	{
		m_setPrimarySinks.insert(MidiSinkSet::value_type( pSink ) );
		hr = S_OK;
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::RemovePrimarySink( IMidiSink *pSink )
{
	HRESULT hr = E_FAIL;

	MidiSinkIt itFindSink = m_setPrimarySinks.find( pSink );
	if (itFindSink != m_setPrimarySinks.end()) // if it is found
	{
		// it was found, remove it.
		m_setPrimarySinks.erase( itFindSink );
		hr = S_OK;
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::RemoveSink( IMidiSink *pSink )
{
	HRESULT hr = E_FAIL;

	MidiSinkIt itFindSink = m_setSinks.find( pSink );
	if (itFindSink != m_setSinks.end()) // if it is found
	{
		// it was found, remove it.
		m_setSinks.erase( itFindSink );
		hr = S_OK;
	}

	return hr;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiSource::deliverToSinks( DWORD dwMsg, DWORD dwTime, CPort* pPort )
{
	// if one of the primary sinks indicates that it "ate"" the message, return.
	BOOL bEaten = FALSE;
	for (MidiSinkIt it = m_setPrimarySinks.begin(); it != m_setPrimarySinks.end(); it++)
	{
		if ( (*it)->OnMidiShortMsg( dwMsg, dwTime, pPort->nPort ) )
			bEaten = TRUE;
	}

	if (bEaten)
		return S_OK;

	// otherwise, deliver it to the other sinks
	std::for_each(  m_setSinks.begin(), m_setSinks.end(), notifySinks( dwMsg, dwTime, pPort ) );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void CMidiSource::OnMidiData( CPort* pPort, DWORD dw1, DWORD dw2 )
{
	pPort->pSource->deliverToSinks( dw1, dw2, pPort );
}

void CMidiSource::OnMidiLongData( CPort* pPort, MIDIHDR* pHdr )
{
	pPort->pSource->m_pSysexInput->OnBufferReceived( pPort, pHdr );
}

//////////////////////////////////////////////////////////////////////

void CALLBACK CMidiSource::callback( HMIDIIN hmi, UINT wMsg, DWORD dwInstance, DWORD dw1, DWORD dw2 )
{
	if (s_bEnableInput)
	{
		if (MIM_DATA == wMsg)
		{
			// in this case, dw1 is the Data for the short MIDI event packed into a DWORD
			// dw2 is a timestamp
			if (0x000000FE == dw1)
				return; // ignore active sensing
			
			CPort *pPort = (CPort*)dwInstance;
			OnMidiData( pPort, dw1, dw2 );
		}
		else if ( MIM_LONGDATA == wMsg )
		{
			MIDIHDR*    pHdr = (MIDIHDR*)dw1;
			ASSERT( pHdr );
			CPort *pPort = (CPort*)dwInstance;
			OnMidiLongData( pPort, pHdr );
		}
	}
}

//////////////////////////////////////////////////////////////////////


COutPort::COutPort()
{
}

COutPort::~COutPort()
{
}


COutPort& COutPort::operator=( const COutPort& rhs )
{
	if( &rhs != this )
	{
		ASSERT(0); // this thing doesn't copy every member.  Why is it here?
		hmo = rhs.hmo;
		uID = rhs.uID;
		pOut = rhs.pOut;
		nPort = rhs.nPort;
	}
	return *this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Static class for enumerating Output Ports.  No need at this point
// instantiate anything which encapsulates Output Ports.  The Sync
// Output is the only thing that uses Midi Out and that just opens
// a single ports as needed.  

//static 
HRESULT CMidiOuts::GetDeviceCount( UINT* pnCount )
{
	*pnCount = midiOutGetNumDevs();
	return S_OK;
}

//static 
HRESULT CMidiOuts::GetDeviceName( UINT ix, CString* pName, bool* pbHwPort ) // = NULL
{
	MIDIOUTCAPS midiCaps;

	if (!pName)
		return E_POINTER;

	if( midiOutGetNumDevs() <= ix )
		return E_INVALIDARG;

	if( MMSYSERR_NOERROR != midiOutGetDevCaps( ix, &midiCaps, sizeof(MIDIOUTCAPS) ) )
		return E_UNEXPECTED;

	*pName = midiCaps.szPname;
	if ( pbHwPort )
		*pbHwPort = (midiCaps.wTechnology == MOD_MIDIPORT);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

CMidiOuts::CMidiOuts( ):
	m_bCanDoMidiOut( FALSE ), 
	m_bPingQueue( FALSE )
{
	m_lTickSent = -1;
}

//////////////////////////////////////////////////////////////////////

CMidiOuts::~CMidiOuts()
{
	HRESULT hr = RemoveAllOutPorts();
	ASSERT( SUCCEEDED( hr ) );
}


//////////////////////////////////////////////////////////////////////

HRESULT CMidiOuts::RemoveAllOutPorts()
{
	CSFKCriticalSectionAuto cs( m_csOutPortCollection );

	for( OutPortVectorIt it = m_vecOutPorts.begin(); it != m_vecOutPorts.end(); it++ )
	{
		COutPort* pPort = *it;
		ASSERT( pPort );
		if (pPort->hmo)
		{
			if( midiOutClose( pPort->hmo ) != MMSYSERR_NOERROR )
				return( E_FAIL ); // The device could not be closed. Maybe it got unplugged?
				// This scenario can be tested by unplugging a MidiMan Oxygen8 while P5 is running.
		}
		delete pPort;	
	}

	m_vecOutPorts.clear();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiOuts::ResetAllOutPorts()
{
	CSFKCriticalSectionAuto cs( m_csOutPortCollection );

	std::vector<UINT> uIdVect;
	for( OutPortVectorIt it = m_vecOutPorts.begin(); it != m_vecOutPorts.end(); it++)
	{
		COutPort* pPort = *it;
		ASSERT( pPort );
		if( pPort->hmo )
		{
			try { uIdVect.push_back( pPort->uID ); } 
			catch(...) { return E_FAIL; }
		}
	}
	
	if( FAILED ( RemoveAllOutPorts() ) )
		return E_FAIL;

	std::vector<UINT>::iterator itr;
	for (itr = uIdVect.begin(); itr != uIdVect.end(); ++itr)
	{
		if (FAILED( AddOutPort( (*itr) ) ))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT  CMidiOuts::AddOutPort( UINT uID )
{
	COutPort* pPort = new COutPort();
	if( pPort == NULL )
		return E_FAIL;

	pPort->pOut = this;
	pPort->nPort = GetPortCount(); // will be labeled by refresh call
	// although the port label is represented by the port's ordinal position,
	// it is not very practical to "look" up the ports index in the middle
	// of a MIDI callback. Instead, we keep a duplicate that is much more
	// quickly accessible, at the expense of having to update it
	// during offline configuration changes - ARR

	pPort->uID = uID;

	// cache the device name in case the device has a problem later and we can't 
	// ask it for its name directly.
	MIDIOUTCAPS moc;
	if( MMSYSERR_NOERROR == midiOutGetDevCaps( uID, &moc, sizeof(moc) ) )
	{
		pPort->strName = moc.szPname;
	}
	else
	{
		ASSERT(0);
		return E_FAIL;
	}

	if( 0 != midiOutOpen( &(pPort->hmo), uID, 0, 0, 0 ) )
	{
		ASSERT(0);
		delete pPort;
		return E_FAIL;
	}

	CSFKCriticalSectionAuto cs( m_csOutPortCollection );
	try { m_vecOutPorts.push_back( pPort ); } 
	catch(...) { return E_FAIL; }

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

UINT CMidiOuts::GetPortCount()
{
	return (UINT)m_vecOutPorts.size();
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiOuts::GetPortName( UINT nPort, TCHAR* szPname, UINT nLength )
{
	if( nPort < 0 )
		return E_INVALIDARG;

	CString str;

	if( nPort >= GetPortCount() ) // if the argument is too big return "no device""
	{
		CString strNoDevice;
		strNoDevice = _T("none");
		str.Format( _T("port %d"), nPort + 1, strNoDevice );
	}
	else
	{
		// grab the name that we cached in the CPort class
		if( nPort < m_vecOutPorts.size() )
			str.Format( _T("port %d"), nPort + 1, m_vecOutPorts[nPort]->strName );
		else
			return E_FAIL;
	}

	int nNameLen = min( nLength, static_cast<unsigned>(str.GetLength()) + 1 );
	_tcsnccpy( szPname, str, nNameLen );
	szPname[nLength - 1] = _T('\0'); // make sure the STL_STRING is terminated
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiOuts::GetPortName( UINT nPort, CString* pStr, UINT nLength )
{
	if( nPort < 0 )
		return E_INVALIDARG;

	if( nPort >= GetPortCount() ) // if the argument is too big return "no device""
	{
		CString strNoDevice;
		strNoDevice = _T("none");
		pStr->Format( _T("port %d"), nPort + 1, strNoDevice );
	}
	else
	{
		// grab the name that we cached in the CPort class
		if( nPort < m_vecOutPorts.size() )
			pStr->Format( _T("port %d"), nPort + 1, m_vecOutPorts[nPort]->strName );
		else
			return E_FAIL;
	}
	return S_OK;
}


COutPort* CMidiOuts::GetOutPort( UINT nPort, BOOL bByID /*= FALSE*/ )
{
	// Are we getting the port by index?
	if( bByID )
	{
		for( OutPortVectorIt it = m_vecOutPorts.begin(); it < m_vecOutPorts.end(); it++ )
		{
			if( (*it)->uID == nPort )
				return (*it);
		}
		
	}
	else // getting it by index into the list
	{
		if( 0 <= nPort && nPort < GetPortCount() )
			return m_vecOutPorts[nPort];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////

bool CMidiOuts::IsDeviceOpen( UINT ix ) const
{
	return ( 0 != std::count_if(	m_vecOutPorts.begin(), m_vecOutPorts.end(), foIsDeviceAtIx( ix ) ) );
}

//////////////////////////////////////////////////////////////////////

UINT CMidiOuts::deviceIdFromPort( UINT nPort )
{
	if ( nPort < 0 || nPort >= GetPortCount() )
	{
		ASSERT( 0 );
		return INT_MAX; // nocando
	}

	COutPort *pPort = m_vecOutPorts[nPort];
	ASSERT( pPort );
	ASSERT( pPort->nPort == nPort );

	return pPort->uID;
}


HRESULT CMidiOuts::SendMidiShort( UINT nPort, DWORD dwMsg )
{
	COutPort* pOut = GetOutPort( nPort );
	if ( !pOut )
		return( E_INVALIDARG );
	return SendMidiShort( pOut->hmo, dwMsg );
}


HRESULT CMidiOuts::SendMidiShort( HMIDIOUT hmo, DWORD dwMsg )
{
	MMRESULT mmr = midiOutShortMsg( hmo, dwMsg );
	if ( 0 != mmr )
		return E_FAIL;
	return S_OK;
}

static bool s_bTraceSend = false;

HRESULT CMidiOuts::SendMidiLong( UINT nPort, const BYTE* pbLongMsg, DWORD cbLongMsg )
{
	if ( s_bTraceSend )
		traceSendLong( pbLongMsg, cbLongMsg );

	COutPort* pOut = GetOutPort( nPort );
	if ( !pOut )
		return( E_INVALIDARG );

	MIDIHDR*	const pHdr = (MIDIHDR*)GlobalAllocPtr( GHND|GMEM_SHARE, sizeof MIDIHDR );
	if (!pHdr)
		return FALSE;

	// Make global, GMEM_SHARE copy of input. 
	LPSTR const pGbData = (LPSTR)GlobalAllocPtr( GHND|GMEM_SHARE, cbLongMsg );
	if (!pGbData)
	{
		GlobalFreePtr( pHdr );
		return FALSE;
	}
   
	hmemcpy( pGbData, pbLongMsg, cbLongMsg ); // copy the data to "safe" memory
	pHdr->lpData			= pGbData;
	pHdr->dwBufferLength	= cbLongMsg;

	MMRESULT mmr = MMSYSERR_NOERROR;

	mmr = midiOutPrepareHeader( pOut->hmo, pHdr, sizeof MIDIHDR );
	if (MMSYSERR_NOERROR == mmr )
	{
		mmr = midiOutLongMsg( pOut->hmo, pHdr, sizeof MIDIHDR );
		if (MMSYSERR_NOERROR == mmr )
		{
			// NOTE: Occasionally, we've seen this thing go into an endless loop
			// caused by driver problems.  To avoid this we monitor progress and use
			// a (arbitrary) maximum timeout.
			DWORD const dwTimeOut = ::GetTickCount() + 5000; // msec resolution: 5 seconds 
			while (!(pHdr->dwFlags & MHDR_DONE))
			{
				if (::GetTickCount() >= dwTimeOut)
					break;
			}
		}

		VERIFY( MMSYSERR_NOERROR == midiOutUnprepareHeader( pOut->hmo, pHdr, sizeof MIDIHDR ) );
	}

   GlobalFreePtr( pGbData );
	GlobalFreePtr( pHdr );

	return MMSYSERR_NOERROR == mmr ? S_OK : E_FAIL;
}

//
void CMidiOuts::traceSendLong( const BYTE* pby, DWORD dwBytes )
{
#if !PRODUCTION
	TRACE( _T("\n") );
	TRACE( _T("Sending Long Msg at %d:\n"), ::GetTickCount() );

	TRACE( _T("Index:\t") );
	for ( DWORD i = 0; i < dwBytes; i++ )
	{
		TRACE( _T("%2d "), i );
	}
	TRACE( _T("\n") );

	TRACE( _T("Data:\t") );
	for ( DWORD i = 0; i < dwBytes; i++ )
	{
		TRACE( _T("%2x "), pby[i] );
	}
	TRACE( _T("\n") );
	TRACE( _T("\n") );
#endif
}

//////////////////////////////////////////////////////////////////////

HRESULT CMidiOuts::Initialize()
{
	return S_OK;
}



//////////////////////////////////////////
//
// CSysxInQueue
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CSysxInQueue::Init(  )
{

	size_t sQ = NUM_SYSEX_BUFFERS * 2 + 1;
	if ( m_sysxQueue.empty() )
		m_sysxQueue.resize( sQ );

	// create buffers for all input drivers
	UINT cPorts = m_pSources->GetPortCount();
	for ( UINT ix = 0; ix < cPorts; ix++ )
	{
		HMIDIIN hmi = m_pSources->GetMidiInHandle( ix );
		for ( size_t ixBuf = 0; ixBuf < NUM_SYSEX_BUFFERS; ixBuf++ )
			createBuffer( hmi );
	}
	return S_OK;
}
	
/////////////////////////////////////////////////////////////////////////////

HRESULT CSysxInQueue::Close( )
{
	if ( m_sysxQueue.empty() )
		return S_OK;

	CSFKCriticalSectionAuto cs( m_cs );

	for ( size_t ix = 0; ix < m_sysxQueue.size(); ix++ )
	{
		LPMIDIHDR pDel = m_sysxQueue[ix].phdr;
		if ( pDel )
			GlobalFreePtr( pDel );
		m_sysxQueue[ix].phdr = NULL;
	}
	m_ixIn = m_ixOut = 0;	// zero these too for good measure

	return S_OK;
}
	
/////////////////////////////////////////////////////////////////////////////

HRESULT CSysxInQueue::OnBufferReceived( CPort* pPort, LPMIDIHDR pHdr )
{
	// Called from the MIDI input callback when the driver returns a buffer.
	if ( m_sysxQueue.empty() )
		return( E_FAIL );

	HeaderCountMapIterator it = m_mapHeaderCount.find( pPort->hmi );
	if ( m_mapHeaderCount.end() != it )
	{
		it->second--;	// we got one back, driver has one fewer
		if ( 0 == it->second )
			m_mapHeaderCount.erase( it );

		m_sysxQueue[ m_ixIn ].phdr = pHdr;
		m_sysxQueue[ m_ixIn ].uPort = pPort->nPort;
		if (++m_ixIn == m_sysxQueue.size())
			m_ixIn = 0;
	}
	return S_OK;
}
	
//------------------------------------------------------------------
// Create a SYSEX header and add it to the given driver
HRESULT CSysxInQueue::createBuffer( HMIDIIN hmi )
{
	char* const pMem = (char*)GlobalAllocPtr( GHND|GMEM_SHARE, SYSEX_BUF_SIZE );
	if (pMem)
	{
		// Here also, be concerned about the GlobalPageLock() issue. Allocate
		// the memory for the MIDIHDR to be at least 4096/64 bytes. That wastes
		// all but the first sizeof MIDIHDR bytes. But it is the only way to
		// guarantee we'll get enough MIDIHDR allocations for which the
		// subsequent midiInPrepareHeader() call will succeed.
		MIDIHDR* const pHdr = (MIDIHDR*)GlobalAllocPtr( GHND|GMEM_SHARE, max( 4096/64, sizeof MIDIHDR ) );
		if (pHdr)
		{
			pHdr->lpData			= pMem;
			pHdr->dwBufferLength	= SYSEX_BUF_SIZE;
			pHdr->dwBytesRecorded= 0;
			pHdr->dwFlags			= 0;	// req'd by MMSYSTEM for bits driver ignores
			pHdr->dwUser			= (DWORD_PTR)hmi;

			// Have driver prepare sysx Rx data buffer.
			if (MMSYSERR_NOERROR == ::midiInPrepareHeader( hmi, pHdr, sizeof MIDIHDR ))
			{
				// Add the buffer to this MIDI In's sysx Rx buffer queue.
				if (MMSYSERR_NOERROR == ::midiInAddBuffer( hmi, pHdr, sizeof MIDIHDR ))
				{
					HeaderCountMapIterator it = m_mapHeaderCount.find( hmi );
					if ( it != m_mapHeaderCount.end() )
						it->second++;
					else
						m_mapHeaderCount[hmi] = 1;
					return S_OK;
					///////////
				}

				MAYBE_TRACE( "midiInAddBuffer() failed\n" );
				VERIFY( MMSYSERR_NOERROR == ::midiInUnprepareHeader( hmi, pHdr, sizeof MIDIHDR ));
			}
			else
				MAYBE_TRACE( "midiInPrepareHeader() failed with\n" );
			GlobalFreePtr( pHdr );
		}
		else
			MAYBE_TRACE( "GlobalAllocPtr() MIDIHDR failed\n" );
		GlobalFreePtr( pMem );
	}
	else
		MAYBE_TRACE( "GlobalAllocPtr( %u ) failed\n", SYSEX_BUF_SIZE );

	return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////

HRESULT CSysxInQueue::Flush( HMIDIIN hmi )
{
	// Reset/start will cause the input driver to cough up everything in its
	// queue, including buffers not yet used.
	MAYBE_TRACE( "Flush: Call midiInReset()\n" );
	::midiInReset( hmi );

	// Get buffers out of the queue.
	// Wait until GetBuffer() returns FALSE 
	while (GetBuffer( NULL, NULL, FALSE, NULL ))
		NULL;

	// All pending sysex buffers should have been returned to us when midiInReset was called above
	if ( m_mapHeaderCount.count( hmi ) > 0 )
	{
		// force to zero since there is nothing else we can do to rectify this
		TRACE( "Pending Sysex buffers nor returned by MIDI input driver\r\n" );
		m_mapHeaderCount.erase( hmi );
	}
	return S_OK;
}


//--------------------------------------------------------------------------
// This copies the buffer data to pMem, then frees the buffer and removes
// it from the queue. It is up to the caller to ensure that pMem points
// to at least as many bytes as was allocated in createBuffer()
// Returns FALSE if no buffers are filled and ready.
//
// To get all waiting buffers, keep calling until this returns FALSE.
BOOL CSysxInQueue::GetBuffer( BYTE* pMem, WORD* pwSize, BOOL bRecycle, UINT* puPort )
{
	if ( m_sysxQueue.empty() || m_ixOut == m_ixIn )
		return FALSE;

	if ( pwSize )
		*pwSize = 0;

	// Get next pointer from queue.
	MIDIHDR* const pHdr = m_sysxQueue[ m_ixOut ].phdr;
	m_sysxQueue[ m_ixOut ].phdr = NULL;
	if ( puPort )
		*puPort = m_sysxQueue[ m_ixOut ].uPort;

	// Bump the queue out index
	if (++m_ixOut == m_sysxQueue.size())
		m_ixOut = 0;

	// If data, copy it to pMem
	if (pHdr->dwBytesRecorded && pMem && pwSize)
	{
		ASSERT( pwSize );	// if pMem !=NULL, so must pwSize
		ASSERT( pHdr->dwBytesRecorded <= USHRT_MAX );
		WORD const wSize = (WORD)pHdr->dwBytesRecorded;
		*pwSize = wSize;
		ASSERT( !IsBadWritePtr( pMem, wSize ) );
		_fmemcpy( pMem, pHdr->lpData, wSize );
	}

	HMIDIIN const hmi = (HMIDIIN)pHdr->dwUser;

	if (bRecycle)
	{
		// Unprepare the header
		VERIFY( MMSYSERR_NOERROR == ::midiInUnprepareHeader( hmi, pHdr, sizeof MIDIHDR ) );

		// We're about to give this right back to the MIDI driver.
		// So reset some fields the same way createBuffer() does.
		// (Some fields' current values should be left to what
		// createBuffer() set them to.)
		pHdr->dwBytesRecorded	= 0;
		pHdr->dwFlags				= 0;	// req'd by MMSYSTEM for bits driver ignores

		// Re-prepare the header:
		VERIFY( MMSYSERR_NOERROR == ::midiInPrepareHeader( hmi, pHdr, sizeof MIDIHDR ) );
	}

	// Recycle or discard the buffer now.
	if (bRecycle && (MMSYSERR_NOERROR == ::midiInAddBuffer( hmi, pHdr, sizeof MIDIHDR )))
	{
		HeaderCountMapIterator it = m_mapHeaderCount.find( hmi );
		if ( it != m_mapHeaderCount.end() )
			it->second++;
		else
			m_mapHeaderCount[hmi] = 1;
	}
	else
   {
		// Free
		VERIFY( MMSYSERR_NOERROR == ::midiInUnprepareHeader( hmi, pHdr, sizeof MIDIHDR ) );
		GlobalFreePtr( pHdr->lpData );
		GlobalFreePtr( pHdr );
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////


