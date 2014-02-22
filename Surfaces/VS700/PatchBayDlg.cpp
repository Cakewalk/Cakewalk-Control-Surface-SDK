// PatchBayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatchBayDlg.h"
#include "..\..\CakeControls\OffscreenDC.h"
#include "..\..\CakeControls\AutoDCRestorer.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define PATCHBAYSELTIMER 128

// CPatchBayDlg dialog

IMPLEMENT_DYNAMIC(CPatchBayDlg, CDialogEx)

CPatchBayDlg::CPatchBayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPatchBayDlg::IDD, pParent),
	m_pSurfaceBay( NULL ),
	m_pBay(this)
{
	// Load background bitmap
	HBITMAP hBmp = (HBITMAP)::LoadImage( ::AfxGetResourceHandle(), MAKEINTRESOURCE( IDB_BAY_BKGD ), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
	VERIFY( m_bmFace.Attach( hBmp ));

}

CPatchBayDlg::~CPatchBayDlg()
{
	VERIFY( m_bmFace.DeleteObject() );
	m_pSurfaceBay->Release();
}

void CPatchBayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO1, m_cOUT12);
	DDX_Control(pDX, IDC_COMBO2, m_cOUT34);
	DDX_Control(pDX, IDC_COMBO3, m_cOUT56);
	DDX_Control(pDX, IDC_COMBO4, m_cOUT78);
	DDX_Control(pDX, IDC_COMBO5, m_cOUT910);
	DDX_Control(pDX, IDC_MONITOR_BTN, m_cMonitor);
	DDX_Control(pDX, IDC_SETTING_BTN, m_cSetting);
	DDX_Control(pDX, IDOK, m_cOK);


}


static CString s_aMIXOUT[] ={_T("DIRECT MIX A"), _T("DIRECT MIX B"),_T("DIRECT MIX C"),_T("DIRECT MIX D"),_T("WAVE OUT 1-2"),_T("WAVE OUT 3-4"),_T("WAVE OUT 5-6"),_T("WAVE OUT 7-8"),_T("WAVE OUT 9-10"), };

BEGIN_MESSAGE_MAP(CPatchBayDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPatchBayDlg::OnCbnSelchangeOUT12)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO2, &CPatchBayDlg::OnCbnSelchangeOUT34)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CPatchBayDlg::OnCbnSelchangeOUT56)
	ON_CBN_SELCHANGE(IDC_COMBO4, &CPatchBayDlg::OnCbnSelchangeOUT78)
	ON_CBN_SELCHANGE(IDC_COMBO5, &CPatchBayDlg::OnCbnSelchangeOUT910)
	ON_BN_CLICKED(IDC_SETTING_BTN, &CPatchBayDlg::OnBnClickedSettingBtn)
	ON_BN_CLICKED(IDC_MONITOR_BTN, &CPatchBayDlg::OnBnClickedMonitorBtn)
	ON_WM_TIMER()



END_MESSAGE_MAP()

void CPatchBayDlg::OnPaint()
{
	//AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	CPaintDC dc(this); // device context for painting

	CRect rcClient;
	GetClientRect( &rcClient );

	// Flicker-free updating
	//CMemDC dcMem( &dc, NULL );
	//CAutoDCRestorer adcMem( &dcMem, CAutoDCRestorer::eUSUAL );

	// Paint bitmap background
	CDC dcFace;
	VERIFY( dcFace.CreateCompatibleDC( &dc ) );
	CAutoDCRestorer adcFace( &dcFace, CAutoDCRestorer::eUSUAL );
	CBitmap* pBmpold = dcFace.SelectObject( &m_bmFace );
	SetBkMode(dcFace, TRANSPARENT);


	dc.BitBlt( rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &dcFace, 2, 2, SRCCOPY );
	dcFace.SelectObject( pBmpold );
	
}

