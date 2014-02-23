/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkMidi.h:	Definitions of the objects that enable MIDI input and
// output for the framework.
/////////////////////////////////////////////////////////////////////////

#pragma once

#define VAL_ANY						0xFFFFFFFF

/////////////////////////////////////////////////////////////////////////
// used by CMidiMsg
// determines the number of bytes preallocated for sysx messages.
// larger messages work, but require reallocating on the fly.
#define SYSX_BUF_MAX					32
#define MAX_PORTS						64

#include <set>
#include <map>

// forwards
class CMidiMsg;
class CHostNotifyListener;
class CControlSurface;

/////////////////////////////////////////////////////////////////////////
// CNrpnContext:
//
// This object will extract NRPN's from an incomming stream of short
// messages. By making a class derive from CNrpnContext,
// all you have to do is call OnController whenever a controller message
// is received and implement OnNrpn to carry out the desired behavior
// when a NRPN is received.
/////////////////////////////////////////////////////////////////////////
class CNrpnContext
{
public:
	enum EPNType { Unknown, NonRegistered, Registered };
	CNrpnContext():
		m_bAccept14BitOnly( FALSE )
	{
		for (int p = 0; p < MAX_PORTS; p++)
		{
			for (int c = 0; c < 16; c++)
			{
				m_ePNType[p][c] = Unknown;
				m_bNrpnComplete[p][c] = FALSE;
			}
		}
	}

	void OnController( DWORD dwShortMsg, DWORD dwPort );

	virtual void OnNrpn(
		CNrpnContext::EPNType ePNType,
		DWORD dwParamNum,
		DWORD dwChan,
		DWORD dwVal,
		DWORD dwPort
	) = 0;

	EPNType	GetPNType( DWORD dwPort, BYTE byChan ) { return m_ePNType[dwPort][byChan]; }
	DWORD		GetParamMsb( DWORD dwPort, BYTE byChan ) { return m_dwPrmMSB[dwPort][byChan]; }
	DWORD		GetParamLsb( DWORD dwPort, BYTE byChan ) { return m_dwPrmLSB[dwPort][byChan]; }
	DWORD		GetParamNum( DWORD dwPort, BYTE byChan )
	{
		return (m_dwPrmMSB[dwPort][byChan] << 7) | m_dwPrmLSB[dwPort][byChan];
	}
	DWORD		GetValueMsb( DWORD dwPort, BYTE byChan ) { return m_dwValMSB[dwPort][byChan]; }
	DWORD		GetValueLsb( DWORD dwPort, BYTE byChan ) { return m_dwValLSB[dwPort][byChan]; }
	DWORD		GetValue( DWORD dwPort, BYTE byChan )
	{
		return (m_dwValMSB[dwPort][byChan] << 7) | m_dwValLSB[dwPort][byChan];
	}
	BOOL		GetNrpnComplete( DWORD dwPort, BYTE byChan ) { return m_bNrpnComplete[dwPort][byChan]; }
	void		SetAccept14BitOnly( BOOL bAccept )
	{
		m_bAccept14BitOnly = bAccept;
	}

private:

	BOOL			m_bAccept14BitOnly;
	EPNType		m_ePNType[MAX_PORTS][16];
	DWORD			m_dwPrmMSB[MAX_PORTS][16];
	DWORD			m_dwPrmLSB[MAX_PORTS][16];
	DWORD			m_dwValMSB[MAX_PORTS][16];
	DWORD			m_dwValLSB[MAX_PORTS][16];
	BOOL			m_bNrpnComplete[MAX_PORTS][16];
};

/////////////////////////////////////////////////////////////////////////
// CMidiInputRouter:
//
// The CMidiInputRouter is responsible for listening and redirecting raw MIDI
// messages into those CMidiMsg(s) that are registered.
// This implementation creates a hash based index to optimize performance.
/////////////////////////////////////////////////////////////////////////

typedef std::set< CMidiMsg*, std::less< CMidiMsg* >, std::allocator< CMidiMsg* > > MsgSet;
typedef MsgSet::iterator MsgSetIt;

class CMidiInputRouter: public CNrpnContext
{
	friend class CControlSurface;

public:
	void OnShortMsg( DWORD dwShortMsg );
	void OnLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg );

	// CNrpnContext override 
	void OnNrpn( CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort);

	BOOL Add( CMidiMsg *pMsg );
	BOOL Remove( CMidiMsg *pMsg );


