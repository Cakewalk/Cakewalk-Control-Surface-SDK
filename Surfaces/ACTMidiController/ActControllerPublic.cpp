#include "stdafx.h"

#include "ACTController.h"

#define CHECK_RANGE( _n, _m ) (_n >= 0 && _n < _m)

/////////////////////////////////////////////////////////////////////////////

void CACTController::RestoreDefaultBindings(bool bUpdateBindings)
{
	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::RestoreDefaultBindings()\n");

	m_bSelectHighlightsTrack = true;
	m_bACTFollowsContext = true;

	m_dwKnobsBinding[0]		= MAKELONG(MIX_PARAM_PAN, 0);
	m_dwSlidersBinding[0]	= MAKELONG(MIX_PARAM_VOL, 0);
	m_dwKnobsBinding[1]		= MAKELONG(MIX_PARAM_SEND_PAN, 0);
	m_dwSlidersBinding[1]	= MAKELONG(MIX_PARAM_SEND_VOL, 0);
	m_dwKnobsBinding[2]		= MAKELONG(MIX_PARAM_SEND_PAN, 1);
	m_dwSlidersBinding[2]	= MAKELONG(MIX_PARAM_SEND_VOL, 1);
	m_dwKnobsBinding[3]		= MAKELONG(MIX_PARAM_SEND_PAN, 2);
	m_dwSlidersBinding[3]	= MAKELONG(MIX_PARAM_SEND_VOL, 2);

	int m, n;

	for (m = 0; m < NUM_BANKS; m++)
	{
		m_bExcludeRotariesACT[m] = false;
		m_bExcludeSlidersACT[m] = false;

		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			m_dwButtonAction[m][n] = CMD_NONE;
			m_bButtonExcludeACT[m][n] = true;
		}
	}

	// Buttons which are the same in all banks
	for (m = 0; m < NUM_BANKS; m++)
	{
		m_dwButtonAction[m][BUTTON_1]		= CMD_REALTIME_REW;
		m_dwButtonAction[m][BUTTON_2]		= CMD_REALTIME_STOP;
		m_dwButtonAction[m][BUTTON_3]		= CMD_REALTIME_PLAY;
		m_dwButtonAction[m][BUTTON_SHIFT_3]	= CMD_REALTIME_RECORD;
		m_dwButtonAction[m][BUTTON_4]		= CMD_ACT_ENABLE;
		m_dwButtonAction[m][BUTTON_5]		= CMD_ROTARIES_MODE;
		m_dwButtonAction[m][BUTTON_6]		= CMD_NEXT_STRIP_TYPE;
		m_dwButtonAction[m][BUTTON_7]		= CMD_NEXT_ROTARIES_AND_SLIDERS_BANK;
		m_dwButtonAction[m][BUTTON_8]		= CMD_NEXT_BUTTONS_BANK;
	}

	// Bank 1
	m_dwButtonAction[0][BUTTON_SHIFT_1]	= CMD_LOOP_TOGGLE;
	m_dwButtonAction[0][BUTTON_SHIFT_2]	= CMD_INSERT_MARKER;
	m_dwButtonAction[0][BUTTON_SHIFT_4]	= CMD_ACT_LOCK;
	m_dwButtonAction[0][BUTTON_SHIFT_5]	= CMD_ACT_LEARN;
	m_dwButtonAction[0][BUTTON_SHIFT_6]	= CMD_PREV_STRIP_TYPE;
	m_dwButtonAction[0][BUTTON_SHIFT_7]	= CMD_PREV_TRACK_BANK;
	m_dwButtonAction[0][BUTTON_SHIFT_8]	= CMD_NEXT_TRACK_BANK;

	// Bank 2
	m_dwButtonAction[1][BUTTON_SHIFT_1]	= CMD_AUTO_READ_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_2]	= CMD_AUTO_WRITE_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_4]	= CMD_MUTE_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_5]	= CMD_SOLO_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_6]	= CMD_REC_ARM_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_7]	= CMD_PREV_SEL_TRACK;
	m_dwButtonAction[1][BUTTON_SHIFT_8]	= CMD_NEXT_SEL_TRACK;

	// Bank 3
	m_dwButtonAction[2][BUTTON_SHIFT_1]	= CMD_GOTO_END;
	m_dwButtonAction[2][BUTTON_SHIFT_2]	= CMD_REALTIME_AUDIO_RUNNING;

	// Buttons available for ACT
	for (m = 0; m < NUM_BANKS; m++)
	{
		m_bButtonExcludeACT[m][BUTTON_SHIFT_5] = false;
		m_bButtonExcludeACT[m][BUTTON_SHIFT_6] = false;
		m_bButtonExcludeACT[m][BUTTON_SHIFT_7] = false;
		m_bButtonExcludeACT[m][BUTTON_SHIFT_8] = false;
	}

	UpdateButtonActionStrings();

	if (bUpdateBindings)
		UpdateBindings();	// Don't call this from the constructor
	else
		BuildDynControlsList();
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ResetMidiLearn()
{
	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::ResetMidiLearn()\n");

	int n;

	for (n = 0; n < NUM_KNOBS; n++)
	{
		m_cMidiKnob[n].SetMessage(0, 0, 0);
		m_cMidiKnob[n].m_eMessageInterpretation = CMidiBinding::MI_Literal;
	}

	for (n = 0; n < NUM_SLIDERS; n++)
	{
		m_cMidiSlider[n].SetMessage(0,0,0);
		m_cMidiSlider[n].m_eMessageInterpretation = CMidiBinding::MI_Literal;
	}

	BYTE b;
	for (n = 0; n < 3; n++)
	{
		m_cMidiButton[n].SetMessage( &b, 0 );
		m_cMidiButton[n].SetMessage(0,0,0);
	}

	for (n = 0; n < 5; n++)
	{
		m_cMidiButton[n + 3].SetMessage(&b, 0);
		m_cMidiButton[n + 3].SetMessage(0,0,0);
	}

	m_cMidiModifierDown.SetMessage(0,0,0);
	m_cMidiModifierDown.SetMatchType( CMidiBinding::MT_MessageAndBool );

	m_cMidiModifierUp.SetMessage(0,0,0);
	m_cMidiModifierUp.SetMatchType( CMidiBinding::MT_MessageAndBool );

	m_bModifierIsDown = false;
	m_pMidiLearnTarget = NULL;

	OnContextSwitch();
}

