// ACTControllerPropPageTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "ACTControllerPropPageTabCtrl.h"
#include "ACTControllerPropPageTab1.h"
#include "ACTControllerPropPageTab2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTabCtrl

CACTControllerPropPageTabCtrl::CACTControllerPropPageTabCtrl()
{
	m_iDialogID[0] = IDD_TAB1;
	m_iDialogID[1] = IDD_TAB2;

	m_pDialog[0] = new CACTControllerPropPageTab1();
	m_pDialog[1] = new CACTControllerPropPageTab2();

	m_iNumTabs = 2;
	m_iCurrentTab = 0;
}

CACTControllerPropPageTabCtrl::~CACTControllerPropPageTabCtrl()
{
	for (int n = 0; n < m_iNumTabs; n++)
		delete m_pDialog[n];
}


BEGIN_MESSAGE_MAP(CACTControllerPropPageTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CACTControllerPropPageTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTabCtrl::InitDialogs(CACTController *pSurface)
{
	int n;

	for (n = 0; n < m_iNumTabs; n++)
		m_pDialog[n]->Init(pSurface);

	for (n = 0; n < m_iNumTabs; n++)
		m_pDialog[n]->Create(m_iDialogID[n], GetParent());

	for (n = 0; n < m_iNumTabs; n++)
		m_pDialog[n]->ShowWindow((n == 0) ? SW_HIDE : SW_SHOW);

	CString strTitle0(MAKEINTRESOURCE(IDS_CONTROLLERS_TAB));
	CString strTitle1(MAKEINTRESOURCE(IDS_OPTIONS_TAB));

	InsertItem(0, strTitle0);
	InsertItem(1, strTitle1);

	ActivateTabDialogs();
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTabCtrl::LoadAllFields()
{
	for (int n = 0; n < m_iNumTabs; n++)
		m_pDialog[n]->LoadAllFields();
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTabCtrl::ActivateTabDialogs()
{
	int iCurrentSel = GetCurSel();

	if (m_pDialog[iCurrentSel]->m_hWnd)
		m_pDialog[iCurrentSel]->ShowWindow(SW_HIDE);

	CRect l_rectClient;
	CRect l_rectWnd;

	GetClientRect(l_rectClient);
	AdjustRect(FALSE, l_rectClient);
	GetWindowRect(l_rectWnd);
	GetParent()->ScreenToClient(l_rectWnd);

	l_rectClient.OffsetRect(l_rectWnd.left, l_rectWnd.top);

	for (int n = 0; n < m_iNumTabs; n++)
	{
		m_pDialog[n]->SetWindowPos(&wndTop,
									l_rectClient.left,
									l_rectClient.top,
									l_rectClient.Width(),
									l_rectClient.Height(),
									(n == iCurrentSel) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
	}

	m_pDialog[iCurrentSel]->Select();
	m_pDialog[iCurrentSel]->ShowWindow(SW_SHOW);

	m_iCurrentTab = iCurrentSel;
}

/////////////////////////////////////////////////////////////////////////////

void CACTControllerPropPageTabCtrl::Refresh()
{
	if (m_pDialog[m_iCurrentTab]->m_hWnd)
		m_pDialog[m_iCurrentTab]->Refresh();
}

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTabCtrl message handlers

void CACTControllerPropPageTabCtrl::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ActivateTabDialogs();

	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
