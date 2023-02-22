// ACTControllerPropPageTab.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "ACTControllerPropPageTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab dialog


CACTControllerPropPageTab::CACTControllerPropPageTab(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent),
	m_pSurface( NULL )
{
	m_cBrush = new CBrush(RGB(255, 255, 255));

	//{{AFX_DATA_INIT(CACTControllerPropPageTab)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CACTControllerPropPageTab::~CACTControllerPropPageTab()
{
	delete m_cBrush;
}


void CACTControllerPropPageTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CACTControllerPropPageTab)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CACTControllerPropPageTab, CDialog)
	//{{AFX_MSG_MAP(CACTControllerPropPageTab)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTab::Init(CACTController *pSurface)
{
	TRACE("CACTControllerPropPageTab::Init(0x%08X)\n", pSurface);

	m_pSurface = pSurface;
}

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab message handlers

HBRUSH CACTControllerPropPageTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
#if 1
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
#else
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkColor(RGB(255, 255, 255));
	return (HBRUSH)(m_cBrush->GetSafeHandle());
#endif
}

////////////////////////////////////////////////////////////////////////////////
