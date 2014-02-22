#pragma once
#include "afxwin.h"


// CCellMidiPropsDlg dialog


class CACTController;



class CCellMidiPropsDlg : public CDialog
{
	DECLARE_DYNAMIC(CCellMidiPropsDlg)

public:
	CCellMidiPropsDlg(CACTController* pSurface, int ixCell, CWnd* pParent = NULL);   // standard constructor
	virtual ~CCellMidiPropsDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CELL_MIDI_PROPS };
public:
	afx_msg void OnBnClickedUseAccel();
	afx_msg void OnBnClickedRadInterpret();
	afx_msg void OnBnClickedRad1Interpret();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void	enableControls();

	DECLARE_MESSAGE_MAP()

	CACTController*	m_pSurface;
	int					m_ixCell;

	CButton m_cUseAccel;
	CEdit m_cHinge;
	CEdit m_cStatus;
	CEdit m_cNum;
	CEdit m_cSysex;
	BOOL m_bUseAccel;
	int m_ixInterpret;
	virtual void OnOK();

	int m_ixKnob;
	int m_ixSlider;
	int m_ixSwitch;
	DWORD m_dwHinge;
	CString m_strStatus;
	CString m_strNumber;
	CString m_strSysex;

};
