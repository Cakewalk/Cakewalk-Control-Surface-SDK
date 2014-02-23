#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include <CakeControls\NWControl.h>
#include "vs100.h"


/////////////////////////////////////////////////////////////////////////////
// CVS100PropPage dialog

class CVS100PropPage :
	public CDialog,
	public IPropertyPage,
	public CNWControlSite
{
// Construction
public:
	CVS100PropPage(CWnd* pParent = NULL);   // standard constructor
	~CVS100PropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CVS100PropPage)
	enum { IDD = IDD_PROPPAGE };
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

	CWnd *					GetCWnd(void){ return this; }
   void						RecalcLayout(void);
	ULONG						GetContainerNumber(void) const { return (ULONG)-1; }
	int						GetContainerIndex(void) const { return 0; }
	void						PopEdit(CProperty *,const CRect &,BOOL &){}


	// Callbacks from Control UI actions
	virtual void			DoCtrlContextMenu( CNWControl* pSource, UINT nFlags, CMenu& rMenu );
	virtual void			DoCtrlActivate( CNWControl* pSource, UINT nFlags );
	virtual void			RefreshControl( CNWControl* pSource );	// request to be refreshed from the App
	virtual void			OnValueChange( CNWControl* pSource );
	virtual void			OnFirstPunchIn( CNWControl* pSource );
	virtual void			OnFinalPunchOut( CNWControl* pSource );
	virtual void			RefreshAllControls();			// cause controls to obtain a new state from the seq.
	virtual void			OnRequestValueString( CNWControl* pSource, CString* pstr, double* pdVal = NULL );


	struct NamedKey
	{
		CString strName;
		DWORD vk;
	};
	static std::vector<NamedKey>		sm_vKeys;

protected:
	void						handleMouseLeave(void){}

private:

	void		createControls();
	void		createACTControls();

	CString	getFriendlyKeystrokeName( CVS100::ButtonActionDefinition& bad );
	CString	getFriendlyTransportName( CVS100::ButtonActionDefinition& bad );

	bool		isCtrlInVector( CNWControl *pCtrl, std::vector< CNWControl * > *pVector, DWORD *pdwIndex );

	// NWControls///
	CNWControl*			m_pfeetSw1;
	CNWControl*			m_pfeetSw2;
	CNWControl*			m_pctlDisableFaderMotors;

	CNWControl*					m_pctlACTContextLbl;
	CNWControl*					m_pctlACTModeLbl;
	std::vector<CNWControl*>	m_vRotaryACTLabels;
	std::vector<CNWControl*>	m_vDMPanACTLabels;
	std::vector<CNWControl*>	m_vDMVolACTLabels;
	std::vector<CNWControl*>	m_vDMSwitchACTLabels;
	CNWControl*						m_pctlValueEncoderACTLbl;
	CNWControl*						m_pctlValueEncoderPushACTLbl;
	CNWControl*						m_pctlCompACTLbl;

	std::set<CNWControl*>		m_setACTLabel;

	void				setCmdForButton( CNWControl* pSource );
	CNWControl*		makeSlider( DWORD dwParam, DWORD dwX );
	void				placeAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor, int nXOffset = 0, int nYOffset = 0 );

	void				layoutACT();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVS100PropPage)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	LONG				m_cRef;
	CVS100*		m_pSurface;
	IPropertyPageSite*	m_pPageSite;
	BOOL				m_bDirty;

	bool					m_bCommandsDirty;
	CBitmap				m_bmpFaceplate;


	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void StartTimer();
	void StopTimer();

	void LoadAllFields();

	bool m_bInitDone;

	UINT_PTR m_uiTimerID;

	SONAR_MIXER_STRIP m_eStripType;

	// Generated message map functions
	//{{AFX_MSG(CVS100PropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