// CPatchBayDlg message handlers
BOOL CPatchBayDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
		
	KillTimer(PATCHBAYSELTIMER); // turn off timer

	BITMAP bmInfo; 
	m_bmFace.GetBitmap( &bmInfo );
	int nHeight = bmInfo.bmHeight + GetSystemMetrics( SM_CYCAPTION );
	SetWindowPos( NULL, 0, 0, bmInfo.bmWidth, nHeight, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE );

	m_cMonitor.SetWindowPos( NULL, 120, 12, 80, 20, SWP_NOZORDER );
	m_cSetting.SetWindowPos( NULL, 207, 12, 80, 20, SWP_NOZORDER );

	m_cOUT12.SetWindowPos( NULL, 154, 52, 120, 18, SWP_NOZORDER );
	m_cOUT34.SetWindowPos( NULL, 154, 82, 120, 18, SWP_NOZORDER );
	m_cOUT56.SetWindowPos( NULL, 154, 112, 120, 18, SWP_NOZORDER );
	m_cOUT78.SetWindowPos( NULL, 154, 142, 120, 18, SWP_NOZORDER );
	m_cOUT910.SetWindowPos( NULL, 154, 172, 120, 18, SWP_NOZORDER );
	//Close
	m_cOK.SetWindowPos( NULL, 185, 215, 80, 22, SWP_NOZORDER );

	//Grab a Ref for the Surface Poiner incase someone elses releases it.  Release on Destruct.
	m_pSurfaceBay->AddRef();
	
	fillMixOutCombos();

	// now start the update timer.
	SetTimer(PATCHBAYSELTIMER, 100, NULL); // update COMBO Boxes while dlg is open


	return TRUE;
}

//-------------------------------------------------------------------------
void CPatchBayDlg::fillMixOutCombos()
{
	m_cOUT12.ResetContent();
	m_cOUT34.ResetContent();
	m_cOUT56.ResetContent();
	m_cOUT78.ResetContent();
	m_cOUT910.ResetContent();

	for ( size_t i = 0; i <= 9; i++ )
	{
		m_cOUT12.AddString( s_aMIXOUT[i] );
		m_cOUT34.AddString( s_aMIXOUT[i] );
		m_cOUT56.AddString( s_aMIXOUT[i] );
		m_cOUT78.AddString( s_aMIXOUT[i] );
		m_cOUT910.AddString( s_aMIXOUT[i] );
	}
	
	
	//Set Selected to what the surface remembers.  Default Setting First Launch.
	m_cOUT12.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12);
	m_cOUT34.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34);
	m_cOUT56.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56);
	m_cOUT78.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78);
	m_cOUT910.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910);
	
	int iSel = m_cOUT12.GetCurSel();
	
	if (iSel == -1 )
		SetDefaults();

}
//--------------------------------------------------------------------

// CPatchBayDlg message handlers


void CPatchBayDlg::OnDestroy() 
{
	//AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CDialogEx::OnDestroy();

}

void  CPatchBayDlg::SetDefaults()
{
		m_cOUT12.SetCurSel(DA);
		m_cOUT34.SetCurSel(OUT34);
		m_cOUT56.SetCurSel(OUT56);
		m_cOUT78.SetCurSel(OUT78);
		m_cOUT910.SetCurSel(OUT910);
}

void  CPatchBayDlg::SetMonitor()
{
		m_cOUT12.SetCurSel(DA);
		m_cOUT34.SetCurSel(DB);
		m_cOUT56.SetCurSel(DC);
		m_cOUT78.SetCurSel(DD);
		m_cOUT910.SetCurSel(DA);
}



void CPatchBayDlg::OnCbnSelchangeOUT12()
{

	// TODO: Add your control notification handler code here
	int iSel = m_cOUT12.GetCurSel();

	switch ( iSel )
	{
	case DA:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)DA );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = DA;
		}
		break;
	case DB:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)DB );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = DB;
		}
		break;
	case DC:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)DC );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = DC;
		}
		break;
	case DD:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)DD );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = DD;
		}
		break;
	case OUT12:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)OUT12 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = OUT12;
		}
	
		break;
	case OUT34:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)OUT34 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = OUT34;
		}
		break;
	case OUT56:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)OUT56 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = OUT56;
		}
		break;
	case OUT78:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)OUT78 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = OUT78;
		}
		break;
	case OUT910:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT12, (float)OUT910 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12 = OUT910;
		}
		break;

	default:
		break;
	}


}

void CPatchBayDlg::OnCbnSelchangeOUT34()
{
	// TODO: Add your control notification handler code here

	int iSel = m_cOUT34.GetCurSel();

	switch ( iSel )
	{
	case DA:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)DA );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = DA;
		
		}
		break;
	case DB:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)DB );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = DB;

		}
		break;
	case DC:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)DC );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = DC;
		}
		break;
	case DD:
		{m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)DD );
		m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = DD;
		}
		break;
	case OUT12:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)OUT12 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = OUT12;

		}
		break;
	case OUT34:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)OUT34 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = OUT34;
		}
		break;
	case OUT56:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)OUT56 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = OUT56;
		}
		break;
	case OUT78:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)OUT78 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = OUT78;
		}
		break;
	case OUT910:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT34, (float)OUT910 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34 = OUT910;
		}
		break;

	default:
		break;
	}


}

