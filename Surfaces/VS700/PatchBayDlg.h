#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "resource.h"
#include "TacomaSurface.h"
#include "..\CakeControls\NWBitmapButton.h"
#include "..\CakeControls\NWDropDownCtrl.h"

// CPatchBayDlg dialog

class CPatchBayDlg : public  CDialogEx
{
	DECLARE_DYNAMIC(CPatchBayDlg)

public:
	CPatchBayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPatchBayDlg();

// Dialog Data
	enum { IDD = IDD_PATCHBAY };

	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	std::vector<CString>		m_vstrOutput;
	CString						m_str;
	CButton						m_cMonitor;
	CButton						m_cSetting;
	CButton						m_cOK;
// Implementation
private:
	CBitmap						m_bmFace;
public:
	void fillMixOutCombos();
	void SetDefaults();
	void SetMonitor();
	void SetOut12();
	CComboBox					m_cOUT12;
	CComboBox					m_cOUT34;
	CComboBox					m_cOUT56;
	CComboBox					m_cOUT78;
	CComboBox					m_cOUT910;



enum MixOut
{
	DA,
	DB,
	DC,
	DD,
	OUT12,
	OUT34,
	OUT56,
	OUT78,
	OUT910,

	OUTMAX
};

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnCbnSelchangeOUT12();
	afx_msg void OnCbnSelchangeOUT34();
	afx_msg void OnCbnSelchangeOUT56();
	afx_msg void OnCbnSelchangeOUT78();
	afx_msg void OnCbnSelchangeOUT910();
	afx_msg void OnTimer(UINT_PTR);

	CTacomaSurface*		m_pSurfaceBay;
	CPatchBayDlg*		m_pBay;
	afx_msg void OnBnClickedSettingBtn();
	afx_msg void OnBnClickedMonitorBtn();
};