/////////////////////////////////////////////////////////////////////////////

SONAR_MIXER_STRIP CACTController::GetStripType()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_eStripType;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetStripType(SONAR_MIXER_STRIP eStripType)
{
	CCriticalSectionAuto csa( &m_cs );

	if (m_eStripType != eStripType)
	{
		m_eStripType = eStripType;

		ShiftStripNum(0, true);
		ShiftSelectedTrack(0, true);
	}
}

/////////////////////////////////////////////////////////////////////////////

vectorDwordCStringPairs *CACTController::GetKnobBindings()
{
	return &m_vKnobBindings;
}

/////////////////////////////////////////////////////////////////////////////

vectorDwordCStringPairs *CACTController::GetSliderBindings()
{
	return &m_vSliderBindings;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetKnobsBinding(int iBank)
{
	if (iBank < 0 || iBank >= NUM_BANKS)
		return 0;

	return m_dwKnobsBinding[iBank];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetKnobsBinding(int iBank, DWORD dwBinding)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (m_dwKnobsBinding[iBank] != dwBinding)
	{
		m_dwKnobsBinding[iBank] = dwBinding;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetSlidersBinding(int iBank)
{
	if (iBank < 0 || iBank >= NUM_BANKS)
		return 0;

	return m_dwSlidersBinding[iBank];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetSlidersBinding(int iBank, DWORD dwBinding)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (m_dwSlidersBinding[iBank] != dwBinding)
	{
		m_dwSlidersBinding[iBank] = dwBinding;

		UpdateBindings();
	}
}


void	CACTController::SetRotaryMidiInterpretation( int n, CMidiBinding::MessageInterpretation mi )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
	{
		ASSERT(0);
		return;
	}

	CCriticalSectionAuto csa( &m_cs );
	m_cMidiKnob[n].m_eMessageInterpretation =  mi;
}

void	CACTController::SetSliderMidiInterpretation( int n, CMidiBinding::MessageInterpretation mi )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
	{
		ASSERT(0);
		return;
	}
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiSlider[n].m_eMessageInterpretation = mi;
}

void CACTController::SetRotaryIncrementHingeValue( int n, DWORD dwHinge )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
	{
		ASSERT(0);
		return;
	}
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiKnob[n].m_dwHinge = dwHinge;
}
void CACTController::SetSliderIncrementHingeValue( int n, DWORD dwHinge )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return;

	CCriticalSectionAuto csa( &m_cs );
	m_cMidiSlider[n].m_dwHinge = dwHinge;
}
DWORD	CACTController::GetRotaryIncrementHingeValue( int n )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiKnob[n].m_dwHinge;
}
DWORD	CACTController::GetSliderIncrementHingeValue( int n )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiSlider[n].m_dwHinge;
}

