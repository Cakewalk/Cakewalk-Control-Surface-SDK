#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "..\..\CakeControls\NWControl.h"
#include "TacomaSurface.h"
#include <sfkutils.h>
#include "PatchBayDlg.h"

class CNWMultiBtn;
class CNWBitmapEditControl;
class CNWDropDownCtrl;


#define SZAPPHELPFILE _T("SONAR_VS-700_Control_Surface.chm")



typedef struct
{
   SONAR_MIXER_PARAM mixParam;
   DWORD dwParam;
	CString strLabel;
} SEncoderParam;

/////////////////////////////////////////////////////////////////////////////
// CTacomaSurfacePropPage dialog

class CTacomaSurfacePropPage :
	public CDialog,
	public IPropertyPage,
	public CNWControlSite
{
// Construction
public:
	CTacomaSurfacePropPage(CWnd* pParent = NULL);   // standard constructor
	~CTacomaSurfacePropPage();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CTacomaSurfacePropPage)
	enum { IDD = IDD_PROPPAGE };
	//}}AFX_DATA

	struct NamedKey
	{
		CString strName;
		DWORD vk;
	};
	static std::vector<NamedKey>		sm_vKeys;


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


	virtual CWnd*			GetCWnd() {return this;}
	// Allow controls to force Layout Recalc on strip
	virtual void			RecalcLayout( void );
	virtual ULONG			GetContainerNumber() const {return (ULONG)-1;}		// container number for this site
	virtual int				GetContainerIndex() const { return -1; }
	virtual void			PopEdit( CProperty* pProp, const CRect& rc, BOOL& bChanged ){}


	// Callbacks from Control UI actions
	virtual void			DoCtrlContextMenu( CNWControl* pSource, UINT nFlags, CMenu& rMenu );
	virtual void			DoCtrlActivate( CNWControl* pSource, UINT nFlags );
	virtual void			RefreshControl( CNWControl* pSource );	// request to be refreshed from the App
	virtual void			OnValueChange( CNWControl* pSource );
	virtual void			OnFirstPunchIn( CNWControl* pSource );
	virtual void			OnFinalPunchOut( CNWControl* pSource );
	virtual void			RefreshAllControls();			// cause controls to obtain a new state from the seq.
	virtual void			OnRequestValueString( CNWControl* pSource, CString* pstr, double* pdVal = NULL );

   CString GetStringForMixParam( SONAR_MIXER_PARAM mixParam, DWORD dwParam );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTacomaSurfacePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	enum DisplayPage
	{
      DP_Preferences = 0,
	  DP_Preamp,
	  DP_DirectMixer,
      DP_ACT,
	  OC_Preamp,
	  OC_DirectMixerA,
	  OC_DirectMixerB,
	  OC_DirectMixerC,
	  OC_DirectMixerD

	};

   typedef std::map<DWORD,CNWControl*>::iterator MapControlsIterator;
	std::map<DWORD,CNWControl*>	m_mapCommandButtons;
	std::map<DWORD,CNWControl*>	m_mapCommandButtonsBmp;
   std::vector< CNWControl* >	   m_vEncoderButtons;

   std::vector< CNWControl* >    m_vRotaryACTLbl;
   std::vector< CNWControl* >    m_vSwitchACTLbl;
   CNWControl *						m_pTBarACTLbl;
   std::vector< CNWControl* >    m_vACTPageButtons;
   std::vector< CNWControl* >    m_vRad;
   std::vector< CNWControl* >    m_vDMRad;

   BOOL isCtrlInVector( CNWControl *pCtrl, std::vector< CNWControl * > *pVector, DWORD *pdwIndex );

	void handleDigitalRadio( CNWControl* pSource,  DWORD dwRadio );
	void refreshDigitalRadio( CNWControl* pSource, DWORD dwRadio );

	struct IOChanControls
	{
		void Enable ( BOOL b )
		{
			pGate->SetEnable( b );
			pAttack->SetEnable( b );
			pCompEnable->SetEnable( b );
			pCompGain->SetEnable( b );
			pGain->SetEnable( b );
			if ( pLink ) pLink->SetEnable( b );
			if ( pLinkOC ) pLinkOC->SetEnable( b );
			pLoCut->SetEnable( b );
			pPad->SetEnable( b );
			pHiz->SetEnable( b );
			pPhantom->SetEnable( b );
			pPhase->SetEnable( b );
			pRatio->SetEnable( b );
			pRelease->SetEnable( b );
			pThreshold->SetEnable( b );
         pGainLabel->SetEnable( b );
		}
		void Show( BOOL b )
		{
			pGate->SetIsVisible( b );
			pAttack->SetIsVisible( b );
			pCompEnable->SetIsVisible( b );
			pCompGain->SetIsVisible( b );
			pGain->SetIsVisible( b );
			if ( pLink ) pLink->SetIsVisible( b );
			if ( pLinkOC ) pLinkOC->SetIsVisible( b );
			pLoCut->SetIsVisible( b );
			pPad->SetIsVisible( b );
			pHiz->SetIsVisible( b );
			pPhantom->SetIsVisible( b );
			pPhase->SetIsVisible( b );
			pRatio->SetIsVisible( b );
			pRelease->SetIsVisible( b );
			pThreshold->SetIsVisible( b );
         pGainLabel->SetIsVisible( b );
		}

		IOChanControls() { ::memset( this, sizeof(IOChanControls), 0 ); }

      CNWControl* pGainLabel;
		CNWControl* pGain;
		CNWControl*	pPhantom;
		CNWControl* pLoCut;
		CNWControl* pPhase;
		CNWControl* pPad;
		CNWControl* pHiz;
		CNWControl*	pLink;
		CNWControl*	pLinkOC;
		CNWControl* pCompEnable;
		CNWControl* pRatio;
		CNWControl* pThreshold;
		CNWControl* pAttack;
		CNWControl* pRelease;
		CNWControl* pCompGain;
		CNWControl* pGate;
	};

	struct IODirectMixControls
	{
		IODirectMixControls() { pLink = 0; ::memset( this, sizeof(IODirectMixControls), 0 ); }
		void Show( BOOL b )
		{
			pDMVol->SetIsVisible( b );
			pDMVolLabel->SetIsVisible( b );
			pOCVol->SetIsVisible( b );
			pOCVolLabel->SetIsVisible( b );
			pOCVolB->SetIsVisible( b );
			pOCVolLabelB->SetIsVisible( b );
			pOCVolC->SetIsVisible( b );
			pOCVolLabelC->SetIsVisible( b );
			pOCVolD->SetIsVisible( b );
			pOCVolLabelD->SetIsVisible( b );
			pDMPan->SetIsVisible( b );
			pOCPan->SetIsVisible( b );
				pOCPanB->SetIsVisible( b );
					pOCPanC->SetIsVisible( b );
						pOCPanD->SetIsVisible( b );

			pDMSend->SetIsVisible( b );
			pDMMute->SetIsVisible( b );
			pDMSolo->SetIsVisible( b );
			pOCMute->SetIsVisible( b );
			pOCSolo->SetIsVisible( b );
			pOCMuteB->SetIsVisible( b );
			pOCSoloB->SetIsVisible( b );
			pOCMuteC->SetIsVisible( b );
			pOCSoloC->SetIsVisible( b );
			pOCMuteD->SetIsVisible( b );
			pOCSoloD->SetIsVisible( b );
			if ( pDMMono )pDMMono->SetIsVisible( b );
			if ( pLink ) pLink->SetIsVisible( b );
			if ( pLinkOC ) pLinkOC->SetIsVisible( b );
			if ( pLinkOCB ) pLinkOCB->SetIsVisible( b );
			if ( pLinkOCC ) pLinkOCC->SetIsVisible( b );
			if ( pLinkOCD ) pLinkOCD->SetIsVisible( b );
		}
		void Enable( BOOL b )
		{
			pDMVol->SetEnable( b );
			pDMVolLabel->SetEnable( b );
			pOCVol->SetEnable( b );
			pOCVolLabel->SetEnable( b );
			pOCVolB->SetEnable( b );
			pOCVolLabelB->SetEnable( b );
			pOCVolC->SetEnable( b );
			pOCVolLabelC->SetEnable( b );
			pOCVolD->SetEnable( b );
			pOCVolLabelD->SetEnable( b );

			pDMPan->SetEnable( b );
			pOCPan->SetEnable( b );
			pOCPanB->SetEnable( b );
			pOCPanC->SetEnable( b );
			pOCPanD->SetEnable( b );
			pDMSend->SetEnable( b );
			pDMMute->SetEnable( b );
			pDMSolo->SetEnable( b );
			pOCMute->SetEnable( b );
			pOCSolo->SetEnable( b );
			pOCMuteB->SetEnable( b );
			pOCSoloB->SetEnable( b );
			pOCMuteC->SetEnable( b );
			pOCSoloC->SetEnable( b );
			pOCMuteD->SetEnable( b );
			pOCSoloD->SetEnable( b );
			if ( pDMMono )pDMMono->SetEnable( b );
			if ( pLink ) pLink->SetEnable( b );
			if ( pLinkOC ) pLinkOC->SetEnable( b );
			if ( pLinkOCB ) pLinkOCB->SetEnable( b );
			if ( pLinkOCC ) pLinkOCC->SetEnable( b );
			if ( pLinkOCD ) pLinkOCD->SetEnable( b );
		}
		CNWControl* pDMVol;
		CNWControl* pDMVolLabel;
		CNWControl* pOCVol;
		CNWControl* pOCVolLabel;
		CNWControl* pOCVolB;
		CNWControl* pOCVolLabelB;
		CNWControl* pOCVolC;
		CNWControl* pOCVolLabelC;
		CNWControl* pOCVolD;
		CNWControl* pOCVolLabelD;
		CNWControl* pDMPan;
		CNWControl* pOCPan;
		CNWControl* pOCPanB;
		CNWControl* pOCPanC;
		CNWControl* pOCPanD;
		CNWControl* pDMSend;
		CNWControl* pDMMute;
		CNWControl* pDMSolo;
		CNWControl* pOCMute;
		CNWControl* pOCSolo;
		CNWControl* pOCMuteB;
		CNWControl* pOCSoloB;
		CNWControl* pOCMuteC;
		CNWControl* pOCSoloC;
		CNWControl* pOCMuteD;
		CNWControl* pOCSoloD;
		CNWControl* pDMMono;
		CNWControl*	pLink;
		CNWControl*	pLinkOC;
		CNWControl*	pLinkOCB;
		CNWControl*	pLinkOCC;
		CNWControl*	pLinkOCD;
	};

	CNWMultiBtn*		m_pbtnPrefTab;
	CNWMultiBtn*		m_pbtnPreampTab;
	CNWMultiBtn*		m_pbtnDMTab;
	CNWMultiBtn*		m_pbtnACTTab;
	CNWMultiBtn*		m_pbtnOCTabA;
	CNWMultiBtn*		m_pbtnOCTabB;
	CNWMultiBtn*		m_pbtnOCTabC;
	CNWMultiBtn*		m_pbtnOCTabD;
	CNWMultiBtn*		m_pbtnOCBay;
	
	CNWMultiBtn*		m_pctlIOPresetSave;
	CNWMultiBtn*		m_pctlIOPresetDelete;

	CNWMultiBtn*		m_pctlDisableFaderMotors;
	CNWMultiBtn*		m_pctlTouchSelects;
	CNWMultiBtn*		m_pctlMBT;
	CNWMultiBtn*		m_pctlHMSF;

	CNWMultiBtn*		m_pctlFlip;
	CNWMultiBtn*		m_pctlTracks;
	CNWMultiBtn*		m_pctlBuses;
	CNWMultiBtn*		m_pctlMains;
	CNWMultiBtn*		m_pctlIO;

   CNWMultiBtn*      m_pctlMarkersInd;
   CNWMultiBtn*      m_pctlACTInd;
   CNWMultiBtn*      m_pctlLayerInd;
   CNWMultiBtn*      m_pctlFxInd;
   CNWMultiBtn*      m_pctlDigSync;
   CNWMultiBtn*      m_pctlInSync;
   CNWMultiBtn*      m_pctlOUT12Ind;
   CNWMultiBtn*      m_pctlOUT34Ind;
   CNWMultiBtn*      m_pctlOUT56Ind;
   CNWMultiBtn*      m_pctlOUT78Ind;
   CNWMultiBtn*      m_pctlOUT910Ind;

	CNWControl*			m_pctlACTContextLbl;
	CNWControl*			m_pctlReturn;
	CNWControl*			m_pctlRevTime;

	CNWMultiBtn*		m_pctlTrackEncFunc;
	CNWMultiBtn*		m_pctlBusEncFunc;
	CNWMultiBtn*		m_pctlIOEncFunc;
	CNWMultiBtn*        m_pBtnMasterLink;

	CNWControl*			m_pfeetSw1;
	CNWControl*			m_pfeetSw2;
	CNWDropDownCtrl*	m_pctlDamp;
	CBrush*				m_pBrush;



	COLORREF						m_rgbCtrlBkgd;				// Control colors
	COLORREF						m_rgbCtrlLEDBright;
	COLORREF						m_rgbCtrlLEDDark;

	CSFKHelpAssist		m_HelpHelper;

	void				createACTControls();
	void				createPrefControls();
	void				createIOControls();
	void				createDMControls();
	//void				createDMControlsA();
	void				createGlobalControls();

	void				setCmdForButton( CMenu& menu, CNWControl* pSource );
   void				setCmdForIOEncoder( CMenu& menu, CNWControl* pSource );
	CNWControl*			makeSlider( DWORD dwParam, DWORD dwX );
	CNWControl*		makeSlider2( DWORD dwParam, DWORD dwX );
	void				placeAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor, int nXOffset, int nYOffset );

	void				layoutPreamp();
	void				layoutOCPreamp();
	void				layoutDirectMixer();
	void				layoutDirectMixerA();
	void				layoutDirectMixerB();
	void				layoutDirectMixerC();
	void				layoutDirectMixerD();
	void				layoutPreferences();
	void				layoutACT();
	void				layoutGlobal();
   void					enablePage( const DisplayPage ePage );
	void				enableDMStrips();
	void				enableMicPreStrips();

	void				onPreampTab();
	void				onOCPreampTab();
	void				onPrefTab();
	void				onDMTab();
	void				onOCDMTabA();
	void				onOCDMTabB();
	void				onOCDMTabC();
	void				onOCDMTabD();
	void				onACTTab();
	void				UpdateControls();

	UINT				m_idbTabBkg;

	void				onPresetSave();
	void				onPresetDelete();

	CTacomaSurface::PresetType getCurrentPresetType();

	void 				StartTimer();
	void 				StopTimer();
	void 				LoadAllFields();
	CString			getFriendlyKeystrokeName( CTacomaSurface::ButtonActionDefinition& bad );
	CString			getFriendlyTransportName( CTacomaSurface::ButtonActionDefinition& bad );
	void				handleMouseLeave() {};

	std::vector<IODirectMixControls>	m_aDMControls;
	std::vector<IODirectMixControls>	m_aDMControlsA;
	std::vector<IOChanControls>	m_aIOControls;
	//std::vector<IOChanControls>	m_aIOControls;
	bool					m_bCommandsDirty;
	bool					m_bInitDone;
	bool					m_bIsOcta;
	bool					m_bActivate;
	UINT_PTR				m_uiTimerID;
	CBitmap				m_bmpFaceplate;
	SONAR_MIXER_STRIP	m_eStripType;
	DisplayPage			m_eDisplayPage;

	int					m_nACTPage;

	enum OutPorts
	{
		DA,
		DB,
		DC,
		DD,

		OutMax
	};

	enum ReverbType
	{
		OFF,
		ECHO,
		ROOM,
		SMALLH,
		LARGEH,

		RevMax
	};

	// Generated message map functions
	//{{AFX_MSG(CTacomaSurfacePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCbnSelchangePreset();
	afx_msg void OnCbnSelchangeInterface();

	void fillPresetCombo();
	void fillInterfaceCombo();
	void fillTypeCombo();
	void fillDampCombo();
	void fillPredelayCombo();
	void Initface();
	void onPatchBay();


   const SEncoderParam * doEncoderMenu( CMenu &rMenu, SONAR_MIXER_STRIP strip, CPoint pt );
	const TacomaIOBoxParam * doEncoderIOMenu( CMenu &rMenu, CPoint pt );

	static bool				sm_bInitialized;

	LONG				m_cRef;
	CTacomaSurface*		m_pSurface;
	CPatchBayDlg*		m_pPatchBay;
	IPropertyPageSite*	m_pPageSite;
	BOOL				m_bDirty;

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	CComboBox m_cPresets;
	CComboBox m_cInterface;
	CComboBox m_cType;
	CComboBox m_cDamp;
	CComboBox m_cPredelay;
	CStatic m_sDisable;
   CStatic m_sTouch;

	LOGFONT						small_lf;

public:
	afx_msg void OnCbnSelchangeRevpredelay();
	afx_msg void OnCbnSelchangeRevtype();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTControllerPROPPAGE_H__00EDAF21_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