void CPatchBayDlg::OnCbnSelchangeOUT56()
{
	// TODO: Add your control notification handler code here
	int iSel = m_cOUT56.GetCurSel();

	switch ( iSel )
	{
	case DA:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)DA );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = DA;
		}
		break;
	case DB:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)DB );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = DB;
		}
		break;
	case DC:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)DC );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = DC;

		}
		break;
	case DD:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)DD );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = DD;
		}
		break;
	case OUT12:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)OUT12 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = OUT12;
		}
		break;
	case OUT34:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)OUT34 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = OUT34;
		}
		break;
	case OUT56:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)OUT56 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = OUT56;
		}
		break;
	case OUT78:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)OUT78 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = OUT78;
		}
		break;
	case OUT910:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT56, (float)OUT910 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56 = OUT910;
		}
		break;

	default:
		break;
	}


}


void CPatchBayDlg::OnCbnSelchangeOUT78()
{
	// TODO: Add your control notification handler code here
	int iSel = m_cOUT78.GetCurSel();

	switch ( iSel )
	{
	case DA:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)DA );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = DA;
		}
		break;
	case DB:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)DB );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = DB;

		}
		break;
	case DC:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)DC );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = DC;
		}
		break;
	case DD:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)DD );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = DD;
		}
		break;
	case OUT12:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)OUT12 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = OUT12;

		}
		break;
	case OUT34:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)OUT34 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = OUT34;
		}
		break;
	case OUT56:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)OUT56 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = OUT56;
		}
		break;
	case OUT78:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)OUT78 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = OUT78;
		}
		break;
	case OUT910:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT78, (float)OUT910 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78 = OUT910;
		}
		break;

	default:
		break;
	}


}



void CPatchBayDlg::OnCbnSelchangeOUT910()
{
	// TODO: Add your control notification handler code here
	int iSel = m_cOUT910.GetCurSel();

	switch ( iSel )
	{
	case DA:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)DA );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = DA;
		}
		break;
	case DB:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)DB );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = DB;

		}
		break;
	case DC:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)DC );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = DC;
		}
		break;
	case DD:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)DD );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = DA;
		}
		break;
	case OUT12:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)OUT12 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = OUT12;
		}
		break;
	case OUT34:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)OUT34 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = OUT34;
		}
		break;
	case OUT56:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)OUT56 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = OUT56;
		}
		break;
	case OUT78:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)OUT78 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = OUT78;
		}
		break;
	case OUT910:
		{
			m_pSurfaceBay->GetIOBoxInterface()->SetParam( 0, (TacomaIOBoxParam) TIOP_OUT910, (float)OUT910 );
			m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910 = OUT910;
		}
		break;

	default:
		break;
	}


}



void CPatchBayDlg::OnBnClickedSettingBtn()
{
	// TODO: Add your control notification handler code here

	SetDefaults();
	OnCbnSelchangeOUT12();
	OnCbnSelchangeOUT34();
	OnCbnSelchangeOUT56();
	OnCbnSelchangeOUT78();
	OnCbnSelchangeOUT910();
}


void CPatchBayDlg::OnBnClickedMonitorBtn()
{
	// TODO: Add your control notification handler code here
	SetMonitor();
	OnCbnSelchangeOUT12();
	OnCbnSelchangeOUT34();
	OnCbnSelchangeOUT56();
	OnCbnSelchangeOUT78();
	OnCbnSelchangeOUT910();
}

// The sole purpose of this timer is to sniff for a pending PatchBay MIDI changes 

void CPatchBayDlg::OnTimer(UINT_PTR uid)
{
	if ( PATCHBAYSELTIMER == uid )
	{
		if ( m_pSurfaceBay->GetIOBoxInterface()->m_bMidiPatch == true )
		{
			if ((m_cOUT12.GetCurSel()) != (m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12))
			m_cOUT12.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT12);

			if ((m_cOUT34.GetCurSel()) != (m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34))
			m_cOUT34.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT34);
	
			if ((m_cOUT56.GetCurSel()) != (m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56))
			m_cOUT56.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT56);
	
			if ((m_cOUT78.GetCurSel()) != (m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78))
			m_cOUT78.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT78);
	
			if ((m_cOUT910.GetCurSel()) != (m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910))
			m_cOUT910.SetCurSel(m_pSurfaceBay->GetIOBoxInterface()->m_iOUT910);

			UpdateData(FALSE);
		}

		m_pSurfaceBay->GetIOBoxInterface()->m_bMidiPatch = false;

	}
}