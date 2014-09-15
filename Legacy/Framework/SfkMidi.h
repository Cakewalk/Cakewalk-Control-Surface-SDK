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
// When a CMidiMsg is created it may be given one or more CMidiInputListeners.
// That action indicates the CMidiMsg is intended for MIDI input,
// and automatically adds the message into the CMidiInputRouter's list.
// Each time a CMidiMsg is added into the list, the index is rebuilt to
// reflect the latest state of the CMidiMsg list.
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

private:

	// private constructor because it is a singleton
	CMidiInputRouter( CControlSurface *pSurface );

	friend class CMidiMsg;

	inline BYTE makeHashByte( DWORD dwShortMsg ); // for all valued short messages
	inline BYTE makeTrigHashByte( DWORD dwShortMsg ); // for all trigger short messages
	
	inline BYTE makeHashByte( DWORD cbLongMsg, const BYTE *pbLongMsg );

	// NRPN / RPN handling
	inline BYTE makeHashByte( DWORD dwParamNum, DWORD dwChan ); // for Nrpns and Rpns

	// Trigger handling
	inline void onTrigger( DWORD dwShortMsg );
	inline void onInvertedCC( DWORD dwShortMsg );

	// helpers
	inline void deliverShortByIndex( BYTE byIndex, DWORD dwShortMsg, BOOL bTrigger );
	void addToIndex( CMidiMsg *pMsg, BYTE byHash );
	BOOL add( CMidiMsg *pMsg );
	BOOL remove( CMidiMsg *pMsg );
	void rebuildIndex();

	// message containers and indexes
	MsgSet			m_setMsgs;
	WORD				m_wExpMsgMask;
	WORD				m_warExpMsgMask[16];
	MsgSet			m_arSetMsgs[16];
	int				m_nMinSysXPrefixLen;
	BOOL				m_bContainsInvertedCCs;
	BOOL				m_bContainsTriggers;

	// helper member classes
	CControlSurface*	m_pSurface;
	DWORD				m_dwMsgCounter;
};

/////////////////////////////////////////////////////////////////////////
// CMidiMsg:
//
// CMidiMsg establishes the identity of a MIDI message for any purpose,
// input or output. It also acts as a multicaster to relay input of MIDI
// events to CMidiMsgListeners.
// It is capable of dealing with all common MIDI messages including
// sysx, CC, notes, Channel aftertouch, NRPN and RPN.
// Several specialized modalities of interpreting sysx, and CC are also
// implemented.
/////////////////////////////////////////////////////////////////////////

// forward
class CMidiMsgListener;

typedef std::set< CMidiMsgListener*, std::less< CMidiMsgListener* >, std::allocator< CMidiMsgListener* > > MsgListenerSet;
typedef MsgListenerSet::iterator MsgListenerSetIt;

class CMidiMsg
{
public:
	enum eMsgType
	{
		mtCC,			// message consists of the 7 bit value of a specified CC
		mtCCInverted,// message is in the CC identity, identity is in the CC value
		mtCCSel,		// message consists of the 7 bit value of a specified CC that follows an enabling CC
		mtCCHiLo,	// message consists of 14 bit value of two consecutive CC's (High byte first)
		mtCCLoHi,	// message consists of 14 bit value of two consecutive CC's (Low byte first)
		mtNrpn,		// message consists of the 14 bit value of a specified NRPN
		mtRpn,		// message consists of the 14 bit value of a specified RPN
		mtSysX7bit,	// message consists of the 7 bit value sandwiched between the specified SYSX Pre and Post strings
		mtSysXHiLo,	// message consists of the 14 bit value sandwiched between the specified SYSX Pre and Post strings (High byte first)
		mtSysXLoHi,	// message consists of the 14 bit value sandwiched between the specified SYSX Pre and Post strings (Low byte first)
		mtSysXString,	// message consists of a fixed length forward ordered ASCII string sandwiched between the specified SYSX Pre and Post strings
		mtWheel,		// message consists of the 14 bit value of the pitch wheel
		mtNote,		// message consists of the 7 bit value in the note's velocity
		mtChAft,		// message consists of the 7 bit value of a Channel Aftertouch
		mtKeyAft	// message consists of the 7 bit value in the note's pressure
	};


	CMidiMsg( CControlSurface* pSurface );