private:

	// private constructor because it is a singleton
	CMidiInputRouter( CControlSurface *pSurface );

	friend class CMidiMsg;

	inline BYTE makeHashByte( DWORD dwShortMsg ); // for all valued short messages
	inline BYTE makeHashByte( DWORD cbLongMsg, const BYTE *pbLongMsg );

	// NRPN / RPN handling
	inline BYTE makeHashByte( DWORD dwParamNum, DWORD dwChan ); // for Nrpns and Rpns

	// Trigger handling
	inline void onTrigger( DWORD dwShortMsg );

	// helpers
	inline void deliverShortByIndex( BYTE byIndex, DWORD dwShortMsg, BOOL bTrigger );
	void addToIndex( CMidiMsg *pMsg, BYTE byHash );
	void rebuildIndex();

	// message containers and indexes
	MsgSet			m_setMsgs;
	WORD				m_wExpMsgMask;
	WORD				m_warExpMsgMask[16];
	MsgSet			m_arSetMsgs[16];
	int				m_nMinSysXPrefixLen;
	BOOL				m_bContainsTriggers;

	// helper member classes
	CControlSurface*	m_pSurface;
	DWORD				m_dwMsgCounter;
};

/////////////////////////////////////////////////////////////////////////
// CMidiMsg:
//
// CMidiMsg establishes the identity of a MIDI message for any purpose,
// input or output. It's primary function is to convert between a normalized
// floating point value and a MIDI message in both directions.
// It is capable of dealing with all common MIDI messages including
// sysx, CC, notes, Channel aftertouch, NRPN and RPN.
// Several specialized modalities of interpreting sysx, and CC are also
// implemented.
/////////////////////////////////////////////////////////////////////////


class CMidiMsg
{
public:
	enum eMsgType
	{
		mtCC,         // message consists of the 7 bit value of a specified CC
		mtNrpn,       // message consists of the 14 bit value of a specified NRPN
		mtRpn,        // message consists of the 14 bit value of a specified RPN
		mtSysX7bit,   // message consists of the 7 bit value sandwiched between the specified SYSX Pre and Post strings
		mtSysXHiLo,   // message consists of the 14 bit value sandwiched between the specified SYSX Pre and Post strings (High byte first)
		mtSysXLoHi,   // message consists of the 14 bit value sandwiched between the specified SYSX Pre and Post strings (Low byte first)
		mtSysXString, // message consists of a fixed length forward ordered ASCII string sandwiched between the specified SYSX Pre and Post strings
		mtWheel,      // message consists of the 14 bit value of the pitch wheel
		mtNote,       // message consists of the 7 bit value in the note's velocity
		mtChAft,      // message consists of the 7 bit value of a Channel Aftertouch
		mtWheel7Bit   // message consists of the 7 bit value of the pitch wheel. Not common, use to support operation of t-bar, LFE-send and joystick on VS-700.
	};

	enum ValueMode
	{
		VM_ABS,				// Floating point output is proportional to the MIDI message Value
		VM_DELTA,			// Floating point output is Incr/Decr from previous 
								// depending on MIDI Message value.  Amount of delta is 1/127 of the range
		VM_DELTA_ACCEL,	// Floating point output is Incr/Decr from previous 
								// depending on MIDI Message value.  Amount of delta is proportinal
								// to the difference from the Hinge value
	};

	// struct to use in Delta Modes
	struct ValueRange
	{
		ValueRange():dwL(0),dwH(127){}
		bool InRange(DWORD d){ return dwL <= d && d <= dwH; }
		DWORD dwL;
		DWORD dwH;
	};

	CMidiMsg( CControlSurface* pSurface, LPCTSTR pszName = NULL, DWORD dwID = (DWORD)-1 );

	virtual ~CMidiMsg();
	HRESULT SetMessageType( eMsgType mtType )
	{ 
		m_mtType = mtType;
		switch (m_mtType)
		{
		case mtCC:
		case mtSysX7bit:
		case mtNote:
		case mtChAft:
		case mtWheel7Bit:
			m_dwMaxValue = 0x7F; // 7 bits, 127
			break;

		case mtNrpn:
		case mtRpn:
		case mtSysXHiLo:
		case mtSysXLoHi:
		case mtWheel:
			m_dwMaxValue = 0x3FFF; // 14 bits, 16383
			break;

		case mtSysXString:
			m_dwMaxValue = 0; // this is not a "receivable" message
			break;

		default:
			_ASSERT( 0 );
			return E_INVALIDARG;
		}
		return S_OK; 
	}

	DWORD		GetId() const		{ return m_dwID; }
	WORD		GetStatusWord();

	eMsgType GetMessageType() const { return m_mtType; }

