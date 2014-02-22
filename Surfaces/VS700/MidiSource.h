// MidiSource.h: declaration of IMidiSink and CMidiSource classes
//
// Copyright (c) 2002 Twelve Tone Systems, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////


#pragma once

#define MAX_PORTS 64

// forward
class CMidiSource;
class CTimer;
class CTimerClient;

#include "MidiSink.h"
#include "sfkutils.h"		//For CTimerClient
#include "MMSystem.h"

//////////////////////////////////////////////////////////////////////

class CPort
{
public:
	UINT				uID;		// port's ID
	HMIDIIN			hmi;		// ports handle
	CMidiSource*	pSource;	// for callback
	UINT				nPort;	// port number exposed to the application
	CString			strName;	// port name exposed to the application

	WORD				wChanMask;
	std::set<WORD> setMessageExclude;
};


class CSysxInQueue
{
public:
// Ctors
	CSysxInQueue( CMidiSource* pSources ) :
		m_ixIn( 0 ),
		m_ixOut( 0 ),
		m_pSources( pSources )
	{ 
	}

// Attributes
	BOOL IsEmpty() { return m_ixIn == m_ixOut; }

// Operations
	HRESULT Init( );
	HRESULT Flush( HMIDIIN hmi );
	BOOL GetBuffer( BYTE* pMem, WORD* pwSize, BOOL bRecycle, UINT* puPort );
	HRESULT Close( );

	HRESULT OnBufferReceived( CPort* pPort, LPMIDIHDR pHdr );

// Implementation
private:

	typedef std::map<HMIDIIN,UINT>	HeaderCountMap;
	typedef HeaderCountMap::iterator	HeaderCountMapIterator;

	HeaderCountMap				m_mapHeaderCount;

	struct SYSEXQNODE
	{
		SYSEXQNODE() : phdr(NULL), uPort(0){}
		LPMIDIHDR	phdr;
		UINT			uPort;
	};

	std::vector<SYSEXQNODE>	m_sysxQueue;
	size_t volatile	m_ixIn;
	size_t				m_ixOut;
	CMidiSource*		m_pSources;


	HRESULT createBuffer( HMIDIIN hmi );

	CSFKCriticalSection		m_cs;
};


//////////////////////////////////////////////////////////////////////

typedef std::set< IMidiSink* > MidiSinkSet;
typedef MidiSinkSet::iterator MidiSinkIt;

typedef std::vector< CPort* >						PortVector;
typedef PortVector::iterator						PortVectorIt;

//////////////////////////////////////////////////////////////////////
// CMidiSource: abstracts the hardware presenting a flat list of ports
// to the application. Forwards MIDI input to attached MidiSinks and
// tags the call with the port identifier.
class CMidiSource :public CTimerClient
{
public:
	// accessors for hardware devices
	HRESULT GetDeviceCount( UINT* pnCount ) const;
	HRESULT GetDeviceName( UINT ix, CString* pName ) const;
	bool IsDeviceOpen( UINT ix ) const;

	HRESULT OnPortsChanged();
	
	//CTimerClient overrides
	virtual void Tick();

	CMidiSource( CTimer *pTimer );
	virtual ~CMidiSource();
	HRESULT Initialize();

	// accessors to get port information
	UINT GetPortCount();	// returns count of active ports
	HMIDIIN	GetMidiInHandle( UINT nPort );

	HRESULT GetPortName( UINT nPort, TCHAR* szPname, UINT nLength );	// returns name of port
	// identified by nPort. nLength is input for capacity of STL_STRING pointed to by szPname

	// accessors to define port
	HRESULT AddInPort( UINT uID );
	HRESULT RemoveAllInPorts();
	HRESULT ResetAllInPorts();

	HRESULT AddPrimarySink( IMidiSink *pSink );
	HRESULT AddSink( IMidiSink *pSink );
	HRESULT RemovePrimarySink( IMidiSink *pSink );
	HRESULT RemoveSink( IMidiSink *pSink );

	static void SetEnableInput( BOOL bEnable );
	HRESULT SetNoEchoMask( UINT nPort, WORD wChanMask, std::set<WORD>& setMessageExclude );
	HRESULT ClearNoEchoMask();

	///////////////////////////////////////////////////////////////////////
	// useful Functors
	struct foIsDeviceAtIx : public std::unary_function<CPort*, bool>
	{
		foIsDeviceAtIx( unsigned ix ): m_pComp(ix){}
		bool operator()( CPort* pPort ) const
		{
			return (pPort->uID == m_pComp);
		}
		unsigned m_pComp;
	};
	
	static void OnMidiData( CPort* pPort, DWORD dw1, DWORD dw2 );
	static void OnMidiLongData( CPort* pPort, MIDIHDR* pHdr );

	CSysxInQueue*	m_pSysexInput;

private:

	static void CALLBACK callback( HMIDIIN hmi, UINT wMsg, DWORD dwInstance, DWORD dw1, DWORD dw2 );

