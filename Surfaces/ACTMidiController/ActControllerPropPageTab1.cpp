// ACTControllerPropPageTab1.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "ACTControllerPropPageTab1.h"
#include "StringCruncher.h"
#include "EditLabel.h"
#include "CellMidiPropsDlg.h"
#include "utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TID_MIDI_LEARN (1)

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab1 dialog


CACTControllerPropPageTab1::CACTControllerPropPageTab1(CWnd* pParent /*=NULL*/)
	: CACTControllerPropPageTab(CACTControllerPropPageTab1::IDD, pParent)
{
	m_iRotaryBank = 0;
	m_iSliderBank = 0;
	m_iButtonBank = 0;

	m_pBankLabel[0] = &m_cRotaryBankLabel;
	m_pBankLabel[1] = &m_cSliderBankLabel;
	m_pBankLabel[2] = &m_cButtonBankLabel;

	m_pRotaryLabel[0] = &m_cR1Label;
	m_pRotaryLabel[1] = &m_cR2Label;
	m_pRotaryLabel[2] = &m_cR3Label;
	m_pRotaryLabel[3] = &m_cR4Label;
	m_pRotaryLabel[4] = &m_cR5Label;
	m_pRotaryLabel[5] = &m_cR6Label;
	m_pRotaryLabel[6] = &m_cR7Label;
	m_pRotaryLabel[7] = &m_cR8Label;

	m_pRotaryName[0] = &m_cR1Name;
	m_pRotaryName[1] = &m_cR2Name;
	m_pRotaryName[2] = &m_cR3Name;
	m_pRotaryName[3] = &m_cR4Name;
	m_pRotaryName[4] = &m_cR5Name;
	m_pRotaryName[5] = &m_cR6Name;
	m_pRotaryName[6] = &m_cR7Name;
	m_pRotaryName[7] = &m_cR8Name;

	m_pRotaryValue[0] = &m_cR1Value;
	m_pRotaryValue[1] = &m_cR2Value;
	m_pRotaryValue[2] = &m_cR3Value;
	m_pRotaryValue[3] = &m_cR4Value;
	m_pRotaryValue[4] = &m_cR5Value;
	m_pRotaryValue[5] = &m_cR6Value;
	m_pRotaryValue[6] = &m_cR7Value;
	m_pRotaryValue[7] = &m_cR8Value;

	m_pSliderLabel[0] = &m_cS1Label;
	m_pSliderLabel[1] = &m_cS2Label;
	m_pSliderLabel[2] = &m_cS3Label;
	m_pSliderLabel[3] = &m_cS4Label;
	m_pSliderLabel[4] = &m_cS5Label;
	m_pSliderLabel[5] = &m_cS6Label;
	m_pSliderLabel[6] = &m_cS7Label;
	m_pSliderLabel[7] = &m_cS8Label;

	m_pSliderName[0] = &m_cS1Name;
	m_pSliderName[1] = &m_cS2Name;
	m_pSliderName[2] = &m_cS3Name;
	m_pSliderName[3] = &m_cS4Name;
	m_pSliderName[4] = &m_cS5Name;
	m_pSliderName[5] = &m_cS6Name;
	m_pSliderName[6] = &m_cS7Name;
	m_pSliderName[7] = &m_cS8Name;

	m_pSliderValue[0] = &m_cS1Value;
	m_pSliderValue[1] = &m_cS2Value;
	m_pSliderValue[2] = &m_cS3Value;
	m_pSliderValue[3] = &m_cS4Value;
	m_pSliderValue[4] = &m_cS5Value;
	m_pSliderValue[5] = &m_cS6Value;
	m_pSliderValue[6] = &m_cS7Value;
	m_pSliderValue[7] = &m_cS8Value;

	m_pButtonLabel[0] = &m_cB1Label;
	m_pButtonLabel[1] = &m_cB2Label;
	m_pButtonLabel[2] = &m_cB3Label;
	m_pButtonLabel[3] = &m_cB4Label;
	m_pButtonLabel[4] = &m_cB5Label;
	m_pButtonLabel[5] = &m_cB6Label;
	m_pButtonLabel[6] = &m_cB7Label;
	m_pButtonLabel[7] = &m_cB8Label;
	m_pButtonLabel[8] = &m_cSB1Label;
	m_pButtonLabel[9] = &m_cSB2Label;
	m_pButtonLabel[10] = &m_cSB3Label;
	m_pButtonLabel[11] = &m_cSB4Label;
	m_pButtonLabel[12] = &m_cSB5Label;
	m_pButtonLabel[13] = &m_cSB6Label;
	m_pButtonLabel[14] = &m_cSB7Label;
	m_pButtonLabel[15] = &m_cSB8Label;

	m_pButtonName[0] = &m_cB1Name;
	m_pButtonName[1] = &m_cB2Name;
	m_pButtonName[2] = &m_cB3Name;
	m_pButtonName[3] = &m_cB4Name;
	m_pButtonName[4] = &m_cB5Name;
	m_pButtonName[5] = &m_cB6Name;
	m_pButtonName[6] = &m_cB7Name;
	m_pButtonName[7] = &m_cB8Name;
	m_pButtonName[8] = &m_cSB1Name;
	m_pButtonName[9] = &m_cSB2Name;
	m_pButtonName[10] = &m_cSB3Name;
	m_pButtonName[11] = &m_cSB4Name;
	m_pButtonName[12] = &m_cSB5Name;
	m_pButtonName[13] = &m_cSB6Name;
	m_pButtonName[14] = &m_cSB7Name;
	m_pButtonName[15] = &m_cSB8Name;

	m_pButtonValue[0] = &m_cB1Value;
	m_pButtonValue[1] = &m_cB2Value;
	m_pButtonValue[2] = &m_cB3Value;
	m_pButtonValue[3] = &m_cB4Value;
	m_pButtonValue[4] = &m_cB5Value;
	m_pButtonValue[5] = &m_cB6Value;
	m_pButtonValue[6] = &m_cB7Value;
	m_pButtonValue[7] = &m_cB8Value;
	m_pButtonValue[8] = &m_cSB1Value;
	m_pButtonValue[9] = &m_cSB2Value;
	m_pButtonValue[10] = &m_cSB3Value;
	m_pButtonValue[11] = &m_cSB4Value;
	m_pButtonValue[12] = &m_cSB5Value;
	m_pButtonValue[13] = &m_cSB6Value;
	m_pButtonValue[14] = &m_cSB7Value;
	m_pButtonValue[15] = &m_cSB8Value;

	m_eButtonIndex[0] = BUTTON_1;
	m_eButtonIndex[1] = BUTTON_2;
	m_eButtonIndex[2] = BUTTON_3;
	m_eButtonIndex[3] = BUTTON_4;
	m_eButtonIndex[4] = BUTTON_5;
	m_eButtonIndex[5] = BUTTON_6;
	m_eButtonIndex[6] = BUTTON_7;
	m_eButtonIndex[7] = BUTTON_8;
	m_eButtonIndex[8] = BUTTON_SHIFT_1;
	m_eButtonIndex[9] = BUTTON_SHIFT_2;
	m_eButtonIndex[10] = BUTTON_SHIFT_3;
	m_eButtonIndex[11] = BUTTON_SHIFT_4;
	m_eButtonIndex[12] = BUTTON_SHIFT_5;
	m_eButtonIndex[13] = BUTTON_SHIFT_6;
	m_eButtonIndex[14] = BUTTON_SHIFT_7;
	m_eButtonIndex[15] = BUTTON_SHIFT_8;

	m_dwUpdateCount = 0;

	//{{AFX_DATA_INIT(CACTControllerPropPageTab1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CACTControllerPropPageTab1::DoDataExchange(CDataExchange* pDX)
{
	TRACE("CACTControllerPropPageTab1::DoDataExchange()\n");

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CACTControllerPropPageTab1)
	DDX_Control(pDX, IDC_ACTIVE_SLIDER_BANK, m_cActiveSliderBank);
	DDX_Control(pDX, IDC_ACTIVE_ROTARY_BANK, m_cActiveRotaryBank);
	DDX_Control(pDX, IDC_ACTIVE_BUTTON_BANK, m_cActiveButtonBank);
	DDX_Control(pDX, IDC_BUTTON_BANK_LABEL, m_cButtonBankLabel);
	DDX_Control(pDX, IDC_SLIDER_BANK_LABEL, m_cSliderBankLabel);
	DDX_Control(pDX, IDC_ROTARY_BANK_LABEL, m_cRotaryBankLabel);
	DDX_Control(pDX, IDC_SB8_LABEL, m_cSB8Label);
	DDX_Control(pDX, IDC_SB7_LABEL, m_cSB7Label);
	DDX_Control(pDX, IDC_SB6_LABEL, m_cSB6Label);
	DDX_Control(pDX, IDC_SB5_LABEL, m_cSB5Label);
	DDX_Control(pDX, IDC_SB4_LABEL, m_cSB4Label);
	DDX_Control(pDX, IDC_SB3_LABEL, m_cSB3Label);
	DDX_Control(pDX, IDC_SB2_LABEL, m_cSB2Label);
	DDX_Control(pDX, IDC_SB1_LABEL, m_cSB1Label);
	DDX_Control(pDX, IDC_S8_LABEL, m_cS8Label);
	DDX_Control(pDX, IDC_S7_LABEL, m_cS7Label);
	DDX_Control(pDX, IDC_S6_LABEL, m_cS6Label);
	DDX_Control(pDX, IDC_S5_LABEL, m_cS5Label);
	DDX_Control(pDX, IDC_S4_LABEL, m_cS4Label);
	DDX_Control(pDX, IDC_S3_LABEL, m_cS3Label);
	DDX_Control(pDX, IDC_S2_LABEL, m_cS2Label);
	DDX_Control(pDX, IDC_S1_LABEL, m_cS1Label);
	DDX_Control(pDX, IDC_R8_LABEL, m_cR8Label);
	DDX_Control(pDX, IDC_R7_LABEL, m_cR7Label);
	DDX_Control(pDX, IDC_R6_LABEL, m_cR6Label);
	DDX_Control(pDX, IDC_R5_LABEL, m_cR5Label);
	DDX_Control(pDX, IDC_R4_LABEL, m_cR4Label);
	DDX_Control(pDX, IDC_R3_LABEL, m_cR3Label);
	DDX_Control(pDX, IDC_R2_LABEL, m_cR2Label);
	DDX_Control(pDX, IDC_R1_LABEL, m_cR1Label);
	DDX_Control(pDX, IDC_B8_LABEL, m_cB8Label);
	DDX_Control(pDX, IDC_B7_LABEL, m_cB7Label);
	DDX_Control(pDX, IDC_B6_LABEL, m_cB6Label);
	DDX_Control(pDX, IDC_B5_LABEL, m_cB5Label);
	DDX_Control(pDX, IDC_B4_LABEL, m_cB4Label);
	DDX_Control(pDX, IDC_B3_LABEL, m_cB3Label);
	DDX_Control(pDX, IDC_B2_LABEL, m_cB2Label);
	DDX_Control(pDX, IDC_B1_LABEL, m_cB1Label);
	DDX_Control(pDX, IDC_SB8_VALUE, m_cSB8Value);
	DDX_Control(pDX, IDC_SB8_NAME, m_cSB8Name);
	DDX_Control(pDX, IDC_SB7_VALUE, m_cSB7Value);
	DDX_Control(pDX, IDC_SB7_NAME, m_cSB7Name);
	DDX_Control(pDX, IDC_SB6_VALUE, m_cSB6Value);
	DDX_Control(pDX, IDC_SB6_NAME, m_cSB6Name);
	DDX_Control(pDX, IDC_SB5_VALUE, m_cSB5Value);
	DDX_Control(pDX, IDC_SB5_NAME, m_cSB5Name);
	DDX_Control(pDX, IDC_SB4_VALUE, m_cSB4Value);
	DDX_Control(pDX, IDC_SB4_NAME, m_cSB4Name);
	DDX_Control(pDX, IDC_SB3_VALUE, m_cSB3Value);
	DDX_Control(pDX, IDC_SB3_NAME, m_cSB3Name);
	DDX_Control(pDX, IDC_SB2_VALUE, m_cSB2Value);
	DDX_Control(pDX, IDC_SB2_NAME, m_cSB2Name);
	DDX_Control(pDX, IDC_SB1_VALUE, m_cSB1Value);
	DDX_Control(pDX, IDC_SB1_NAME, m_cSB1Name);
	DDX_Control(pDX, IDC_B8_VALUE, m_cB8Value);
	DDX_Control(pDX, IDC_B8_NAME, m_cB8Name);
	DDX_Control(pDX, IDC_B7_VALUE, m_cB7Value);
	DDX_Control(pDX, IDC_B7_NAME, m_cB7Name);
	DDX_Control(pDX, IDC_B6_VALUE, m_cB6Value);
	DDX_Control(pDX, IDC_B6_NAME, m_cB6Name);
	DDX_Control(pDX, IDC_B5_VALUE, m_cB5Value);
	DDX_Control(pDX, IDC_B5_NAME, m_cB5Name);
	DDX_Control(pDX, IDC_B4_VALUE, m_cB4Value);
	DDX_Control(pDX, IDC_B4_NAME, m_cB4Name);
	DDX_Control(pDX, IDC_B3_VALUE, m_cB3Value);
	DDX_Control(pDX, IDC_B3_NAME, m_cB3Name);
	DDX_Control(pDX, IDC_B2_VALUE, m_cB2Value);
	DDX_Control(pDX, IDC_B2_NAME, m_cB2Name);
	DDX_Control(pDX, IDC_B1_VALUE, m_cB1Value);
	DDX_Control(pDX, IDC_B1_NAME, m_cB1Name);
	DDX_Control(pDX, IDC_S8_VALUE, m_cS8Value);
	DDX_Control(pDX, IDC_S8_NAME, m_cS8Name);
	DDX_Control(pDX, IDC_S7_VALUE, m_cS7Value);
	DDX_Control(pDX, IDC_S7_NAME, m_cS7Name);
	DDX_Control(pDX, IDC_S6_VALUE, m_cS6Value);
	DDX_Control(pDX, IDC_S6_NAME, m_cS6Name);
	DDX_Control(pDX, IDC_S5_VALUE, m_cS5Value);
	DDX_Control(pDX, IDC_S5_NAME, m_cS5Name);
	DDX_Control(pDX, IDC_S4_VALUE, m_cS4Value);
	DDX_Control(pDX, IDC_S4_NAME, m_cS4Name);
	DDX_Control(pDX, IDC_S3_VALUE, m_cS3Value);
	DDX_Control(pDX, IDC_S3_NAME, m_cS3Name);
	DDX_Control(pDX, IDC_S2_VALUE, m_cS2Value);
	DDX_Control(pDX, IDC_S2_NAME, m_cS2Name);
	DDX_Control(pDX, IDC_S1_VALUE, m_cS1Value);
	DDX_Control(pDX, IDC_S1_NAME, m_cS1Name);
	DDX_Control(pDX, IDC_R8_VALUE, m_cR8Value);
	DDX_Control(pDX, IDC_R7_VALUE, m_cR7Value);
	DDX_Control(pDX, IDC_R6_VALUE, m_cR6Value);
	DDX_Control(pDX, IDC_R5_VALUE, m_cR5Value);
	DDX_Control(pDX, IDC_R4_VALUE, m_cR4Value);
	DDX_Control(pDX, IDC_R3_VALUE, m_cR3Value);
	DDX_Control(pDX, IDC_R2_VALUE, m_cR2Value);
	DDX_Control(pDX, IDC_R1_VALUE, m_cR1Value);
	DDX_Control(pDX, IDC_R8_NAME, m_cR8Name);
	DDX_Control(pDX, IDC_R7_NAME, m_cR7Name);
	DDX_Control(pDX, IDC_R6_NAME, m_cR6Name);
	DDX_Control(pDX, IDC_R5_NAME, m_cR5Name);
	DDX_Control(pDX, IDC_R4_NAME, m_cR4Name);
	DDX_Control(pDX, IDC_R3_NAME, m_cR3Name);
	DDX_Control(pDX, IDC_R2_NAME, m_cR2Name);
	DDX_Control(pDX, IDC_R1_NAME, m_cR1Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CACTControllerPropPageTab1, CACTControllerPropPageTab)
	//{{AFX_MSG_MAP(CACTControllerPropPageTab1)
	ON_CBN_SELCHANGE(IDC_ACTIVE_ROTARY_BANK, OnSelchangeActiveRotaryBank)
	ON_CBN_SELCHANGE(IDC_ACTIVE_SLIDER_BANK, OnSelchangeActiveSliderBank)
	ON_CBN_SELCHANGE(IDC_ACTIVE_BUTTON_BANK, OnSelchangeActiveButtonBank)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::LoadAllFields()
{
	TRACE("CACTControllerPropPageTab1::LoadAllFields()\n");

	UpdateBankCombos(true);
	UpdateControllerTextBoxes(true);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::Select()
{
	UpdateBankCombos();
	UpdateControllerTextBoxes();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::Refresh()
{
	UpdateBankCombos();
	UpdateControllerTextBoxes();
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::UpdateBankCombos(bool bForce)
{
	if (m_pSurface == NULL)
	{
		TRACE("CACTControllerPropPageTab1::UpdateBankCombos(%d) m_pSurface is NULL\n", bForce);
		return;
	}

	int iBank;

	iBank = m_pSurface->GetRotaryBank();
	if (bForce || m_iRotaryBank != iBank)
	{
		m_cActiveRotaryBank.SetCurSel(iBank);
		m_iRotaryBank = iBank;
	}

	iBank = m_pSurface->GetSliderBank();
	if (bForce || m_iSliderBank != iBank)
	{
		m_cActiveSliderBank.SetCurSel(iBank);
		m_iSliderBank = iBank;
	}

	iBank = m_pSurface->GetButtonBank();
	if (bForce || m_iButtonBank != iBank)
	{
		m_cActiveButtonBank.SetCurSel(iBank);
		m_iButtonBank = iBank;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::UpdateControllerTextBoxes(bool bForce)
{
	CString strText;
	int n;

	if (m_pSurface == NULL)
	{
		TRACE("CACTControllerPropPageTab1::UpdateControllerTextBoxes(%d) m_pSurface is NULL\n", bForce);
		return;
	}

	DWORD dwUpdateCount = m_pSurface->GetUpdateCount();
	if (bForce || m_dwUpdateCount != dwUpdateCount)
	{
		m_dwUpdateCount = dwUpdateCount;

		bool bACTEnabled = m_pSurface->GetUseDynamicMappings();
		COLORREF crBkgndNrm = RGB(204, 227, 193);
		COLORREF crBkgndACT = ACTBgColour();

		COLORREF crBkgnd;

		crBkgnd = (m_pSurface->GetRotariesACTMode(m_pSurface->GetRotaryBank())) ? crBkgndACT : crBkgndNrm;
		for (n = 0; n < NUM_KNOBS; n++)
		{
			m_pRotaryName[n]->SetBkColor(crBkgnd);
			m_pRotaryValue[n]->SetBkColor(crBkgnd);
		}

		crBkgnd = (m_pSurface->GetSlidersACTMode(m_pSurface->GetSliderBank())) ? crBkgndACT : crBkgndNrm;
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			m_pSliderName[n]->SetBkColor(crBkgnd);
			m_pSliderValue[n]->SetBkColor(crBkgnd);
		}

		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			crBkgnd = (m_pSurface->GetButtonACTMode(m_pSurface->GetButtonBank(), m_eButtonIndex[n])) ? crBkgndACT : crBkgndNrm;

			m_pButtonName[n]->SetBkColor(crBkgnd);
			m_pButtonValue[n]->SetBkColor(crBkgnd);
		}
	}

	CStringCruncher cruncher;
	char szCrunched[2048] = { 0 }; // Make this the same length as szTxt, as for Japanese it does a straight copy
	CString cszCrunch;
	char szTxt[2048] = { 0 };
	int width = 16;

	for (n = 0; n < NUM_KNOBS; n++)
	{
		m_pSurface->GetRotaryLabel(n, &strText);
		if (bForce || m_strRotaryLabel[n] != strText)
		{			
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			m_strRotaryLabel[n] = strText;
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pRotaryLabel[n]->SetText(cszCrunch);
			
		}

		m_pSurface->GetRotaryName(n, &strText);
		if (bForce || m_strRotaryName[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			m_strRotaryName[n] = strText;
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pRotaryName[n]->SetText(cszCrunch);

		}

		m_pSurface->GetRotaryValue(n, &strText);
		if (bForce || m_strRotaryValue[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pRotaryValue[n]->SetText(cszCrunch);
			m_strRotaryValue[n] = strText;
		}
	}

	for (n = 0; n < NUM_SLIDERS; n++)
	{
		m_pSurface->GetSliderLabel(n, &strText);
		if (bForce || m_strSliderLabel[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pSliderLabel[n]->SetText(cszCrunch);
			m_strSliderLabel[n] = strText;
		}

		m_pSurface->GetSliderName(n, &strText);
		if (bForce || m_strSliderName[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pSliderName[n]->SetText(cszCrunch);
			m_strSliderName[n] = strText;
		}

		m_pSurface->GetSliderValue(n, &strText);
		if (bForce || m_strSliderValue[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pSliderValue[n]->SetText(cszCrunch);
			m_strSliderValue[n] = strText;
		}
	}

	for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
	{
		m_pSurface->GetButtonLabel(m_eButtonIndex[n], &strText);
		if (bForce || m_strButtonLabel[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pButtonLabel[n]->SetText(cszCrunch);
			m_strButtonLabel[n] = strText;
		}

		m_pSurface->GetButtonName(m_eButtonIndex[n], &strText);
		if (bForce || m_strButtonName[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pButtonName[n]->SetText(cszCrunch);
			m_strButtonName[n] = strText;
		}

		m_pSurface->GetButtonValue(m_eButtonIndex[n], &strText);
		if (bForce || m_strButtonValue[n] != strText)
		{
			TCHAR2Char(szTxt, strText, 2048);
			cruncher.CrunchString(szTxt, szCrunched, width, 0);
			Char2TCHAR(cszCrunch.GetBufferSetLength(2048), szCrunched, 2048);
			cszCrunch.ReleaseBuffer();
			m_pButtonValue[n]->SetText(cszCrunch);
			m_strButtonValue[n] = strText;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab1 message handlers

BOOL CACTControllerPropPageTab1::OnInitDialog() 
{
	BOOL bRet = CACTControllerPropPageTab::OnInitDialog();

	COLORREF crBkgnd = RGB(106, 114, 129);
	COLORREF crText  = RGB(255, 255, 255);
	int n;

	CString strFont;
	strFont.LoadString( IDS_FONT_NAME );

	for (n = 0; n < NUM_KNOBS; n++)
	{
		m_pRotaryLabel[n]->SetBkColor(crBkgnd);
		m_pRotaryLabel[n]->SetTextColor(crText);
		m_pRotaryLabel[n]->SetFontBold(TRUE);
		m_pRotaryLabel[n]->SetLink(TRUE, TRUE);

		m_pRotaryName[n]->SetLink(TRUE, TRUE);
		m_pRotaryName[n]->SetFontName( strFont );
		m_pRotaryValue[n]->SetLink(TRUE, TRUE);
	}

	for (n = 0; n < NUM_SLIDERS; n++)
	{
		m_pSliderLabel[n]->SetBkColor(crBkgnd);
		m_pSliderLabel[n]->SetTextColor(crText);
		m_pSliderLabel[n]->SetFontBold(TRUE);
		m_pSliderLabel[n]->SetLink(TRUE, TRUE);

		m_pSliderName[n]->SetLink(TRUE, TRUE);
		m_pSliderName[n]->SetFontName( strFont );
		m_pSliderValue[n]->SetLink(TRUE, TRUE);
	}


	for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
	{
		m_pButtonLabel[n]->SetBkColor(crBkgnd);
		m_pButtonLabel[n]->SetTextColor(crText);
		m_pButtonLabel[n]->SetFontBold(TRUE);
		m_pButtonLabel[n]->SetLink(TRUE, TRUE);
		m_pButtonName[n]->SetFontName( strFont );
		m_pButtonValue[n]->SetLink(TRUE, TRUE);
	}


	CString strBank(MAKEINTRESOURCE(IDS_BANK));

	for (n = 0; n < 3; n++)
	{
		m_pBankLabel[n]->SetBkColor(crBkgnd);
		m_pBankLabel[n]->SetTextColor(crText);
		m_pBankLabel[n]->SetFontBold(TRUE);
		m_pBankLabel[n]->SetText(strBank);
	}

	m_cActiveRotaryBank.ResetContent();
	m_cActiveSliderBank.ResetContent();
	m_cActiveButtonBank.ResetContent();

	for (n = 0; n < NUM_BANKS; n++)
	{
		CString str;
		int idx;

		str.Format(_T("%d"), n + 1);

		idx = m_cActiveRotaryBank.AddString(str);
		m_cActiveRotaryBank.SetItemData(idx, n);

		idx = m_cActiveSliderBank.AddString(str);
		m_cActiveSliderBank.SetItemData(idx, n);

		idx = m_cActiveButtonBank.AddString(str);
		m_cActiveButtonBank.SetItemData(idx, n);
	}

	m_cActiveRotaryBank.SetCurSel(0);
	m_cActiveSliderBank.SetCurSel(0);
	m_cActiveButtonBank.SetCurSel(0);

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CACTControllerPropPageTab1::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
//	TRACE("CACTControllerPropPageTab1(%d)\n", wParam);

	NMHDR* pnmhdr = (NMHDR*)lParam;
	
	bool bWantProps = 0 != (::GetAsyncKeyState( VK_CONTROL ) & 0x8000);
	TRACE( "code: %d\n", pnmhdr->code );
	int ixCell = -1;

	bool bStartTimer = false;
	bool done = false;
	int n;

	for (n = 0; n < NUM_KNOBS; n++)
	{
		if (m_pRotaryName[n]->GetDlgCtrlID() == wParam ||
			m_pRotaryValue[n]->GetDlgCtrlID() == wParam)
		{
			if ( bWantProps )
				ixCell = n;
			else if ( m_pSurface )
			{
				m_pSurface->MidiLearnRotary(n);
				bStartTimer =  m_pSurface->GetRotaryMessageInterpretation(n) == CMidiBinding::MI_Increment;
			}
			done = true;
			break;
		}
	}

	if (!done)
	{
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			if (m_pSliderName[n]->GetDlgCtrlID() == wParam ||
				m_pSliderValue[n]->GetDlgCtrlID() == wParam)
			{
				if ( bWantProps )
					ixCell = n + NUM_KNOBS;
				else if ( m_pSurface )
				{
					m_pSurface->MidiLearnSlider(n);
					bStartTimer =  m_pSurface->GetSliderMessageInterpretation(n) == CMidiBinding::MI_Increment;
				}
				done = true;
				break;
			}
		}
	}

	if (!done)
	{
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			if (m_pButtonName[n]->GetDlgCtrlID() == wParam ||
				m_pButtonValue[n]->GetDlgCtrlID() == wParam)
			{
				if ( bWantProps )
					ixCell = n + NUM_KNOBS + NUM_SLIDERS;
				else if ( m_pSurface )
					m_pSurface->MidiLearnButton(n);
				done = true;
				break;
			}
		}
	}

	if (!done)
	{
		for (n = 0; n < NUM_KNOBS; n++)
		{
			if (m_pRotaryLabel[n]->GetDlgCtrlID() == wParam)
			{
				CEditLabel dlgEditLabel;
				if ( m_pSurface )
				{
					m_pSurface->GetRotaryLabel(n, &dlgEditLabel.m_strLabel);
					if (dlgEditLabel.DoModal() == IDOK)
						m_pSurface->SetRotaryLabel(n, dlgEditLabel.m_strLabel);
				}
				done = true;
				break;
			}
		}
	}

	if (!done)
	{
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			if (m_pSliderLabel[n]->GetDlgCtrlID() == wParam)
			{
				CEditLabel dlgEditLabel;
				if ( m_pSurface )
				{
					m_pSurface->GetSliderLabel(n, &dlgEditLabel.m_strLabel);
					if (dlgEditLabel.DoModal() == IDOK)
						m_pSurface->SetSliderLabel(n, dlgEditLabel.m_strLabel);
				}
				done = true;
				break;
			}
		}
	}

	if (!done)
	{
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			if (m_pButtonLabel[n]->GetDlgCtrlID() == wParam)
			{
				CEditLabel dlgEditLabel;
				if ( m_pSurface )
				{
					m_pSurface->GetButtonLabel(m_eButtonIndex[n], &dlgEditLabel.m_strLabel);
					if (dlgEditLabel.DoModal() == IDOK)
						m_pSurface->SetButtonLabel(m_eButtonIndex[n], dlgEditLabel.m_strLabel);
				}
				done = true;
				break;
			}
		}
	}

	if ( bStartTimer )
		SetTimer( TID_MIDI_LEARN, 4000, NULL );

	if ( bWantProps && ixCell != -1 )
	{
		// props dialog
		CCellMidiPropsDlg dlg( m_pSurface, ixCell );
		dlg.DoModal();
	}


	return CACTControllerPropPageTab::OnNotify(wParam, lParam, pResult);
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::OnSelchangeActiveRotaryBank() 
{
	int idx = m_cActiveRotaryBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iRotaryBank = m_cActiveRotaryBank.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetRotaryBank(int(m_iRotaryBank));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::OnSelchangeActiveSliderBank() 
{
	int idx = m_cActiveSliderBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iSliderBank = m_cActiveSliderBank.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetSliderBank(int(m_iSliderBank));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::OnSelchangeActiveButtonBank() 
{
	int idx = m_cActiveButtonBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iButtonBank = m_cActiveButtonBank.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetButtonBank(int(m_iButtonBank));
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab1::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == TID_MIDI_LEARN )
	{
		KillTimer( nIDEvent );
		if ( m_pSurface )
			m_pSurface->EndMidiLearn();
	}

	CACTControllerPropPageTab::OnTimer(nIDEvent);
}
