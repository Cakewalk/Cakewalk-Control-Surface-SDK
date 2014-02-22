#if !defined(AFX_ACTCONTROLLERPROPPAGETABCTRL_H__C914E007_DC43_4F6A_9A44_242C9DD7A852__INCLUDED_)
#define AFX_ACTCONTROLLERPROPPAGETABCTRL_H__C914E007_DC43_4F6A_9A44_242C9DD7A852__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ACTControllerPropPageTabCtrl.h : header file
//

#include "ACTControllerPropPageTab.h"

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTabCtrl window

class CACTControllerPropPageTabCtrl : public CTabCtrl
{
// Construction
public:
	CACTControllerPropPageTabCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerPropPageTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CACTControllerPropPageTabCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CACTControllerPropPageTabCtrl)
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:		
	void InitDialogs(CACTController *pSurface);
	void LoadAllFields();
	void ActivateTabDialogs();
	void Refresh();

protected:		
	CACTControllerPropPageTab *m_pDialog[3];
	int m_iDialogID[3];
	int m_iNumTabs;
	int m_iCurrentTab;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTCONTROLLERPROPPAGETABCTRL_H__C914E007_DC43_4F6A_9A44_242C9DD7A852__INCLUDED_)
