// EditLabel.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "EditLabel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditLabel dialog


CEditLabel::CEditLabel(CWnd* pParent /*=NULL*/)
	: CDialog(CEditLabel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditLabel)
	m_strLabel = _T("");
	//}}AFX_DATA_INIT
}


void CEditLabel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditLabel)
	DDX_Control(pDX, IDC_LABEL, m_cLabel);
	DDX_Text(pDX, IDC_LABEL, m_strLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditLabel, CDialog)
	//{{AFX_MSG_MAP(CEditLabel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditLabel message handlers

BOOL CEditLabel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_cLabel.SetLimitText(12);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
