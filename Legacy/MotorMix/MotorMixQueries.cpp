/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixQueries.cpp
/////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MotorMixQueries.h"
#include "StateDefs.h"

/////////////////////////////////////////////////////////////////////////
// CMMQuery:
/////////////////////////////////////////////////////////////////////////
CMMQuery::CMMQuery( int nQueryState, CControlSurface *pSurface ) :
	CMultiStateListener( pSurface ),
	m_nQueryState( nQueryState ),
	m_dwBaseOption( 0 ),
	m_msgSelectBtns( pSurface ),
	m_msgSelectLED( pSurface ),
	m_msgRightBtnLED( pSurface ),
	m_msgLeftBtnLED( pSurface ),
	m_msgRightBtns( pSurface ),
	m_msgScribble( pSurface )
{
	// setup the select buttons message.
	// this will be used for the "soft" buttons
	m_msgSelectBtns.SetMessageType( CMidiMsg::mtCCHiLo );
	m_msgSelectBtns.SetCCNum( 0x0f );
	m_msgSelectBtns.SetCC2Num( 0x2f );

	// keep in mind that this message cover all the select buttons.
	// depending on the received value we can determine which button
	// "did something" and whether it was presed or released.

	// setup the select buttons LED message
	m_msgSelectLED.SetMessageType( CMidiMsg::mtCCHiLo );
	m_msgSelectLED.SetCCNum( 0x0c );
	m_msgSelectLED.SetCC2Num( 0x2c );

	m_msgRightBtnLED.SetMessageType( CMidiMsg::mtCCSel );
	m_msgRightBtnLED.SetCCSelNum( 0x0C );
	m_msgRightBtnLED.SetCCSelVal( 0x09 );
	m_msgRightBtnLED.SetCCNum( 0x2C );

	m_msgLeftBtnLED.SetMessageType( CMidiMsg::mtCCSel );
	m_msgLeftBtnLED.SetCCSelNum( 0x0C );
	m_msgLeftBtnLED.SetCCSelVal( 0x08 );
	m_msgLeftBtnLED.SetCCNum( 0x2C );

	// configure m_msgRightBtns to listen for buttons on the right hand side
	// this will be used for the escape key.
	m_msgRightBtns.SetMessageType( CMidiMsg::mtCCSel );
	m_msgRightBtns.SetCCNum( 0x2f );
	m_msgRightBtns.SetCCSelNum( 0x0f );
	m_msgRightBtns.SetCCSelVal( 0x09 );

	// register THIS as a listener for input from all input messages
	m_msgSelectBtns.AddListener( this );
	m_msgRightBtns.AddListener( this );

	// set scribble message for query text
	m_msgScribble.SetMessageType( CMidiMsg::mtSysXString );
	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x10, 0x00 };	
	BYTE pczPostString[] = {0xF7};

	m_msgScribble.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribble.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribble.SetSysXTextLen( 40 );
	m_msgScribble.SetSysXTextFillChar( 0x20 );


	// subscribe at the very end
	SubscribeToState( stQueryState );
}

