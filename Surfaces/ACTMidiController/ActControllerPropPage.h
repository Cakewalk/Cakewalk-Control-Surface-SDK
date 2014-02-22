#if !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////

#include "Label.h"
#include "ACTControllerPropPageTabCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPage dialog

class CACTControllerPropPage :
	public CDialog,
	public IPropertyPage
{
// Construction
public:
	CACTControllerPropPage(CWnd* pParent = NULL);   // standard constructor
	~CACTControllerPropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CACTControllerPropPage)
	enum { IDD = IDD_PROPPAGE };
	CButton	m_cMidiLearnShift;
	CACTControllerPropPageTabCtrl	m_cTabCtrl;
	CLabel	m_cACTName;
	CButton	m_cACTLock;
	CButton	m_cACTEnable;
	//}}AFX_DATA

	// *** IPropertyPage methods ***
	STDMETHODIMP_(HRESULT)	SetPageSite(LPPROPERTYPAGESITE pPageSite);
	STDMETHODIMP_(HRESULT)	Activate(HWND hwndParent, LPCRECT prect, BOOL fModal);
	STDMETHODIMP_(HRESULT)	Deactivate(void);
	STDMETHODIMP_(HRESULT)	GetPageInfo(LPPROPPAGEINFO pPageInfo);
	STDMETHODIMP_(HRESULT)	SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
	STDMETHODIMP_(HRESULT)	Show(UINT nCmdShow);
	STDMETHODIMP_(HRESULT)	Move(LPCRECT prect);
	STDMETHODIMP_(HRESULT)	IsPageDirty(void);
	STDMETHODIMP_(HRESULT)	Apply(void);
	STDMETHODIMP_(HRESULT)	Help(LPCWSTR lpszHelpDir);
	STDMETHODIMP_(HRESULT)	TranslateAccelerator(LPMSG lpMsg);

	// *** IPlugInPropPageMFC methods ***
	// Implemented in ACTControllerGen.cpp
	STDMETHODIMP_(BOOL)	CreateMFCDialog( HWND hwndParent );
	STDMETHODIMP_(BOOL)	DestroyMFCDialog();

	// *** IUnknown methods ***
	// Implemented in ACTControllerGen.cpp
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)	AddRef( void );
	STDMETHODIMP_(ULONG)	Release( void );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACTControllerPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Implementation
private:
	LONG				m_cRef;
	CACTController*		m_pSurface;
	IPropertyPageSite*	m_pPageSite;
	BOOL				m_bDirty;

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void StartTimer();
	void StopTimer();

	void LoadAllFields();

	void UpdateACTStatus(bool bForce =false);
	void GreyACTFields();

	void UpdateGroupStatus(bool bForce =false);
	void UpdateRotariesMode(bool bForce =false);

	bool m_bInitDone;

	UINT_PTR m_uiTimerID;

	CString m_strACTName;
	bool m_bACTEnabled;
	bool m_bACTLocked;

	SONAR_MIXER_STRIP m_eStripType;
	AssignmentMode m_eRotariesMode;

	// Generated message map functions
	//{{AFX_MSG(CACTControllerPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnACTEnable();
	afx_msg void OnACTLock();
	afx_msg void OnGroupTrack();
	afx_msg void OnGroupBus();
	afx_msg void OnGroupMain();
	afx_msg void OnMultiChannel();
	afx_msg void OnChannelStrip();
	afx_msg void OnMidiLearnShift();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
