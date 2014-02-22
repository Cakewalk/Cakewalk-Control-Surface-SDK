#pragma once
#include "afxwin.h"


class CACTController;


// CMidiInitializationMsgDlg dialog

class CMidiInitializationMsgDlg : public CDialog
{
	DECLARE_DYNAMIC(CMidiInitializationMsgDlg)

public:
	CMidiInitializationMsgDlg(CACTController* pSurface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMidiInitializationMsgDlg();

// Dialog Data
	enum { IDD = IDD_MIDI_INIT };

public:
	CEdit m_cStatus;
	CEdit m_cNum;
	CEdit m_cValue;
	CEdit m_cSysex;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CACTController* m_pSurface;
	std::vector<DWORD>	m_vdwShortMsg;
	std::vector<BYTE>		m_vbyLongMsg;

public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	afx_msg void OnBnClickedLoadSysex();
public:
	afx_msg void OnBnClickedClearSysex();
};
