#pragma once


// CRegReminderDlg dialog

class CRegReminderDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegReminderDlg)

public:
	CRegReminderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegReminderDlg();

// Dialog Data
	enum { IDD = IDD_REG_REMINDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
