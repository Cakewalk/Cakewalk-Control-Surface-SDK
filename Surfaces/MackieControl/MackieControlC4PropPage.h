#if !defined(AFX_MackieControlC4PROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControlC4PROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CMackieControlC4PropPage dialog

class CMackieControlC4PropPage : public CDialog,
									public IPropertyPage
{
// Construction
public:
	CMackieControlC4PropPage(CWnd* pParent = NULL);   // standard constructor
	~CMackieControlC4PropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CMackieControlC4PropPage)
	enum { IDD = IDD_PROPPAGE_C4 };
	CEdit	m_cDisp8;
	CEdit	m_cDisp7;
	CEdit	m_cDisp6;
	CEdit	m_cDisp5;
	CEdit	m_cDisp4;
	CEdit	m_cDisp3;
	CEdit	m_cDisp2;
	CEdit	m_cDisp1;
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
	// Implemented in MackieControlC4Gen.cpp
	STDMETHODIMP_(BOOL)	CreateMFCDialog( HWND hwndParent );
	STDMETHODIMP_(BOOL)	DestroyMFCDialog();

	// *** IUnknown methods ***
	// Implemented in MackieControlC4Gen.cpp
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef( void );
	STDMETHODIMP_(ULONG)		Release( void );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMackieControlC4PropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Implementation
private:
	LONG					m_cRef;
	CMackieControlC4*		m_pSurface;
	IPropertyPageSite*		m_pPageSite;
	BOOL					m_bDirty;

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void TransferSettings(bool bSave);
	void SelectItemData(CComboBox *pBox, DWORD dwData);
	bool m_bInitDone;
	bool m_bIgnoreSave;

	// Generated message map functions
	//{{AFX_MSG(CMackieControlC4PropPage)
	afx_msg void OnSelchangeFunction1();
	afx_msg void OnSelchangeFunction2();
	afx_msg void OnSelchangeFunction3();
	afx_msg void OnSelchangeFunction4();
	afx_msg void OnSelchangeFunction5();
	afx_msg void OnSelchangeFunction6();
	afx_msg void OnSelchangeFunction7();
	afx_msg void OnSelchangeFunction8();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateDisp1();
	afx_msg void OnUpdateDisp2();
	afx_msg void OnUpdateDisp3();
	afx_msg void OnUpdateDisp4();
	afx_msg void OnUpdateDisp5();
	afx_msg void OnUpdateDisp6();
	afx_msg void OnUpdateDisp7();
	afx_msg void OnUpdateDisp8();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CMackieControlC4PropPageFactory : public IClassFactory
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
	CMackieControlC4PropPageFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControlC4PROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
