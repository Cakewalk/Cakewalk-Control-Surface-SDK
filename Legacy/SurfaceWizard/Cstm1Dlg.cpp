// cstm1dlg.cpp : implementation file
//

#include "stdafx.h"
#include "SurfaceWizard.h"
#include "cstm1dlg.h"
#include "SurfaceWizardaw.h"
#include "paint.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog


CCustom1Dlg::CCustom1Dlg()
	: CAppWizStepDlg(CCustom1Dlg::IDD)
{
	//{{AFX_DATA_INIT(CCustom1Dlg)
	m_strFriendlyName = _T("");
	//}}AFX_DATA_INIT
}


void CCustom1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom1Dlg)
	DDX_Text(pDX, IDC_FRIENDLYNAME, m_strFriendlyName);
	//}}AFX_DATA_MAP
}

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom1Dlg::OnDismiss()
{
	if (!UpdateData(TRUE))
		return FALSE;
    else 
	{
		if (!m_strFriendlyName.IsEmpty())
			SurfaceWizardaw.m_Dictionary["friendly_name"]=m_strFriendlyName;		
		else 
		{
			AfxMessageBox(IDP_NONAME);
			GetDlgItem( IDC_FRIENDLYNAME )->SetFocus();
			return FALSE;
		}

		AddGuidSymbol( "clsid_surface" );
		AddGuidSymbol( "clsid_surface_proppage" );
		AddGuidSymbol( "libid_surface" );
	}
	return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}


BEGIN_MESSAGE_MAP(CCustom1Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom1Dlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg message handlers

BOOL CCustom1Dlg::OnInitDialog() 
{
	m_strFriendlyName = SurfaceWizardaw.m_Dictionary["root"] + " Control Surface";
	CAppWizStepDlg::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg additional utility functions

void CCustom1Dlg::AddGuidSymbol( const TCHAR* pszSymbol )
{
	GUID guid;
	::CoCreateGuid( &guid );
	if (guid == GUID_NULL)
	{
		AfxMessageBox(IDP_ERR_CREATE_GUID);
		EndDialog(IDABORT);
	}

	TCHAR			sz[ 128 ];
	const char*	pszFmt = "{ 0x%08x, 0x%04x, 0x%04x, { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x } }";

	wsprintf( sz, pszFmt, (DWORD)guid.Data1, (DWORD)guid.Data2, (DWORD)guid.Data3,
				 (DWORD)guid.Data4[0], (DWORD)guid.Data4[1],
				 (DWORD)guid.Data4[2], (DWORD)guid.Data4[3],
				 (DWORD)guid.Data4[4], (DWORD)guid.Data4[5],
				 (DWORD)guid.Data4[6], (DWORD)guid.Data4[7] );

	SurfaceWizardaw.m_Dictionary[ pszSymbol ] = sz;
}

////////////////////////////////////////////////////////////////////////////////

#define STEP1_LEFT			15
#define STEP1_TOP			45
#define STEP1_WIDTH			179
#define STEP1_HEIGHT		180

void CCustom1Dlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackground(&dc, this);	

	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&dc))
		return;

	// Picture
//	PaintBitmap(IDB_CAKEWALK, STEP1_LEFT, STEP1_TOP, STEP1_WIDTH, STEP1_HEIGHT, &dc, &dcMem);

	// Do not call CAppWizStepDlg::OnPaint() for painting messages
}
