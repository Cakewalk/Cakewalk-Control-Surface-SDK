#if !defined(AFX_CONTROLSURFACEPROPPAGE_H__533341D0_197B_457F_B415_ECAC27C3B53D__INCLUDED_)
#define AFX_CONTROLSURFACEPROPPAGE_H__533341D0_197B_457F_B415_ECAC27C3B53D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ControlSurfacePropPage.h : header file
//

#define SZAPPHELPFILE "MotorMix.chm"

/////////////////////////////////////////////////////////////////////////////
// CControlSurfacePropPage dialog

class CControlSurfacePropPage : public COlePropertyPage
{
// Construction
public:

	CControlSurfacePropPage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CControlSurfacePropPage)
	enum { IDD = IDD_SURFACE_PROPPAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlSurfacePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnObjectsChanged();
	virtual BOOL OnInitDialog();
	virtual BOOL OnHelp(LPCTSTR str);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CControlSurfacePropPage)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

	CComPtr<ICSProperties>			m_spICSProperties;

	CHelpAssist							m_HelpHelper;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLSURFACEPROPPAGE_H__533341D0_197B_457F_B415_ECAC27C3B53D__INCLUDED_)
