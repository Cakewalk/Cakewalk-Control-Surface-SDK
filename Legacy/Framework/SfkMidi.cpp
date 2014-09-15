/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkMidi.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning (disable:4100)

#include "ControlSurface.h"
#include "SfkMidi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	INDETERMINATE_DIRECTION 0
#define NEGATIVE_DIRECTION 1
#define POSITIVE_DIRECTION 2

static DWORD dwLastTimePanIncreased = 0;
static DWORD dwLastTimePanDecreased = 0;

static DWORD dwTimeMotionStarted = 0;
static DWORD dwCurrentMotionDirection = INDETERMINATE_DIRECTION;


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
	m_bContainsInvertedCCs( FALSE ),
	m_bContainsTriggers( FALSE ),
	m_dwMsgCounter( 0 )
{
}

void CMidiInputRouter::OnShortMsg( DWORD dwShortMsg )
{
	BYTE bKind = BYTE(dwShortMsg & 0x000000F0);

	onTrigger( dwShortMsg );

	onInvertedCC( dwShortMsg );

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
		BYTE byIndex = makeTrigHashByte( dwShortMsg );
		deliverShortByIndex( byIndex, dwShortMsg, TRUE );
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiInputRouter::onInvertedCC( DWORD dwShortMsg )
{
	// if the message is a CC and we have some inverted CC's in the list:
	if (m_bContainsInvertedCCs && ((dwShortMsg & 0xF0) == 0xB0) )
	{
		// we use the third byte (starting from the least significant)
		// masked to the 7 ls bits as the identity for an inverted CC.
		// channel is passed as usual.
		BYTE byIndex = makeHashByte( (dwShortMsg >> 16) & 0x7F, dwShortMsg & 0x0F );
		deliverShortByIndex( byIndex, dwShortMsg, FALSE );
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
	m_pSurface->GetStateMgr()->deliverPostedStateChanges();
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
		m_pSurface->GetStateMgr()->deliverPostedStateChanges();
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
BYTE CMidiInputRouter::makeTrigHashByte( DWORD dwShortMsg )
{
	// for triggers, hash the whole message (least significant 24 bits)
	return BYTE( (dwShortMsg & 0x000000FF) ^ ( (dwShortMsg & 0x0000FF00) >> 8 ) ^ ( (dwShortMsg & 0x00FF0000) >> 16 ) );
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
BOOL CMidiInputRouter::add( CMidiMsg *pMsg )
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
BOOL CMidiInputRouter::remove( CMidiMsg *pMsg )
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
	m_bContainsInvertedCCs = FALSE;
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

			case CMidiMsg::mtCCInverted:
				{
					// CCInvIdentity is the actual 7 bit value of a received message.
					// we would have to hash that as the identity during input
					// to locate the CMidiMsg that should service it.
					BYTE byHash = makeHashByte( pMsg->GetCCInvIdentity(), pMsg->GetChannel() );
					addToIndex( pMsg, byHash );
					m_bContainsInvertedCCs = TRUE;
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

					BYTE byHash;
					if (pMsg->IsTrigger() && ix == (pMsg->GetShortMsgCount() - 1))
					{
						// the last (or if one, the only) message for a trigger is hashed differently.
						byHash = makeTrigHashByte( dwMsg );
						m_bContainsTriggers = TRUE;
					}
					else
						byHash = makeHashByte( dwMsg );

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
CMidiMsg::CMidiMsg( CControlSurface *pSurface ) :
	m_pSurface( pSurface ),
	m_mtType(mtCC),
	m_dwMaxValue( 127 ),
	m_wCC( 0 ),
	m_wCC2(0),
	m_wCCInvIdentity(0),
	m_wCCSelVal(0),
	m_wCCSelNum(0),
	m_wNrpn(0),
	m_wRpn(0),
	m_pbySysXPre(0),
	m_dwLenSysXPre(0),
	m_pbySysXPost(0),
	m_dwLenSysXPost(0),
	m_dwSysXTextLen(0),
	m_dwSysXPadLen(0),
	m_cFill( ' ' ),
	m_wChannel(0),
	m_wPort(0),
	m_wNote(0),
	m_pbyLastSysXSent( NULL ),
	m_dwSizeOfLastSysX( -1 ),
	m_dwLastSentVal( -1 ),
	m_bThinOutput( TRUE ),
	m_bIsTrigger( FALSE ),
	m_dwTrigValue( 0 ),
	m_dwLastMsgCount( -1 ),
	m_bUseTextCruncher( TRUE )
{
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

	case mtCCInverted:
		if (ix == 0)
		{
			dwRet = 0xB0 | ((dwVal & 0x7f) << 8) | ((GetCCInvIdentity() & 0x7f ) << 16);
		}
		break;

	case mtCCSel:
		if (ix == 0)
		{
			dwRet = 0xB0 | ((GetCCSelNum() & 0x7f) << 8) | ((GetCCSelVal() & 0x7f ) << 16);
		}
		else if (ix == 1)
		{
			dwRet = 0xB0 | ((GetCCNum() & 0x7f) << 8) | ((dwVal & 0x7f ) << 16);
		}
		break;
	case mtCCHiLo:
		if (ix == 0)
		{
			dwRet = 0xB0 | ((GetCCNum() & 0x7f) << 8) | (((dwVal >> 7) & 0x7f ) << 16);
			
		}
		else if (ix == 1)
		{
			dwRet = 0xB0 | ((GetCC2Num() & 0x7f) << 8) | ((dwVal & 0x7f ) << 16);
		}
		break;
	case mtCCLoHi:
		if (ix == 0)
		{
			dwRet = 0xB0 | ((GetCCNum() & 0x7f) << 8) | ((dwVal & 0x7f ) << 16);	
		}
		else if (ix == 1)
		{
			dwRet = 0xB0 | ((GetCC2Num() & 0x7f) << 8) | (((dwVal >> 7) & 0x7f ) << 16);
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
		break;
	case mtKeyAft:
		if (ix == 0)
		{
			dwRet = 0xA0 | ((GetNoteNum() & 0x7f) << 8) | ((dwVal & 0x7f) << 16);
		}
		break;

	}

	dwRet |= (0x0000000f & GetChannel());
	return dwRet;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnShortMsg( DWORD dwMsgCount, DWORD dwShortMsg )
{
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
				m_dwCurValue = (dwShortMsg >> 16) & 0x7f;
				bSendValue = TRUE;
			}
		}
		break;

	case mtCCInverted:
		if (byKind == 0xB0)
		{
			if (GetCCInvIdentity() == ((dwShortMsg >> 16) & 0x7f))
			{
				// message matches
				m_dwCurValue = (dwShortMsg >> 8) & 0x7f;
				bSendValue = TRUE;
			}
		}
		break;

	case mtCCSel:
		if (byKind == 0xB0)
		{
			DWORD dwCCNum = ((dwShortMsg >> 8) & 0x7f);
			if (GetCCNum() == dwCCNum) // if message matches
			{
				// and the selection CC matches...
				DWORD dwSelCC = (GetCCSelVal() & 0x7f) << 16 | (GetCCSelNum() & 0x7f) << 8 | 0xB0 | (0x0f & GetChannel());
				if (dwSelCC == m_dwLastCC)
				{
					// message matches
					m_dwCurValue = (dwShortMsg >> 16) & 0x7f;
					bSendValue = TRUE;
				}
			}
		}
		m_dwLastCC = dwShortMsg;// remember the last CC
		break;

	case mtCCHiLo:
		if (byKind == 0xB0)
		{
			DWORD dwCCNum = ((dwShortMsg >> 8) & 0x7f);
			if (GetCCNum() == dwCCNum)
			{
				// message matches for first byte (hi)
				// overwrite with hi byte
				m_dwCurValue = ((dwShortMsg >> 9) & 0x3f80);
			}
			else if (GetCC2Num() == dwCCNum)
			{
				// message matches for first byte (hi)
				// add the low byte.
				m_dwCurValue |= (dwShortMsg >> 16) & 0x7f;

				// if the HiCC was the previous message...
				DWORD dwHiCC = (GetCCNum() & 0x7f) << 8 | 0xB0 | (0x0f & GetChannel());
				if ((m_dwLastCC & 0x0000ffff) == dwHiCC)
					bSendValue = TRUE;
			}

			m_dwLastCC = dwShortMsg;// remember the last CC
		}
		break;

	case mtCCLoHi:
		if (byKind == 0xB0)
		{
			DWORD dwCCNum = ((dwShortMsg >> 8) & 0x7f);
			if (GetCCNum() == dwCCNum)
			{
				// message matches for first byte (hi)
				// overwrite with low byte.
				m_dwCurValue = (dwShortMsg >> 16) & 0x7f;
			}
			else if (GetCC2Num() == dwCCNum)
			{
				// message matches for first byte (hi)
				// add hi byte
				// if the HiCC was the previous message...
				DWORD dwLoCC = (GetCC2Num() & 0x7f) << 8 | 0xB0 | (0x0f & GetChannel());
				m_dwCurValue |= ((dwShortMsg >> 9) & 0x3f80);
				if ((m_dwLastCC & 0x0000ffff) == dwLoCC)
					bSendValue = TRUE;
			}

			m_dwLastCC = dwShortMsg;// remember the last CC
		}
		break;

	case mtChAft:
		if (byKind == 0xD0)
		{
			m_dwCurValue = ((dwShortMsg >> 8) & 0x7f);
			bSendValue = TRUE;
		}
		break;

	case mtKeyAft:
		if (byKind == 0xA0)
		{
			if (GetNoteNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				m_dwCurValue = (dwShortMsg >> 16) & 0x7f;
				bSendValue = TRUE;
			}
		}
		break;

	case mtWheel:
		if (byKind == 0xE0)
		{
			m_dwCurValue = ((dwShortMsg >> 8) & 0x7f); // lsb
			m_dwCurValue |= ((dwShortMsg >> 9) & 0x3f80); // msb
			bSendValue = TRUE;
		}
		break;

	case mtNote:
		if (byKind == 0x90)
		{
			if (GetNoteNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				m_dwCurValue = (dwShortMsg >> 16) & 0x7f;
				bSendValue = TRUE;
			}
		}
		if (byKind == 0x80)
		{
			if (GetNoteNum() == ((dwShortMsg >> 8) & 0x7f))
			{
				// message matches
				m_dwCurValue = 0;
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
		deliverToListeners();
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMidiMsg::AddListener( CMidiMsgListener *pListener )
{
	if (pListener == NULL)
		return E_POINTER;

	if (GetMessageType() == mtSysXString)
	{
		// this is not a message you can "receive"
		_ASSERT( 0 );
	}

	int nListenerSetSizeWas = (int)m_setListeners.size();
	MsgListenerSetIt it = m_setListeners.find( pListener );
	if (it == m_setListeners.end())
	{
		m_setListeners.insert( MsgListenerSet::value_type( pListener ));
	}
	// otherwise, it was already there, no need to add again
	
	if (nListenerSetSizeWas == 0)
	{
		_ASSERT( m_setListeners.size() > 0 );

		// add to input router
		m_pSurface->GetMidiInputRouter()->add( this );
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMidiMsg::RemoveListener( CMidiMsgListener *pListener )
{
	if (pListener == NULL)
		return E_POINTER;

	MsgListenerSetIt it = m_setListeners.find( pListener );
	if (it != m_setListeners.end())
	{
		// if found, remove it
		m_setListeners.erase( it );
	}
	else
		return S_FALSE;

	if (m_setListeners.size() == 0)
	{
		// remove from input router
		m_pSurface->GetMidiInputRouter()->remove( this );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::deliverToListeners()
{
	if (m_setListeners.size() != 0)
	{
		MsgListenerSetIt it;
		for (it = m_setListeners.begin(); it != m_setListeners.end(); it++)
		{
			CMidiMsgListener *pListener = *it;
			if (pListener != NULL)
			{
				pListener->notifyValue( this, m_dwCurValue, m_dwMaxValue );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnLongMsg( DWORD dwMsgCount, DWORD cbLongMsg, const BYTE *pbLongMsg )
{
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
		m_dwCurValue = pbLongMsg[dwPreLen];

		break; // falthrough returns true

	case mtSysXHiLo:

		// the byte sitting exactly at m_dwLenSysXPre contains the hi value,
		m_dwCurValue = pbLongMsg[dwPreLen] * 128;

		// the next one contains the lo value
		m_dwCurValue += pbLongMsg[dwPreLen + 1];

		break; // falthrough returns true

	case mtSysXLoHi:

		// the byte sitting exactly at m_dwLenSysXPre contains the lo value,
		m_dwCurValue = pbLongMsg[dwPreLen];

		// the next one contains the hi value
		m_dwCurValue += pbLongMsg[dwPreLen + 1] * 128;

		break; // falthrough returns true

	default:
		_ASSERT( 0 );
		return;
	}

	m_dwLastMsgCount = dwMsgCount;
	deliverToListeners();
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::OnNrpn( DWORD dwMsgCount, CNrpnContext::EPNType ePNType, DWORD dwParamNum, DWORD dwChan, DWORD dwVal, DWORD dwPort )
{
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
			m_dwCurValue = dwVal;
			bSendValue = TRUE;
		}
		break;
	case CNrpnContext::Registered:
		if (dwParamNum == GetRpn() && GetMessageType() == mtRpn)
		{
			m_dwCurValue = dwVal;
			bSendValue = TRUE;
		}
		break;
	}

	if (bSendValue)
	{
		m_dwLastMsgCount = dwMsgCount;
		deliverToListeners();
	}
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::Send( float fVal )
{
	_ASSERT( fVal >= 0.0 );
	_ASSERT( fVal <= 1.0 );

	ASSERT( m_dwMaxValue <= 16383 );

	// m_dwMaxValue contains either 127 or 16383 depending on message's resolution.
	Send( DWORD((fVal * m_dwMaxValue) + 0.5) );
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsg::Send( DWORD dwVal )
{
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
	m_dwSizeOfLastSysX = -1;
	m_dwLastSentVal = -1;
}

/////////////////////////////////////////////////////////////////////////
// CMidiMsgListener:
/////////////////////////////////////////////////////////////////////////
CMidiMsgListener::CMidiMsgListener( EEffect eEffect /*= mdSet */ ) :
	m_dwToggleValue( 0 ),
	m_dwOnVal( 0 ),
	m_dwOffVal( 0 ),
	m_bUseNulling( FALSE ),
	m_dwPrevVal( -1 ),
	m_bIsDirty( TRUE ),
	m_bIsEnabled( TRUE )
{
	m_eCurrentEffect = eEffect;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMidiMsgListener::notifyValue( CMidiMsg *pMsg, DWORD dwVal, DWORD dwMaxValue )
{
	if (m_bIsEnabled == FALSE)
		return S_FALSE;

	switch (m_eCurrentEffect)
	{
	case mdSet:
		if (m_bUseNulling)
		{
			if (m_bIsDirty)
			{
				if (m_dwPrevVal == -1)
				{
					m_dwPrevVal = dwVal;
					return S_FALSE;
				}

				// allow some deadband for nulling since some plugins seem to lag behind
				DWORD dwDeadband = pMsg->GetMaxNotifyValue() / 16;

				if (
					( m_dwLastValue > dwVal + dwDeadband && m_dwLastValue > m_dwPrevVal + dwDeadband ) ||
					( m_dwLastValue + dwDeadband < dwVal && m_dwLastValue + dwDeadband < m_dwPrevVal ) )
				{
					m_dwPrevVal = dwVal;
					// if values have not crossed known last value
					return S_FALSE;
				}
				else
					m_bIsDirty = FALSE;
			}
		}

		m_dwLastValue = dwVal;
		break;

	case mdDelta:
		{
			// lookup delta at this value:
			DeltaMapIt it = m_mapDeltas.find( dwVal );
			if (it != m_mapDeltas.end())
			{
				int nDelta = (*it).second;
				int nMin = (int)getMin();
				int nMax = (int)getMax();
				m_dwLastValue = max( nDelta + (int /*force into signed*/)m_dwLastValue, nMin );
				m_dwLastValue = min( (int/*force into signed*/)m_dwLastValue, nMax );
			}
		}
		break;
	case mdSignedDelta:
		{
			// make sure dwMax is a power of two minus one (127, 16383, etc.) 
			// this is not supposed to be an exhaustive test but is cheap and sufficient.
			_ASSERT( ( ( ( dwMaxValue << 1 ) + 1 ) & dwMaxValue ) == dwMaxValue ); 

			int nInc = dwVal & dwMaxValue >> 1; // value
			DWORD dwSign = dwVal & ( ( dwMaxValue >> 1 ) + 1 ); // top bit, or sign
			if (dwSign != 0) // negative
				nInc = -1 - dwMaxValue + dwVal; // sign extension

			int nMin = (int)getMin();
			int nMax = (int)getMax();
			m_dwLastValue = max( nInc + (int /*force into signed*/)m_dwLastValue, nMin );
			m_dwLastValue = min( (int/*force into signed*/)m_dwLastValue, nMax );
		}
		break;
	case mdContinuousDelta:
		{
			// make sure dwMax is a power of two minus one (127, 16383, etc.) 
			// this is not supposed to be an exhaustive test but is cheap and sufficient.
			_ASSERT( ( ( ( dwMaxValue << 1 ) + 1 ) & dwMaxValue ) == dwMaxValue ); 

			int nInc = dwVal & dwMaxValue >> 1; // value
			DWORD dwSign = dwVal & ( ( dwMaxValue >> 1 ) + 1 ); // top bit, or sign
			if (dwSign == 0)
				nInc = 0 - nInc;

			int nMin = (int)getMin();
			int nMax = (int)getMax();
			m_dwLastValue = max( nInc + (int /*force into signed*/)m_dwLastValue, nMin );
			m_dwLastValue = min( (int/*force into signed*/)m_dwLastValue, nMax );
		}
		break;
	case mdIncrementalWheelDelta:
		{
			// make sure dwMax is a power of two minus one (127, 16383, etc.) 
			// this is not supposed to be an exhaustive test but is cheap and sufficient.
			_ASSERT( ( ( ( dwMaxValue << 1 ) + 1 ) & dwMaxValue ) == dwMaxValue ); 

			int nInc; // value
			DWORD dwSign = dwVal & ( ( dwMaxValue >> 1 ) + 1 ); // top bit, or sign
			if (dwSign == 0)
			{
				if( GetTickCount() - dwTimeMotionStarted < 50 && NEGATIVE_DIRECTION == dwCurrentMotionDirection )
				{
					break;
				}
				else
				{
					dwTimeMotionStarted = GetTickCount();
					dwCurrentMotionDirection = POSITIVE_DIRECTION;
				}

				if( GetTickCount() - dwLastTimePanIncreased < 10 )
				{
					nInc = 4;
				}
				else if( GetTickCount() - dwLastTimePanIncreased < 20 )
				{
					nInc = 2;
				}
				else
				{ 
					nInc = 1;
				}
				
				dwLastTimePanIncreased = GetTickCount();
			}
			else
			{
				if( GetTickCount() - dwTimeMotionStarted < 50 && POSITIVE_DIRECTION == dwCurrentMotionDirection )
				{
					break;
				}
				else
				{
					dwTimeMotionStarted = GetTickCount();
					dwCurrentMotionDirection = NEGATIVE_DIRECTION;
				}


				if( GetTickCount() - dwLastTimePanDecreased < 10 )
				{
					nInc = -4;
				}
				else if( GetTickCount() - dwLastTimePanDecreased < 20 )
				{
					nInc = -2;
				}
				else
				{
					nInc = -1;
				}

				dwLastTimePanDecreased = GetTickCount();
			}
			int nMin = (int)getMin();
			int nMax = (int)getMax();
			m_dwLastValue = max( nInc + (int /*force into signed*/)m_dwLastValue, nMin );
			m_dwLastValue = min( (int/*force into signed*/)m_dwLastValue, nMax );
		}
		break;
	case mdToggle:
		if (m_dwToggleValue == dwVal)
		{
			if (m_dwLastValue > 0)
				m_dwLastValue = 0;
			else
				m_dwLastValue = 1;
		}
		else
			return S_FALSE;
		break;
	case mdOnOff:
		if (dwVal == m_dwOnVal)
			m_dwLastValue = 1;
		else if (dwVal == m_dwOffVal)
			m_dwLastValue = 0;
		else
			// neither value? discard
			return S_FALSE;
		break;
	}

	EValueType eType = getValueType();
	
	HRESULT hr = S_OK;
	switch (eType)
	{
	case TypeFloat:
		if (m_eCurrentEffect == mdDelta || m_eCurrentEffect == mdContinuousDelta || m_eCurrentEffect == mdSignedDelta )
		{
			hr = setValue( pMsg, float(m_dwLastValue - getMin()) / float(getMax() - getMin()) );
		}
		else if( m_eCurrentEffect == mdIncrementalWheelDelta )
		{
			hr = setValue( pMsg, float(m_dwLastValue - getMin()) / float(getMax() - getMin()) );
		}
		else
			hr = setValue( pMsg, float(m_dwLastValue) / float(dwMaxValue) );
		break;

	case TypeBool:
		hr = setValue( pMsg, (m_dwLastValue > 0) );
		break;

	case TypeInt:
		// we will just send the raw value
		break;

	default:
		_ASSERT( 0 ); // unknown type
		break;
	}

	if (FAILED( hr ))
		return hr;

	// always call the "raw" set value
	return setValue( pMsg, m_dwLastValue );
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsgListener::setLastValue( float fVal, CMidiMsg* pMsg )
{
	// dirty the internal status from MIDI state
	if (m_bIsDirty == FALSE)
		m_dwPrevVal = m_dwLastValue;

	_ASSERT( getValueType() == TypeFloat );

	if (pMsg)
	{
		if (m_eCurrentEffect == mdDelta || m_eCurrentEffect == mdContinuousDelta || m_eCurrentEffect == mdSignedDelta )
		{
			m_dwLastValue = floatToMsgVal( fVal, getMax() - getMin() ) + getMin();
		}
		else if( m_eCurrentEffect == mdIncrementalWheelDelta )
		{
			m_dwLastValue = floatToMsgVal( fVal, getMax() - getMin() ) + getMin();

		}
		else
		{
			m_dwLastValue = floatToMsgVal( fVal, pMsg->GetMaxNotifyValue() );
		}
	}

	if (m_dwPrevVal != m_dwLastValue)
		m_bIsDirty = TRUE;
}

/////////////////////////////////////////////////////////////////////////
DWORD CMidiMsgListener::floatToMsgVal( float fVal, DWORD dwMaxValue )
{
	ASSERT( dwMaxValue <= 16383 );

	return DWORD( ( fVal * dwMaxValue ) + 0.5 );
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsgListener::setLastValue( BOOL bVal )
{
	_ASSERT( getValueType() == TypeBool );

	m_dwLastValue = boolToMsgVal( bVal );
}

/////////////////////////////////////////////////////////////////////////
DWORD CMidiMsgListener::boolToMsgVal( BOOL bVal )
{
	if (bVal)
		return 1;
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsgListener::setLastValue( DWORD dwVal )
{
	_ASSERT( getValueType() == TypeInt );

	m_dwLastValue = dwVal;
}

/////////////////////////////////////////////////////////////////////////
void CMidiMsgListener::AddDelta( DWORD dwMidiValue, int nDelta )
{
	m_mapDeltas.insert( DeltaMap::value_type(dwMidiValue, nDelta ) );
}

/////////////////////////////////////////////////////////////////////////