void	CACTController::SetRotaryIncrementUseAccel( int n, bool b )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return;
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiKnob[n].m_bUseAccel = b;
}
void	CACTController::SetSliderIncrementUseAccel( int n, bool b )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return;
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiSlider[n].m_bUseAccel = b;
}
bool	CACTController::GetRotaryIncrementUseAccel( int n )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiKnob[n].m_bUseAccel;
}
bool	CACTController::GetSliderIncrementUseAccel( int n )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return  m_cMidiSlider[n].m_bUseAccel;
}

void	CACTController::SetRotaryMessage( int n, WORD w )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return;
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiKnob[n].SetMessage( (BYTE)(w & 0xff), (BYTE)((w >> 8) & 0xff ) );
}
void	CACTController::SetSliderMessage( int n, WORD w )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return;

	CCriticalSectionAuto csa( &m_cs );
	m_cMidiSlider[n].SetMessage( (BYTE)(w & 0xff), (BYTE)((w >> 8) & 0xff ) );
}
WORD	CACTController::GetRotaryMessage( int n )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiKnob[n].GetMessage();
}
WORD	CACTController::GetSliderMessage( int n )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiSlider[n].GetMessage();
}


void CACTController::GetButtonSysex( int n, std::vector<BYTE>* pv )
{
	if ( !CHECK_RANGE( n, NUM_BUTTONS) )
		return;
	CCriticalSectionAuto csa( &m_cs );

	m_cMidiButton[n].GetSysex( pv );
}
void CACTController::SetButtonSysex( int n, std::vector<BYTE>& v )
{
	if ( !CHECK_RANGE( n, NUM_BUTTONS) )
		return;
	CCriticalSectionAuto csa( &m_cs );

	m_cMidiButton[n].SetSysex( v );
}

void	CACTController::SetButtonMessage( int n, WORD w )
{
	if ( !CHECK_RANGE( n, NUM_BUTTONS) )
		return;
	CCriticalSectionAuto csa( &m_cs );
	m_cMidiButton[n].SetMessage((BYTE)(w & 0xff), (BYTE)((w >> 8) & 0xff ));
}
WORD	CACTController::GetButtonMessage( int n )
{
	if ( !CHECK_RANGE( n, NUM_BUTTONS) )
		return 0;
	CCriticalSectionAuto csa( &m_cs );
	return m_cMidiButton[n].GetMessage();
}


/////////////////////////////////////////////////////////////////////////////

vectorDwordCStringPairs *CACTController::GetButtonNames()
{
	int n;

	m_vButtonNames.clear();

	for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		AddItem(&m_vButtonNames, m_strButtonLabel[n].GetBuffer(), n);

	return &m_vButtonNames;
}

/////////////////////////////////////////////////////////////////////////////

vectorDwordCStringPairs	*CACTController::GetButtonActions()
{
	return &m_vButtonActions;
}

/////////////////////////////////////////////////////////////////////////////

