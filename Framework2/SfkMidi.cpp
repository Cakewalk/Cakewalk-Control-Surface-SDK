/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkMidi.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning(disable:4100)
#pragma warning(disable:4189)

#include "sfkMidi.h"
#include "surface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Use MAYBE_TRACE() for items we'd like to switch off when not actively
// working on this file.
#if 0
	#define MAYBE_TRACE TRACE
#else
	#define MAYBE_TRACE TRUE ? (void)0 : TRACE
#endif

#define ONLY_TRACE_VALUE_CHANGE	0

/////////////////////////////////////////////////////////////////////////
// CNrpnContext:
/////////////////////////////////////////////////////////////////////////
void CNrpnContext::OnController( DWORD dwShortMsg, DWORD dwPort )
{
	_ASSERT( (dwShortMsg & 0x000000F0) == 0xB0 ); // it must be a controller

	register BYTE byNum = BYTE((dwShortMsg & 0x00007F00) >> 8);
	register BYTE byVal = BYTE((dwShortMsg & 0x007F0000) >> 16);
	register BYTE byChan = BYTE(dwShortMsg & 0x0000000f);

	// handles NRPNS
	m_bNrpnComplete[dwPort][byChan] = FALSE;

	switch (byNum)
	{
	case 101:	// Registered Prm MSB
		m_ePNType[dwPort][byChan] = Registered;
		m_dwPrmMSB[dwPort][byChan] = byVal;
		m_dwPrmLSB[dwPort][byChan] = 0;	// reset to zero, as recommended by MIDI spec
		m_dwValMSB[dwPort][byChan] = 0;
		m_dwValLSB[dwPort][byChan] = 0;
		break;

	case 100:	// Registered Prm LSB
		if (m_ePNType[dwPort][byChan] != Registered)
		{
			m_ePNType[dwPort][byChan] = Registered;
			m_dwPrmMSB[dwPort][byChan] = 0;
		}
		// else m_PrmMSB remains whatever it is
		m_dwPrmLSB[dwPort][byChan]	= byVal;
		m_dwValMSB[dwPort][byChan]	= 0;
		m_dwValLSB[dwPort][byChan]	= 0;
		break;

	case 99:		// NonRegistered Prm MSB
		m_ePNType[dwPort][byChan] = NonRegistered;
		m_dwPrmMSB[dwPort][byChan]	= byVal;
		m_dwPrmLSB[dwPort][byChan]	= 0;	// reset to zero, as recommended by MIDI spec
		m_dwValMSB[dwPort][byChan]	= 0;
		m_dwValLSB[dwPort][byChan]	= 0;
		break;

	case 98:		// NonRegistered Prm LSB
		if (m_ePNType[dwPort][byChan] != NonRegistered)
		{
			m_ePNType[dwPort][byChan]	= NonRegistered;
			m_dwPrmMSB[dwPort][byChan] = 0;
		}
		// else m_PrmMSB remains whatever it is
		m_dwPrmLSB[dwPort][byChan]	= byVal;
		m_dwValMSB[dwPort][byChan]	= 0;
		m_dwValLSB[dwPort][byChan]	= 0;
		break;

	case 6:		// Val MSB
		if (m_ePNType[dwPort][byChan] != Unknown)
		{
			m_dwValMSB[dwPort][byChan] = byVal;
			m_dwValLSB[dwPort][byChan] = 0;		// reset to zero, as recommended by MIDI spec
			if (!m_bAccept14BitOnly)
				m_bNrpnComplete[dwPort][byChan] = TRUE;
		}
		break;

	case 38:		// Val LSB			
		if (m_ePNType[dwPort][byChan] != Unknown)
		{
			m_dwValLSB[dwPort][byChan] = byVal;
			m_bNrpnComplete[dwPort][byChan] = TRUE;
		}
		break;

	case 96:		// Value Increment
		if (m_ePNType[dwPort][byChan] != Unknown)
		{
			int n = (m_dwValMSB[dwPort][byChan] << 7) | m_dwValLSB[dwPort][byChan];
			if (++n < 16383)
			{
				m_dwValMSB[dwPort][byChan] = (n >> 7) & 0x7F;
				m_dwValLSB[dwPort][byChan] = n & 0x7F;
				m_bNrpnComplete[dwPort][byChan] = TRUE;
			}
			// else ignore
		}
		break;

	case 97:		// Value Decrement
		if (m_ePNType[dwPort][byChan] != Unknown)
		{
			int n = (m_dwValMSB[dwPort][byChan] << 7) | m_dwValLSB[dwPort][byChan];
			if (0 < n--)
			{
				m_dwValMSB[dwPort][byChan] = (n >> 7) & 0x7F;
				m_dwValLSB[dwPort][byChan] = n & 0x7F;
				m_bNrpnComplete[dwPort][byChan] = TRUE;
			}
			// else ignore
		}		
		break;
		
	}

	if (m_bNrpnComplete[dwPort][byChan])
	{
		DWORD dwParamNum = m_dwPrmLSB[dwPort][byChan] | (m_dwPrmMSB[dwPort][byChan] << 7);
		DWORD dwValue = m_dwValLSB[dwPort][byChan] | (m_dwValMSB[dwPort][byChan] << 7);
		OnNrpn( m_ePNType[dwPort][byChan], dwParamNum, byChan, dwValue, dwPort );
	}
}