/////////////////////////////////////////////////////////////////////////
CMMQuery::~CMMQuery()
{
	m_msgSelectBtns.RemoveListener( this );
	m_msgRightBtns.RemoveListener( this );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMQuery::OnStateChange( DWORD dwStateID, int nNewState )
{
	onQueryMode( GetStateForID( stQueryState ) == m_nQueryState );

	if (GetStateForID( stQueryState ) == m_nQueryState)
	{
		drawOptions();
	}
	else if (GetStateForID( stQueryState ) == qsIdle)
	{
		// no need to stop flashing the buttons since someone else will paint right on them

		// turn off escape button
		flashEscape( false );

		// no need to blank the scribble strip since someone else will paint right on it
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// Either flash, or turn off the escape button LED
void CMMQuery::flashEscape( bool bFlash )
{
	if ( bFlash )
	{
		// flash escape button
		m_msgRightBtnLED.Send( (DWORD)0x50 );
	}
	else
	{
		// turn off escape button
		m_msgRightBtnLED.Send( (DWORD)0x00 );
	}
}


/////////////////////////////////////////////////////////////////////////
void CMMQuery::drawOptions()
{
	// flash select LEDS, populate LCD with text
	char strDisplay[41];
	char *pStrDisplay = NULL;

	int ixOption = getBaseOption();

	// flash the select button leds
	for (int ix = 0; ix < 8; ix++)
	{
		pStrDisplay = strDisplay + ( 5 * ix );

		// print the display text for this column
		if (hasMoreLeftOptions() && ix == 0)
		{
			m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x51) );
			strcpy(pStrDisplay, getLeftArrowText() );
		}
		else if (hasMoreRightOptions() && ix == 7)
		{
			m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x51) );
			strcpy(pStrDisplay, getRightArrowText() );
		}
		else
		{
			switch (getOptionLedState( ixOption ))
			{
			case LedFlash:
				m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x51) );
				break;

			case LedOff:
				m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x01) );
				break;

			case LedOn:
				m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x41) );
				break;

			default:
				_ASSERT( 0 ); // unknown led state
			}
			getOptionText( ixOption, pStrDisplay );
		}
		ixOption++;
	}

	// because thinning would otherwise prevent the scribble from being sent,
	// after query states are finished, we must invalidate
	// the scribble messages so that
	// the value gets updated
	m_msgScribble.Invalidate();
	m_msgScribble.SendText( 40, strDisplay );

	flashEscape( true );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMQuery::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	if (pMsg == &m_msgSelectBtns)
	{
		if (GetStateForID( stQueryState ) == m_nQueryState)
		{
			DWORD dwButtonIx = dwVal >> 7;
			DWORD dwButtonState = dwVal & 0x7f;

			if (dwButtonIx > 7)
				return S_FALSE;

			if (dwButtonState == 0x41) // button was pressed
			{
				if (hasMoreLeftOptions() && dwButtonIx == 0)
				{
					// we have more left options and they pressed the left arrow
					decBaseOption();
					drawOptions();
				}
				else if (hasMoreRightOptions() && dwButtonIx == 7)
				{
					incBaseOption();
					drawOptions();
				}
				else
				{
					if (getOptionLedState( dwButtonIx ) == LedOff)
						return S_FALSE;

					// carry out the "task"
					onOption( dwButtonIx + getBaseOption() );

					// because a choice was made, shift the query state to the "finishing" one:
					m_pSurface->GetStateMgr()->PostStateChange( stQueryState, getFinishQueryState( dwButtonIx ) );

					// by using PostStateChange, we are deferring the change until
					// everyone has been notified in this round for the received MIDI message.
					// This is strongly suggested if you ever change a state programatically
					// in response to a MIDI message.
				}
			}
		}
	}
	else if (pMsg == &m_msgRightBtns) // escape key
	{
		if (GetStateForID( stShiftKey ) == skShiftIn)
			return S_FALSE;

		if (dwVal == 0x40) // if escape key was pushed
		{
			m_pSurface->GetStateMgr()->PostStateChange( stQueryState, getEscapeQueryState() );
		}
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
int CMMQuery::getEscapeQueryState()
{
	// by default, go idle when escape is pushed
	return qsIdle;
}

/////////////////////////////////////////////////////////////////////////
int CMMQuery::getFinishQueryState( DWORD dwButtonIx )
{
	// by default, go idle when a query is completed
	return qsIdle;
}

/////////////////////////////////////////////////////////////////////////
DWORD CMMQuery::getBaseOption()
{
	return m_dwBaseOption;
}

/////////////////////////////////////////////////////////////////////////
void CMMQuery::setBaseOption( DWORD dwBaseOpt )
{
	m_dwBaseOption = dwBaseOpt;
}

/////////////////////////////////////////////////////////////////////////
void CMMQuery::incBaseOption()
{
	m_dwBaseOption++;
}

/////////////////////////////////////////////////////////////////////////
void CMMQuery::decBaseOption()
{
	m_dwBaseOption--;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMMQuery::hasMoreLeftOptions()
{
	if ( ( getOptionCount() > 8 ) && ( getBaseOption() > 0 ) )
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
BOOL CMMQuery::hasMoreRightOptions()
{
	if ( ( getOptionCount() > 8 ) && ( getBaseOption() < (DWORD)( getOptionCount() - 8 ) ) )
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
char* CMMQuery::getLeftArrowText()
{
	return "<<   ";
}

/////////////////////////////////////////////////////////////////////////
char* CMMQuery::getRightArrowText()
{
	return "   >>";
}



///////////////////////////////////////////////////////////////////////
// CMMMeterMode
//////////////////////////////////////////////////////////////////////
CMMMeterMode::CMMMeterMode( CControlSurface *pSurface ) :
	CMMQuery( qsMeterMode, pSurface )
{
}

HRESULT CMMMeterMode::onQueryMode( BOOL bActive )
{
	return S_OK;
}


ELedState CMMMeterMode::getOptionLedState( int ix )
{
	switch (ix)
	{
	case 0:
		if (GetStateForID( stMeterMode ) == meMeterOn)
			return LedOn;
		else
			return LedFlash;
		break;

	case 1:
		if (GetStateForID( stMeterMode ) == meMeterOff)
			return LedOn;
		else
			return LedFlash;
		break;
	}
	return LedOff;
}


/////////////////////////////////////////////////////////////////////////
void CMMMeterMode::getOptionText( int ix, char *pText )
{
	switch (ix)
	{
	case 0:
		strcpy( pText, "Metr " );
		break;
	case 1:
		strcpy( pText, "Pram " );
		break;
	default:
		strcpy( pText, "     " );
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMMeterMode::onOption( int ix )
{
	int nNewMeterClass = meMeterOff;

	switch (ix)
	{
	case 0:
		nNewMeterClass = meMeterOn;
		break;

	case 1:
		nNewMeterClass = meMeterOff;
		break;
	}

	m_pSurface->GetStateMgr()->PostStateChange( stMeterMode, nNewMeterClass );

	return S_OK;
}









/////////////////////////////////////////////////////////////////////////
// CMMTransport:
/////////////////////////////////////////////////////////////////////////
CMMTransport::CMMTransport( CControlSurface *pSurface ) :
	CMMQuery( qsTransport, pSurface )
{
}

/////////////////////////////////////////////////////////////////////////
ELedState CMMTransport::getOptionLedState( int ix )
{
	if ( ix >= 0 && ix <=7 )
		return LedFlash;
	return LedOff;
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::getOptionText( int ix, char *pText )
{
	switch (ix)
	{
	case 0:
		strcpy( pText, "Rec  " );
		return;

	case 1:
		strcpy( pText, "Writ " );
		return;

	case 2:
		strcpy( pText, "PchI " );
		return;

	case 3:
		strcpy( pText, "PchO " );
		return;

	case 4:
		strcpy( pText, "Lp I " );
		return;

	case 5:
		strcpy( pText, "Lp O " );
		return;

	case 7:
		if (isLoopEnabled())
		{
			strcpy( pText, "NoLp " );
			return;
		}
		else
		{
			strcpy( pText, "Loop " );
			return;
		}

	default:
		strcpy( pText, "     " );
		return;
	}
}

HRESULT CMMTransport::onOption( int ix )
{
	switch (ix)
	{
	case 0:
		record();
		break;

	case 1:
		write();
		break;
	case 2:
		setPunchIn();
		break;
	case 3:
		setPunchOut();
		break;
	case 4:
		setLoopIn();
		break;
	case 5:
		setLoopOut();
		break;
	case 7:
		setEnableLoop( !isLoopEnabled() );
	default:
		// ignore
		break;
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMTransport::onQueryMode( BOOL bActive )
{
	m_pSurface->GetTransport()->SetIsEnabled( !bActive );
	if (bActive)
		// light up "transport" play button
		m_msgRightBtnLED.Send( (DWORD)0x47 );
	else
		// turn off "transport" play button
		m_msgRightBtnLED.Send( (DWORD)0x07 );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::record()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	pSonarTransport->SetTransportState( TRANSPORT_STATE_REC, TRUE );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::write()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	pSonarTransport->SetTransportState( TRANSPORT_STATE_REC_AUTOMATION, TRUE );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::setPunchIn()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	MFX_TIME tWhen;
	tWhen.timeFormat = TF_SECONDS;

	pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );

	pSonarTransport->SetTransportTime( TRANSPORT_TIME_PUNCH_IN, &tWhen );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::setPunchOut()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	MFX_TIME tWhen;
	tWhen.timeFormat = TF_SECONDS;

	pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );

	pSonarTransport->SetTransportTime( TRANSPORT_TIME_PUNCH_OUT, &tWhen );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::setLoopIn()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	MFX_TIME tWhen;
	tWhen.timeFormat = TF_SECONDS;

	pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );

	pSonarTransport->SetTransportTime( TRANSPORT_TIME_LOOP_IN, &tWhen );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::setLoopOut()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	MFX_TIME tWhen;
	tWhen.timeFormat = TF_SECONDS;

	pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &tWhen );

	pSonarTransport->SetTransportTime( TRANSPORT_TIME_LOOP_OUT, &tWhen );
}

/////////////////////////////////////////////////////////////////////////
void CMMTransport::setEnableLoop( BOOL bEnable )
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	pSonarTransport->SetTransportState( TRANSPORT_STATE_LOOP, bEnable );
}

/////////////////////////////////////////////////////////////////////////
BOOL CMMTransport::isLoopEnabled()
{
	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return FALSE;
	}

	BOOL bIsLoopEnabled = FALSE;

	pSonarTransport->GetTransportState( TRANSPORT_STATE_LOOP, &bIsLoopEnabled);

	return bIsLoopEnabled;
}

/////////////////////////////////////////////////////////////////////////
// CMMContainerMode:
/////////////////////////////////////////////////////////////////////////
CMMContainerMode::CMMContainerMode( CControlSurface *pSurface ):
	CMMQuery( qsContainerMode, pSurface )
{
	SubscribeToState( stContainerClass );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMContainerMode::onOption( int ix )
{
	int nNewContainerClass = ccTracks;

	switch (ix)
	{
	case 0:
		nNewContainerClass = ccTracks;
		break;

	case 1:
		nNewContainerClass = ccBus;
		break;

	case 2:
		nNewContainerClass = ccMains;
		break;
	}

	m_pSurface->GetStateMgr()->PostStateChange( stContainerClass, nNewContainerClass );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
ELedState CMMContainerMode::getOptionLedState( int ix )
{
	switch (ix)
	{
	case 0:
		if (GetStateForID( stContainerClass ) == ccTracks)
			return LedOn;
		else
			return LedFlash;
		break;

	case 1:
		if (GetStateForID( stContainerClass ) == ccBus)
			return LedOn;
		else
			return LedFlash;
		break;

	case 2:
		if (GetStateForID( stContainerClass ) == ccMains)
			return LedOn;
		else
			return LedFlash;
		break;

		

	default:
		return LedOff;
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMContainerMode::getOptionText( int ix, char *pText )
{
	switch (ix)
	{
	case 0:
		strcpy( pText, " Tks " );
		break;

	case 1:
		strcpy( pText, " Bus " );
		break;

	case 2:
		strcpy( pText, " Mns " );
		break;

	default:
		strcpy( pText, "     " );
		break;
	}

	switch (GetStateForID( stContainerClass ))
	{
	case ccTracks:
		if (ix == 0)
		{
			pText[0] = '*';
		}
		break;

	case ccBus:
		if (ix == 1)
		{
			pText[0] = '*';
		}
		break;

	case ccMains:
		if (ix == 2)
		{
			pText[0] = '*';
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
int CMMContainerMode::getOptionCount()
{
	return 3;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMContainerMode::onQueryMode( BOOL bActive )
{
	m_pSurface->GetTransport()->SetIsEnabled( !bActive );
	if (bActive)
		// light up "transport" play button
		m_msgLeftBtnLED.Send( (DWORD)0x47 );
	else
		// turn off "transport" play button
		m_msgLeftBtnLED.Send( (DWORD)0x07 );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
// CMMLocator:
/////////////////////////////////////////////////////////////////////////
CMMLocator::CMMLocator( CControlSurface *pSurface ) :
	CMMQuery( qsLocator, pSurface )
{
	// init locate points as undefined
	for (int ix = 0; ix < MAX_LOCATE_POINTS; ix++)
	{
		m_bIsLocDefined[ ix ] = FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMLocator::onOption( int ix )
{
	if (0 <= ix && ix < 4)
	{
		setLoc( ix );
	}
	else if (4 <= ix && ix < 8)
	{
		gotoLoc( ix - 4 );
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
ELedState CMMLocator::getOptionLedState( int ix )
{
	switch (ix)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return LedFlash;
	default:
		if (4 <= ix && ix < 8)
		{
			return m_bIsLocDefined[ ix - 4 ] ? LedFlash : LedOff;
		}
		else
			return LedOff;
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMLocator::getOptionText( int ix, char *pText)
{
	switch (ix)
	{
	case 0:
		strcpy( pText, "StL1 " );
		return;
	case 1:
		strcpy( pText, "StL2 " );
		return;
	case 2:
		strcpy( pText, "StL3 " );
		return;
	case 3:
		strcpy( pText, "StL4 " );
		return;
	case 4:
		strcpy( pText, "GoL1 " );
		return;
	case 5:
		strcpy( pText, "GoL2 " );
		return;
	case 6:
		strcpy( pText, "GoL3 " );
		return;
	case 7:
		strcpy( pText, "GoL4 " );
		return;
	default:
		_ASSERT( 0 ); // unknown option
		strcpy( pText, "     " );
		return;
	}

}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMLocator::onQueryMode( BOOL bActive )
{
	// we want the transport disabled since this mode is triggered by using a transport key.
	m_pSurface->GetTransport()->SetIsEnabled( !bActive );
	if (bActive)
		// light up "stop" stop button
		m_msgRightBtnLED.Send( (DWORD)0x46 );
	else
		// turn off "locate" stop button
		m_msgRightBtnLED.Send( (DWORD)0x06 );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMLocator::setLoc( int nLoc )
{
	if (MAX_LOCATE_POINTS <= nLoc)
	{
		_ASSERT( 0 ); //index out of range.
		return;
	}

	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	m_LocatePoints[ nLoc ].timeFormat = TF_SECONDS;

	m_bIsLocDefined[ nLoc ] = TRUE;
	pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &(m_LocatePoints[ nLoc ]) );
}

/////////////////////////////////////////////////////////////////////////
void CMMLocator::gotoLoc( int nLoc )
{
	if (MAX_LOCATE_POINTS <= nLoc)
	{
		_ASSERT( 0 ); //index out of range.
		return;
	}

	CComPtr<ISonarTransport> pSonarTransport;
	HRESULT hr = m_pSurface->GetSonarTransport( &pSonarTransport );
	if (FAILED( hr ))
	{
		_ASSERT( 0 );
		return;
	}

	m_LocatePoints[ nLoc ].timeFormat = TF_SECONDS;

	pSonarTransport->SetTransportTime( TRANSPORT_TIME_CURSOR, &(m_LocatePoints[ nLoc ]) );
}

/////////////////////////////////////////////////////////////////////////
// CMMChooseInsert:
/////////////////////////////////////////////////////////////////////////
CMMChooseInsert::CMMChooseInsert( CControlSurface *pSurface ):
	CMMQuery( qsChooseInsert, pSurface )
{
	SubscribeToState( stContainerClass );
	SubscribeToState( stCurrentTrack );
	SubscribeToState( stCurrentMain );
	SubscribeToState( stCurrentBus );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMChooseInsert::onOption( int ix )
{
	m_pSurface->GetStateMgr()->PostStateChange( stCurrentEffect, ix );
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
ELedState CMMChooseInsert::getOptionLedState( int ix )
{
	if (ix < getOptionCount())
	{
		if (ix == GetStateForID( stCurrentEffect))
			return LedOn;
		else
			return LedFlash;
	}
	else
		return LedOff;
}

/////////////////////////////////////////////////////////////////////////
void CMMChooseInsert::getOptionText( int ix, char *pText )
{
	if (ix < getOptionCount())
	{
		char szParamText[64];
		DWORD dwLength = 64;

		CComPtr<ISonarMixer> pSonarMixer;
		HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
		if (FAILED( hr ))
		{
			// pass a blank back
			strcpy(pText, "    ");
			pText[4] = ' ';
			return;
		}
		pSonarMixer->GetMixParamLabel(
			m_pSurface->GetCurrentStripKind(),
			m_pSurface->GetCurrentStrip(),
			MIX_PARAM_FX,
			ix,
			szParamText,
			&dwLength
		);

		CrunchString( szParamText, (DWORD)strlen( szParamText ), pText, 4, ' ' );
		pText[4] = ' ';
	}
	else
	{
		strcpy( pText, "     " );
	}
	return;
}

/////////////////////////////////////////////////////////////////////////
int CMMChooseInsert::getOptionCount()
{
	CComPtr<ISonarMixer> pSonarMixer;
	HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
	if (FAILED( hr ))
		return 0;

	float fVal = 0;
	pSonarMixer->GetMixParam( m_pSurface->GetCurrentStripKind(), m_pSurface->GetCurrentStrip(), MIX_PARAM_FX_COUNT, 0, &fVal );
		
	return (int)fVal;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMChooseInsert::onQueryMode( BOOL bActive )
{
	// we want the transport disabled since this mode is triggered by using a transport key.
	m_pSurface->GetTransport()->SetIsEnabled( !bActive );

	// we let the CChoiceLedUpdater take care of the led state,
	// because the led depends on two states: the stQueryState
	// and the stInsertEditMode.
	// Notice that we could have allowed all of the query state LED's
	// to be controlled by the CChoiceLedUpdater, although in this particular
	// case it is absolutely necessary.
	return S_OK;
}
