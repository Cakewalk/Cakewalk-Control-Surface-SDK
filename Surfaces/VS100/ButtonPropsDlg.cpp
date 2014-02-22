// ButtonPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButtonPropsDlg.h"
#include "VS100PropPage.h"


// CButtonPropsDlg dialog

IMPLEMENT_DYNAMIC(CButtonPropsDlg, CDialog)


CButtonPropsDlg::CButtonPropsDlg( CVS100* pSurface, CVS100::ButtonActionDefinition& bad, CWnd* pParent )// = NULL)
	: CDialog(CButtonPropsDlg::IDD, pParent)
	,m_bad( bad )
	,m_pSurface(pSurface)
{

}

CButtonPropsDlg::~CButtonPropsDlg()
{
}

void CButtonPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMMANDLIST, m_cCmdList);
	DDX_Control(pDX, IDC_TRANSPORT_STATE_LIST, m_cTransportList );
	DDX_Control(pDX, IDC_SHIFT, m_cShift);
	DDX_Control(pDX, IDC_CTRL, m_cControl);
	DDX_Control(pDX, IDC_ALT, m_cAlt);
	DDX_Control(pDX, IDC_KEYLIST, m_cKeyList);
}


void CButtonPropsDlg::enableControls()
{
	m_cKeyList.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Key);
	m_cShift.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Key );
	m_cControl.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Key );
	m_cAlt.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Key );

	m_cCmdList.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Command );

	m_cTransportList.EnableWindow( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Transport );
}


BEGIN_MESSAGE_MAP(CButtonPropsDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMMANDLIST, &CButtonPropsDlg::OnCbnSelchangeCommandlist)
	ON_CBN_SELCHANGE(IDC_TRANSPORT_STATE_LIST, &CButtonPropsDlg::OnCbnSelchangeTransportlist)
	ON_BN_CLICKED(IDC_RADIO1, &CButtonPropsDlg::OnBnClickedRadioCommand)
	ON_BN_CLICKED(IDC_RADIO2, &CButtonPropsDlg::OnBnClickedRadioKey)
	ON_BN_CLICKED(IDC_RADIO3, &CButtonPropsDlg::OnBnClickedRadio3)
	ON_WM_CHAR()
	ON_BN_CLICKED(IDC_SHIFT, &CButtonPropsDlg::OnBnClickedShift)
	ON_CBN_SELCHANGE(IDC_KEYLIST, &CButtonPropsDlg::OnCbnSelchangeKeylist)
END_MESSAGE_MAP()


// CButtonPropsDlg message handlers

void CButtonPropsDlg::OnCbnSelchangeCommandlist()
{
	int iCur = m_cCmdList.GetCurSel();
	if ( -1 == iCur )
		return;
	m_bad.dwCommandOrKey = (DWORD)m_cCmdList.GetItemData( iCur );
}
void CButtonPropsDlg::OnCbnSelchangeTransportlist()
{
	int iCur = m_cTransportList.GetCurSel();
	if ( -1 == iCur )
		return;
	m_bad.transportState = (SONAR_TRANSPORT_STATE)m_cTransportList.GetItemData( iCur );
}

void CButtonPropsDlg::OnBnClickedRadioCommand()
{
	m_bad.eActionType= CVS100::ButtonActionDefinition::AT_Command;
	enableControls();
}

void CButtonPropsDlg::OnBnClickedRadioKey()
{
	m_bad.eActionType = CVS100::ButtonActionDefinition::AT_Key;
	enableControls();
}

void CButtonPropsDlg::OnBnClickedRadio3()
{
	m_bad.eActionType = CVS100::ButtonActionDefinition::AT_Transport;
	enableControls();
}


void CButtonPropsDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnChar(nChar, nRepCnt, nFlags);
}


BOOL CButtonPropsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	DWORD dwCount;
	m_pSurface->GetCommandInterface()->GetCommandCount( &dwCount );
	for ( DWORD i = 0; i < dwCount; i++ )
	{
		char sz[32];
		DWORD dwLen = sizeof(sz);
		DWORD dwId = 0;
		m_pSurface->GetCommandInterface()->GetCommandInfo( i, &dwId, sz, &dwLen );
		CString str(sz);	// converts to UNICODE

//		TRACE( "Cmd %s = %d\n", sz, dwId );

		int iIns = m_cCmdList.AddString( str );
		m_cCmdList.SetItemData( iIns, (DWORD_PTR) dwId );
	}

	// look for the current one
	for ( int i = 0; i < m_cCmdList.GetCount(); i++ )
	{
		DWORD_PTR data = m_cCmdList.GetItemData( i );
		if ( (DWORD)data == m_bad.dwCommandOrKey )
		{
			m_cCmdList.SetCurSel( i );
			break;
		}
	}

	// Add Transport Commands
	int iIns = m_cTransportList.AddString( _T("Play") );
	m_cTransportList.SetItemData( iIns, (DWORD_PTR)TRANSPORT_STATE_PLAY );
	iIns = m_cTransportList.AddString( _T("Record") );
	m_cTransportList.SetItemData( iIns, (DWORD_PTR)TRANSPORT_STATE_REC );
	iIns = m_cTransportList.AddString( _T("Rewind") );
	m_cTransportList.SetItemData( iIns, (DWORD_PTR)TRANSPORT_STATE_REWIND );
	iIns = m_cTransportList.AddString( _T("Fast Fwd") );
	m_cTransportList.SetItemData( iIns, (DWORD_PTR)TRANSPORT_STATE_FFWD );

	// look for the current one
	for ( int i = 0; i < m_cTransportList.GetCount(); i++ )
	{
		DWORD_PTR data = m_cTransportList.GetItemData( i );
		if ( (SONAR_TRANSPORT_STATE)data == m_bad.transportState)
		{
			m_cTransportList.SetCurSel( i );
			break;
		}
	}


	// fill key list
	int iCur = -1;
	for ( size_t ixK = 0; ixK < CVS100PropPage::sm_vKeys.size(); ixK++ )
	{
		CVS100PropPage::NamedKey& nk = CVS100PropPage::sm_vKeys[ixK];
		int ins = m_cKeyList.AddString( nk.strName );
		m_cKeyList.SetItemData( ins, (DWORD_PTR)nk.vk );

		if ( nk.vk == m_bad.dwCommandOrKey )
			iCur = (int)ixK;
	}

	if ( m_bad.eActionType == CVS100::ButtonActionDefinition::AT_Key && iCur != -1 )
		m_cKeyList.SetCurSel( iCur );

	if ( (m_bad.wModKeys & CVS100::SMK_SHIFT) != 0)
		m_cShift.SetCheck( TRUE );

	int id = 0;
	switch (m_bad.eActionType)
	{
	case CVS100::ButtonActionDefinition::AT_Command:
		id = IDC_RADIO1;
		break;
	case CVS100::ButtonActionDefinition::AT_Key:
		id = IDC_RADIO2;
		break;
	case CVS100::ButtonActionDefinition::AT_Transport:
		id = IDC_RADIO3;
		break;
	}
	CheckRadioButton( IDC_RADIO1, IDC_RADIO3, id );

	enableControls();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CButtonPropsDlg::OnBnClickedShift()
{
	if ( m_cShift.GetCheck() )
		m_bad.wModKeys |= CVS100::SMK_SHIFT;
	else
		m_bad.wModKeys &= ~CVS100::SMK_SHIFT;
}


void CButtonPropsDlg::OnCbnSelchangeKeylist()
{
	int i = m_cKeyList.GetCurSel();
	m_bad.dwCommandOrKey = (DWORD)m_cKeyList.GetItemData( i );
}

