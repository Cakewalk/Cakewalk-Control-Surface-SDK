#if !defined(AFX_MackieControlMasterPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlMasterPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CMackieControlMasterPropPage dialog

class CMackieControlMasterPropPage : public CDialog,
									public IPropertyPage
{
// Construction
public:
	CMackieControlMasterPropPage(CWnd* pParent = NULL);   // standard constructor
	~CMackieControlMasterPropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CMackieControlMasterPropPage)
	enum { IDD = IDD_PROPPAGE_MASTER };
	CComboBox	m_cMeters;
	CComboBox	m_cVirtualMainType;
	CButton	m_cSelectHighlightsTrack;
	CButton	m_cFaderSelectsChannel;
	CButton	m_cDisableRelayClick;
	CButton	m_cDisableLCD;
	CButton	m_cConfigureLayout;
	CComboBox	m_cVirtualMain;
	CButton	m_cSoloSelectsChannel;
	CButton	m_cDisableFaders;
	CComboBox	m_cFootSwitchB;
	CComboBox	m_cFootSwitchA;
	CComboBox	m_cFunction8;
	CComboBox	m_cFunction7;
	CComboBox	m_cFunction6;
	CComboBox	m_cFunction5;
	CComboBox	m_cFunction4;
	CComboBox	m_cFunction3;
	CComboBox	m_cFunction2;
	CComboBox	m_cFunction1;
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
	// Implemented in MackieControlMasterGen.cpp
	STDMETHODIMP_(BOOL)	CreateMFCDialog( HWND hwndParent );
	STDMETHODIMP_(BOOL)	DestroyMFCDialog();

	// *** IUnknown methods ***
	// Implemented in MackieControlMasterGen.cpp
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)	AddRef( void );
	STDMETHODIMP_(ULONG)	Release( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMackieControlMasterPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
// Implementation
private:
	LONG						m_cRef;
	CMackieControlMaster*		m_pSurface;
	IPropertyPageSite*			m_pPageSite;
	BOOL						m_bDirty;

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void TransferSettings(bool bSave);
	void SelectItemData(CComboBox *pBox, DWORD dwData);
	void FillVirtualMainCombo();

	bool m_bInitDone;
	JogResolution m_eJogResolution;
	JogResolution m_eTransportResolution;
	bool m_bDisplaySMPTE;

	// Generated message map functions
	//{{AFX_MSG(CMackieControlMasterPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFunction1();
	afx_msg void OnSelchangeFunction2();
	afx_msg void OnSelchangeFunction3();
	afx_msg void OnSelchangeFunction4();
	afx_msg void OnSelchangeFunction5();
	afx_msg void OnSelchangeFunction6();
	afx_msg void OnSelchangeFunction7();
	afx_msg void OnSelchangeFunction8();
	afx_msg void OnJogMeasures();
	afx_msg void OnJogBeats();
	afx_msg void OnJogTicks();
	afx_msg void OnTimeFormatMBT();
	afx_msg void OnTimeFormatHMSF();
	afx_msg void OnSelchangeFootSwitchA();
	afx_msg void OnSelchangeFootSwitchB();
	afx_msg void OnDisableFaders();
	afx_msg void OnSoloSelects();
	afx_msg void OnSelchangeVirtualMain();
	afx_msg void OnConfigureLayout();
	afx_msg void OnDisableLCDUpdates();
	afx_msg void OnTransportMeasures();
	afx_msg void OnTransportBeats();
	afx_msg void OnTransportTicks();
	afx_msg void OnDisableRelayClick();
	afx_msg void OnFaderSelects();
	afx_msg void OnSelectHighlights();
	afx_msg void OnSelchangeVirtualMainType();
	afx_msg void OnSelchangeMeters();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CMackieControlMasterPropPageFactory : public IClassFactory
{
public:
	// IUnknown
	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// Interface IClassFactory
	STDMETHODIMP_(HRESULT) CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP_(HRESULT) LockServer( BOOL bLock ); 

	// Constructor
	CMackieControlMasterPropPageFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlMasterPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
