/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixSubclasses.cpp
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotorMixSubclasses.h"
#include "StateDefs.h"


/////////////////////////////////////////////////////////////////////////
// CMMChoiceLedUpdater:
/////////////////////////////////////////////////////////////////////////
CMMChoiceLedUpdater::CMMChoiceLedUpdater( CControlSurface *pSurface ) :
	CMultiStateListener( pSurface ),
	m_msgUpperLeftLed( pSurface ),
	m_msgUpperRightLed( pSurface ),
	m_msgLowerLeftLed( pSurface ),
	m_msg7Segment( pSurface ),
	m_msgScribble( pSurface )
{
	m_msgUpperLeftLed.SetMessageType( CMidiMsg::mtCCSel );
	m_msgUpperLeftLed.SetCCSelNum( 0x0C );
	m_msgUpperLeftLed.SetCCSelVal( 0x0A);
	m_msgUpperLeftLed.SetCCNum( 0x2C );

	m_msgUpperRightLed.SetMessageType( CMidiMsg::mtCCSel );
	m_msgUpperRightLed.SetCCSelNum( 0x0C );
	m_msgUpperRightLed.SetCCSelVal( 0x0B);
	m_msgUpperRightLed.SetCCNum( 0x2C );

	m_msgLowerLeftLed.SetMessageType( CMidiMsg::mtCCSel );
	m_msgLowerLeftLed.SetCCSelNum( 0x0C );
	m_msgLowerLeftLed.SetCCSelVal( 0x08);
	m_msgLowerLeftLed.SetCCNum( 0x2C );

	
	// 7 segment display setup
	m_msg7Segment.SetMessageType( CMidiMsg::mtSysXString );

	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x12 };
	BYTE pczPostString[] = {0xF7};

	m_msg7Segment.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msg7Segment.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msg7Segment.SetSysXTextLen( 4 );

	// notice that subscriptions are always done at the end of construction code,
	// because one possible side effect of construction is 
	// that the OnStateChange method may get called.
	SubscribeToState( stBankShiftMode );
	SubscribeToState( stBurnButtonsChoice );
	SubscribeToState( stMultiButtonsChoice );
	SubscribeToState( stTrackRotaryMapping );
	SubscribeToState( stBusRotaryMapping );
	SubscribeToState( stSendPanOrLevel );
	SubscribeToState( stContainerClass );
	SubscribeToState( stShiftKey );
	SubscribeToState( stInsertEditMode );
	SubscribeToState( stFineTweakMode );
	SubscribeToState( stQueryState );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMChoiceLedUpdater::OnStateChange( DWORD dwStateID, int nNewState)
{
	switch (dwStateID)
	{
	case stBankShiftMode:
		switch (nNewState)
		{
		case smOneByOne:
			m_msgUpperLeftLed.Send( (DWORD)0x00 );
			break;

		case smByBanks:
			m_msgUpperLeftLed.Send( (DWORD)0x40 );
			break;

		default:
			_ASSERT( 0 ); // unknown bank shift mode
		}
		break;

	case stBurnButtonsChoice:
		switch (nNewState)
		{
		case bbRecRdy:
			m_msgUpperLeftLed.Send( (DWORD)0x42 );
			m_msgUpperLeftLed.Send( (DWORD)0x04 );
			m_msgUpperLeftLed.Send( (DWORD)0x06 );
			break;

		case bbWrite:
			m_msgUpperLeftLed.Send( (DWORD)0x02 );
			m_msgUpperLeftLed.Send( (DWORD)0x44 );
			m_msgUpperLeftLed.Send( (DWORD)0x06 );
			break;

		case bbOther:
			m_msgUpperLeftLed.Send( (DWORD)0x02 );
			m_msgUpperLeftLed.Send( (DWORD)0x04 );
			m_msgUpperLeftLed.Send( (DWORD)0x46 );
			break;

		default:
			_ASSERT( 0 ); // unknown burn buttons choice
		}
		break;

	case stMultiButtonsChoice:
		switch (nNewState)
		{
		case mbFxBypassE1:
			m_msgUpperRightLed.Send( (DWORD)0x40 );
			m_msgUpperRightLed.Send( (DWORD)0x02 );
			m_msgUpperRightLed.Send( (DWORD)0x04 );
			m_msgUpperRightLed.Send( (DWORD)0x06 );
			break;

		case mbSMuteE2:
			m_msgUpperRightLed.Send( (DWORD)0x00 );
			m_msgUpperRightLed.Send( (DWORD)0x42 );
			m_msgUpperRightLed.Send( (DWORD)0x04 );
			m_msgUpperRightLed.Send( (DWORD)0x06 );
			break;

		case mbPrePostE3:
			m_msgUpperRightLed.Send( (DWORD)0x00 );
			m_msgUpperRightLed.Send( (DWORD)0x02 );
			m_msgUpperRightLed.Send( (DWORD)0x44 );
			m_msgUpperRightLed.Send( (DWORD)0x06 );
			break;

		case mbSelectE4:
			m_msgUpperRightLed.Send( (DWORD)0x00 );
			m_msgUpperRightLed.Send( (DWORD)0x02 );
			m_msgUpperRightLed.Send( (DWORD)0x04 );
			m_msgUpperRightLed.Send( (DWORD)0x46 );
			break;

		default:
			_ASSERT( 0 ); // unknown multi buttons choice
		}
		break;

	case stTrackRotaryMapping:
	case stBusRotaryMapping:
	case stSendPanOrLevel:
	case stContainerClass:
		{
			updateRotaryMappingLED();
		}
		break;

	case stShiftKey:
		{
			switch (nNewState)
			{
			case skShiftIn:
				m_msgLowerLeftLed.Send( (DWORD)0x40 );
				break;

			case skShiftOut:
				m_msgLowerLeftLed.Send( (DWORD)0x00 );
				break;

			default:
				_ASSERT( 0 ); // unknown state for shift key
			}
			break;
		}
	case stInsertEditMode:
		updateRotaryMappingLED();
		// break left out on purpose
		
	case stQueryState:
		updatePlugInBtnLED(); // this LED depends on both the stQueryState and the 
		// stInsertEditMode.
		break;

	case stFineTweakMode:
		{
			switch (nNewState)
			{
			case ftEngaged:
				m_msgLowerLeftLed.Send( (DWORD)0x43 );
				break;

			case ftEngagedTimed:
			case ftDisengaged:
				m_msgLowerLeftLed.Send( (DWORD)0x03 );
				break;

			default:
				_ASSERT( 0 ); // unknown fine tweak mode
			}
			break;
		}
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMChoiceLedUpdater::updatePlugInBtnLED()
{
	switch (GetStateForID( stInsertEditMode ))
	{
	case ieEngaged:
		m_msgLowerLeftLed.Send( (DWORD)0x45 );
		break;

	case ieDisengaged:
		if (GetStateForID( stQueryState ) == qsChooseInsert )
			m_msgLowerLeftLed.Send( (DWORD)0x45 );
		else
			m_msgLowerLeftLed.Send( (DWORD)0x05 );
		break;

	default:
		_ASSERT( 0 ); // unknown insert edit mode
	}
}

/////////////////////////////////////////////////////////////////////////
void CMMChoiceLedUpdater::updateRotaryMappingLED()
{
	// construct the 7 segment display message from the current aux number
	char pbyMsg[4];
	char strDigits[3];

	const SONAR_MIXER_STRIP strip = m_pSurface->GetCurrentStripKind();
	const int nRotState = (strip == MIX_STRIP_TRACK) ?  GetStateForID( stTrackRotaryMapping ) : GetStateForID( stBusRotaryMapping );

	if (GetStateForID( stInsertEditMode ) == ieDisengaged)
	{

		if ( strip == MIX_STRIP_MASTER )
		{
			::strcpy( strDigits, "  " );
		}
		else
		{
			const int nSendState = GetStateForID( stSendPanOrLevel );
			
			if ( nRotState == trPan || nRotState == buPan )
			{
				::strcpy( strDigits, "PN" );
			}
			else
			{
				const int nSendIx = min( 8, nRotState - trSendBase );
				if ( sendLevel == nSendState )
					::sprintf( strDigits, "L%d", nSendIx + 1 );
				else
					::sprintf( strDigits, "P%d", nSendIx + 1 );
			}
		}
	}
	else
	{
		::sprintf( strDigits, "EF" );
	}

	if (strlen( strDigits ) > 1)
	{
		pbyMsg[0] = strDigits[0] >> 4;
		pbyMsg[1] = strDigits[0] & 0x0f;
		pbyMsg[2] = strDigits[1] >> 4;
		pbyMsg[3] = strDigits[1] & 0x0f;
	}
	else // length is one
	{
		_ASSERT( 0 );
	}

	if ( (strip == MIX_STRIP_TRACK || strip == MIX_STRIP_BUS ) && nRotState == sendPan)
	{
		// put a DOT in the display
		pbyMsg[0] |= 0x40;
	}

	m_msg7Segment.SendText( 4, pbyMsg );

}

/////////////////////////////////////////////////////////////////////////
// CMMEncoderStateShifter:
/////////////////////////////////////////////////////////////////////////
CMMEncoderStateShifter::CMMEncoderStateShifter(
	CControlSurface *pSurface,
	CMidiMsg *pMsg,
	DWORD dwStateID,
	DWORD dwInitialState /* =0 */
	) :
	CStateShifter( pSurface, dwStateID, dwInitialState ),
	m_pSurface( pSurface ),
	m_pMsgRotary( pMsg )
{
	if (m_pMsgRotary)
		m_pMsgRotary->AddListener( this );

	SubscribeToState( stInsertEditMode );
	SubscribeToState( stContainerClass );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMEncoderStateShifter::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	if (m_pMsgRotary != pMsg) // ignore it
		return S_FALSE;

	// determine where to shift to
	int nNewState = 0;

	int nInc = dwVal & 0x3f; // value
	DWORD dwSign = dwVal & 0x40; // top bit, or sign
	if (dwSign == 0)
		nInc = 0 - nInc;

	if ((GetStateID() == stTrackRotaryMapping && 
		(m_pSurface->GetCurrentStripKind() == MIX_STRIP_TRACK)) ||
				(GetStateID() == stBusRotaryMapping && 
				(m_pSurface->GetCurrentStripKind() == MIX_STRIP_BUS) ))
	{
		if (GetStateForID( stInsertEditMode ) == ieEngaged)
		{
			int nBaseFxParam = m_pSurface->GetStateMgr()->GetShifterFromID( stBaseEffectParam )->GetCurrentState();
			m_pSurface->GetStateMgr()->PostStateChange( stBaseEffectParam, nBaseFxParam + nInc );
			return S_OK;
		}
		else if (m_pSurface->GetCurrentStripKind() != MIX_STRIP_MAIN)
		{
			// shift my own state
			nNewState = GetCurrentState() + nInc;
			return SetNewState( nNewState );
		}
		else
			return S_FALSE;
	}
	else
		return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////
int CMMEncoderStateShifter::GetMaxState()
{
	DWORD nMax = 0;

	if (GetStateID() == stTrackRotaryMapping || GetStateID() == stBusRotaryMapping)
	{
		return 10;
	}
	else
	{
		_ASSERT( 0 ); // unknown rotary mapping ID
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMEncoderStateShifter::SetNewState( int nNewState )
{
	// When loading, sonar may not have yet defined its containers,
	// and a validation of them may fail.
	// Instead, simply set the state variable without validating it
	// and wait until the next call to OnHostNotify to validate
	if (m_pSurface->GetStateMgr()->IsLoading())
	{
		m_nCurrentState = nNewState;
		return S_FALSE;
	}
	else
		return CStateShifter::SetNewState( nNewState );
}

/////////////////////////////////////////////////////////////////////////
CMMEncoderStateShifter::~CMMEncoderStateShifter()
{
	if (m_pMsgRotary)
		m_pMsgRotary->RemoveListener( this );
}

/////////////////////////////////////////////////////////////////////////
// CMMRotaryParam:
/////////////////////////////////////////////////////////////////////////
CMMRotaryParam::CMMRotaryParam(
		CControlSurface *pSurface,
		DWORD dwHWStripNum,
		CMidiMsg *pMsgInput,
		CMidiMsg *pMsgOutput /*= NULL */,
		CMidiMsg *pMsgCapture /* = NULL */, DWORD dwCaptureVal /* = VAL_ANY */,
		CMidiMsg *pMsgRelease /* = NULL */, DWORD dwReleaseVal /* = VAL_ANY */
	):
	CMixParamFloat(
		pSurface,
		pMsgInput,
		pMsgOutput,
		pMsgCapture,
		dwCaptureVal,
		pMsgRelease,
		dwReleaseVal
	),
	m_dwHWStripNum( dwHWStripNum ),
	m_msgPointerGraph( pSurface )
{
	// scribble bargraph message setup
	m_msgPointerGraph.SetMessageType( CMidiMsg::mtSysX7bit );

	SetPointerType( 0x03 );
	BYTE pczPostString[] = {0xF7};
	m_msgPointerGraph.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
}

/////////////////////////////////////////////////////////////////////////
void CMMRotaryParam::SetPointerType( DWORD dwPointerType )
{
	//                                                                     **** pointer type is index 8
	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x11, 0x00, 0x00 }; // last byte is replaced with the "address" for the scribble strip.
	pczPreString[9] = BYTE(m_dwHWStripNum);
	pczPreString[8] = BYTE(dwPointerType);
	m_msgPointerGraph.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMRotaryParam::SetIsEnabled( BOOL bIsEnabled )
{
	if (bIsEnabled == FALSE)
		m_msgPointerGraph.Send( (DWORD)0 );

	return CMixParamFloat::SetIsEnabled( bIsEnabled );
}

/////////////////////////////////////////////////////////////////////////
void CMMRotaryParam::refreshParam( float *pfValue /*= NULL*/ )
{
	if (m_bIsEnabled == FALSE)
		return;

	if (IsParamValid( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum ) == S_OK)
	{
		float fValue = 0;
		if (pfValue == NULL)
		{
			CComPtr<ISonarMixer> pSonarMixer;
			HRESULT hr = m_pSurface->GetSonarMixer( &pSonarMixer );
			if (FAILED( hr ))
			{
				fValue = 0; // nothing else we can do
			}
			else
			{
				pSonarMixer->GetMixParam( m_mixerStrip, m_dwStripNum, m_mixerParam, m_dwParamNum, &fValue );

				// for metering, clamp the value to 1.0f
				if ( MIX_PARAM_AUDIO_METER == m_mixerParam )
					fValue = min( 1.0f, fValue );
			}
		}
		else
			fValue = *pfValue;

		m_msgPointerGraph.Send( fValue );
		CMixParamFloat::refreshParam( pfValue );
	}
	else
		m_msgPointerGraph.Send( (DWORD)0 );
}

/////////////////////////////////////////////////////////////////////////
// CMMContainerPicker:
/////////////////////////////////////////////////////////////////////////

#define OUT_OF_SCOPE_LED	(8)
#define INVALID_LED			(-1)

CMMContainerPicker::CMMContainerPicker( CControlSurface *pSurface ) :
	CMultiStateListener( pSurface ),
	m_nLastLedIndex( INVALID_LED ),	// we do our own thinning
	m_msgSelectBtns( pSurface ),
	m_msgSelectLED( pSurface )
{
	// setup the select buttons
	m_msgSelectBtns.SetMessageType( CMidiMsg::mtCCHiLo );
	m_msgSelectBtns.SetCCNum( 0x0f );
	m_msgSelectBtns.SetCC2Num( 0x2f );

	// setup the select buttons LED message
	m_msgSelectLED.SetMessageType( CMidiMsg::mtCCHiLo );
	m_msgSelectLED.SetCCNum( 0x0c );
	m_msgSelectLED.SetCC2Num( 0x2c );

	// we do our own thinning since this message gets used
	// for all 8 LEDs
	m_msgSelectLED.SetIsOutputThin( FALSE );

	m_msgSelectBtns.AddListener( this );

	SubscribeToState( stContainerClass );
	SubscribeToState( stShiftKey );
	SubscribeToState( stBaseTrack );
	SubscribeToState( stBaseBus );
	SubscribeToState( stBaseMain );
	SubscribeToState( stQueryState );
}

/////////////////////////////////////////////////////////////////////////
CMMContainerPicker::~CMMContainerPicker()
{
	m_msgSelectBtns.RemoveListener( this );
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMContainerPicker::OnStateChange( DWORD dwStateID, int nNewState )
{
	switch (dwStateID)
	{
	case stQueryState:
	case stShiftKey:
		m_nLastLedIndex = INVALID_LED;
	}
	return refreshButtonLEDs();
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMContainerPicker::refreshButtonLEDs()
{
	if (GetStateForID( stQueryState ) != qsIdle)
		return S_FALSE;

	int nLedIndex = 0; // this index may be less than 0 or more than 7 if the
	// current container is out of the bank's scope. This is not a problem.

	switch (GetStateForID( stContainerClass ))
	{
	case ccTracks:
		nLedIndex = GetStateForID( stCurrentTrack ) - GetStateForID( stBaseTrack );
		break;

	case ccBus:
		nLedIndex = GetStateForID( stCurrentBus ) - GetStateForID( stBaseBus );
		break;

	case ccMains:
		nLedIndex = GetStateForID( stCurrentMain ) - GetStateForID( stBaseMain );
		break;

	default:
		_ASSERT( 0 ); // unknown container class
	}

	if (nLedIndex > 7 || nLedIndex < 0)
		nLedIndex = OUT_OF_SCOPE_LED;

	// Because we reuse the same m_msgSelectLED for all the LEDs
	// thinning does not work (since we send a different message for each LED)
	// A way this could have been solved would have been by having
	// 8 m_msgSelects one for each LED. However, I preffer
	// do my own thinning locally.

	if (nLedIndex == m_nLastLedIndex) // if unchanged, dont resend
		return S_FALSE;

	// flash the select button leds
	for (int ix = 0; ix < 8; ix++)
	{
		if (nLedIndex == ix)
			m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x41) );
		else
			m_msgSelectLED.Send( (DWORD)( ( ix << 7 ) + 0x01) );
	}
	m_nLastLedIndex = nLedIndex;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMContainerPicker::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	if (pMsg == &m_msgSelectBtns)
	{
		// allow picking a container only when the shift key is out.
		

		if ( ( GetStateForID( stShiftKey ) == skShiftOut ) &&
			( GetStateForID( stQueryState ) == qsIdle ) )
		{
			DWORD dwButtonIx = dwVal >> 7;
			DWORD dwButtonState = dwVal & 0x7f;

			if (dwButtonIx > 7)
				return S_FALSE;

			if (dwButtonState == 0x41) // button was pressed
			{
				switch (GetStateForID( stContainerClass ))
				{
				case ccTracks:
					{
						DWORD dwNewTrack = dwButtonIx + GetStateForID( stBaseTrack );
						m_pSurface->GetStateMgr()->PostStateChange( stCurrentTrack, dwNewTrack );
					}
					break;

				case ccBus:
					{
						DWORD dwNewBus = dwButtonIx + GetStateForID( stBaseBus );
						m_pSurface->GetStateMgr()->PostStateChange( stCurrentBus, dwNewBus );
					}
					break;

				case ccMains:
					{
						DWORD dwNewMain = dwButtonIx + GetStateForID( stBaseMain );
						m_pSurface->GetStateMgr()->PostStateChange( stCurrentMain, dwNewMain );
					}
					break;

				default:
					_ASSERT( 0 ); // unknown container class
				}
			}
		}
		HRESULT hr = refreshButtonLEDs();
		_ASSERT( SUCCEEDED( hr ) );
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
CMMFineTweakMode::CMMFineTweakMode( CControlSurface *pSurface ) :
	m_pSurface( pSurface ),
	CHostNotifyListener( 0, pSurface ),
	CMultiStateListener( pSurface ),
	CLastParamChangeListener( pSurface ),
	m_msgScribbleContainer( pSurface ),
	m_msgScribbleSubCont( pSurface ),
	m_msgScribbleParam( pSurface ),
	m_msgScribbleValue( pSurface ),
	m_nRefreshCount( 0 )
{
	// setup scribble message
	BYTE pczPreString[] = {0xF0, 0x00, 0x01, 0x0F, 0x00, 0x11, 0x00, 0x10, 0x00 };	
	BYTE pczPostString[] = {0xF7};

	pczPreString[8] = BYTE(0);

	m_msgScribbleContainer.SetMessageType( CMidiMsg::mtSysXString );
	m_msgScribbleContainer.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribbleContainer.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribbleContainer.SetSysXTextLen( 10 ); // print 10 characters out each time
	m_msgScribbleContainer.SetSysXPadLen( 1 ); // force last one to be padding
	m_msgScribbleContainer.SetSysXTextFillChar( ' ' ); // padding is space bar

	pczPreString[8] = BYTE(10);
	m_msgScribbleSubCont.SetMessageType( CMidiMsg::mtSysXString );
	m_msgScribbleSubCont.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribbleSubCont.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribbleSubCont.SetSysXTextLen( 10 ); // print 10 characters out each time
	m_msgScribbleSubCont.SetSysXPadLen( 1 ); // force last one to be padding
	m_msgScribbleSubCont.SetSysXTextFillChar( ' ' ); // padding is space bar

	pczPreString[8] = BYTE(20);
	m_msgScribbleParam.SetMessageType( CMidiMsg::mtSysXString );
	m_msgScribbleParam.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribbleParam.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribbleParam.SetSysXTextLen( 10 ); // print 10 characters out each time
	m_msgScribbleParam.SetSysXPadLen( 1 ); // force last one to be padding
	m_msgScribbleParam.SetSysXTextFillChar( ' ' ); // padding is space bar

	pczPreString[8] = BYTE(30);
	m_msgScribbleValue.SetMessageType( CMidiMsg::mtSysXString );
	m_msgScribbleValue.SetSysXPreString(	(const unsigned char *)( pczPreString ), sizeof(pczPreString) );
	m_msgScribbleValue.SetSysXPostString(	(const unsigned char *)( pczPostString ), sizeof(pczPostString) );
	m_msgScribbleValue.SetSysXTextLen( 10 ); // print 10 characters out each time
	m_msgScribbleValue.SetSysXTextFillChar( ' ' ); // padding is space bar

	SubscribeToState( stFineTweakMode );
}

/////////////////////////////////////////////////////////////////////////
CMMFineTweakMode::~CMMFineTweakMode()
{
}

/////////////////////////////////////////////////////////////////////////
HRESULT CMMFineTweakMode::OnStateChange( DWORD dwStateID, int nNewState )
{
	if (m_pSurface->GetLastParamChange()->IsDefined() && ( GetStateForID( stFineTweakMode ) != ftDisengaged ) )
	{
		// show it on the screen
		m_msgScribbleContainer.Invalidate();
		m_msgScribbleSubCont.Invalidate();
		m_msgScribbleValue.Invalidate();
		m_msgScribbleParam.Invalidate();
		RequestRefresh();
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CMMFineTweakMode::OnParamChange()
{
	if (GetStateForID(stFineTweakMode) == ftDisengaged)
	{
		m_pSurface->GetStateMgr()->PostStateChange( stFineTweakMode, ftEngagedTimed );
	}
	else
	{
		RequestRefresh();
	}

	if (GetStateForID(stFineTweakMode) != ftEngaged)
		// only bypass scheduled disengagement if it is "engaged"
		m_pSurface->GetStateMgr()->ScheduleStateChange( stFineTweakMode, ftDisengaged, 2000, TRUE );
}

/////////////////////////////////////////////////////////////////////////

void CMMFineTweakMode::OnHostNotify( DWORD fdwRefresh, DWORD dwCookie )
{
	if (GetStateForID(stFineTweakMode) == ftDisengaged)
		return;

	if ( m_nRefreshCount++ % 2 != 0 )
		return;

	// get the fine text for the last parameter change:
	SONAR_MIXER_STRIP mixerStrip = MIX_STRIP_TRACK;
	DWORD dwStripNum = 0;
	SONAR_MIXER_PARAM mixerParam = MIX_PARAM_VOL;
	DWORD dwParamNum = 0;

	m_pSurface->GetLastParamChange()->GetLastParamID( &mixerStrip, &dwStripNum, &mixerParam, &dwParamNum );
	CComPtr<ISonarMixer> pSonarMixer;
	if (FAILED( m_pSurface->GetSonarMixer( &pSonarMixer ) ))
		return;

	char szParamText[64];
	DWORD dwLength = 64;

	pSonarMixer->GetMixStripName( mixerStrip, dwStripNum, szParamText, &dwLength );
	m_msgScribbleContainer.SendText( (DWORD)strlen( szParamText ), szParamText );


	if (mixerParam == MIX_PARAM_FX_PARAM)
	{
		dwLength = 64;
		pSonarMixer->GetMixParamLabel( mixerStrip, dwStripNum, MIX_PARAM_FX, LOWORD( dwParamNum ), szParamText, &dwLength );
		m_msgScribbleSubCont.SendText( (DWORD)strlen( szParamText ), szParamText );
	}
	else
		m_msgScribbleSubCont.SendText( 10, "          " );


	dwLength = 64;
	pSonarMixer->GetMixParamLabel( mixerStrip, dwStripNum, mixerParam, dwParamNum, szParamText, &dwLength );
	m_msgScribbleParam.SendText( (DWORD)strlen( szParamText ), szParamText );

	float fFoo = 0;
	pSonarMixer->GetMixParam( mixerStrip, dwStripNum, mixerParam, dwParamNum, &fFoo );

	dwLength = 64;
	pSonarMixer->GetMixParamValueText( mixerStrip, dwStripNum, mixerParam, dwParamNum, fFoo, szParamText, &dwLength );
	m_msgScribbleValue.SendText( (DWORD)strlen( szParamText ), szParamText );
}




//----------------------------------------------------------------------
CMMCommand::CMMCommand( CControlSurface *pSurface ) :
	CMultiStateListener( pSurface ),
	m_msgUndoSave( pSurface ),
	m_nCurShift( 1 )
{
	m_msgUndoSave.SetMessageType( CMidiMsg::mtCCSel );
	m_msgUndoSave.SetCCSelNum( 0x0f );
	m_msgUndoSave.SetCCSelVal( 0x08 );
	m_msgUndoSave.SetCCNum( 0x2F );
	m_msgUndoSave.AddListener( this );

	SubscribeToState( stShiftKey );
}

CMMCommand::~CMMCommand()
{
	m_msgUndoSave.RemoveListener( this );
}



// CMultiStateListener override
HRESULT	CMMCommand::OnStateChange( DWORD dwStateID, int nNewState )
{
	switch( dwStateID )
	{
	case stShiftKey:
		m_nCurShift = nNewState;
		break;
	}

	return S_OK;
}

// CMidiMsgListener override
HRESULT	CMMCommand::setValue( CMidiMsg *pMsg, DWORD dwVal )
{
	CComPtr<ISonarCommands> pISonarCommands;
	MM_CHECKHR( m_pSurface->GetSonarCommands( &pISonarCommands ) );
	if ( 0x41 == dwVal )		// Undo/Save button
	{
		if ( 0 == m_nCurShift )
			MM_CHECKHR( pISonarCommands->DoCommand( CMD_FILE_SAVE ) );
		else
			MM_CHECKHR( pISonarCommands->DoCommand( CMD_EDIT_UNDO ) );
	}
	else if ( 0x42 == dwVal )	// "Default" button as REDO
	{
		MM_CHECKHR( pISonarCommands->DoCommand( CMD_EDIT_REDO ) );
	}

	return S_OK;
}





