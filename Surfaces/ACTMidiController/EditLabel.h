#if !defined(AFX_EDITLABEL_H__F68FA8FE_267F_49A3_BCCD_73A8556CD649__INCLUDED_)
#define AFX_EDITLABEL_H__F68FA8FE_267F_49A3_BCCD_73A8556CD649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditLabel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditLabel dialog

class CEditLabel : public CDialog
{
// Construction
public:
	CEditLabel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditLabel)
	enum { IDD = IDD_EDIT_LABEL };
	CEdit	m_cLabel;
	CString	m_strLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditLabel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditLabel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLABEL_H__F68FA8FE_267F_49A3_BCCD_73A8556CD649__INCLUDED_)
