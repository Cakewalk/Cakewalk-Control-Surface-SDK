// CellMidiPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "CellMidiPropsDlg.h"
#include "utils.h"


// CCellMidiPropsDlg dialog

IMPLEMENT_DYNAMIC(CCellMidiPropsDlg, CDialog)

CCellMidiPropsDlg::CCellMidiPropsDlg(CACTController* pSurface, int ixCell, CWnd* pParent /*=NULL*/)
	: CDialog(CCellMidiPropsDlg::IDD, pParent),
	m_pSurface( pSurface ),
	m_ixCell( ixCell )
	, m_bUseAccel(FALSE)
	,m_ixInterpret(-1)
	,m_ixKnob(-1)
	,m_ixSlider(-1)
	,m_ixSwitch(-1)
	, m_dwHinge(0)
	, m_strStatus(_T(""))
	, m_strNumber(_T(""))
	, m_strSysex(_T(""))
{
	if ( m_ixCell < NUM_KNOBS )
		m_ixKnob = m_ixCell;
	else if ( m_ixCell < NUM_SLIDERS + NUM_KNOBS )
		m_ixSlider = m_ixCell - NUM_SLIDERS;
	else
	{
		m_ixSwitch = m_ixCell - (NUM_SLIDERS + NUM_KNOBS);
		m_ixSwitch = m_ixSwitch % NUM_BUTTONS;
	}
}

CCellMidiPropsDlg::~CCellMidiPropsDlg()
{
}

void CCellMidiPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CENTER_VAL, m_cHinge);
	DDX_Control(pDX, IDC_USE_ACCEL, m_cUseAccel);
	DDX_Control(pDX, IDC_STATUS, m_cStatus);
	DDX_Control(pDX, IDC_NUM, m_cNum);
	DDX_Control(pDX, IDC_SYSEX_STRING, m_cSysex);
	DDX_Check(pDX, IDC_USE_ACCEL, m_bUseAccel);
	DDX_Radio(pDX, IDC_RAD_INTERPRET, m_ixInterpret );

	enableControls();
	DDX_Text(pDX, IDC_CENTER_VAL, m_dwHinge);
	DDX_Text(pDX, IDC_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_NUM, m_strNumber);
	DDX_Text(pDX, IDC_SYSEX_STRING, m_strSysex);
}


BEGIN_MESSAGE_MAP(CCellMidiPropsDlg, CDialog)
	ON_BN_CLICKED(IDC_USE_ACCEL, &CCellMidiPropsDlg::OnBnClickedUseAccel)
	ON_BN_CLICKED(IDC_RAD_INTERPRET, &CCellMidiPropsDlg::OnBnClickedRadInterpret)
	ON_BN_CLICKED(IDC_RAD1_INTERPRET, &CCellMidiPropsDlg::OnBnClickedRad1Interpret)
END_MESSAGE_MAP()


void CCellMidiPropsDlg::enableControls()
{
	m_cUseAccel.EnableWindow( m_ixInterpret == 1 && m_ixSwitch == -1 );
	m_cHinge.EnableWindow( m_ixInterpret == 1 && m_ixSwitch == -1 );
	m_cSysex.EnableWindow( m_ixSwitch != -1 );
//	m_cAbsolute.EnableWindow( m_ixSwitch != -1 );
//	m_cIncrement.EnableWindow( m_ixSwitch != -1 );
}



// CCellMidiPropsDlg message handlers

BOOL CCellMidiPropsDlg::OnInitDialog()
{
	m_ixInterpret = 0;
	if ( m_ixKnob != -1 )
	{
		CMidiBinding::MessageInterpretation mi = m_pSurface->GetRotaryMessageInterpretation( m_ixKnob );
		if ( CMidiBinding::MI_Increment == mi )
		{
			m_ixInterpret = 1;
		}
		m_dwHinge = m_pSurface->GetRotaryIncrementHingeValue( m_ixKnob );
		m_bUseAccel = m_pSurface->GetRotaryIncrementUseAccel( m_ixKnob );
		WORD wStatus = m_pSurface->GetRotaryMessage( m_ixKnob );
		m_strStatus.Format( _T("%02x"), wStatus & 0xff );
		m_strNumber.Format( _T("%02x"), (wStatus >> 8) & 0xff);
	}
	else if ( m_ixSlider != -1 )
	{
		CMidiBinding::MessageInterpretation mi = m_pSurface->GetSliderMessageInterpretation( m_ixSlider );
		if ( CMidiBinding::MI_Increment == mi )
		{
			m_ixInterpret = 1;
		}
		m_dwHinge = m_pSurface->GetSliderIncrementHingeValue( m_ixSlider );
		m_bUseAccel = m_pSurface->GetSliderIncrementUseAccel( m_ixSlider );
		WORD wStatus = m_pSurface->GetSliderMessage( m_ixSlider );
		m_strStatus.Format( _T("%02x"), wStatus & 0xff );
		m_strNumber.Format( _T("%02x"), (wStatus >> 8) & 0xff);
	}
	else if ( m_ixSwitch != -1 )
	{
		WORD wStatus = m_pSurface->GetButtonMessage( m_ixSwitch );
		m_strStatus.Format( _T("%02x"), wStatus & 0xff );
		m_strNumber.Format( _T("%02x"), (wStatus >> 8) & 0xff);

		std::vector<BYTE> v;
		m_pSurface->GetButtonSysex( m_ixSwitch, &v );

		SysexToString( v, &m_strSysex );
	}


	CDialog::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCellMidiPropsDlg::OnOK()
{
	CDialog::OnOK();

	BYTE byStatus = (BYTE)::_tcstoul( m_strStatus, 0, 16 );
	BYTE byNumber = (BYTE)::_tcstoul( m_strNumber, 0, 16 );
	WORD wmsg = byStatus | (byNumber<<8);

	CMidiBinding::MessageInterpretation mi = m_ixInterpret == 0 ? CMidiBinding::MI_Literal : CMidiBinding::MI_Increment;
	if ( m_ixKnob != -1 )
	{
		m_pSurface->SetRotaryMidiInterpretation( m_ixKnob, mi );
		m_pSurface->SetRotaryIncrementHingeValue( m_ixKnob, m_dwHinge );
		m_pSurface->SetRotaryIncrementUseAccel( m_ixKnob, !!m_bUseAccel );
		m_pSurface->SetRotaryMessage( m_ixKnob, wmsg );
	}
	else if ( m_ixSlider != -1 )
	{
		m_pSurface->SetSliderMidiInterpretation( m_ixSlider, mi );
		m_pSurface->SetSliderIncrementHingeValue( m_ixSlider, m_dwHinge );
		m_pSurface->SetSliderIncrementUseAccel( m_ixSlider, !!m_bUseAccel );
		m_pSurface->SetSliderMessage( m_ixSlider, wmsg );
	}
	else if ( m_ixSwitch != -1 )
	{
		m_pSurface->SetSliderMessage( m_ixSwitch, wmsg );
		std::vector<BYTE> v;
		StringToSysex( m_strSysex, &v );
		m_pSurface->SetButtonSysex( m_ixSwitch, v );
	}

}

void CCellMidiPropsDlg::OnBnClickedUseAccel()
{
	UpdateData();
}

void CCellMidiPropsDlg::OnBnClickedRadInterpret()
{
	UpdateData();
}

void CCellMidiPropsDlg::OnBnClickedRad1Interpret()
{
	UpdateData();
}
