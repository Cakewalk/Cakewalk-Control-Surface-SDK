#if !defined(AFX_ACTCONTROLLERPROPPAGETAB2_H__C32AD468_4989_4E1A_A43C_783D23DFC63A__INCLUDED_)
#define AFX_ACTCONTROLLERPROPPAGETAB2_H__C32AD468_4989_4E1A_A43C_783D23DFC63A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ACTControllerPropPageTab2.h : header file
//

#include "ACTControllerPropPageTab.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab2 dialog

class CACTControllerPropPageTab2 : public CACTControllerPropPageTab
{
// Construction
public:
	CACTControllerPropPageTab2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CACTControllerPropPageTab2)
	enum { IDD = IDD_TAB2 };
	CEdit	m_cComments;
	CComboBox	m_cSlidersBank;
	CComboBox	m_cKnobsBank;
	CComboBox	m_cButtonBank;
	CButton	m_cACTFollowsContext;
	CButton	m_cSelectHighlightsTrack;
	CComboBox	m_cSlidersBinding;
	CComboBox	m_cKnobsBinding;
	CComboBox	m_cKnobsCapture;
	CComboBox	m_cSlidersCapture;
	CButton	m_cExcludeSlidersACT;
	CButton	m_cExcludeRotariesACT;
	CButton	m_cButtonExcludeACT;
	CComboBox	m_cButtonAction;
	CComboBox	m_cButtonSelect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerPropPageTab2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void LoadAllFields();
	virtual void Select();
	virtual void Refresh();

	afx_msg void OnCbnSelchangeKnobsCaptmode();
	afx_msg void OnCbnSelchangeSlidersCaptmode();
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedEditInit();

protected:
	void UpdateACTStatus(bool bForce =false);
	void GreyACTFields();

	bool m_bACTEnabled;
	bool m_bExcludeRotariesACT;
	bool m_bExcludeSlidersACT;
	bool m_bSelectHighlightsTrack;
	bool m_bACTFollowsContext;

	DWORD_PTR m_iRotaryBank;
	DWORD_PTR m_iSliderBank;
	DWORD_PTR m_iButtonBank;

	// Generated message map functions
	//{{AFX_MSG(CACTControllerPropPageTab2)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeButtonSelect();
	afx_msg void OnSelchangeButtonAction();
	afx_msg void OnButtonExcludeACT();
	afx_msg void OnExcludeRotariesACT();
	afx_msg void OnExcludeSlidersACT();
	afx_msg void OnSelchangeKnobsBinding();
	afx_msg void OnSelchangeSlidersBinding();
	afx_msg void OnSelectHighlightsTrack();
	afx_msg void OnACTFollowsContext();
	afx_msg void OnDefaults();
	afx_msg void OnSelchangeButtonBank();
	afx_msg void OnSelchangeKnobsBank();
	afx_msg void OnSelchangeSlidersBank();
	afx_msg void OnKillfocusCommets();
	afx_msg void OnResetMidiLearn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTCONTROLLERPROPPAGETAB2_H__C32AD468_4989_4E1A_A43C_783D23DFC63A__INCLUDED_)
