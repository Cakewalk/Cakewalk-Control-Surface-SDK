#pragma once
#include "afxwin.h"
#include "TacomaSurface.h"


// CButtonPropsDlg dialog

class CButtonPropsDlg : public CDialog
{
	DECLARE_DYNAMIC(CButtonPropsDlg)

public:
	CButtonPropsDlg( CTacomaSurface* pSurface, CTacomaSurface::ButtonActionDefinition& bad, CWnd* pParent = NULL);   // standard constructor
	virtual ~CButtonPropsDlg();

// Dialog Data
	enum { IDD = IDD_BUTTON_PROPS };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CTacomaSurface*						m_pSurface;

	void		enableControls();



public:
	CComboBox m_cCmdList;
	CComboBox m_cTransportList;
	CButton m_cShift;
	CButton m_cControl;
	CButton m_cAlt;
	CComboBox m_cKeyList;	

	CTacomaSurface::ButtonActionDefinition		m_bad;
	afx_msg void OnCbnSelchangeCommandlist();
	afx_msg void OnCbnSelchangeTransportlist();
	afx_msg void OnBnClickedLearnkey();
	afx_msg void OnBnClickedRadioCommand();
	afx_msg void OnBnClickedRadioKey();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedShift();
	afx_msg void OnBnClickedCtrl();
	afx_msg void OnBnClickedAlt();
	afx_msg void OnCbnSelchangeKeylist();
	afx_msg void OnBnClickedRadio3();
};