	HRESULT SetCCNum( int iCC )
	{ 
		_ASSERT( m_mtType == mtCC );
		_ASSERT( isValidCC( (WORD)iCC ) ); 
		m_wCC = (WORD)iCC; return S_OK; 
	}	

	DWORD GetCCNum() const { return m_wCC; }


	HRESULT SetNoteNum( int iNote )
	{
		_ASSERT( m_mtType == mtNote );
		_ASSERT( isValidNote( (WORD)iNote ) ); 
		m_wNote = (WORD)iNote; return S_OK; 
	}

	DWORD GetNoteNum() const { return m_wNote; }

	HRESULT SetNrpn( int iNrpn )
	{
		_ASSERT( m_mtType == mtNrpn );
		_ASSERT ( isValidNrpn( (WORD)iNrpn ) );
		m_wNrpn = (WORD)iNrpn; return S_OK;
	}

	DWORD GetNrpn() const { return m_wNrpn; }

	HRESULT SetRpn( int iRpn ) 
	{ 
		_ASSERT( m_mtType == mtRpn );
		_ASSERT ( isValidNrpn( (WORD)iRpn ) ); 
		m_wRpn = (WORD)iRpn; return S_OK; 
	}

	DWORD GetRpn() const { return m_wRpn; }

	HRESULT SetChannel( int iChannel )
	{
		_ASSERT( isValidChannel( (WORD)iChannel ) );
		m_wChannel = (WORD)iChannel; return S_OK;
	}

	WORD GetChannel() const { return m_wChannel; }

	HRESULT SetPort( int iPort )
	{
		_ASSERT( isValidPort( (WORD)iPort ) ); 
		m_wPort = (WORD)iPort;
		return S_OK; 
	}

	WORD GetPort() const { return m_wPort; }

	HRESULT SetSysXPreString( const BYTE *pbyData, DWORD dwLen );
	void GetSysXPreString( BYTE **ppbyData, DWORD *pdwLen );
	DWORD GetSysXPreStrLen() const { return m_dwLenSysXPre; }

	HRESULT SetSysXPostString( const BYTE *pbyData, DWORD dwLen );
	void GetSysXPostString( BYTE **ppbyData, DWORD *pdwLen );
	DWORD GetSysXPostStrLen() const { return m_dwLenSysXPost; }

	// determines truncation (or [space] fill) length for SysXString messages
	// a value of 0 indicates truncation will be done according to
	// length supplied in SendString()
	HRESULT SetSysXTextLen( DWORD dwLen )
	{
		m_dwSysXTextLen = dwLen;
		return S_OK;
	}
	DWORD GetSysXTextLen() { return m_dwSysXTextLen; }

	HRESULT SetSysXPadLen( DWORD dwLen )
	{
		m_dwSysXPadLen = dwLen;
		return S_OK;
	}
	DWORD GetSysXPadLen() { return m_dwSysXPadLen; }

	HRESULT SetSysXTextFillChar( char cFill )
	{
		m_cFill = cFill;

		return S_OK;
	}
	char GetSysXTextFillChar() { return m_cFill; }

	bool IsShortMsg()
	{
		// messages that are sent using one or more short messages
		switch( m_mtType )
		{
		case mtCC:
		case mtNrpn:
		case mtRpn:
		case mtNote:
		case mtChAft:
		case mtWheel:
		case mtWheel7Bit:
			return true;
		}
		return false;
	}

	DWORD GetShortMsgCount()
	{
		switch( m_mtType )
		{
		case mtCC:
		case mtNote:
		case mtChAft:
		case mtWheel:
		case mtWheel7Bit:
			return 1;
		case mtNrpn:
		case mtRpn:
			return 4;
		}
		return 0;
	}

	DWORD MakeShortMsg( DWORD dwVal, int ix = 0 );

	HRESULT SetIsTrigger( bool bIsTrigger, DWORD dwTrigValue )
	{
		switch( m_mtType )
		{
		case mtCC:
		case mtNote:
		case mtChAft:
		case mtWheel:
		case mtWheel7Bit:
			m_bIsTrigger = bIsTrigger;
			m_dwTrigValue = dwTrigValue;
			return S_OK;
		default:
			_ASSERT( 0 );
			// trigger only allowed for above listed message types
			return S_FALSE;
		}
	}

	BOOL IsTrigger() { return m_bIsTrigger; }

	void	SetValueMode (ValueMode vm, ValueRange* prangeCCW = NULL, ValueRange* prangeCW = NULL);

