#include "afxwin.h"
#if !defined(AFX_ControlSurfaceProbePROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ControlSurfaceProbePROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CControlSurfaceProbePropPage dialog

class CControlSurfaceProbePropPage : public CDialog,
									public IPropertyPage
{
// Construction
public:
	CControlSurfaceProbePropPage(CWnd* pParent = NULL);   // standard constructor
	~CControlSurfaceProbePropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CControlSurfaceProbePropPage)
	enum { IDD = IDD_PROPPAGE };
	CSpinButtonCtrl	m_cMarkerTickSpin;
	CSpinButtonCtrl	m_cMarkerBeatSpin;
	CSpinButtonCtrl	m_cMarkerMeasSpin;
	CStatic	m_cMarkerTheNearest;
	CEdit	m_cMarkerTicks;
	CEdit	m_cMarkerBeats;
	CEdit	m_cMarkerMeasures;
	CStatic	m_cMarkerNearest;
	CEdit	m_cMarkerList;
	CStatic	m_cMarkerCount;
	CStatic	m_cStripMeterValues;
	CStatic	m_cStripNumMeters;
	CStatic	m_cTransportAutoPunch;
	CStatic	m_cUniqueID;
	CStatic	m_cHostVersion;
	CStatic	m_cHostName;
	CComboBox	m_cCommands;
	CEdit	m_cParamNumLow;
	CEdit	m_cParamNumHigh;
	CSpinButtonCtrl	m_cParamNumSpinLow;
	CSpinButtonCtrl	m_cParamNumSpinHigh;
	CStatic	m_cMixValueArm;
	CStatic	m_cNumMasters;
	CStatic	m_cNumBuses;
	CStatic	m_cTransportScrub;
	CStatic	m_cTransportRecAutomation;
	CStatic	m_cTransportPlay;
	CStatic	m_cTransportLoop;
	CStatic	m_cTransportAudio;
	CStatic	m_sTransportRec;
	CEdit	m_cNewVal;
	CStatic	m_cNumAuxs;
	CStatic	m_cNumMains;
	CStatic	m_cNumTracks;
	CStatic	m_cStripName;
	CStatic	m_cUpdateCount;
	CSpinButtonCtrl	m_cStripNumSpin;
	CSpinButtonCtrl	m_cParamNumSpin;
	CStatic	m_cMixValueText;
	CStatic	m_cMixValueLabel;
	CEdit	m_cStripNum;
	CStatic	m_cMixValue;
	CEdit	m_cParamNum;
	CComboBox	m_cMixParam;
	CComboBox	m_cMixStrip;
	CButton		m_cWrite;
	CButton		m_cRead;

	// dynamic mapped controls
	CSliderCtrl	m_cDynRotary1;
	CSliderCtrl	m_cDynRotary2;
	CSliderCtrl	m_cDynRotary3;
	CSliderCtrl	m_cDynRotary4;
	CSliderCtrl	m_cDynSlider1;
	CSliderCtrl	m_cDynSlider2;
	CSliderCtrl	m_cDynSlider3;
	CSliderCtrl	m_cDynSlider4;
	CButton		m_cDynSwitch;
	CStatic		m_cMapName;
	CStatic		m_cUIContext;
	CStatic		m_cDynSliderParam1;
	CStatic		m_cDynSliderParam2;
	CStatic		m_cDynSliderParam3;
	CStatic		m_cDynSliderParam4;
	CStatic		m_cDynRotaryParam1;
	CStatic		m_cDynRotaryParam2;
	CStatic		m_cDynRotaryParam3;
	CStatic		m_cDynRotaryParam4;
	CStatic		m_cDynSwitchParam;
	CButton		m_cLockContext;
	CButton		m_cEnableLearn;

   CComboBox   m_cWindowType;
   CStatic     m_cWindowState;

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
	// Implemented in ControlSurfaceProbeGen.cpp
	STDMETHODIMP_(BOOL)	CreateMFCDialog( HWND hwndParent );
	STDMETHODIMP_(BOOL)	DestroyMFCDialog();

	// *** IUnknown methods ***
	// Implemented in ControlSurfaceProbeGen.cpp
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef( void );
	STDMETHODIMP_(ULONG)		Release( void );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlSurfaceProbePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Implementation
