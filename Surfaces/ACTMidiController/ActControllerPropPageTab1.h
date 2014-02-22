#if !defined(AFX_ACTCONTROLLERPROPPAGETAB1_H__467677E9_C6BF_40BA_B982_3DC991AAC9DE__INCLUDED_)
#define AFX_ACTCONTROLLERPROPPAGETAB1_H__467677E9_C6BF_40BA_B982_3DC991AAC9DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ACTControllerPropPageTab1.h : header file
//

#include "Label.h"
#include "ACTControllerPropPageTab.h"

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPageTab1 dialog

class CACTControllerPropPageTab1 : public CACTControllerPropPageTab
{
// Construction
public:
	CACTControllerPropPageTab1(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CACTControllerPropPageTab1)
	enum { IDD = IDD_TAB1 };
	CComboBox	m_cActiveSliderBank;
	CComboBox	m_cActiveRotaryBank;
	CComboBox	m_cActiveButtonBank;
	CLabel	m_cButtonBankLabel;
	CLabel	m_cSliderBankLabel;
	CLabel	m_cRotaryBankLabel;
	CLabel	m_cSB8Label;
	CLabel	m_cSB7Label;
	CLabel	m_cSB6Label;
	CLabel	m_cSB5Label;
	CLabel	m_cSB4Label;
	CLabel	m_cSB3Label;
	CLabel	m_cSB2Label;
	CLabel	m_cSB1Label;
	CLabel	m_cS8Label;
	CLabel	m_cS7Label;
	CLabel	m_cS6Label;
	CLabel	m_cS5Label;
	CLabel	m_cS4Label;
	CLabel	m_cS3Label;
	CLabel	m_cS2Label;
	CLabel	m_cS1Label;
	CLabel	m_cR8Label;
	CLabel	m_cR7Label;
	CLabel	m_cR6Label;
	CLabel	m_cR5Label;
	CLabel	m_cR4Label;
	CLabel	m_cR3Label;
	CLabel	m_cR2Label;
	CLabel	m_cR1Label;
	CLabel	m_cB8Label;
	CLabel	m_cB7Label;
	CLabel	m_cB6Label;
	CLabel	m_cB5Label;
	CLabel	m_cB4Label;
	CLabel	m_cB3Label;
	CLabel	m_cB2Label;
	CLabel	m_cB1Label;
	CLabel	m_cSB8Value;
	CLabel	m_cSB8Name;
	CLabel	m_cSB7Value;
	CLabel	m_cSB7Name;
	CLabel	m_cSB6Value;
	CLabel	m_cSB6Name;
	CLabel	m_cSB5Value;
	CLabel	m_cSB5Name;
	CLabel	m_cSB4Value;
	CLabel	m_cSB4Name;
	CLabel	m_cSB3Value;
	CLabel	m_cSB3Name;
	CLabel	m_cSB2Value;
	CLabel	m_cSB2Name;
	CLabel	m_cSB1Value;
	CLabel	m_cSB1Name;
	CLabel	m_cB8Value;
	CLabel	m_cB8Name;
	CLabel	m_cB7Value;
	CLabel	m_cB7Name;
	CLabel	m_cB6Value;
	CLabel	m_cB6Name;
	CLabel	m_cB5Value;
	CLabel	m_cB5Name;
	CLabel	m_cB4Value;
	CLabel	m_cB4Name;
	CLabel	m_cB3Value;
	CLabel	m_cB3Name;
	CLabel	m_cB2Value;
	CLabel	m_cB2Name;
	CLabel	m_cB1Value;
	CLabel	m_cB1Name;
	CLabel	m_cS8Value;
	CLabel	m_cS8Name;
	CLabel	m_cS7Value;
	CLabel	m_cS7Name;
	CLabel	m_cS6Value;
	CLabel	m_cS6Name;
	CLabel	m_cS5Value;
	CLabel	m_cS5Name;
	CLabel	m_cS4Value;
	CLabel	m_cS4Name;
	CLabel	m_cS3Value;
	CLabel	m_cS3Name;
	CLabel	m_cS2Value;
	CLabel	m_cS2Name;
	CLabel	m_cS1Value;
	CLabel	m_cS1Name;
	CLabel	m_cR8Value;
	CLabel	m_cR7Value;
	CLabel	m_cR6Value;
	CLabel	m_cR5Value;
	CLabel	m_cR4Value;
	CLabel	m_cR3Value;
	CLabel	m_cR2Value;
	CLabel	m_cR1Value;
	CLabel	m_cR8Name;
	CLabel	m_cR7Name;
	CLabel	m_cR6Name;
	CLabel	m_cR5Name;
	CLabel	m_cR4Name;
	CLabel	m_cR3Name;
	CLabel	m_cR2Name;
	CLabel	m_cR1Name;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerPropPageTab1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void LoadAllFields();
	virtual void Select();
	virtual void Refresh();

protected:
	void UpdateBankCombos(bool bForce =false);
	void UpdateControllerTextBoxes(bool bForce =false);

	DWORD_PTR			m_iRotaryBank;
	DWORD_PTR			m_iSliderBank;
	DWORD_PTR			m_iButtonBank;

	CLabel *m_pBankLabel[3];

	CLabel *m_pRotaryLabel[NUM_KNOBS];
	CLabel *m_pRotaryName[NUM_KNOBS];
	CLabel *m_pRotaryValue[NUM_KNOBS];
	CLabel *m_pSliderLabel[NUM_SLIDERS];
	CLabel *m_pSliderName[NUM_SLIDERS];
	CLabel *m_pSliderValue[NUM_SLIDERS];
	CLabel *m_pButtonLabel[NUM_VIRTUAL_BUTTONS];
	CLabel *m_pButtonName[NUM_VIRTUAL_BUTTONS];
	CLabel *m_pButtonValue[NUM_VIRTUAL_BUTTONS];

	CString m_strRotaryLabel[NUM_KNOBS];
	CString m_strRotaryName[NUM_KNOBS];
	CString m_strRotaryValue[NUM_KNOBS];
	CString m_strSliderLabel[NUM_SLIDERS];
	CString m_strSliderName[NUM_SLIDERS];
	CString m_strSliderValue[NUM_SLIDERS];
	CString m_strButtonLabel[NUM_VIRTUAL_BUTTONS];
	CString m_strButtonName[NUM_VIRTUAL_BUTTONS];
	CString m_strButtonValue[NUM_VIRTUAL_BUTTONS];

	VirtualButton m_eButtonIndex[NUM_VIRTUAL_BUTTONS];

	DWORD m_dwUpdateCount;

	// Generated message map functions
	//{{AFX_MSG(CACTControllerPropPageTab1)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeActiveRotaryBank();
	afx_msg void OnSelchangeActiveSliderBank();
	afx_msg void OnSelchangeActiveButtonBank();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTCONTROLLERPROPPAGETAB1_H__467677E9_C6BF_40BA_B982_3DC991AAC9DE__INCLUDED_)