	// when true, attempting to send the same message repeatedly will
	// just send it once.
	bool IsOutputThin() { return m_bThinOutput; }
	void SetIsOutputThin( bool bEnable ) { m_bThinOutput = bEnable; }

	// sends fVal encoded as a MIDI message with the specified attributes
	void Send( float fVal );

	// sends dwVal encoded as a MIDI message with the specified attributes
	void Send( DWORD dwVal );

	// applies to mtSysXString
	void SendText( DWORD cbyString, LPCSTR pszString ); // receives a string of text to send
	// text is sent sandwiched between sysx pre and post strings.
	// the SysXTextLen and SysXTextTrunc parameters control what we do with the string before
	// sending.
	// when m_dwSysXTextLen is nonzero, the string will be either truncated or padded
	// to fit the specified length (using the specified fill character)
	// when m_dwSysXPadLen is nonzero, this number represents the number of pad characters
	// that we force at the end of the string. m_dwSysXTextLen must be nonzero
	// for m_dwSysXPadLen to take effect.

	void Invalidate();
	void SetUseTextCruncher( bool bUseIt ) { m_bUseTextCruncher = bUseIt; }
	void OnShortMsg( DWORD dwMsgCount, DWORD dwShortMsg );
	void OnLongMsg( DWORD dwMsgCount, DWORD cbLongMsg, const BYTE *pbLongMsg );
	void OnNrpn( DWORD dwMsgCount, CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort );

	void SetVal( float f );
	float GetVal() const;
private:
	// Value Validation
	BOOL isValidPort( WORD wPort ) const { return (wPort >= 0 && wPort <= MAX_PORTS); }
	BOOL isValidChannel( WORD wChannel ) const { return (wChannel >= 0 && wChannel <= 15); }
	BOOL isValidNrpn( WORD wNrpn ) const { return (wNrpn >= 0 && wNrpn <= 16383); }
	BOOL isValidCC( WORD wCC ) const { return (wCC >= 0 && wCC <= 127); };
	BOOL isValidNote( WORD wNote ) const { return (wNote >= 0 && wNote <= 127); }

	// helpers
	void sendString( DWORD cbyString, LPCSTR pszString );
	BOOL useTextCruncher() { return m_bUseTextCruncher; }

public:
	enum ValueChange
	{
		VC_None,
		VC_Decrease,
		VC_Increase,
	};
	CString					m_strName;	// useful for debugging
private:
	void setCurrentVal( DWORD dwVal, ValueChange* pvc );		// will apply delta modes


	eMsgType		m_mtType;

	// applies when m_mtType is m_mtCCXXX
	WORD			m_wCC;


	// applies when m_mtType is mtSysXXXX
	BYTE*			m_pbySysXPre; // partial SysX string
	DWORD			m_dwLenSysXPre; // length for above
	BYTE*			m_pbySysXPost; // partial SysX string
	DWORD			m_dwLenSysXPost; // length for above

	// applies when m_mtType is mtSysXText
	DWORD			m_dwSysXTextLen;	// length to which we truncate text strings being sent as sysx
	DWORD			m_dwSysXPadLen;	// when m_dwSysXTextLen is non zero, represents how many pad characters
											// to force at the end of the string.
	char			m_cFill;				// identifies the pad character
	bool			m_bUseTextCruncher;// when true, we reduce the string's length with a smart algorithm
											// to preserve intelligibility.

	// applies when m_mtType is mtNrpn
	WORD			m_wNrpn;

	// applies when m_mtType is mtRpn
	WORD			m_wRpn;

	// applies when m_mtType is mtRpn
	WORD			m_wNote;

	// determines the message's channel (for all message types except SysX)
	WORD			m_wChannel;

	// not implemented, determines the message port
	WORD			m_wPort;

	DWORD			m_dwMaxValue;
	DWORD			m_dwCurValue;
	DWORD			m_dwLastCC;
	BYTE			m_bySysXBuffer[SYSX_BUF_MAX]; // this buffer is used to prevent reallocating for short SysXString messages
	BYTE*			m_pbyLastSysXSent;				// remember the last sysx buffer sent for thinning purposes.
	DWORD			m_dwSizeOfLastSysX;
	DWORD			m_dwLastSentVal;

	bool			m_bThinOutput;
	bool			m_bIsTrigger;
	DWORD			m_dwTrigValue;

	ValueMode	m_eValueMode;

	ValueRange	m_rangeCCW;
	ValueRange	m_rangeCW;


	CControlSurface*		m_pSurface;
	DWORD						m_dwLastMsgCount;

	DWORD						m_dwID;		// a unique ID for callback identification
};

