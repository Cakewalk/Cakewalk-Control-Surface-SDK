// MidiInitializationMsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ACTController.h"
#include "MidiInitializationMsgDlg.h"
#include "utils.h"


// CMidiInitializationMsgDlg dialog

IMPLEMENT_DYNAMIC(CMidiInitializationMsgDlg, CDialog)

CMidiInitializationMsgDlg::CMidiInitializationMsgDlg(CACTController* pSurface, CWnd* pParent /*=NULL*/)
	: CDialog(CMidiInitializationMsgDlg::IDD, pParent),
	m_pSurface( pSurface )
{

}

CMidiInitializationMsgDlg::~CMidiInitializationMsgDlg()
{
}

void CMidiInitializationMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATUS, m_cStatus);
	DDX_Control(pDX, IDC_NUM, m_cNum);
	DDX_Control(pDX, IDC_VALUE, m_cValue);
	DDX_Control(pDX, IDC_SYSEX_STRING, m_cSysex);

	BYTE byStatus = 0;
	BYTE byNum = 0;
	BYTE byVal = 0;
	DWORD dwMsg = 0;
	CString str;

	if ( pDX->m_bSaveAndValidate )
	{
		// from the cells
		m_cStatus.GetWindowText( str );
		if ( !str.IsEmpty() )
		{
			m_vdwShortMsg.clear();
			byStatus = (BYTE)::_tcstoul( str, 0, 16 );
			if ( byStatus )
			{
				m_cNum.GetWindowText( str );
				byNum = (BYTE)::_tcstoul( str, 0, 16 );
				m_cValue.GetWindowText( str );
				byVal = (BYTE)::_tcstoul( str, 0, 16 );

				dwMsg = byStatus | (byNum << 8) | (byVal << 16);
				m_vdwShortMsg.push_back( dwMsg );
			}
		}
		else
			m_vdwShortMsg.clear();

		m_cSysex.GetWindowText( str );
		StringToSysex( str, &m_vbyLongMsg );
	}
	else
	{
		// to the cells
		if ( !m_vdwShortMsg.empty() )
		{
			dwMsg = m_vdwShortMsg[0];
			byStatus = (BYTE)(dwMsg & 0xFF);
			byNum = (BYTE)((dwMsg >> 8) & 0xFF);
			byVal = (BYTE)((dwMsg >> 16) & 0xFF);

			str.Format( _T("%02x"), byStatus );
			m_cStatus.SetWindowText( str );

			str.Format( _T("%02x"), byNum );
			m_cNum.SetWindowText( str );

			str.Format( _T("%02x"), byVal );
			m_cValue.SetWindowText( str );
		}

		str = "";
		SysexToString( m_vbyLongMsg, &str );
		m_cSysex.SetWindowText( str );
		m_cSysex.EnableWindow( m_vbyLongMsg.size() < 255 );
	}
}


BEGIN_MESSAGE_MAP(CMidiInitializationMsgDlg, CDialog)
	ON_BN_CLICKED(IDC_LOAD_SYSEX, &CMidiInitializationMsgDlg::OnBnClickedLoadSysex)
	ON_BN_CLICKED(IDC_CLEAR_SYSEX, &CMidiInitializationMsgDlg::OnBnClickedClearSysex)
END_MESSAGE_MAP()


// CMidiInitializationMsgDlg message handlers

BOOL CMidiInitializationMsgDlg::OnInitDialog()
{
	m_pSurface->XFerInitShortMsgs( &m_vdwShortMsg, false );
	m_pSurface->XferInitLongMsgs( &m_vbyLongMsg, false );

	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMidiInitializationMsgDlg::OnOK()
{
	CDialog::OnOK();

	m_pSurface->XFerInitShortMsgs( &m_vdwShortMsg, true );
	m_pSurface->XferInitLongMsgs( &m_vbyLongMsg, true );
}


void CMidiInitializationMsgDlg::OnBnClickedLoadSysex()
{
	static TCHAR s_szFilter[ 256 ];
	int const nCount = ::LoadString( ::AfxGetResourceHandle(), IDS_SYSX_OFN_LIST, s_szFilter, 256 );

	static const TCHAR cNullMarker = _T('^');

	int nSubCount = 0;		// Counter for substrings within resource string
	int n;
	for (n=0; n < nCount; ++n)
	{
		// Look for characters to replace. . .
		if (s_szFilter[ n ] == cNullMarker || s_szFilter[ n ] == 0)
		{
			// . . . and turn them into \0 chars
			s_szFilter[n] = _T('\0');
			++nSubCount;
		}
	}

	// Make sure it's terminated with a double \0:
	if (s_szFilter[ n - 1 ] != 0)
	{
		s_szFilter[++n] = _T('\0');
		++nSubCount;
	}


	CFileDialog dlg( TRUE );
	dlg.m_ofn.lpstrFilter = s_szFilter;
	if (IDOK == dlg.DoModal())
	{
		CString strPathName = dlg.GetPathName();
		CFile file;	// Might as well use Raw file: single Read() call
		if (file.Open( strPathName, CFile::modeRead ))
		{
			// Get file length
			file.Seek( 0, CFile::end );
			DWORD const dwLen = static_cast<DWORD>( file.GetPosition() ) - 2;
			file.Seek( 1, CFile::begin );
			m_vbyLongMsg.resize( dwLen );
			file.Read( &(m_vbyLongMsg[0]), dwLen );
			m_pSurface->XferInitLongMsgs( &m_vbyLongMsg, true );
			UpdateData( FALSE );
		}
	}
}

void CMidiInitializationMsgDlg::OnBnClickedClearSysex()
{
	m_vbyLongMsg.clear();
	m_pSurface->XferInitLongMsgs( &m_vbyLongMsg, true );
	UpdateData( FALSE );
}
