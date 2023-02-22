// ACTControllerPropPageTab2.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "ACTControllerPropPageTab2.h"
#include "MidiInitializationMsgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TID_REFRESH (1)
#define PER_REFRESH (500)


/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab2 dialog


CACTControllerPropPageTab2::CACTControllerPropPageTab2(CWnd* pParent /*=NULL*/)
	: CACTControllerPropPageTab(CACTControllerPropPageTab2::IDD, pParent)
{
	m_bACTEnabled = false;
	m_bExcludeRotariesACT = false;
	m_bExcludeSlidersACT = false;
	m_bSelectHighlightsTrack = false;
	m_bACTFollowsContext = false;

	m_iRotaryBank = 0;
	m_iSliderBank = 0;
	m_iButtonBank = 0;

	//{{AFX_DATA_INIT(CACTControllerPropPageTab2)
	//}}AFX_DATA_INIT
}


void CACTControllerPropPageTab2::DoDataExchange(CDataExchange* pDX)
{
	TRACE("CACTControllerPropPageTab2::DoDataExchange()\n");

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CACTControllerPropPageTab2)
	DDX_Control(pDX, IDC_COMMETS, m_cComments);
	DDX_Control(pDX, IDC_SLIDERS_BANK, m_cSlidersBank);
	DDX_Control(pDX, IDC_KNOBS_BANK, m_cKnobsBank);
	DDX_Control(pDX, IDC_BUTTON_BANK, m_cButtonBank);
	DDX_Control(pDX, IDC_ACT_FOLLOWS_CONTEXT, m_cACTFollowsContext);
	DDX_Control(pDX, IDC_SELECT_HIGHLIGHTS_TRACK, m_cSelectHighlightsTrack);
	DDX_Control(pDX, IDC_SLIDERS_BINDING, m_cSlidersBinding);
	DDX_Control(pDX, IDC_KNOBS_BINDING, m_cKnobsBinding);
	DDX_Control(pDX, IDC_KNOBS_CAPTMODE, m_cKnobsCapture);
	DDX_Control(pDX, IDC_SLIDERS_CAPTMODE, m_cSlidersCapture);
	DDX_Control(pDX, IDC_EXCLUDE_SLIDERS_ACT, m_cExcludeSlidersACT);
	DDX_Control(pDX, IDC_EXCLUDE_ROTARIES_ACT, m_cExcludeRotariesACT);
	DDX_Control(pDX, IDC_BUTTON_EXCLUDE_ACT, m_cButtonExcludeACT);
	DDX_Control(pDX, IDC_BUTTON_ACTION, m_cButtonAction);
	DDX_Control(pDX, IDC_BUTTON_SELECT, m_cButtonSelect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CACTControllerPropPageTab2, CACTControllerPropPageTab)
	//{{AFX_MSG_MAP(CACTControllerPropPageTab2)
	ON_CBN_SELCHANGE(IDC_BUTTON_SELECT, OnSelchangeButtonSelect)
	ON_CBN_SELCHANGE(IDC_BUTTON_ACTION, OnSelchangeButtonAction)
	ON_BN_CLICKED(IDC_BUTTON_EXCLUDE_ACT, OnButtonExcludeACT)
	ON_BN_CLICKED(IDC_EXCLUDE_ROTARIES_ACT, OnExcludeRotariesACT)
	ON_BN_CLICKED(IDC_EXCLUDE_SLIDERS_ACT, OnExcludeSlidersACT)
	ON_CBN_SELCHANGE(IDC_KNOBS_BINDING, OnSelchangeKnobsBinding)
	ON_CBN_SELCHANGE(IDC_SLIDERS_BINDING, OnSelchangeSlidersBinding)
	ON_BN_CLICKED(IDC_SELECT_HIGHLIGHTS_TRACK, OnSelectHighlightsTrack)
	ON_BN_CLICKED(IDC_ACT_FOLLOWS_CONTEXT, OnACTFollowsContext)
	ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
	ON_CBN_SELCHANGE(IDC_BUTTON_BANK, OnSelchangeButtonBank)
	ON_CBN_SELCHANGE(IDC_KNOBS_BANK, OnSelchangeKnobsBank)
	ON_CBN_SELCHANGE(IDC_SLIDERS_BANK, OnSelchangeSlidersBank)
	ON_EN_KILLFOCUS(IDC_COMMETS, OnKillfocusCommets)
	ON_BN_CLICKED(IDC_RESET_MIDI_LEARN, OnResetMidiLearn)
	ON_CBN_SELCHANGE(IDC_KNOBS_CAPTMODE, &CACTControllerPropPageTab2::OnCbnSelchangeKnobsCaptmode)
	ON_CBN_SELCHANGE(IDC_SLIDERS_CAPTMODE, &CACTControllerPropPageTab2::OnCbnSelchangeSlidersCaptmode)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SEND, &CACTControllerPropPageTab2::OnBnClickedSend)
	ON_BN_CLICKED(IDC_EDIT_INIT, &CACTControllerPropPageTab2::OnBnClickedEditInit)
	ON_WM_TIMER()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::LoadAllFields()
{
	TRACE("CACTControllerPropPageTab2::LoadAllFields()\n");

	if ( !m_pSurface )
		return;

	SelectByItemData(&m_cKnobsBinding, m_pSurface->GetKnobsBinding(int(m_iRotaryBank)));
	SelectByItemData(&m_cSlidersBinding, m_pSurface->GetSlidersBinding(int(m_iSliderBank)));

	int idx = m_cButtonSelect.GetCurSel();
	FillComboBox(&m_cButtonSelect, m_pSurface->GetButtonNames());
	m_cButtonSelect.SetCurSel(idx);

	OnSelchangeButtonSelect();

	UpdateACTStatus(true);

	GreyACTFields();

	CString strComments;

	CMixParam::CaptureType ct = m_pSurface->GetRotaryCaptureType(int(m_iRotaryBank));
	for ( int ix = 0; ix < m_cKnobsCapture.GetCount(); ix++ )
	{
		if ( ct == (CMixParam::CaptureType)m_cKnobsCapture.GetItemData( ix ) )
		{
			m_cKnobsCapture.SetCurSel( ix );
			break;
		}
	}
	ct = m_pSurface->GetSliderCaptureType(int(m_iSliderBank));
	for ( int ix = 0; ix < m_cSlidersCapture.GetCount(); ix++ )
	{
		if ( ct == (CMixParam::CaptureType)m_cSlidersCapture.GetItemData( ix ) )
		{
			m_cSlidersCapture.SetCurSel( ix );
			break;
		}
	}


	m_pSurface->GetComments(&strComments);
	m_cComments.SetWindowText(strComments);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::Select()
{
	if ( !m_pSurface )
		return;

	int idx = m_cButtonSelect.GetCurSel();
	FillComboBox(&m_cButtonSelect, m_pSurface->GetButtonNames());
	m_cButtonSelect.SetCurSel(idx);

	UpdateACTStatus();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::Refresh()
{
	UpdateACTStatus();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::UpdateACTStatus(bool bForce)
{
	if (m_pSurface == NULL)
	{
		TRACE("CACTControllerPropPageTab2::UpdateACTStatus(%d) m_pSurface is NULL\n", bForce);
		return;
	}

	bool bACTEnabled = m_pSurface->GetUseDynamicMappings();
	if (bForce || m_bACTEnabled != bACTEnabled)
	{
		m_bACTEnabled = bACTEnabled;
		GreyACTFields();
	}

	bool bExcludeRotariesACT = m_pSurface->GetExcludeRotariesACT(int(m_iRotaryBank));
	if (bForce || m_bExcludeRotariesACT != bExcludeRotariesACT)
	{
		m_cExcludeRotariesACT.SetCheck(bExcludeRotariesACT ? 1 : 0);
		m_bExcludeRotariesACT = bExcludeRotariesACT;
	}

	bool bExcludeSlidersACT = m_pSurface->GetExcludeSlidersACT(int(m_iSliderBank));
	if (bForce || m_bExcludeSlidersACT != bExcludeSlidersACT)
	{
		m_cExcludeSlidersACT.SetCheck(bExcludeSlidersACT ? 1 : 0);
		m_bExcludeSlidersACT = bExcludeSlidersACT;
	}

	bool bSelectHighlightsTrack = m_pSurface->GetSelectHighlightsTrack();
	if (bForce || m_bSelectHighlightsTrack != bSelectHighlightsTrack)
	{
		m_cSelectHighlightsTrack.SetCheck(bSelectHighlightsTrack ? 1 : 0);
		m_bSelectHighlightsTrack = bSelectHighlightsTrack;
	}

	bool bACTFollowsContext = m_pSurface->GetACTFollowsContext();
	if (bForce || m_bACTFollowsContext != bACTFollowsContext)
	{
		m_cACTFollowsContext.SetCheck(bACTFollowsContext ? 1 : 0);
		m_bACTFollowsContext = bACTFollowsContext;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::GreyACTFields()
{
	BOOL bEnable = ( m_pSurface && m_pSurface->SupportsDynamicMappings()) ? TRUE : FALSE;

	m_cButtonExcludeACT.EnableWindow(bEnable);
	m_cExcludeRotariesACT.EnableWindow(bEnable);
	m_cExcludeSlidersACT.EnableWindow(bEnable);
}

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab2 message handlers

BOOL CACTControllerPropPageTab2::OnInitDialog() 
{
	BOOL bRet = CACTControllerPropPageTab::OnInitDialog();

	m_cComments.SetLimitText(1024);

	if (m_pSurface == NULL)
	{
		TRACE("CACTControllerPropPageTab2::OnInitDialog() m_pSurface is NULL\n");
	}
	else
	{
		FillComboBox(&m_cKnobsBank,		m_pSurface->GetBankNames());
		FillComboBox(&m_cKnobsBinding,	m_pSurface->GetKnobBindings());

		FillComboBox(&m_cSlidersBank,	m_pSurface->GetBankNames());
		FillComboBox(&m_cSlidersBinding,m_pSurface->GetSliderBindings());

		FillComboBox(&m_cButtonBank,	m_pSurface->GetBankNames());
		FillComboBox(&m_cButtonSelect,	m_pSurface->GetButtonNames());
		FillComboBox(&m_cButtonAction,	m_pSurface->GetButtonActions());

		m_cKnobsBank.SetCurSel(0);
		m_cSlidersBank.SetCurSel(0);
		m_cButtonBank.SetCurSel(0);

		m_cButtonSelect.SetCurSel(0);

		// capture mode combos
		CString str;

		str.LoadString( IDS_CAPT_JUMP );
		int ix = m_cKnobsCapture.AddString( str );
		m_cKnobsCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Jump );
		ix = m_cSlidersCapture.AddString( str );
		m_cSlidersCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Jump );

		str.LoadString( IDS_CAPT_MATCH );
		ix = m_cKnobsCapture.AddString( str );
		m_cKnobsCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Match );
		ix = m_cSlidersCapture.AddString( str );
		m_cSlidersCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Match );
/*
		str.LoadString( IDS_CAPT_CONVERGE );
		ix = m_cKnobsCapture.AddString( str );
		m_cKnobsCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Converge );
		ix = m_cSlidersCapture.AddString( str );
		m_cSlidersCapture.SetItemData( ix, (DWORD_PTR)CMixParam::CT_Converge );
*/

		GetDlgItem( IDC_SEND )->EnableWindow( m_pSurface->HasInitMessage() );

		SetTimer( TID_REFRESH, PER_REFRESH, NULL );
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeButtonSelect() 
{
	if ( !m_pSurface )
		return;

	int idx = m_cButtonSelect.GetCurSel();
	if (idx != CB_ERR)
	{
		DWORD_PTR dwButton = m_cButtonSelect.GetItemData(idx);
		DWORD_PTR dwAction = m_pSurface->GetButtonAction(int(m_iButtonBank), (VirtualButton)dwButton);

		SelectByItemData(&m_cButtonAction, DWORD(dwAction));

		bool bExcludeACT = m_pSurface->GetButtonExcludeACT(int(m_iButtonBank), (VirtualButton)dwButton);

		m_cButtonExcludeACT.SetCheck(bExcludeACT ? 1 : 0);
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeButtonAction() 
{
	int idx = m_cButtonAction.GetCurSel();
	if (idx != CB_ERR)
	{
		DWORD_PTR dwAction = m_cButtonAction.GetItemData(idx);

		idx = m_cButtonSelect.GetCurSel();
		if (idx != CB_ERR)
		{
			DWORD_PTR dwButton = m_cButtonSelect.GetItemData(idx);
			if ( m_pSurface )
				m_pSurface->SetButtonAction(int(m_iButtonBank), (VirtualButton)dwButton, DWORD(dwAction));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnButtonExcludeACT() 
{
	bool bExclude = m_cButtonExcludeACT.GetCheck() ? true : false;

	int idx = m_cButtonSelect.GetCurSel();
	if (idx != CB_ERR)
	{
		DWORD_PTR dwButton = m_cButtonSelect.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetButtonExcludeACT(int(m_iButtonBank), (VirtualButton)dwButton, bExclude);
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnExcludeRotariesACT() 
{
	m_bExcludeRotariesACT = (m_cExcludeRotariesACT.GetCheck() != 0);
	if ( m_pSurface )
		m_pSurface->SetExcludeRotariesACT(int(m_iRotaryBank), m_bExcludeRotariesACT);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnExcludeSlidersACT() 
{
	m_bExcludeSlidersACT = (m_cExcludeSlidersACT.GetCheck() != 0);
	if ( m_pSurface )
		m_pSurface->SetExcludeSlidersACT(int(m_iSliderBank), m_bExcludeSlidersACT);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeKnobsBinding() 
{
	int idx = m_cKnobsBinding.GetCurSel();
	if (idx != CB_ERR)
	{
		DWORD_PTR dwBinding = m_cKnobsBinding.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetKnobsBinding(int(m_iRotaryBank), DWORD(dwBinding));
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeSlidersBinding() 
{
	int idx = m_cSlidersBinding.GetCurSel();
	if (idx != CB_ERR)
	{
		DWORD_PTR dwBinding = m_cSlidersBinding.GetItemData(idx);
		if ( m_pSurface )
			m_pSurface->SetSlidersBinding(int(m_iSliderBank), DWORD(dwBinding));
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelectHighlightsTrack() 
{
	m_bSelectHighlightsTrack = (m_cSelectHighlightsTrack.GetCheck() != 0);
	if ( m_pSurface )
		m_pSurface->SetSelectHighlightsTrack(m_bSelectHighlightsTrack);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnACTFollowsContext() 
{
	m_bACTFollowsContext = (m_cACTFollowsContext.GetCheck() != 0);
	if ( m_pSurface )
		m_pSurface->SetACTFollowsContext(m_bACTFollowsContext);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnDefaults() 
{
	if ( m_pSurface )
		m_pSurface->RestoreDefaultBindings(true);

	LoadAllFields();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnResetMidiLearn() 
{
	if ( m_pSurface )
		m_pSurface->ResetMidiLearn();
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeButtonBank() 
{
	int idx = m_cButtonBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iButtonBank = m_cButtonBank.GetItemData(idx);

		LoadAllFields();
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeKnobsBank() 
{
	int idx = m_cKnobsBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iRotaryBank = m_cKnobsBank.GetItemData(idx);

		LoadAllFields();
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnSelchangeSlidersBank() 
{
	int idx = m_cSlidersBank.GetCurSel();
	if (idx != CB_ERR)
	{
		m_iSliderBank = m_cSlidersBank.GetItemData(idx);

		LoadAllFields();
	}
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnKillfocusCommets() 
{
	TRACE("CACTControllerPropPageTab2::OnKillfocusCommets()\n");

	CString strComments;

	m_cComments.GetWindowText(strComments);
	if ( m_pSurface )
		m_pSurface->SetComments(strComments);
}

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab2::OnCbnSelchangeKnobsCaptmode()
{
	int ix = m_cKnobsCapture.GetCurSel();
	if ( m_pSurface )
		m_pSurface->SetRotaryCaptureType( int(m_iRotaryBank), (CMixParam::CaptureType)m_cKnobsCapture.GetItemData(ix) );
}

void CACTControllerPropPageTab2::OnCbnSelchangeSlidersCaptmode()
{
	int ix = m_cSlidersCapture.GetCurSel();
	if ( m_pSurface )
		m_pSurface->SetSliderCaptureType( int(m_iSliderBank), (CMixParam::CaptureType)m_cSlidersCapture.GetItemData(ix) );
}

void CACTControllerPropPageTab2::OnBnClickedSend()
{
	CWaitCursor wc;
	if ( m_pSurface )
		m_pSurface->SendInitMessages();
}

void CACTControllerPropPageTab2::OnBnClickedEditInit()
{
	CMidiInitializationMsgDlg dlg( m_pSurface );
	dlg.DoModal();

}

void CACTControllerPropPageTab2::OnTimer(UINT_PTR nIDEvent)
{
	if ( TID_REFRESH == nIDEvent && m_pSurface )
		GetDlgItem( IDC_SEND )->EnableWindow( m_pSurface->HasInitMessage() );

	CACTControllerPropPageTab::OnTimer(nIDEvent);
}
