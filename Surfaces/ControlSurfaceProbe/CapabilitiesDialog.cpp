// CapabilitiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ControlSurfaceProbe.h"
#include "CapabilitiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCapabilitiesDialog dialog


CCapabilitiesDialog::CCapabilitiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCapabilitiesDialog::IDD, pParent),
	m_pIdentity( NULL)
{
	//{{AFX_DATA_INIT(CCapabilitiesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCapabilitiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCapabilitiesDialog)
	DDX_Control(pDX, IDC_CAPABILITIES, m_cCapabilities);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCapabilitiesDialog, CDialog)
	//{{AFX_MSG_MAP(CCapabilitiesDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapabilitiesDialog message handlers

BOOL CCapabilitiesDialog::OnInitDialog() 
{
	BOOL bRet = CDialog::OnInitDialog();

	if (m_pIdentity)
	{
		CheckCap("CAP_FLEX_BUSES",		CAP_FLEX_BUSES);
		CheckCap("CAP_SURROUND_BUSES",	CAP_SURROUND_BUSES);
		CheckCap("CAP_SURROUND_TRACKS",	CAP_SURROUND_TRACKS);
		CheckCap("CAP_AUDIO_METERS",	CAP_AUDIO_METERS);
		CheckCap("CAP_MIDI_METERS",		CAP_MIDI_METERS);
		CheckCap("CAP_INPUT_ECHO",		CAP_INPUT_ECHO);
		CheckCap("CAP_PARAM_ANY",		CAP_PARAM_ANY);
		CheckCap("CAP_STRIP_ANY",		CAP_STRIP_ANY);
		CheckCap("CAP_STRIP_FILTER",	CAP_STRIP_FILTER);
		CheckCap("CAP_LAYER_MUTESOLO",	CAP_LAYER_MUTESOLO);
	}
	else
	{
		m_strText = "m_pIdentity is NULL";
	}

	m_cCapabilities.SetWindowText(m_strText);
	
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////

void CCapabilitiesDialog::CheckCap(const char *pszName, HOST_CAPABILITY eCap)
{
	if (!m_pIdentity)
		return;

	HRESULT hr = m_pIdentity->HasCapability(eCap);

	CString strResult;

	strResult.Format(_T("%s: "), pszName);

	if (FAILED(hr))
		strResult += _T("HasCapability FAILED");
	else
		strResult += (hr == S_OK) ? _T("Yes") : _T("No");

	m_strText += strResult;
	m_strText += _T("\r\n");
}

/////////////////////////////////////////////////////////////////////////////
