// RegReminderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TacomaSurface.h"
#include "RegReminderDlg.h"


// CRegReminderDlg dialog

IMPLEMENT_DYNAMIC(CRegReminderDlg, CDialog)

CRegReminderDlg::CRegReminderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegReminderDlg::IDD, pParent)
{

}

CRegReminderDlg::~CRegReminderDlg()
{
}

void CRegReminderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRegReminderDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegReminderDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRegReminderDlg message handlers

static LPCTSTR s_szRegAppKey = _T("SOFTWARE\\Cakewalk Music Software\\SONAR Producer\\8.0");
static LPCTSTR s_szRegUrlVal = _T("RegUrl");

void CRegReminderDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	// get the registration URL 
	HKEY hk = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, s_szRegAppKey, 0, KEY_READ, &hk ) )
	{

	}
	else
	{
		TCHAR sz[200];
		DWORD dwType = 0;
		DWORD dwcbData = sizeof(sz);
		::RegQueryValueEx( hk, s_szRegUrlVal, NULL, &dwType, (BYTE*)sz, &dwcbData );

		CString strUrl;
		strUrl.Format( _T("%s&vs700=Y"), sz );
		::ShellExecute( m_hWnd,  NULL, strUrl, NULL, NULL, 0 );
	}



	OnOK();
}