private:
	LONG					      m_cRef;
	DWORD					      m_dwSurfaceID;
	CControlSurfaceProbe*   m_pSurface;
	IPropertyPageSite*		m_pPageSite;
	BOOL					      m_bDirty;
   WindowType              m_windowType;     

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void StartTimer();
	void StopTimer();
	void UpdateDisplay();

	void TransferSettings(bool bSave);
	void ComboAddEntry(CComboBox *pCBox, const char *str, DWORD dwData =0);
	void ComboSelectItemByData(CComboBox *pCBox, DWORD dwData);
	DWORD ComboGetSelectedData(CComboBox *pCBox);
	DWORD GetEditAsDword(CEdit *pCEdit);
	void SetEditToDword(CEdit *pCEdit, DWORD dwVal);

	UINT_PTR m_uiTimerID;

	bool m_bInitDone;
	DWORD m_dwCount;

	// Generated message map functions
	//{{AFX_MSG(CControlSurfaceProbePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeMixStrip();
	afx_msg void OnSelchangeMixParam();
	afx_msg void OnChangeStripNum();
	afx_msg void OnChangeParamNum();
	afx_msg void OnMixSend();
	afx_msg void OnDeltaposStripNumSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposParamNumSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMixArm();
	afx_msg void OnMixDisarm();
	afx_msg void OnChangeParamNumHigh();
	afx_msg void OnChangeParamNumLow();
	afx_msg void OnDeltaposParamNumSpinHigh(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposParamNumSpinLow(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTsAudioSet();
	afx_msg void OnTsAudioClear();
	afx_msg void OnTsPlaySet();
	afx_msg void OnTsPlayClear();
	afx_msg void OnTsScrubSet();
	afx_msg void OnTsScrubClear();
	afx_msg void OnTsRecSet();
	afx_msg void OnTsRecClear();
	afx_msg void OnTsRecAutoSet();
	afx_msg void OnTsRecAutoClear();
	afx_msg void OnTsLoopSet();
	afx_msg void OnTsLoopClear();
	afx_msg void OnDoCommand();
	afx_msg void OnCapabilities();
	afx_msg void OnTsAutoPunchSet();
	afx_msg void OnTsAutoPunchClear();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMarkersNearest();
	afx_msg void OnMarkerGetNearest();
	afx_msg void OnDeltaposMarkerMeasSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposMarkerBeatSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposMarkerTickSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSctSwitch();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedLockContext();
	afx_msg void OnBnClickedReadEnable();
	afx_msg void OnBnClickedLearnEnable();
   afx_msg void OnBnClickedWndZoomUp();
   afx_msg void OnBnClickedWndZoomUp2();
   afx_msg void OnBnClickedWndZoomDown();
   afx_msg void OnBnClickedWndZoomDown2();
   afx_msg void OnBnClickedWndZoomLeft();
   afx_msg void OnBnClickedWndZoomLeft2();
   afx_msg void OnBnClickedWndZoomRight();
   afx_msg void OnBnClickedWndZoomRight2();
   afx_msg void OnBnClickedWndScrollUp();
   afx_msg void OnBnClickedWndScrollUp2();
   afx_msg void OnBnClickedWndScrollDown();
   afx_msg void OnBnClickedWndScrollDown2();
   afx_msg void OnBnClickedWndScrollLeft();
   afx_msg void OnBnClickedWndScrollLeft2();
   afx_msg void OnBnClickedWndScrollRight();
   afx_msg void OnBnClickedWndScrollRight2();
   afx_msg void OnBnClickedWndOpen();
   afx_msg void OnBnClickedWndClose();
   afx_msg void OnBnClickedWndMin();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ControlSurfaceProbePROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