	virtual ~CMidiMsg();
	HRESULT SetMessageType( eMsgType mtType )
	{ 
		m_mtType = mtType;
		switch (m_mtType)
		{
		case mtCC:
		case mtCCInverted:
		case mtCCSel:
		case mtSysX7bit:
		case mtNote:
		case mtChAft:
		case mtKeyAft:
			m_dwMaxValue = 127;
			break;
		case mtCCHiLo:
		case mtCCLoHi:
		case mtNrpn:
		case mtRpn:
		case mtSysXHiLo:
		case mtSysXLoHi:
		case mtWheel:
			m_dwMaxValue = 16383;
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

	DWORD GetMaxNotifyValue() { return m_dwMaxValue; }

	eMsgType GetMessageType() const { return m_mtType; }

	HRESULT SetCCNum( int iCC )
	{ 
		_ASSERT( isValidCC( (WORD)iCC ) ); 
		m_wCC = (WORD)iCC; return S_OK; 
	}	

	DWORD GetCCNum() const { return m_wCC; }

	HRESULT SetCC2Num( int iCC )
	{ 
		_ASSERT( isValidCC( (WORD)iCC ) ); 
		m_wCC2 = (WORD)iCC; return S_OK; 
	}	

	DWORD GetCC2Num() const { return m_wCC2; }

	HRESULT SetCCInvIdentity( int iCCInvId )
	{
		_ASSERT( isValidCC( (WORD)iCCInvId ) ); 
		m_wCCInvIdentity = (WORD)iCCInvId; return S_OK; 
	}

	DWORD GetCCInvIdentity() { return m_wCCInvIdentity; }

	HRESULT SetCCSelVal( int iCC )
	{ 
		_ASSERT( isValidCC( (WORD)iCC ) ); 
		m_wCCSelVal = (WORD)iCC; return S_OK; 
	}	

	DWORD GetCCSelVal() const { return m_wCCSelVal; }

	HRESULT SetCCSelNum( int iCC )
	{ 
		_ASSERT( isValidCC( (WORD)iCC ) ); 
		m_wCCSelNum = (WORD)iCC; return S_OK; 
	}	

	DWORD GetCCSelNum() const { return m_wCCSelNum; }

	HRESULT SetNoteNum( int iNote )
	{
		_ASSERT( isValidNote( (WORD)iNote ) ); 
		m_wNote = (WORD)iNote; return S_OK; 
	}

	DWORD GetNoteNum() const { return m_wNote; }

	HRESULT SetNrpn( int iNrpn )
	{
		_ASSERT ( isValidNrpn( (WORD)iNrpn ) );
		m_wNrpn = (WORD)iNrpn; return S_OK;
	}

	DWORD GetNrpn() const { return m_wNrpn; }

	HRESULT SetRpn( int iRpn ) 
	{ 
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

	// adds / removes an object on which incomming MIDI messages have an effect
	// it is safe to call this method repeatedly.
	HRESULT AddListener( CMidiMsgListener *pListener );
	HRESULT RemoveListener( CMidiMsgListener *pListener );

	void OnMsg( WORD wMidiVal );

	BOOL IsShortMsg()
	{
		// messages that are sent using one or more short messages
		switch( m_mtType )
		{
		case mtCC:
		case mtCCInverted:
		case mtCCSel:
		case mtCCHiLo:
		case mtCCLoHi:
		case mtNrpn:
		case mtRpn:
		case mtNote:
		case mtChAft:
		case mtKeyAft:
		case mtWheel:
			return TRUE;
		}
		return FALSE;
	}

	DWORD GetShortMsgCount()
	{
		switch( m_mtType )
		{
		case mtCC:
		case mtCCInverted:
		case mtNote:
		case mtChAft:
		case mtKeyAft:
		case mtWheel:
			return 1;
		case mtCCSel:
		case mtCCHiLo:
		case mtCCLoHi:
			return 2;
		case mtNrpn:
		case mtRpn:
			return 4;
		}
		return 0;
	}

	DWORD MakeShortMsg( DWORD dwVal, int ix = 0 );

	HRESULT SetIsTrigger( BOOL bIsTrigger, DWORD dwTrigValue )
	{
		switch( m_mtType )
		{
		case mtCC:
		case mtCCInverted:
		case mtNote:
		case mtChAft:
		case mtKeyAft:
		case mtWheel:
		case mtCCSel:
		case mtCCHiLo:
		case mtCCLoHi:
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

	// when true, attempting to send the same message repeatedly will
	// just send it once.
	BOOL IsOutputThin() { return m_bThinOutput; }
	void SetIsOutputThin( BOOL bEnable ) { m_bThinOutput = bEnable; }

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
	void SetUseTextCruncher( BOOL bUseIt ) { m_bUseTextCruncher = bUseIt; }
	void OnShortMsg( DWORD dwMsgCount, DWORD dwShortMsg );
	void OnLongMsg( DWORD dwMsgCount, DWORD cbLongMsg, const BYTE *pbLongMsg );
	void OnNrpn( DWORD dwMsgCount, CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort );

private:
	// Value Validation
	BOOL isValidPort( WORD wPort ) const { return (wPort >= 0 && wPort <= MAX_PORTS); }
	BOOL isValidChannel( WORD wChannel ) const { return (wChannel >= 0 && wChannel <= 15); }
	BOOL isValidNrpn( WORD wNrpn ) const { return (wNrpn >= 0 && wNrpn <= 16383); }
	BOOL isValidCC( WORD wCC ) const { return (wCC >= 0 && wCC <= 127); };
	BOOL isValidNote( WORD wNote ) const { return (wNote >= 0 && wNote <= 127); }

	// helpers
	void sendString( DWORD cbyString, LPCSTR pszString );
	void deliverToListeners();
	BOOL useTextCruncher() { return m_bUseTextCruncher; }

	eMsgType		m_mtType;

	// applies when m_mtType is m_mtCCXXX
	WORD			m_wCC;

	// applies when m_mtType is m_mtCCHiLo or m_mtCCLoHi
	WORD			m_wCC2;

	// applies when m_mtType is m_mtCCInverted
	WORD			m_wCCInvIdentity;

	// applies when m_mtType is mtCCSel
	WORD			m_wCCSelVal;
	WORD			m_wCCSelNum;

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
	BOOL			m_bUseTextCruncher;// when true, we reduce the string's length with a smart algorithm
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

	MsgListenerSet	m_setListeners;

	DWORD			m_dwMaxValue;
	DWORD			m_dwCurValue;
	DWORD			m_dwLastCC;
	BYTE			m_bySysXBuffer[SYSX_BUF_MAX]; // this buffer is used to prevent reallocating for short SysXString messages
	BYTE*			m_pbyLastSysXSent;				// remember the last sysx buffer sent for thinning purposes.
	DWORD			m_dwSizeOfLastSysX;
	DWORD			m_dwLastSentVal;

	BOOL			m_bThinOutput;
	BOOL			m_bIsTrigger;
	DWORD			m_dwTrigValue;

	CControlSurface*		m_pSurface;
	DWORD						m_dwLastMsgCount;
};

/////////////////////////////////////////////////////////////////////////
// CMidiMsgListener:
//
// Object capable of being notified of MidiInput. The CMidiMsgListener
// specifies the way the input notification will be interpreted.
// Objects that require midi input notifications should be derived from
// CMidiMsgListener.
/////////////////////////////////////////////////////////////////////////

typedef std::map< DWORD, int > DeltaMap;
typedef DeltaMap::iterator DeltaMapIt;

class CMidiMsgListener
{
	friend class CMidiMsg;
public:
	enum EValueType { TypeFloat, TypeBool, TypeInt };

	enum EEffect	// defines interaction with CMidiMsg
	{
		mdSet,	// value is set according to type
		mdDelta,	// value is increased by nDelta when wMidiValue is received
					// relations between midi values and dec/increment values are established via AddDelta.
		mdContinuousDelta, // value is increased or decreased by the wMidiValue received.
					//	The wMidiValue is reinterpreted in the following fashion:
					//		top bit means direction (0 left, 1 right), other bits are the increment magnitude.
					// Useful for some kinds of rotary encoders.

		mdSignedDelta, // value is added with the input value, which is interpreted as a signed binary number.
							// in the number of bits occupied by the known max value.
		mdIncrementalWheelDelta, // inc or dec based on the sign of the 14 bit value from the controller
		mdToggle,// alternate between two states (max and min) on each event
		mdOnOff, // a given midi value for the message means ON and another value for the same message means OFF.
	};

	CMidiMsgListener( EEffect eEffect = mdSet );

	// what effect does an input have on the controlled parameter.
	void SetEffect ( EEffect eEffect ) { m_eCurrentEffect = eEffect; }
	EEffect GetEffect() const { return m_eCurrentEffect; }

	// add or clear deltas
	// deltas (when in delta mode) establish the relationship between
	// a received midi value and the amount by wich the controlled
	// parameter is dec/incremented.
	void AddDelta( DWORD dwMidiValue, int nDelta );
	void ClearDeltas() { m_mapDeltas.clear(); }

	// mdOnOff values:
	void SetOnOffValues( DWORD dwOn, DWORD dwOff )
	{
		_ASSERT( dwOn != dwOff ); // do you wish to use toggle instead?
		m_dwOnVal = dwOn;
		m_dwOffVal = dwOff;
	}

	void SetToggleValue( DWORD dwToggle )
	{
		m_dwToggleValue = dwToggle;
	}
	
	void SetIsListeningEnabled( BOOL bEnable )
	{
		m_bIsEnabled = bEnable;
	}

	BOOL IsListeningEnabled()
	{
		return m_bIsEnabled;
	}

	void SetUseNulling( BOOL bUseNulling )
	{
		m_bUseNulling = bUseNulling;
	}

protected:
	// called by CMidiMsg when messages are received
	virtual HRESULT notifyValue( CMidiMsg *pMsg, DWORD dwVal, DWORD dwMaxValue );

	virtual HRESULT setValue( CMidiMsg *pMsg, float fVal ) { return E_NOTIMPL; }	// sets the current value as a float
	virtual HRESULT setValue( CMidiMsg *pMsg, BOOL bVal ) { return E_NOTIMPL; }	// sets the current value as a bool
	virtual HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal ) { return E_NOTIMPL; }	// sets the current value as an int or enum

	virtual EValueType getValueType() { return TypeFloat; }

	// a class derived from CMidiMsgListener may override
	// getMin and getMax if delta mode is desired
	// the default behavior is to return 0 thru 127, which gives delta mode
	// a granularity on par with most MIDI messages.
	virtual DWORD getMin() { return 0; }
	virtual DWORD getMax() { return 127; }

	void setLastValue( float fVal, CMidiMsg *pMsg ); // you must identify the input message
	// for which you want to update the last value.
	void setLastValue( BOOL bVal );
	void setLastValue( DWORD dwVal );

	DWORD floatToMsgVal( float fVal, DWORD dwMaxValue );
	DWORD boolToMsgVal( BOOL fVal );

private:

	// member variables:
	BOOL				m_bIsEnabled;

	// mdDelta
	DeltaMap			m_mapDeltas;

	// used by mdDelta and mdToggle
	DWORD				m_dwLastValue;

	// mdOnOff trigger values for comparison
	DWORD				m_dwOnVal;
	DWORD				m_dwOffVal;

	// mdToggle trigger value for comparison
	DWORD				m_dwToggleValue;

	EEffect			m_eCurrentEffect;

	//	management of NULL mode (where the MIDI message does not change
	// the target until it crosses the target's current value
	BOOL				m_bUseNulling;
	DWORD				m_dwPrevVal;
	BOOL				m_bIsDirty;
};

/////////////////////////////////////////////////////////////////////////
// CMidiTrigger:
//
// Pairs together a MIDI message and a specific value. Useful
// for components that maintain tables of MIDI event triggers
/////////////////////////////////////////////////////////////////////////

class CMidiTrigger
{
public:
	CMidiTrigger()
	{
		m_pMsg = NULL;
		m_dwVal = VAL_ANY;
	}

	BOOL Test( CMidiMsg *pMsg, DWORD dwVal )
	{
		if ((pMsg == m_pMsg) && ((dwVal == m_dwVal) || (m_dwVal == VAL_ANY)))
			return TRUE;
		else
			return FALSE;
	}

	void SetMsg( CMidiMsg *pInMsg, DWORD dwInVal, CMidiMsgListener *pListener )
	{
		HRESULT hr;
		if (m_pMsg != NULL)
		{
			hr = m_pMsg->RemoveListener( pListener );
			_ASSERT( SUCCEEDED( hr ) );
		}

		if (pInMsg != NULL)
		{
			hr = pInMsg->AddListener( pListener );
			_ASSERT( SUCCEEDED( hr ) );
		}
		m_pMsg = pInMsg;
		m_dwVal = dwInVal;
	}

	bool operator <( const CMidiTrigger& rhs ) const
	{
		if (m_pMsg < rhs.m_pMsg)
			return TRUE;
		else if (m_pMsg == rhs.m_pMsg)
		{
			if (m_dwVal == VAL_ANY)
				return FALSE;
			else
				return m_dwVal < rhs.m_dwVal;
		}
		else
			return FALSE;
	}

	CMidiMsg*	m_pMsg;
	DWORD			m_dwVal;
};

/////////////////////////////////////////////////////////////////////////