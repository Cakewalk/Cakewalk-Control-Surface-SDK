// ControlSurfacePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ControlSurfacePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CControlSurfacePropPage dialog


CControlSurfacePropPage::CControlSurfacePropPage(CWnd* pParent /*=NULL*/)
	:	COlePropertyPage(CControlSurfacePropPage::IDD, IDS_NAME_SURFACE)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	//{{AFX_DATA_INIT(CControlSurfacePropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

#if _DEBUG
	// Turn off obnoxious "non-standard size" warnings from MFC.  At least
	// they gave us a flag to do this!
	m_bNonStandardSize = TRUE;
#endif
}

/////////////////////////////////////////////////////////////////////////////

BOOL CControlSurfacePropPage::OnInitDialog()
{
	// init the help system				
	CString strPath;
	char szDllPath[_MAX_PATH];

	MakeExePathName( AfxGetInstanceHandle(), szDllPath, _MAX_PATH );
	strPath = szDllPath;
	m_HelpHelper.Init( AfxGetInstanceHandle(), GetSafeHwnd(), strPath + "\\" + SZAPPHELPFILE );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfacePropPage::OnObjectsChanged()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Release old ICSProperties
	if (m_spICSProperties)
		m_spICSProperties = NULL;

	// Look for a new IFilter
	ULONG cObj = 0;
	IDispatch** apIDisp = GetObjectArray( &cObj );
	for (ULONG i = 0; i < cObj; ++i)
	{
		if (S_OK == apIDisp[i]->QueryInterface( IID_ICSProperties, (void**)&m_spICSProperties ))
			break;
	}

	// Update controls if we've got a new object and we're activated
	if (m_spICSProperties && ::IsWindow( m_hWnd ))
		UpdateData( FALSE );
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfacePropPage::OnDestroy()
{
	m_HelpHelper.Terminate();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CControlSurfacePropPage::OnHelp(LPCTSTR str)
{
	m_HelpHelper.Launch( HELP_FINDER, 0 );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CControlSurfacePropPage::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CControlSurfacePropPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CControlSurfacePropPage, CDialog)
	//{{AFX_MSG_MAP(CControlSurfacePropPage)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlSurfacePropPage message handlers
