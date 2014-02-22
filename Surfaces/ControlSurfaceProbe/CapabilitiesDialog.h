#if !defined(AFX_CAPABILITIESDIALOG_H__9DFBBFFA_6D1B_4AE7_837E_470C101041B1__INCLUDED_)
#define AFX_CAPABILITIESDIALOG_H__9DFBBFFA_6D1B_4AE7_837E_470C101041B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CapabilitiesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCapabilitiesDialog dialog

class CCapabilitiesDialog : public CDialog
{
// Construction
public:
	CCapabilitiesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCapabilitiesDialog)
	enum { IDD = IDD_CAPABILITIES };
	CEdit	m_cCapabilities;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapabilitiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetIdentity(ISonarIdentity *pIdentity)	{ m_pIdentity = pIdentity; };

protected:
	ISonarIdentity*		m_pIdentity;

	CString				m_strText;

	void CheckCap(const char *pszName, HOST_CAPABILITY eCap);

	// Generated message map functions
	//{{AFX_MSG(CCapabilitiesDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPABILITIESDIALOG_H__9DFBBFFA_6D1B_4AE7_837E_470C101041B1__INCLUDED_)