vectorDwordCStringPairs	*CACTController::GetBankNames()
{
	return &m_vBankNames;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetButtonAction(int iBank, VirtualButton bButton)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return CMD_NONE;

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return CMD_NONE;

	return m_dwButtonAction[iBank][bButton];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetButtonAction(int iBank, VirtualButton bButton, DWORD dwAction)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return;

	if (m_dwButtonAction[iBank][bButton] != dwAction)
	{
		m_dwButtonAction[iBank][bButton] = dwAction;

		UpdateButtonActionString(iBank, bButton);
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetButtonExcludeACT(int iBank, VirtualButton bButton)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return false;

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return false;

	return m_bButtonExcludeACT[iBank][bButton];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetButtonExcludeACT(int iBank, VirtualButton bButton, bool bExclude)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return;

	if (m_bButtonExcludeACT[iBank][bButton] != bExclude)
	{
		m_bButtonExcludeACT[iBank][bButton] = bExclude;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetButtonLabel(VirtualButton bButton, CString strText)
{
	CCriticalSectionAuto csa( &m_cs );

	if (bButton > NUM_VIRTUAL_BUTTONS)
		return;

	m_strButtonLabel[bButton] = strText;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetButtonLabel(VirtualButton bButton, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bButton > NUM_VIRTUAL_BUTTONS)
		return;

	*strText = m_strButtonLabel[bButton];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetButtonName(VirtualButton bButton, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return;

	int n = bButton;

	if (n >= NUM_BUTTONS)
		n -= NUM_BUTTONS;

	if (&m_cMidiButton[n] == m_pMidiLearnTarget)
	{
		*strText = m_strMidiLearn;
	}
	else if (m_pMidiLearnTarget)
	{
		return;
	}
	else if (GetButtonACTMode(m_iButtonBank, bButton))
	{
		GetStripNameAndParamLabel(&m_SwButton[m_iButtonBank][bButton], strText);
	}
	else
	{
		*strText = m_strButtonAction[m_iButtonBank][bButton];
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetButtonValue(VirtualButton bButton, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return;

	if (m_pMidiLearnTarget)
		return;

	if (GetButtonACTMode(m_iButtonBank, bButton))
	{
		DWORD dwLen = 256;
		char szTxt[256] = { NULL };

		HRESULT hr = m_SwButton[m_iButtonBank][bButton].GetValueText(szTxt, &dwLen);
		
		if (FAILED(hr))
			strText->Empty();
		else
		{
			Char2TCHAR(strText->GetBufferSetLength(256), szTxt, 256);
			strText->ReleaseBuffer();
		}
	}
	else
	{
		switch (m_dwButtonAction[m_iButtonBank][bButton])
		{
			case CMD_ACT_ENABLE:
				*strText = GetUseDynamicMappings() ? m_strOn : m_strOff;
				break;

			case CMD_ACT_LOCK:
				*strText = GetLockDynamicMappings() ? m_strOn : m_strOff;
				break;

			case CMD_ACT_LEARN:
				*strText = GetLearnDynamicMappings() ? m_strOn : m_strOff;
				break;

			case CMD_ROTARIES_MODE:
				*strText = (m_eRotariesMode == MCS_ASSIGNMENT_MUTLI_CHANNEL) ? m_strMultiChannel : m_strChannelStrip;
				break;

			case CMD_MUTE_SEL_TRACK:
				*strText = GetSelectedTrackParam(MIX_PARAM_MUTE) ? m_strOn : m_strOff;
				break;

			case CMD_SOLO_SEL_TRACK:
				*strText = GetSelectedTrackParam(MIX_PARAM_SOLO) ? m_strOn : m_strOff;
				break;

			case CMD_REC_ARM_SEL_TRACK:
				*strText = GetSelectedTrackParam(MIX_PARAM_RECORD_ARM) ? m_strOn : m_strOff;
				break;

			case CMD_AUTO_READ_SEL_TRACK:
				*strText = GetSelectedTrackAutomationRead() ? m_strOn : m_strOff;
				break;

			case CMD_AUTO_WRITE_SEL_TRACK:
				*strText = GetSelectedTrackAutomationWrite() ? m_strOn : m_strOff;
				break;

#if 0
			case CMD_REALTIME_STOP:
				*strText = !GetTransportState(TRANSPORT_STATE_PLAY) ? m_strOn : m_strOff;
				break;

			case CMD_REALTIME_PLAY:
				*strText = GetTransportState(TRANSPORT_STATE_PLAY) ? m_strOn : m_strOff;
				break;

			case CMD_REALTIME_RECORD:
				*strText = (GetTransportState(TRANSPORT_STATE_REC) || 
							GetTransportState(TRANSPORT_STATE_REC_AUTOMATION)) ? m_strOn : m_strOff;
				break;
#endif

			case CMD_REALTIME_AUDIO_RUNNING:
				*strText = GetTransportState(TRANSPORT_STATE_AUDIO) ? m_strOn : m_strOff;
				break;

			default:
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetRotaryLabel(BYTE bKnob, CString strText)
{
	CCriticalSectionAuto csa( &m_cs );

	if (bKnob > NUM_KNOBS)
		return;

	m_strRotaryLabel[bKnob] = strText;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetRotaryLabel(BYTE bKnob, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bKnob > NUM_KNOBS)
		return;

	*strText = m_strRotaryLabel[bKnob];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetRotaryName(BYTE bKnob, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bKnob >= NUM_KNOBS)
		return;

	if (&m_cMidiKnob[bKnob] == m_pMidiLearnTarget)
	{
		*strText = m_strMidiLearn;
	}
	else if (m_pMidiLearnTarget)
	{
		return;
	}
	else
	{
		GetStripNameAndParamLabel(&m_SwKnob[m_iRotaryBank][bKnob], strText);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetRotaryValue(BYTE bKnob, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bKnob >= NUM_KNOBS)
		return;

	if (m_pMidiLearnTarget)
		return;

	DWORD dwLen = 256;
	char szTxt[256] = { NULL };

	HRESULT hr = m_SwKnob[m_iRotaryBank][bKnob].GetValueText(szTxt, &dwLen);	

	if (FAILED(hr))
		strText->Empty();
	else
	{
		Char2TCHAR(strText->GetBufferSetLength(256), szTxt, 256);
		strText->ReleaseBuffer();
	}
}


/////////////////////////////////////////////////////////////////////////////

void CACTController::SetSliderLabel(BYTE bSlider, CString strText)
{
	CCriticalSectionAuto csa( &m_cs );

	if (bSlider > NUM_SLIDERS)
		return;

	m_strSliderLabel[bSlider] = strText;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetSliderLabel(BYTE bSlider, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bSlider > NUM_SLIDERS)
		return;

	*strText = m_strSliderLabel[bSlider];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetSliderName(BYTE bSlider, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bSlider >= NUM_SLIDERS)
		return;

	if (&m_cMidiSlider[bSlider] == m_pMidiLearnTarget)
	{
		*strText = m_strMidiLearn;
	}
	else if (m_pMidiLearnTarget)
	{
		return;
	}
	else
	{
		GetStripNameAndParamLabel(&m_SwSlider[m_iSliderBank][bSlider], strText);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetSliderValue(BYTE bSlider, CString *strText)
{
	CCriticalSectionAuto csa( &m_cs );

	strText->Empty();

	if (bSlider >= NUM_SLIDERS)
		return;

	if (m_pMidiLearnTarget)
		return;

	DWORD dwLen = 256;
	char szTxt[256] = { NULL };

	HRESULT hr = m_SwSlider[m_iSliderBank][bSlider].GetValueText(szTxt, &dwLen);

	if (FAILED(hr))
		strText->Empty();
	else
	{
		Char2TCHAR(strText->GetBufferSetLength(256), szTxt, 256);
		strText->ReleaseBuffer();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::SupportsDynamicMappings()
{
	return (m_pSonarParamMapping != NULL);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetUseDynamicMappings()
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return false;

	return m_bUseDynamicMappings;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetUseDynamicMappings(bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return;

	if (m_bUseDynamicMappings != b)
	{
		m_bUseDynamicMappings = b;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetLockDynamicMappings()
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return false;

	BOOL bLock = false;

	HRESULT hr = m_pSonarParamMapping->GetMapContextLock(m_dwSurfaceId, &bLock);

	if (FAILED(hr))
		return false;

	return (bLock != FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetLockDynamicMappings(bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return;

	if (GetLockDynamicMappings() != b)
	{
		m_pSonarParamMapping->SetMapContextLock(m_dwSurfaceId, b ? TRUE : FALSE);

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetLearnDynamicMappings()
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return false;

	BOOL bLearn;

	HRESULT hr = m_pSonarParamMapping->GetActLearnEnable(&bLearn);

	if (FAILED(hr))
		return false;

	return (bLearn != FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetLearnDynamicMappings(bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pSonarParamMapping)
		return;

	m_pSonarParamMapping->EnableACTLearnMode(b ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetDynamicMappingName(CString *strName)
{
	CCriticalSectionAuto csa( &m_cs );

	strName->Empty();

	if (!m_pSonarParamMapping)
		return;

	if (!m_bUseDynamicMappings ||
		(m_bACTFollowsContext && m_uiContext != UIC_PLUGIN))
	{
		*strName = m_strStripParameters;
		return;
	}

	DWORD dwLen = 256;
	char szTxt[256] = { NULL };

	HRESULT hr = m_pSonarParamMapping->GetMapName(m_dwSurfaceId,
													szTxt,
													&dwLen);
	
	if (FAILED(hr))
		strName->Empty();
	else
	{
		Char2TCHAR(strName->GetBufferSetLength(256), szTxt, 256);
		strName->ReleaseBuffer();
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetUpdateCount()
{
	return m_dwUpdateCount;
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetButtonACTMode(int iBank, VirtualButton bButton)
{
	CCriticalSectionAuto csa( &m_cs );

	if (bButton >= NUM_VIRTUAL_BUTTONS)
		return false;

	if (!m_bUseDynamicMappings || m_bButtonExcludeACT[iBank][bButton])
		return false;

	if (!m_bACTFollowsContext)
		return true;

	return (m_uiContext == UIC_PLUGIN);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetRotariesACTMode(int iBank)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_bUseDynamicMappings || m_bExcludeRotariesACT[iBank])
		return false;

	if (!m_bACTFollowsContext)
		return true;

	return (m_uiContext == UIC_PLUGIN);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetSlidersACTMode(int iBank)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_bUseDynamicMappings || m_bExcludeSlidersACT[iBank])
		return false;

	if (!m_bACTFollowsContext)
		return true;

	return (m_uiContext == UIC_PLUGIN);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetExcludeRotariesACT(int iBank)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return false;

	return m_bExcludeRotariesACT[iBank];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetExcludeRotariesACT(int iBank, bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (m_bExcludeRotariesACT[iBank] != b)
	{
		m_bExcludeRotariesACT[iBank] = b;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetExcludeSlidersACT(int iBank)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return false;

	return m_bExcludeSlidersACT[iBank];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetExcludeSlidersACT(int iBank, bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (iBank < 0 || iBank >= NUM_BANKS)
		return;

	if (m_bExcludeSlidersACT[iBank] != b)
	{
		m_bExcludeSlidersACT[iBank] = b;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

AssignmentMode CACTController::GetRotariesMode()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_eRotariesMode;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetRotariesMode(AssignmentMode eMode)
{
	CCriticalSectionAuto csa( &m_cs );

	if (m_eRotariesMode != eMode)
	{
		m_eRotariesMode = eMode;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetSelectHighlightsTrack()
{
	return m_bSelectHighlightsTrack;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetSelectHighlightsTrack(bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	m_bSelectHighlightsTrack = b;
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetACTFollowsContext()
{
	return m_bACTFollowsContext;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetACTFollowsContext(bool b)
{
	CCriticalSectionAuto csa( &m_cs );

	if (m_bACTFollowsContext != b)
	{
		m_bACTFollowsContext = b;

		UpdateBindings();
	}
}


CMidiBinding::MessageInterpretation CACTController::GetRotaryMessageInterpretation( int n )
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return CMidiBinding::MI_Literal;

	CCriticalSectionAuto csa( &m_cs );

	return m_cMidiKnob[n].m_eMessageInterpretation;
}

CMidiBinding::MessageInterpretation CACTController::GetSliderMessageInterpretation( int n )
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return CMidiBinding::MI_Literal;
	CCriticalSectionAuto csa( &m_cs );

	return m_cMidiSlider[n].m_eMessageInterpretation;
}


void CACTController::EndMidiLearn()
{
	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::EndMidiLearn()\n");

	if ( m_pMidiLearnTarget && CMidiBinding::MI_Increment == m_pMidiLearnTarget->m_eMessageInterpretation && !m_setLastReceivedLearnValue.empty() )
	{
		// the bytes are already sorted in the set
		BYTE byLo = *m_setLastReceivedLearnValue.begin();
		BYTE byHi = *m_setLastReceivedLearnValue.rbegin();
		BYTE byMedian = (byLo + byHi) / 2;

		m_pMidiLearnTarget->m_dwHinge = byMedian;
	}

	m_pMidiLearnTarget = NULL;
	m_setLastReceivedLearnValue.clear();
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::MidiLearnRotary(int n)
{
	if ( !CHECK_RANGE( n, NUM_KNOBS ) )
		return;

	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::MidiLearnRotary(%d)\n", n);

	if ( m_pMidiLearnTarget )
	{
		EndMidiLearn();
	}
	else
	{
		m_pMidiLearnTarget = &m_cMidiKnob[n];
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::MidiLearnSlider(int n)
{
	if ( !CHECK_RANGE( n, NUM_SLIDERS ) )
		return;

	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::MidiLearnSlider(%d)\n", n);

	if ( m_pMidiLearnTarget )
	{
		EndMidiLearn();
	}
	else
	{
		m_pMidiLearnTarget = &m_cMidiSlider[n];
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::MidiLearnButton(int n)
{
	if ( !CHECK_RANGE( n, NUM_BUTTONS ) )
		return;

	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::MidiLearnButton(%d)\n", n);

	if (n >= NUM_BUTTONS)
		n -= NUM_BUTTONS;

	if ( m_pMidiLearnTarget )
	{
		EndMidiLearn();
	}
	else
	{
		m_pMidiLearnTarget = &m_cMidiButton[n];
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::MidiLearnShift()
{
	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::MidiLearnShift()\n");

	m_pMidiLearnTarget = (m_pMidiLearnTarget) ? NULL : &m_cMidiModifierDown;
}

/////////////////////////////////////////////////////////////////////////////

int CACTController::GetRotaryBank()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_iRotaryBank;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetRotaryBank(int iBank)
{
	if ( !CHECK_RANGE( iBank, NUM_BANKS))
		return;

	CCriticalSectionAuto csa( &m_cs );

	if (m_iRotaryBank != iBank)
	{
		m_iRotaryBank = iBank;

		onCaptureModeChange();
		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

int CACTController::GetSliderBank()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_iSliderBank;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetSliderBank(int iBank)
{
	if ( !CHECK_RANGE( iBank, NUM_BANKS))
		return;
	CCriticalSectionAuto csa( &m_cs );

	if (m_iSliderBank != iBank)
	{
		m_iSliderBank = iBank;

		onCaptureModeChange();
		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

int CACTController::GetButtonBank()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_iButtonBank;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetButtonBank(int iBank)
{
	if ( !CHECK_RANGE( iBank, NUM_BANKS))
		return;
	CCriticalSectionAuto csa( &m_cs );

	if (m_iButtonBank != iBank)
	{
		m_iButtonBank = iBank;

		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetComments(CString *strComments)
{
	*strComments = m_strComments;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetComments(CString strComments)
{
	m_strComments = strComments;
}

/////////////////////////////////////////////////////////////////////////////


void CACTController::XFerInitShortMsgs( std::vector<DWORD>* pv, bool bFromUI )
{
	if ( bFromUI )
	{
		m_vdwInitShortMsg = *pv;
		m_bInitSent = false;
	}
	else
		*pv = m_vdwInitShortMsg;
}


void CACTController::XferInitLongMsgs( std::vector<BYTE>* pv, bool bFromUI )
{
	if ( bFromUI )
	{
		m_vdwInitSysexMsg.clear();
		if ( !pv->empty() )
		{
			m_vdwInitSysexMsg.push_back( 0xF0 );
			for ( size_t ix = 0 ; ix < pv->size(); ix++ )
				m_vdwInitSysexMsg.push_back( (*pv)[ix] );
			m_vdwInitSysexMsg.push_back( 0xF7 );
		}
		m_bInitSent = false;
	}
	else
	{
		pv->clear();
		if ( m_vdwInitSysexMsg.size() > 2 )
		{
			for ( size_t ix = 1 ; ix < m_vdwInitSysexMsg.size() - 1; ix++ )
				pv->push_back( m_vdwInitSysexMsg[ix] );
		}
	}
}
