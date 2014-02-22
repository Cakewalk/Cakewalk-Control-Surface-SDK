#if !defined(AFX_ACTCONTROLLERPROPPAGETAB_H__DBABEEC6_E6C0_4F1B_8A92_B7527A61D0DD__INCLUDED_)
#define AFX_ACTCONTROLLERPROPPAGETAB_H__DBABEEC6_E6C0_4F1B_8A92_B7527A61D0DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ACTControllerPropPageTab.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab dialog

class CACTControllerPropPageTab : public CDialog
{
// Construction
public:
	CACTControllerPropPageTab(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CACTControllerPropPageTab)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerPropPageTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	void Init(CACTController *pSurface);
	virtual void LoadAllFields() =0;
	virtual void Select() =0;
	virtual void Refresh() =0;

// Implementation
protected:
	CACTController *m_pSurface;
	CBrush *m_cBrush;

	// Generated message map functions
	//{{AFX_MSG(CACTControllerPropPageTab)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTCONTROLLERPROPPAGETAB_H__DBABEEC6_E6C0_4F1B_8A92_B7527A61D0DD__INCLUDED_)