/////////////////////////////////////////////////////////////////////////
// CMidiInputRouter:
/////////////////////////////////////////////////////////////////////////
CMidiInputRouter::CMidiInputRouter( CControlSurface *pSurface ):
	m_pSurface( pSurface ),
	m_nMinSysXPrefixLen( INT_MAX ),
	m_bContainsTriggers( FALSE ),
	m_dwMsgCounter( 0 )
{
}

void CMidiInputRouter::OnShortMsg( DWORD dwShortMsg )
{
	BYTE bKind = BYTE(dwShortMsg & 0x000000F0);

	onTrigger( dwShortMsg );

	if (bKind == 0xB0)
		OnController( dwShortMsg, 0 );

	BYTE byIndex = makeHashByte( dwShortMsg );
	deliverShortByIndex( byIndex, dwShortMsg, FALSE );
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::onTrigger( DWORD dwShortMsg )
{
	if (m_bContainsTriggers)
	{
		BYTE byIndex = makeHashByte( dwShortMsg );
		deliverShortByIndex( byIndex, dwShortMsg, TRUE );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::deliverShortByIndex( BYTE byIndex, DWORD dwShortMsg, BOOL bTrigger )
{
	register BYTE byIndex1 = (byIndex & 0x0F);
	register BYTE byIndex2 = ((byIndex & 0xF0) >> 4);
	if (m_wExpMsgMask & (1 << byIndex1))
	{
		if (m_warExpMsgMask[byIndex1] & (1 << byIndex2))
		{
			MsgSet &setMsgs = m_arSetMsgs[byIndex1]; // handy ref
			for (MsgSetIt itMS = setMsgs.begin(); itMS != setMsgs.end(); itMS++)
			{
				if (bTrigger == (*itMS)->IsTrigger())
					(*itMS)->OnShortMsg( m_dwMsgCounter, dwShortMsg );
			}
		}
	}
	m_dwMsgCounter++;
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::OnNrpn( CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort )
{
	register BYTE byHash = makeHashByte( dwParamNum, dwChan );

	register BYTE byIndex1 = (byHash & 0x0F);
	register BYTE byIndex2 = ((byHash & 0xF0) >> 4);

	if (m_wExpMsgMask & (1 << byIndex1))
	{
		if (m_warExpMsgMask[byIndex1] & (1 << byIndex2))
		{
			MsgSet &setMsgs = m_arSetMsgs[byIndex1]; // handy ref
			for (MsgSetIt itMS = setMsgs.begin(); itMS != setMsgs.end(); itMS++)
			{
				(*itMS)->OnNrpn( m_dwMsgCounter, ePNType, dwParamNum, dwChan, dwVal, dwPort );
			}
		}
	}
	m_dwMsgCounter++;
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::OnLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg )
{
	register DWORD ix;
	register BYTE byHash = 0;

	for (ix = 0; ix < cbLongMsg; ix++)
	{
		byHash = byHash ^ pbLongMsg[ix];
		if (ix < (DWORD)m_nMinSysXPrefixLen - 1)
			continue;

		register BYTE byIndex1 = (byHash & 0x0F);
		register BYTE byIndex2 = ((byHash & 0xF0) >> 4);
		if (m_wExpMsgMask & (1 << byIndex1))
		{
			if (m_warExpMsgMask[byIndex1] & (1 << byIndex2))
			{
				MsgSet &setMsgs = m_arSetMsgs[byIndex1]; // handy ref
				for (MsgSetIt itMS = setMsgs.begin(); itMS != setMsgs.end(); itMS++)
				{
					(*itMS)->OnLongMsg( m_dwMsgCounter, cbLongMsg, pbLongMsg );
				}
			}
		}
	}
	m_dwMsgCounter++;
}

/////////////////////////////////////////////////////////////////////////
BYTE CMidiInputRouter::makeHashByte( DWORD dwShortMsg )
{
	register BYTE byStat = BYTE(dwShortMsg & 0x000000FF);

	register BYTE byKind = BYTE(dwShortMsg & 0x000000F0);

	if (0xD0 == byKind) // ch aft
		return byStat;
	else if (0xE0 == byKind) // pitch wheel
		return byStat;

	return BYTE( byStat ^ ( (dwShortMsg & 0x0000FF00) >> 8 ) );
}


/////////////////////////////////////////////////////////////////////////
BYTE CMidiInputRouter::makeHashByte( DWORD dwParamNum, DWORD dwChan )
{
	return BYTE((dwParamNum & 0x7f) ^ ( (dwParamNum & 0x0000FF00) >> 8 ) ^ (dwChan & 0x0F));
}

/////////////////////////////////////////////////////////////////////////
BYTE CMidiInputRouter::makeHashByte( DWORD cbLongMsg, const BYTE *pbLongMsg )
{
	register DWORD ix;
	register BYTE retByte = 0;

	for (ix = 0; ix < cbLongMsg; ix++)
		retByte = retByte ^ pbLongMsg[ix];

	return retByte;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMidiInputRouter::Add( CMidiMsg *pMsg )
{
	BOOL bRet = FALSE;

	MsgSetIt itFindMsg = m_setMsgs.find( pMsg );
	if (itFindMsg == m_setMsgs.end()) // if it is unique
	{
		m_setMsgs.insert(MsgSet::value_type( pMsg ) );
		bRet = TRUE;
	}

	rebuildIndex();

	return bRet;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMidiInputRouter::Remove( CMidiMsg *pMsg )
{
	BOOL bRet = FALSE;

	MsgSetIt itFindMsg = m_setMsgs.find( pMsg );
	if (itFindMsg != m_setMsgs.end()) // if it is found
	{
		// it was found, remove it.
		m_setMsgs.erase( itFindMsg );
		bRet = TRUE;
	}

	rebuildIndex();

	return bRet;
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::rebuildIndex()
{
	// clear all index information
	m_wExpMsgMask = 0;
	m_bContainsTriggers = FALSE;
	m_nMinSysXPrefixLen = INT_MAX;

	for (int ix = 0; ix < 16; ix++)
	{
		m_warExpMsgMask[ix] = 0;
		m_arSetMsgs[ix].clear();
	}

	// for each message in the "main" list (m_SetMsgs)
	for (MsgSetIt itMS = m_setMsgs.begin(); itMS != m_setMsgs.end(); itMS++)
	{
		CMidiMsg *pMsg = (*itMS); // handy ref

		if (pMsg->IsShortMsg())
		{
			// for this message, create one or more index entries.
			// example: mtCCSel requires two index entries, while mtCC requires one.
			// it all depends on the number of short messages required to send transmit this kind of message.

			// NRPNS and RPNS are handled specially, because they use the same controller numbers for parameter
			// selection, and would therefore result in a poorly balanced index.
			switch (pMsg->GetMessageType())
			{
			case CMidiMsg::mtNrpn:
				{
					BYTE byHash = makeHashByte( pMsg->GetNrpn(), pMsg->GetChannel() );
					addToIndex( pMsg, byHash );
				}
				break;

			case CMidiMsg::mtRpn:
				{
					BYTE byHash = makeHashByte( pMsg->GetRpn(), pMsg->GetChannel() );
					addToIndex( pMsg, byHash );
				}
				break;

			case CMidiMsg::mtNote:
				{
					// put in the index a note off for this note as well.
					DWORD dwMsg = pMsg->MakeShortMsg( 0, 0 );
					dwMsg = (dwMsg & 0xffffff0f) | 0x80;
					BYTE byHash = makeHashByte( dwMsg );
					addToIndex( pMsg, byHash );
				}
				// break left out on purpose

			default:
				for (DWORD ix = 0; ix < pMsg->GetShortMsgCount(); ix++)
				{
					DWORD dwMsg = pMsg->MakeShortMsg( 0, ix );

					BYTE byHash = makeHashByte( dwMsg );
					if (pMsg->IsTrigger() && ix == (pMsg->GetShortMsgCount() - 1))
						m_bContainsTriggers = TRUE;

					addToIndex( pMsg, byHash );
				}
			}
		}
		else
		{
			BYTE *pLongMsg = NULL;
			DWORD dwLen = 0;
			pMsg->GetSysXPreString( &pLongMsg, &dwLen );
			BYTE byHash = makeHashByte( dwLen, pLongMsg );
			addToIndex( pMsg, byHash );
			m_nMinSysXPrefixLen = min( (DWORD)m_nMinSysXPrefixLen, dwLen );
		}
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::addToIndex( CMidiMsg *pMsg, BYTE byHash )
{
	BYTE byIndex1 = (byHash & 0x0F);
	BYTE byIndex2 = ((byHash & 0xF0) >> 4);

	m_wExpMsgMask |= (1 << byIndex1);

	m_warExpMsgMask[byIndex1] |= (1 << byIndex2);

	m_arSetMsgs[byIndex1].insert(MsgSet::value_type( pMsg ) );
}

/////////////////////////////////////////////////////////////////////////
// CMidiMsg:
/////////////////////////////////////////////////////////////////////////
CMidiMsg::CMidiMsg( CControlSurface *pSurface, 
						 LPCTSTR pszName, // = NULL
						 DWORD dwID ) :	// = (DWORD)-1
	m_pSurface( pSurface )
	,m_mtType(mtCC)
	,m_dwMaxValue(127)
	,m_wCC(0)
	,m_wNrpn(0)
	,m_wRpn(0)
	,m_pbySysXPre(0)
	,m_dwLenSysXPre(0)
	,m_pbySysXPost(0)
	,m_dwLenSysXPost(0)
	,m_dwSysXTextLen(0)
	,m_dwSysXPadLen(0)
	,m_cFill( ' ' )
	,m_wChannel(0)
	,m_wPort(0)
	,m_wNote(0)
	,m_pbyLastSysXSent( NULL )
	,m_dwSizeOfLastSysX( DWORD(-1) )
	,m_dwLastSentVal( DWORD(-1) )
	,m_bThinOutput( true )
	,m_bIsTrigger( false )
	,m_dwTrigValue( 0 )
	,m_dwLastMsgCount( DWORD(-1) )
	,m_bUseTextCruncher( true )
	,m_eValueMode(VM_ABS)
	,m_dwID(dwID)
{
	if ( pszName )
		m_strName = pszName;
	else
		m_strName = "unnamed";
}

/////////////////////////////////////////////////////////////////////////
CMidiMsg::~CMidiMsg()
{
	// delete any alocated memory
	if (m_pbySysXPre)
	{
		::CoTaskMemFree( m_pbySysXPre );
		m_pbySysXPre = NULL;
	}

	if (m_pbySysXPost)
	{
		::CoTaskMemFree( m_pbySysXPost );
		m_pbySysXPost = NULL;
	}

	if (m_pbyLastSysXSent)
	{
		::CoTaskMemFree( m_pbyLastSysXSent );
		m_pbyLastSysXSent = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////
HRESULT CMidiMsg::SetSysXPreString ( const BYTE *pbyData, DWORD dwLen )
{
	if (pbyData == NULL)
		return E_POINTER;

	if (m_pbySysXPre)
	{
		::CoTaskMemFree( m_pbySysXPre );
		m_pbySysXPre = NULL;
	}

	m_dwLenSysXPre = dwLen;
	if (dwLen == 0)
		return S_FALSE;
	
	m_pbySysXPre = (BYTE*)::CoTaskMemAlloc( dwLen );

	_ASSERT( m_pbySysXPre );
	if (!m_pbySysXPre) return E_OUTOFMEMORY;

	memcpy( m_pbySysXPre, pbyData, dwLen );
	

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMidiMsg::SetSysXPostString ( const BYTE *pbyData, DWORD dwLen )
{
	if (pbyData == NULL)
		return E_POINTER;

	if (m_pbySysXPost)
	{
		::CoTaskMemFree( m_pbySysXPost );
		m_pbySysXPost = NULL;
	}

	m_dwLenSysXPost = dwLen;
	if (dwLen == 0)
		return S_FALSE;

	m_pbySysXPost = (BYTE*)::CoTaskMemAlloc( dwLen );

	_ASSERT( m_pbySysXPost );
	if (!m_pbySysXPost) return E_OUTOFMEMORY;

	memcpy( m_pbySysXPost, pbyData, dwLen );
	

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::GetSysXPreString ( BYTE **ppbyData, DWORD *pdwLen )
{
	_ASSERT( ppbyData && pdwLen );

	*pdwLen = m_dwLenSysXPre;
	*ppbyData = m_pbySysXPre;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::GetSysXPostString ( BYTE **ppbyData, DWORD *pdwLen )
{
	_ASSERT( ppbyData && pdwLen );

	*pdwLen = m_dwLenSysXPost;
	*ppbyData = m_pbySysXPost;
}

/////////////////////////////////////////////////////////////////////////
DWORD CMidiMsg::MakeShortMsg( DWORD dwVal, int ix /* = 0*/)
{
	DWORD dwRet = 0;
	switch(GetMessageType())
	{
	case mtCC:
		if (ix == 0)
		{
			dwRet = 0xB0 | ((GetCCNum() & 0x7f) << 8) | ((dwVal & 0x7f ) << 16);
		}
		break;

	case mtNrpn:
		if (ix == 0)
		{
			// parameter LSB
			dwRet = 0xB0 | (98 << 8) | ((GetNrpn() & 0x7f ) << 16);	
		}
		else if (ix == 1)
		{
			// parameter MSB
			dwRet = 0xB0 | (99 << 8) | (((GetNrpn() >> 7) & 0x7f ) << 16);	
		}
		else if (ix == 2)
		{
			// value MSB
			dwRet = 0xB0 | (6 << 8) | (((dwVal >> 7) & 0x7f ) << 16);	
		}
		else if (ix == 3)
		{
			// parameter LSB
			dwRet = 0xB0 | (38 << 8) | ((dwVal & 0x7f ) << 16);	
		}
		break;
	case mtRpn:
		if (ix == 0)
		{
			// parameter LSB
			dwRet = 0xB0 | (100 << 8) | ((GetNrpn() & 0x7f ) << 16);	
		}
		else if (ix == 1)
		{
			// parameter MSB
			dwRet = 0xB0 | (101 << 8) | (((GetNrpn() >> 7) & 0x7f ) << 16);	
		}
		else if (ix == 2)
		{
			// value MSB
			dwRet = 0xB0 | (6 << 8) | (((dwVal >> 7) & 0x7f ) << 16);	
		}
		else if (ix == 3)
		{
			// parameter LSB
			dwRet = 0xB0 | (38 << 8) | ((dwVal & 0x7f ) << 16);	
		}
		break;
	case mtWheel:
		if (ix == 0)
		{
			// LSB first, MSB second
			dwRet = 0xE0 | ((dwVal & 0x7f) << 8) | (((dwVal) & 0x3f80 ) << 9);	
		}
		break;
	case mtNote:
		if (ix == 0)
		{
			dwRet = 0x90 | ((GetNoteNum() & 0x7f) << 8) | ((dwVal & 0x7f) << 16);
		}
		break;
	case mtChAft:	
		if (ix == 0)
		{
			dwRet = 0xD0 | ((dwVal & 0x7f) << 8);
		}
	}

	dwRet |= (0x0000000f & GetChannel());
	return dwRet;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnShortMsg( DWORD dwMsgCount, DWORD dwShortMsg )
{
	ValueChange vc = VC_None;

	// Protect against multiple calls within the same input service
	if (dwMsgCount == m_dwLastMsgCount)
		return;

	BYTE byKind = BYTE(dwShortMsg & 0xF0);

	if (GetChannel() != (dwShortMsg & 0x0F))
		return; // if wrong channel, ignore

	BOOL bSendValue = FALSE;

	switch (GetMessageType())
	{
	case mtCC:
		if (byKind == 0xB0)
		{
			if (GetCCNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				setCurrentVal((dwShortMsg >> 16) & 0x7f, &vc );
				bSendValue = TRUE;
			}
		}
		break;

	case mtChAft:
		if (byKind == 0xD0)
		{
			setCurrentVal( ((dwShortMsg >> 8) & 0x7f), &vc );
			bSendValue = TRUE;
		}
		break;
	case mtWheel:
		if (byKind == 0xE0)
		{
			DWORD dwCur = ((dwShortMsg >> 8) & 0x7f); // lsb
			dwCur |= ((dwShortMsg >> 9) & 0x3f80); // msb
			setCurrentVal( dwCur, &vc );
			bSendValue = TRUE;
		}
		break;
	case mtWheel7Bit:
		if (byKind == 0xE0)
		{
			DWORD dwCur = ((dwShortMsg >> 16) & 0x7f); // msb
			setCurrentVal( dwCur, &vc );
			bSendValue = TRUE;
		}
		break;
	case mtNote:
		if (byKind == 0x90)
		{
			if (GetNoteNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				setCurrentVal( (dwShortMsg >> 16) & 0x7f, &vc );
				bSendValue = TRUE;
			}
		}
		if (byKind == 0x80)
		{
			if (GetNoteNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				setCurrentVal( 0, &vc );
				bSendValue = TRUE;
			}
		}
		break;
	}

	if (IsTrigger() && (m_dwCurValue != m_dwTrigValue))
		bSendValue = FALSE;

	if (bSendValue)
	{
		m_dwLastMsgCount = dwMsgCount;
		m_pSurface->OnMIDIMessageDelivered( this, m_dwCurValue/(float)m_dwMaxValue, vc );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnLongMsg( DWORD dwMsgCount, DWORD cbLongMsg, const BYTE *pbLongMsg )
{
	ValueChange vc = VC_None;

	// Protect against multiple calls within the same input service
	if (dwMsgCount == m_dwLastMsgCount)
		return;

	// compare incoming long message with sysx pre string, 
	// then extract value if possible

	switch (GetMessageType())
	{
	case mtSysX7bit:
	case mtSysXHiLo:
	case mtSysXLoHi:
		break;
	default:
		return; // if not sysX, ignore
	}

	_ASSERT( pbLongMsg );

	BYTE *pbyMySysXPre;
	DWORD dwPreLen;
	GetSysXPreString( &pbyMySysXPre, &dwPreLen );
	
	BYTE *pbyMySysXPost;
	DWORD dwPostLen;
	GetSysXPostString( &pbyMySysXPost, &dwPostLen );

	DWORD dwDataBytes;
	switch(GetMessageType())
	{
	case mtSysX7bit:
		dwDataBytes = 1;
		break;
	case mtSysXLoHi:
	case mtSysXHiLo:
		dwDataBytes = 2;
		break;
	default:
		_ASSERT(0);// this should only be invoked for sysx events . . .!
		return;
	}

	if (cbLongMsg != dwPreLen + dwPostLen + dwDataBytes)
	{
		// length mismatch, ignore message
		return;
	}

	if (0 != memcmp(pbLongMsg, pbyMySysXPre, dwPreLen ))
	{
		// prestring mismatch, ignore
		return;
	}

	if (dwPostLen > 0)
	{
		if (0 != memcmp(pbLongMsg + dwPreLen + dwDataBytes , pbyMySysXPost, dwPostLen ))
		{
			// poststring mismatch, ignore
			return;
		}
	}

	switch (GetMessageType())
	{
	case mtSysX7bit:

		// the byte sitting exactly at m_dwLenSysXPre contains the value
		setCurrentVal( pbLongMsg[dwPreLen], &vc );

		break; // falthrough returns true

	case mtSysXHiLo:

		// the byte sitting exactly at m_dwLenSysXPre contains the hi value,
		setCurrentVal( pbLongMsg[dwPreLen] * 128, &vc );

		// the next one contains the lo value
		m_dwCurValue += pbLongMsg[dwPreLen + 1];

		break; // falthrough returns true

	case mtSysXLoHi:

		// the byte sitting exactly at m_dwLenSysXPre contains the lo value,
		setCurrentVal( pbLongMsg[dwPreLen], &vc );

		// the next one contains the hi value
		m_dwCurValue += pbLongMsg[dwPreLen + 1] * 128;

		break; // falthrough returns true

	default:
		_ASSERT( 0 );
		return;
	}

	m_dwLastMsgCount = dwMsgCount;
	m_pSurface->OnMIDIMessageDelivered( this, m_dwCurValue/(float)m_dwMaxValue, vc );
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnNrpn( DWORD dwMsgCount, CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort )
{
	ValueChange vc = VC_None;

	// Protect against multiple calls within the same input service
	if (dwMsgCount == m_dwLastMsgCount)
		return;

	if (dwChan != GetChannel())
		return;

	if (dwPort != GetPort())
		return;

	BOOL bSendValue = FALSE;
	switch (ePNType)
	{
	case CNrpnContext::NonRegistered:
		if (dwParamNum == GetNrpn() && GetMessageType() == mtNrpn)
		{
			setCurrentVal( dwVal, &vc );
			bSendValue = TRUE;
		}
		break;
	case CNrpnContext::Registered:
		if (dwParamNum == GetRpn() && GetMessageType() == mtRpn)
		{
			setCurrentVal( dwVal, &vc );
			bSendValue = TRUE;
		}
		break;
	}

	if (bSendValue)
	{
		m_dwLastMsgCount = dwMsgCount;
		m_pSurface->OnMIDIMessageDelivered( this, m_dwCurValue/(float)m_dwMaxValue, vc );
	}
}


//////////////////////////////////////////////////////////////////
// Use this to set m_dwCurVal internally instead of setting it 
// directly from the MIDI message.  This will take the delta modes
// into account
void CMidiMsg::setCurrentVal( DWORD dwVal,ValueChange* pvc )
{
	*pvc = VC_None;

	if( VM_ABS == m_eValueMode )
	{
		if ( m_dwCurValue != dwVal )
		{
			*pvc = m_dwCurValue < dwVal ? VC_Increase : VC_Decrease;
			m_dwCurValue = dwVal;	// simple - just set
		}
	}
	else
	{
		// a delta mode.  Determine if in the CCW or CW range

		int nDelta = 0;
		if ( m_rangeCCW.InRange( dwVal ) )		// cw?
		{
			nDelta = -1;
			*pvc = VC_Decrease;
		}
		else if ( m_rangeCW.InRange( dwVal ) )	// ccw?
		{
			nDelta = 1;
			*pvc = VC_Increase;
		}

		// scale up to 1/127th of max value
		nDelta *= (m_dwMaxValue / 127);

		// Accel mode?
		if ( VM_DELTA_ACCEL == m_eValueMode && nDelta != 0 )
		{
			// which range?
			ValueRange* prange = nDelta > 0 ? &m_rangeCW : &m_rangeCCW;
			int nAcc = dwVal - prange->dwL;
			if ( nDelta > 0 )
				nDelta += (int)(nAcc * 2);
			else
				nDelta -= (int)(nAcc * 2);
		}
		int nVal = (int)m_dwCurValue;
		nVal += nDelta;
		nVal = max( 0, nVal );
		nVal = min( (int)m_dwMaxValue, nVal );

		m_dwCurValue = (DWORD)nVal;
	}
}

void CMidiMsg::SetVal( float fVal )
{
	m_dwCurValue = DWORD((fVal * m_dwMaxValue) + 0.5);
}

float CMidiMsg::GetVal() const
{
	return (float)m_dwCurValue / m_dwMaxValue;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::Send( float fVal )
{
#if ONLY_TRACE_VALUE_CHANGE
	const float fOld = GetVal();
	if ( fOld != fVal )
	{
		MAYBE_TRACE( _T("CMidiMsg::Send() - old %.1f: "), fOld );
#endif

	MAYBE_TRACE( _T("Send %.1f on %s\n"), fVal, m_strName );

#if ONLY_TRACE_VALUE_CHANGE
	}
#endif

	_ASSERT( fVal >= 0.0 );
	_ASSERT( fVal <= 1.0 );

	ASSERT( m_dwMaxValue <= 16383 );

	// m_dwMaxValue contains either 127 or 16383 depending on message's resolution.
	Send( DWORD((fVal * m_dwMaxValue) + 0.5) );
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::Send( DWORD dwVal )
{
	m_dwCurValue = dwVal;

	if (IsShortMsg())
	{
		// thinning
		if (dwVal == m_dwLastSentVal && m_bThinOutput)
			return;

		m_dwLastSentVal = dwVal;

		DWORD dwCount = GetShortMsgCount();
		_ASSERT( dwCount > 0 );
		HRESULT hr;
		for (DWORD ix = 0; ix < dwCount; ix++)
		{
			DWORD dwMsg = MakeShortMsg( dwVal, ix );
			// send dwMsg out
			
			hr = m_pSurface->MidiOutShortMsg( dwMsg );
		}
	}
	else
	{
		// compose a string and send message out:
		switch (GetMessageType())
		{
		case mtSysX7bit:
			{
				const char byMsg = BYTE(dwVal & 0x7f);
				sendString( 1, &byMsg );
			}
			break;
		case mtSysXHiLo:
			{
				char pbyMsg[2];
				pbyMsg[0] = BYTE((dwVal >> 8) & 0x7f);
				pbyMsg[1] = BYTE(dwVal & 0x7f);

				sendString( 2, pbyMsg );
			}
			break;
		case mtSysXLoHi:
			{
				char pbyMsg[2];
				pbyMsg[0] = BYTE(dwVal & 0x7f);
				pbyMsg[1] = BYTE((dwVal >> 8) & 0x7f);

				sendString( 2, pbyMsg );
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::SendText( DWORD cbyString, LPCSTR pszString )
{
	if (GetMessageType() == mtSysXString)
	{
		if (m_dwSysXTextLen != 0)		// length is specified
		{
			if (m_dwSysXTextLen - m_dwSysXPadLen >= cbyString) // not longer? must fill in to make length
			{
				// make a new buffer with the specified length
				char *pbySendBuffer = (char*)::CoTaskMemAlloc( m_dwSysXTextLen );

				if (pbySendBuffer != NULL)
				{
					// we must fill in
					memset( pbySendBuffer, m_cFill, m_dwSysXTextLen );
					_ASSERT( m_dwSysXTextLen - m_dwSysXPadLen > 0 );
					memcpy( pbySendBuffer, pszString, min( cbyString, m_dwSysXTextLen - m_dwSysXPadLen) );
					sendString( m_dwSysXTextLen, pbySendBuffer );
					::CoTaskMemFree( pbySendBuffer );
				}
				else
					_ASSERT( 0 ); // could not allocate string buffer.
			}
			else // longer
				if (useTextCruncher()) // "crunch down"
				{
					char *pBuf = (char*)::CoTaskMemAlloc( m_dwSysXTextLen );
					memset( pBuf, m_cFill, m_dwSysXTextLen );
					_ASSERT( m_dwSysXTextLen - m_dwSysXPadLen > 0 );
					CrunchString( pszString, cbyString, pBuf, m_dwSysXTextLen - m_dwSysXPadLen, m_cFill );
					sendString( m_dwSysXTextLen, pBuf );
					::CoTaskMemFree( pBuf );
				}
				else // just truncate
				{
					sendString( m_dwSysXTextLen, pszString );
				}
		}
		else // length is not specified, send as is.
			sendString( cbyString, pszString );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::sendString( DWORD cbyString, LPCSTR pszString )
{
	// send message as text sandwiched between the pre and post strings.
	DWORD dwPreLen, dwPostLen;
	BYTE *pbyPreData, *pbyPostData;

	GetSysXPreString( &pbyPreData, &dwPreLen );
	GetSysXPostString( &pbyPostData, &dwPostLen );
	BYTE *pbyBuffer = m_bySysXBuffer;
	DWORD dwTotalLen = cbyString + dwPreLen + dwPostLen;

	BOOL bMustAllocate = dwTotalLen > SYSX_BUF_MAX;
	if (bMustAllocate)
	{
		// allocate the required buffer and send.. notice that this is a long message anyway, so
		// there is no point in optimizing memory allocation.
		pbyBuffer = (BYTE*)::CoTaskMemAlloc( cbyString + dwPreLen + dwPostLen );
	}

	if (pbyBuffer != NULL)
	{
		memcpy( pbyBuffer, pbyPreData, dwPreLen );
		memcpy( pbyBuffer + dwPreLen, pszString, cbyString );
		memcpy( pbyBuffer + dwPreLen + cbyString, pbyPostData, dwPostLen );

		if (m_bThinOutput)
		{
			BOOL bMatch = FALSE;
			if (dwTotalLen == m_dwSizeOfLastSysX)
			{
				if (m_pbyLastSysXSent)
					bMatch = (memcmp( m_pbyLastSysXSent, pbyBuffer, dwTotalLen ) == 0);
			}

			if (bMatch && m_bThinOutput)
				return;

			if (dwTotalLen != m_dwSizeOfLastSysX)
			{
				::CoTaskMemFree( m_pbyLastSysXSent );
				m_pbyLastSysXSent = NULL;
			}

			if (m_pbyLastSysXSent == NULL)
			{
				m_pbyLastSysXSent = (BYTE*)::CoTaskMemAlloc( dwTotalLen );
				m_dwSizeOfLastSysX = dwTotalLen;
			}

			memcpy( m_pbyLastSysXSent, pbyBuffer, dwTotalLen );
		}
		// send message out
		HRESULT hr = m_pSurface->MidiOutLongMsg( dwTotalLen, pbyBuffer );

		if (bMustAllocate)
			::CoTaskMemFree( pbyBuffer );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::Invalidate()
{
	m_dwSizeOfLastSysX = DWORD(-1);
	m_dwLastSentVal = DWORD(-1);
}

//////////////////////////////////////////////////////////////////////////
// Set either a absolute position mode or a delta mode
void	CMidiMsg::SetValueMode (ValueMode vm, ValueRange* prangeCCW,//= NULL, 
															ValueRange* prangeCW )//= NULL
{
	// Either specify both ranges or none
	_ASSERT( (prangeCCW && prangeCW) || (!prangeCCW && !prangeCW) );

	m_eValueMode = vm;

	if ( m_eValueMode == VM_DELTA || m_eValueMode == VM_DELTA_ACCEL )
	{
		// if no ccw and cw ranges were specified, default to some guess
		if ( prangeCCW && prangeCW )
		{
			m_rangeCCW = *prangeCCW;
			m_rangeCW = *prangeCW;
		}
		else
		{
			m_rangeCCW.dwL = 0;
			m_rangeCCW.dwH = m_dwMaxValue / 2;
			m_rangeCW.dwL = m_dwMaxValue / 2 + 1;
			m_rangeCW.dwH = m_dwMaxValue;
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// Obtain a status word
WORD CMidiMsg::GetStatusWord()
{
	WORD w = m_wChannel;

	switch (m_mtType)
	{
	case mtCC:
		w |= 0xb0;
		w |= (m_wCC << 8);
		break;

	case mtWheel:
	case mtWheel7Bit:
		w |= 0xe0;
		break;

	case mtNote:
		w |= 0x90;
		w |= (m_wNote << 8);
		break;
	}

	return w;
}