	HRESULT deliverToSinks( DWORD dwMsg, DWORD dwTime, CPort* pPort );
	UINT deviceIdFromPort( UINT nPort );

private:
	CSFKCriticalSection		m_csPortCollection;

	// data members
	MidiSinkSet		m_setPrimarySinks; // sinks who can "swallow"" the midi events.
	MidiSinkSet		m_setSinks;

	// allow priveledged access to the dialogs that modify it.
	friend class CMidiPortsDlg; 
	friend class CMidiPortWarnDlg;
	PortVector		m_vecPorts;

	//////////////////////////////////////////////
	// Function objects
	struct notifySinks : public std::unary_function<IMidiSink*,void>
	{
		notifySinks( DWORD dwMsg, DWORD dwTime, CPort* pPort ) : m_dwMsg(dwMsg), m_dwTime(dwTime), m_pPort(pPort)
		{
		}
		void const operator () (IMidiSink* pSink);
		DWORD m_dwMsg;
		DWORD m_dwTime;
		CPort* m_pPort;
	};

	CTimer*	m_pTimer;
};



// Forward declarations for CMidiOuts
class CMidiOuts;
class CMfxEventQueue;
class CMfxDataQueue;
class CMfxEvent;


class COutPort
{
public:
	COutPort();
	~COutPort();

	COutPort& operator=( const COutPort& rhs );

	UINT					uID;			// port's ID
	HMIDIOUT				hmo;			// ports handle
	CMidiOuts*			pOut;			// for callback
	UINT					nPort;		// port number exposed to the application
	CString				strName;		// port name exposed to the application
};

//////////////////////////////////////////////////////////////////////

typedef std::vector< COutPort* >		OutPortVector;
typedef OutPortVector::iterator		OutPortVectorIt;

class MIDIDWEVENT
{
public:
	MIDIDWEVENT& operator=(MIDIDWEVENT e)
	{
		ASSERT(0); // this thing doesn't copy every member.  Why is it here?
		tTime = e.tTime;
		dwData = e.dwData;
		hmo = e.hmo;
		return *this;
	}

	LONG tTime;
	LONG tDur;
	DWORD dwData;
	HMIDIOUT hmo;
};


typedef std::vector< MIDIDWEVENT* >		MIDIDWEVENTQUEUE;
typedef MIDIDWEVENTQUEUE::iterator	MidiQVectorIt;



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
 
class CMidiOuts
{
public:
	CMidiOuts();
	virtual ~CMidiOuts();
	HRESULT Initialize();

	//Accessors to get device information
	static HRESULT GetDeviceCount( UINT* pnCount );
	static HRESULT GetDeviceName( UINT ix, CString* pName, bool* pbHwPort = NULL );
					  
	bool IsDeviceOpen( UINT ix ) const;

	// accessors to get port information
	UINT GetPortCount();	// returns count of active ports
	HRESULT GetPortName( UINT nPort, TCHAR* szPname, UINT nLength );	// returns name of port
	// identified by nPort. nLength is input for capacity of STL_STRING pointed to by szPname

	HRESULT GetPortName( UINT nPort, CString* pStr, UINT nLength );

	// accessors to define port
	HRESULT AddOutPort( UINT uID );
	HRESULT RemoveAllOutPorts();
	HRESULT ResetAllOutPorts();

	void SetCanDoMidiOut( BOOL b ) { m_bCanDoMidiOut = b; }

	///////////////////////////////////////////////////////////////////////
	// useful Functors
	struct foIsDeviceAtIx : public std::unary_function<CPort*, bool>
	{
		foIsDeviceAtIx( unsigned ix ): m_pComp(ix){}
		bool operator()( COutPort* pPort ) const
		{
			return( pPort->uID == m_pComp );
		}
		unsigned m_pComp;
	};

	COutPort* GetOutPort( UINT nPort, BOOL bByID = FALSE );

	HRESULT SendMidiShort( HMIDIOUT hmo, DWORD dwMsg );
	HRESULT SendMidiShort( UINT nPort, DWORD dwMsg );
	HRESULT SendMidiLong( UINT nPort, const BYTE* pbLongMsg, DWORD cbLongMsg );

protected:
	UINT deviceIdFromPort( UINT nPort );

private:
	void traceSendLong( const BYTE* pby, DWORD dwBytes );

	CSFKCriticalSection		m_csOutPortCollection;
	CSFKCriticalSection		m_csNotePingQueue;
	CSFKCriticalSection		m_csNotePongQueue;

	OutPortVector			m_vecOutPorts;

	BOOL						m_bCanDoMidiOut;

	BOOL						m_bPingQueue;		// If true, ping queue is read queue, pong is write and vice versa

	LONG						m_lTickSent;

	friend class CMidiPortsDlg; 
	friend class CMidiPortWarnDlg;
};


