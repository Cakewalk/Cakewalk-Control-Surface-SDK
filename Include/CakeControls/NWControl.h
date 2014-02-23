// NWControl.h: interface for the CNWControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NWCONTROL_H__374E4A75_4689_4097_A83B_9B49FC42787F__INCLUDED_)
#define AFX_NWCONTROL_H__374E4A75_4689_4097_A83B_9B49FC42787F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// clamp _v between _n and _x inclusive
#define CLAMP( _v,_n,_x ) ( max( (_n), min ( (_x), (_v) ) ) )


class CNWControl;
class CProperty;
class CCtrlMouseTip;
class COurToolTipCtrl;
class CWGraphicsCtx; // forward
class CNWControlSiteUIAProvider;
class CNWControlUIAProvider;

typedef std::vector<CNWControl*> NWControlList;
typedef NWControlList::iterator	NWControlIterator;
typedef NWControlList::const_iterator	NWControlConstIterator;
typedef NWControlList::reverse_iterator NWControlRevIterator;

typedef std::set<CNWControl*> NWControlSet;
typedef NWControlSet::iterator NWControlSetIterator;

typedef GUID TYPEID;



/////////////////////////////////////////////////////////////////////////////
//
//	We need to define some "Generic" widget types to associate "types" of controls
// for tabbing, etc.  For example, to be able to tab from one Volume to another.

// {B2951F44-05C9-4718-A47E-8D7CD8F6000D}
static const GUID TYPE_GENERIC_VOLUME = 
{ 0xb2951f44, 0x5c9, 0x4718, { 0xa4, 0x7e, 0x8d, 0x7c, 0xd8, 0xf6, 0x0, 0xd } };

// {8D216F5C-71C0-45ca-83F7-44E10C0A6CC7}
static const GUID TYPE_GENERIC_SURROUND_PAN = 
{ 0x8d216f5c, 0x71c0, 0x45ca, { 0x83, 0xf7, 0x44, 0xe1, 0xc, 0xa, 0x6c, 0xc7 } };

// {8814EDF6-A3A8-4b26-BD02-E979D4E3A8A2}
static const GUID TYPE_GENERIC_INPUT_VOLUME = 
{ 0x8814edf6, 0xa3a8, 0x4b26, { 0xbd, 0x2, 0xe9, 0x79, 0xd4, 0xe3, 0xa8, 0xa2 } };

// {7C3138C6-24EE-4506-8198-526AF724354C}
static const GUID TYPE_GENERIC_PAN = 
{ 0x7c3138c6, 0x24ee, 0x4506, { 0x81, 0x98, 0x52, 0x6a, 0xf7, 0x24, 0x35, 0x4c } };

// {2651BCD7-BA84-4708-8233-65BE8441AAC3}
static const GUID TYPE_GENERIC_INPUT_PAN = 
{ 0x2651bcd7, 0xba84, 0x4708, { 0x82, 0x33, 0x65, 0xbe, 0x84, 0x41, 0xaa, 0xc3 } };

// {B9EBE3C1-5A0A-436f-9748-E49ED956F516}
static const GUID TYPE_GENERIC_TRIM = 
{ 0xb9ebe3c1, 0x5a0a, 0x436f, { 0x97, 0x48, 0xe4, 0x9e, 0xd9, 0x56, 0xf5, 0x16 } };

// {345475E8-5506-46d2-B426-5AB69C7FAAEB}
static const GUID TYPE_GENERIC_INPUT = 
{ 0x345475e8, 0x5506, 0x46d2, { 0xb4, 0x26, 0x5a, 0xb6, 0x9c, 0x7f, 0xaa, 0xeb } };

// {1B8B4F50-7000-4306-AEF6-D3ABB7B36264}
static const GUID TYPE_GENERIC_OUTPUT = 
{ 0x1b8b4f50, 0x7000, 0x4306, { 0xae, 0xf6, 0xd3, 0xab, 0xb7, 0xb3, 0x62, 0x64 } };

// {C8432CE4-7361-45ee-A8EF-5199B5579C3F}
static const GUID TYPE_GENERIC_INSERT = 
{ 0xc8432ce4, 0x7361, 0x45ee, { 0xa8, 0xef, 0x51, 0x99, 0xb5, 0x57, 0x9c, 0x3f } };

// {CC6AA7AD-F698-4928-82A2-49BCE36AEA98}
static const GUID TYPE_GENERIC_NAME = 
{ 0xcc6aa7ad, 0xf698, 0x4928, { 0x82, 0xa2, 0x49, 0xbc, 0xe3, 0x6a, 0xea, 0x98 } };

// {340A03C8-2C5A-45cd-8F70-6F4BF6FF5AD8}
static const GUID TYPE_GENERIC_MINIMIZE = 
{ 0x340a03c8, 0x2c5a, 0x45cd, { 0x8f, 0x70, 0x6f, 0x4b, 0xf6, 0xff, 0x5a, 0xd8 } };

// {F4F81E46-CA32-4af7-B106-D556504BAF48}
static const GUID TYPE_GENERIC_MAXIMIZE = 
{ 0xf4f81e46, 0xca32, 0x4af7, { 0xb1, 0x6, 0xd5, 0x56, 0x50, 0x4b, 0xaf, 0x48 } };

// {B0E085F3-2308-4ef1-BD96-5A6F18FD761A}
static const GUID TYPE_GENERIC_MUTE = 
{ 0xb0e085f3, 0x2308, 0x4ef1, { 0xbd, 0x96, 0x5a, 0x6f, 0x18, 0xfd, 0x76, 0x1a } };

// {1B76CA65-042F-4a99-906E-83F212508490}
static const GUID TYPE_GENERIC_SOLO = 
{ 0x1b76ca65, 0x42f, 0x4a99, { 0x90, 0x6e, 0x83, 0xf2, 0x12, 0x50, 0x84, 0x90 } };

// {8175215D-12A1-4529-8C09-6B1D46D78ACB}
static const GUID TYPE_GENERIC_ARM = 
{ 0x8175215d, 0x12a1, 0x4529, { 0x8c, 0x9, 0x6b, 0x1d, 0x46, 0xd7, 0x8a, 0xcb } };

// {D0118E40-D99B-4b14-A3F5-03C64542470C}
static const GUID TYPE_GENERIC_IM = 
{ 0xd0118e40, 0xd99b, 0x4b14, { 0xa3, 0xf5, 0x3, 0xc6, 0x45, 0x42, 0x47, 0xc } };

// Pitch Snapping
// {4BCADFE2-C788-4a19-9EA2-330FFFB7FCF8}
static const GUID TYPE_GENERIC_ROOTNOTE = 
{ 0x4bcadfe2, 0xc788, 0x4a19, { 0x9e, 0xa2, 0x33, 0xf, 0xff, 0xb7, 0xfc, 0xf8 } };

// {9AF871C4-4EA4-41d3-B9D4-B9863D6153E7}
static const GUID TYPE_GENERIC_SCALE = 
{ 0x9af871c4, 0x4ea4, 0x41d3, { 0xb9, 0xd4, 0xb9, 0x86, 0x3d, 0x61, 0x53, 0xe7 } };

// {5B8A36C5-50DB-4757-9B87-54DF13D6BD81}
static const GUID TYPE_GENERIC_SNAPENABLE = 
{ 0x5b8a36c5, 0x50db, 0x4757, { 0x9b, 0x87, 0x54, 0xdf, 0x13, 0xd6, 0xbd, 0x81 } };


// {DFF3E97A-CA0D-4786-8297-19396F0D8242}
static const GUID GUID_GENERIC_MIDI_CHAN = 
{ 0xdff3e97a, 0xca0d, 0x4786, { 0x82, 0x97, 0x19, 0x39, 0x6f, 0xd, 0x82, 0x42 } };
// {EB752AA6-C039-4575-8D65-EC20D7B7F6FD}

static const GUID GUID_GENERIC_MIDI_PATCH = 
{ 0xeb752aa6, 0xc039, 0x4575, { 0x8d, 0x65, 0xec, 0x20, 0xd7, 0xb7, 0xf6, 0xfd } };

// {1B54F722-C86D-4a2f-8BDB-A1944FFDD2B1}
static const GUID GUID_GENERIC_MIDI_BANK = 
{ 0x1b54f722, 0xc86d, 0x4a2f, { 0x8b, 0xdb, 0xa1, 0x94, 0x4f, 0xfd, 0xd2, 0xb1 } };

// Input Quantize
// {7DF4AE2F-35B5-4549-BE64-36ABE8B2BE01}
static const GUID TYPE_GENERIC_QUANTIZE_RES = 
{ 0x7df4ae2f, 0x35b5, 0x4549, { 0xbe, 0x64, 0x36, 0xab, 0xe8, 0xb2, 0xbe, 0x1 } };

// {69CBECDD-8B92-4883-813E-C91742A4DC23}
static const GUID TYPE_GENERIC_QUANTIZE_ENABLE = 
{ 0x69cbecdd, 0x8b92, 0x4883, { 0x81, 0x3e, 0xc9, 0x17, 0x42, 0xa4, 0xdc, 0x23 } };

class CNWControlSite;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

// NWCONTROLATTRIBS
struct NWCTRLATTRIB
{
	COLORREF		crBkNormal;			// Normal BK Color
	COLORREF		crBkHighlight;		// Highlighted BK Color
	COLORREF		crBkDisable;		// Disabled BK Color
	COLORREF		crTxtNormal;		// NOrmal Text Color
	COLORREF		crTxtHighlight;	// Highlighted Text Color
	COLORREF		crTxtDisable;		// Disabled Text Color
	COLORREF		crFocusColor;		// focus rect color

	// : 2 syntax is a bitfield.  Do not persist a bitfield struct as it will not be portable (compiler specific)
	// rather, persist the values in a more portable way.
	BOOL			bShowLabel : 2;		// Display Label
	BOOL			bShowValue : 2;		// Display Value
	BOOL			bHover : 2;				// Allow Hover Effects
	BOOL			bShowFocus : 2;		// display Focus rect
	BOOL			bDynamicTips : 2;		// Allow Dynamic Tooltips
	BOOL			bBoldTxt : 2;			// Use Bold Font
	BOOL			bModifySelf : 2;		// Allow control to modify its own value
	BOOL			bTransparent : 2;		// Should the control draw transparently over the background
	// 16 bits
	BOOL			bUserFont : 2;			// Should the control draw text with a specified user font?	
	BOOL			bMouseWheel : 2;		// Should the control respond to the mouse wheel?
	// 4 bits
	short			nTextOffset : 12;			// DT_LEFT or DT_RIGHT offsets from edge (up to 4095)	
	// brings up to 16 bits for alignment

	DWORD			fTextAlign;				// DT_LEFT, DT_CENTER, or DT_RIGHT (see win32 api DrawText)
	
	BYTE			nFocusType : 4; // up to 16 dif types (won't need that many)
	short			nShortLabelBudget : 12;	// Length to crunch Short label to (up to 4095)
	// 16 bits

	short			nShortLabelThresh;	// Use short label if full label is longer than this	

	enum EMask
	{
		FBkNormal		= 0x01,
		FBkHighlight	= 0x02,
		FBkDisable		= 0x04,
		FTxtNormal		= 0x08,
		FTxtHighlight	= 0x10,
		FTxtDisable		= 0x20,
		FShowLabel		= 0x40,
		FShowValue		= 0x80,
		FHover			= 0x100,
		FShowFocus		= 0x200,
		FDynamicTips	= 0x400,
		FBoldTxt			= 0x800,
		FTextAlign		= 0x1000,
		FTextOffset		= 0x2000,
		FDrawTransparent = 0x4000,
		FShortLabel		= 0x8000,
		FModifySelf		= 0x10000,
		FUserFont		= 0x20000,
		FFocusColor		= 0x40000,
		FMouseWheel		= 0x80000,

		FAll				= 0xffffffff
	};
	DWORD	 dwFlags;

	enum EFocusType // up to 16 based on bit field size.
	{
		FocusRectInset = 0,
		FocusDottedRectInset,
		FocusRectFull,
		FocusDottedRectFull,
		FocusRoundedRect,
		FocusDottedRoundedRect,
		FocusEllipse,
		FocusDottedEllipse,
		FocusCorners
	};

	NWCTRLATTRIB()
		:	crBkNormal( ::GetSysColor( COLOR_BTNFACE ) ),
			crBkHighlight( ::GetSysColor( COLOR_BTNFACE ) ),
			crBkDisable( ::GetSysColor( COLOR_BTNFACE ) ),
			crTxtNormal( ::GetSysColor( COLOR_BTNTEXT ) ),
			crTxtHighlight( ::GetSysColor( COLOR_BTNTEXT ) ),
			crTxtDisable( ::GetSysColor( COLOR_GRAYTEXT ) ),
			crFocusColor( ::GetSysColor( COLOR_3DHILIGHT ) ),
			bShowLabel( FALSE ),
			bShowValue( FALSE ),
			bHover( TRUE ),
			bShowFocus( FALSE ),
			bDynamicTips( TRUE ),
			bBoldTxt( FALSE ),
			bModifySelf( TRUE ),
			fTextAlign( DT_LEFT ),
			nTextOffset( 0 ),
			nShortLabelBudget( 8 ),
			bTransparent( FALSE ),
			bUserFont( FALSE ),
			bMouseWheel( FALSE ),
			nFocusType( FocusCorners ),
			dwFlags( (DWORD)FAll )
	{
		nShortLabelThresh = (short)(nShortLabelBudget * 1.5);
	}
};



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class CNWControl
{
public:
	friend class CNWControlSetFont;
	friend class CNWControlSetFontCWG;

	virtual ~CNWControl();

	enum EKeyboard { None = 0, 
						Min, Max, 
						SmallDn, SmallUp, 
						LargeDn, LargeUp, 
						Enter, Menu,
						Insert, Delete, 
						Prev, Next,
						Escape
#if _DEBUG
						,TweakLeft, TweakRight, TweakUp, TweakDown
#endif
	
	};

	static DWORD		ActivityTimeout() { return 300; }

	virtual void		Render( CDC* pDC ) PURE;	// pure virtual Paint function
	virtual void		RenderCWG( CWGraphicsCtx * pcwg ) PURE; // parallel version for graphics lib.

	virtual void		OnColorChange( BOOL bRedraw = FALSE ) {};
	virtual void		SetFocus( BOOL bFocus, BOOL bRedraw = TRUE );
	const BOOL			IsFocus() const {return m_bFocus; }
	BOOL					IsAvailable() const { return m_bAvailable; }
	void					SetAvailable( BOOL bAvailable );

	virtual void		RefreshFromApp();			// override to force controls to get a new state from the app
	
	const BOOL			GetIsVisible() const {return m_bVisible;}

	// Layout / Positioning code
	virtual CRect		GetControlRect() const { return m_rctLocation; }
	virtual void		SetSize( const CSize& size, BOOL bRepaint = FALSE );
	virtual CSize		GetSize() const { return m_rctLocation.Size(); }
	virtual void		SetOrigin( const CPoint& ptOrigin, BOOL bRepaint = FALSE );
	const CPoint&		GetOrigin( void ) const { return m_rctLocation.TopLeft (); }
	virtual BOOL		GetIsFixed( void ) const { return m_bIsFixed; }
	virtual void		SetIsFixed( BOOL bIsFixed ) { m_bIsFixed = bIsFixed; }
	virtual void		SetIsVisible( BOOL bVisible );

	virtual void		SetEnable( BOOL bEnable );	// Like CWnd::EnableWindow()
	virtual BOOL		IsEnabled() const { return m_bEnabled; }
	virtual DWORD		GetMsSinceLastChange( ) const;
	virtual void		IndicateActivity( );

	virtual void		SetAttributes( const NWCTRLATTRIB& attribs );
	virtual void		GetAttributes( NWCTRLATTRIB* pattribs );
	virtual void		SetSharedAttributes( NWCTRLATTRIB* pAttribs );
	const NWCTRLATTRIB&	GetAttributes() { return m_LocalAttributes; }

	COLORREF				GetGangColor() const { return m_rgbGang; }
	BOOL					GetIsGanged() const { return m_bGanged; }

	virtual BOOL		HitTest( const CPoint& pt ) const {	return m_rctLocation.PtInRect( pt ) && m_bVisible; }
	virtual BOOL		HasCapture() const { return m_bCapture; }
	virtual void		ReleaseCapture() { m_bCapture = FALSE; }
	virtual void		HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void		HandleRMouseDN( UINT nFlags, const CPoint& pt){ m_ptClick = pt; m_bCapture = TRUE; }
	virtual void		HandleRMouseUP( UINT nFlags, const CPoint& pt){ m_bCapture = FALSE; }
	virtual void		HandleLMouseDClk( UINT nFlags, const CPoint& pt );
	virtual void		HandleMouseMove( UINT nFlags, const CPoint& pt ){ m_ptPrevious = pt;}
	virtual BOOL		HandleMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual BOOL		CanHandleMouseWheel() const { return m_LocalAttributes.bMouseWheel; }

	virtual BOOL		HandleKeyboardDN( EKeyboard kbd );
	virtual BOOL		HandleKeyboardUP( EKeyboard kbd ) { return FALSE; }

	virtual void		HandleTimer( UINT nIdEvent ) {};

	virtual void		HandleMouseEnter();
	virtual void		HandleMouseLeave();
	virtual BOOL		IsSelfDrop( const CPoint& point ) const { return TRUE; }	// override to signal a drop onto itself
	virtual BOOL		HandleDrop( const CPoint& point, COleDataObject* pDataObject, DROPEFFECT dropEffect ){ return FALSE; }
	virtual BOOL		HandleDragEnter( const CPoint& point, COleDataObject* pDataObject ) { return FALSE; }
	virtual BOOL		HandleDragOver( const CPoint& point, COleDataObject* pDataObject ) { return FALSE; }
	virtual void		OnActiveDropTarget( BOOL bActive ) {}
	
	virtual HRESULT	StartDrag( CNWControl* pCtrl ) { return S_FALSE; }

	virtual HRESULT	PunchIn();
	virtual HRESULT	PunchOut();

	virtual const CString&	GetCtrlName( ) const { return m_strName; }
	virtual const CString&	GetCtrlShortName( ) const { return m_strShortName; }
	virtual void			SetCtrlName( const CString& strName );
	virtual void			SetCtrlName( UINT ids );
	virtual void			SetTipStr( const CString& strTip );
	virtual void			SetTipStr( UINT ids );
	virtual const CString&	GetCtrlValueStr() const { return m_strValue; }	
	virtual void			SetCtrlValueString( const CString& strVal );
	virtual void			GetAccessibilityString( CString* pstrAccess );

	virtual void			SetIcon( UINT i );
	virtual UINT			GetIcon( ) { return m_idbIcon; }
	virtual int				GetThemeIndex() const { return m_nTheme; }
	void						SetThemeIndex( int i ) { m_nTheme = i; }

	// Value Get/Set
	virtual double		GetCtrlValue() const { return m_dValue; }
	virtual void		SetCtrlValue( double d, BOOL bSelf, BOOL bRedraw = TRUE );
	virtual double		GetSnapValue() const;
	virtual void		SetSnapValue( double d );
	virtual void		GetRange( double* pdMin, double* pdMax );
	virtual void		SetRange( double dMin, double dMax ) { m_dMin = dMin; m_dMax = dMax; }
	virtual void		SetNumValueSteps( int iValueSteps ) { m_nValueSteps = iValueSteps; }
	virtual const CString&	GetCtrlValString() const { return m_strValue; }

	// Gang indicator setup
	virtual void		SetGangOffsets( int cxGang, int cyGang ){ m_cxGangOffset = cxGang; m_cyGangOffset = cyGang; }

	// Value Small/Large Increment Steps Set
	void					SetNumSmallSteps( UINT nSteps ) { m_dSmallSteps = static_cast<double>( nSteps ); }
	void					SetNumLargeSteps( UINT nSteps ) { m_dLargeSteps = static_cast<double>( nSteps ); }

	// Tooltip Support
	UINT					GetTipId() const { return m_uId; }
	virtual void		SetTipRect();
	virtual void		SetTipDelimiter ( const CString& strDelim ) { m_strTTDelimiter = strDelim; }
	virtual void		GetTipLabel( CString& strLabel );
	virtual void		GetTipString( CString& rStr );

	// Microsoft UI Automation support
	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );
	
	// A "generic" type identifier for the control
	void					SetGenericType( const GUID& typeGeneric ) { m_typeGeneric = typeGeneric; }
	const GUID&			GetGenericType() { return m_typeGeneric; }

	// More Type Info
	void					SetTypeInfo( const GUID& typeId, DWORD dwParam, const CLSID* pclsidFilter ){ m_typeId = typeId; m_dwParameter = dwParam; m_pclsidFilter = pclsidFilter; }
	const GUID&			GetTypeId( ) { return m_typeId; }
	const CLSID *		GetFilterCLSID( ) { return m_pclsidFilter; }
	virtual DWORD		GetParameter() const { return m_dwParameter; }
	virtual void		SetParameter( DWORD dw ) { m_dwParameter = dw; }	

	// some generic Automation states
	virtual void		SetHasData( bool b );
	virtual void		SetTrimMode( BOOL b );
	virtual void		SetGanged( BOOL b );
	virtual void		SetGangColor( COLORREF cr );
	virtual void		SetArmed( BOOL b );
	virtual BOOL		IsArmed() const { return m_bArmed; }
	virtual void		SetRecording( BOOL b);
	virtual void		SetAutomationEnabled( BOOL b );

	virtual void		SetNotifyHostAlways( bool b ) { m_bNotifySiteAlways = b; }
	virtual void		SetDrawEllipsis( bool b ) { m_bDrawEllipsis = b; }
	virtual void		SetMakeShortStr( BOOL b ) { m_bMakeShortStr = b; }
	
	void					SetReassignable( BOOL bReassignable ) { m_bReassignable = bReassignable; }
	BOOL					IsReassignable( ) const { return m_bReassignable; }
	BOOL              GetPtClick( CPoint* ppt ); 

	void					SetUserFont(const LOGFONT &lf);
	CFont *				GetUserFont() { return m_pFntUser; }

	virtual void		RecalcShortNameBudget();

	void GetCurrentFont(LOGFONT &lf)
	{
		if ( m_pFntUser )
		{
			m_pFntUser->GetLogFont(&lf);
		}
		else
		{
			if ( m_pAttributes->bBoldTxt )
				CNWControl::sm_FntBold.GetLogFont(&lf);
			else
				CNWControl::sm_FntSmall.GetLogFont(&lf);
		}
	}

	void				SetSharedUserFont(CFont * pFont);

	static int			NWC2X( int nCol ) { return sm_nFntCx * nCol; }
	static int			NWC2Y( int nRow ) { return sm_nFntCy * nRow; }

	void					SetAutomationIcon( UINT idb );
	static void			SetNoFontSmoothing(bool bNoSmooth)
	{
		sm_bNoFontSmoothing = bNoSmooth;
	}

	static CFont		sm_FntTiny;
	static CFont		sm_FntSmall;
	static CFont		sm_FntSmallBold;
	static CFont		sm_FntStd;
	static CFont		sm_FntBold;
	static int			sm_nFntCx;
	static int			sm_nFntCy;

	static bool			sm_bNoFontSmoothing;

protected:
							CNWControl( CNWControlSite* pSite );

public:
	CNWControlSite*	GetControlSite( void ) const { return m_pControlSite; }


	//  value setting helpers
	virtual double		GetSmallIncrement() const { return (m_dMax - m_dMin) / m_dSmallSteps; }
	virtual double		GetLargeIncrement() const { return (m_dMax - m_dMin) / m_dLargeSteps; }

protected:
	// Display / Ctrl value transforms
	virtual double		displayToCtrl( double d ) { return d; }
	virtual double		ctrlToDisplay( double d ) { return d; }

	virtual void		makeShortLabel();

	// display rendering sub functions
	virtual void		showFocus( CDC* pDC );
	virtual void		showGanged( CDC* pDC );
	virtual void		drawIcon( CDC* pDC );

	// graphics lib versions
	//                       //////////////////////////
	virtual void		showFocusCWG( CWGraphicsCtx * pcwg );
	virtual void		showGangedCWG( CWGraphicsCtx *pcwg );
	virtual void		drawIconCWG( CWGraphicsCtx * pcwg );
	////////////////////////////////////////////////////

	virtual void		showDynamicToolTip( CPoint pt );
	virtual void		updateDynamicToolTip( CPoint* pPt = NULL );
	virtual void		hideDynamicToolTip();
	
	virtual void		showAutomationDataState( int left, int top, CDC *pDC );

	virtual void		showArmed( CDC* pDC );
	virtual void		showRecording( CDC* pDC );

	// graphics lib versions
	//                       ////////////////////////////
	virtual void		showAutomationDataStateCWG( int left, int top, CWGraphicsCtx * pcwg );

	virtual void		showArmedCWG( CWGraphicsCtx * pcwg );
	virtual void		showRecordingCWG( CWGraphicsCtx * pcwg );
	//////////////////////////////////////////////////////

	virtual void		cacheRects() {}	// override to cache any special rects for controls

	// override to provide smaller rect to invalidate during value changes
	virtual CRect*		getValueRect() { return &m_rctLocation; }	

	virtual void		setWantsMouseWheel( BOOL bWantWheel ) { m_LocalAttributes.bMouseWheel = bWantWheel; }

protected:
	CNWControlSite*	m_pControlSite;
	UINT					m_uId;					// used for TooltipID. Obtained from CNWControlSite in constructor

	const NWCTRLATTRIB*		m_pAttributes;			// Pointer to Attributes to use

	// control type info
	GUID					m_typeGeneric;			// for finding "similar" control
	GUID					m_typeId;				// Specific GUID type ID 
	mutable const CLSID *	m_pclsidFilter;		// we also need a filter (for prochans to work properly with quick group, track nav)
	DWORD					m_dwParameter;			// A way to indentify this control with a parameter ( should you want )

	CRect					m_rctLocation;			// Location of the control
	UINT					m_idbIcon;				// to associate an icon with this control
	int					m_nTheme;				// some app-specific theme index?

	CPoint				m_ptClick;				// point where mousedown events occured
	CPoint				m_ptPrevious;			// used for tracking movement

	double				m_dValue;				// *THE* Proportional Value ( m_dMin..m_dMax domain )
	double				m_dValueSnap;			// snap ( dbl click ) value
	double				m_dValueTrimSnap;		// snap value in trim mode
	double				m_dLastSetValue;		// unquantized raw last set value
	double				m_dMin;					// internal ctrl value min/max (usually 0..1)
	double				m_dMax;
	double				m_dSmallSteps;			// Number of small increment steps to take
	double				m_dLargeSteps;			// Number of larbe increment steps to take
	int					m_nValueSteps;			// 0 = floating point res.  non-0 = number of allowd steps between min and max
	CString				m_strName;				// Display name
	CString				m_strShortName;		// Maybe a crunched version of the name
	CString				m_strTip;				// some controls may want a different tip from the name
	CString				m_strValue;				// some controls may want to cache this
	CString				m_strTTDelimiter;		// Tool Tip Delimiter String

	DWORD				m_dwTicLastChange;	// System mS count of last value change
	
	int					m_nPunchCount;
	
	CNWControlUIAProvider*	m_pCtrlUIAProvider;	// UI Automation provider for this control
	

	// WARNING: yes, the m_bIsFixed : 2; syntax is a bitfield.  It is not portable code when dealing with 
	// externally produced data files.
	// C gives no guarantee of the ordering of fields within machine words, so if you do use them for the latter reason(externally produced data files)
	// , your program will not only be non-portable, it will be compiler-dependent too. 
	// The number after the : is the number of bits used for the field.
	BOOL					m_bIsFixed : 2;			// Is the position of this control fixed
	BOOL					m_bCapture : 2;			// Does it have mouse capture (virtual mouse capture)
	BOOL					m_bVisible : 2;			// Is it visible
	BOOL					m_bAvailable : 2;			// Control may be "unavailable" (ie: user tabs in SONAR )
	BOOL					m_bFocus : 2;				// Does this control have focus
	BOOL					m_bHoverEffect : 2;		// Turned on when MouseEnter is called
	BOOL					m_bPtClickSet : 2;		// Whether m_ptClick is set for Right Click tool
	BOOL					m_bReassignable : 2;		// can change the Parameter Number?
	// 16-bits
	BOOL					m_bEnabled : 2;			// Like a CWnd::EnableWindow() API
	BOOL					m_bActivity : 2;			// used to indicate Activity
	// Some Generic Automation states:
	BOOL					m_bTrimMode : 2;		// Could mean SONAR's Offset Mode
	BOOL					m_bGanged : 2;			// we are associated with some other controls
	BOOL					m_bArmed : 2;			// Armed for some sort of Automation recording
	BOOL					m_bRecording : 2;		// Currently Recording Automation (punched in)
	BOOL					m_bAutoEnable : 2;		// Automation is Enabled
	BOOL					m_bMakeShortStr : 2;	// normally TRUE.  if false, don't call the makeShortLabel
	// 16-bits
	
	bool					m_bNotifySiteAlways;		// normall FALSE.  If true, always call OnValueChange() even if bSelf is false
	bool					m_bHasData;				// Automation data exists for this control
	bool					m_bSharedUserFont;
	bool					m_bDrawEllipsis;			// Use an ellipsis when text is too long for the control

	COLORREF				m_rgbGang;				// a color to associate with other controls in the gang


	int					m_cxGangOffset;		// x offset for Gang indicator ( - means from right side )
	int					m_cyGangOffset;		// y offset for Gang indicator ( - means from bottom )

	static int			sm_nInstance;

	static CPen			sm_PenArmed;
	static CPen			sm_PenPunched;

	UINT					m_idbAutoIndicator;	// ID of an automation indicator bitmap

	CFont *				m_pFntUser; //not static since a site will probably create the user font and give it to us.

	double				m_dCtrlValDeltaCoarse;	// coarse control value delta for mouse wheel increments (0-1)
	double				m_dCtrlValDeltaFine;		// fine control value delta for mouse wheel increments (0-1)

private:
	// a concrete instance of Attributes.  m_pAttributes points to this by default
	NWCTRLATTRIB		m_LocalAttributes;	

}; // CNWControl






/////////////////////////////////////////////////////////////////////////////
// CNWControlSite
// A mostly abstract interface which a CWnd Based host window 
// can implement to host a list of CNWControls.
class CNWControlSite
{
public:
	virtual ~CNWControlSite();

	// Application startup and cleanup
	static HRESULT InitInstance();
	static HRESULT ExitInstance();

	// PURE virtuals
	virtual void			InvalidateAllControls() { GetCWnd()->Invalidate(); }
	// Windows Function wrappers
	virtual void			LocalToScreen( CPoint *ppt ) { GetCWnd()->ClientToScreen( ppt ); }
	virtual void			ScreenToLocal( CPoint *ppt ) { GetCWnd()->ScreenToClient( ppt ); }
	virtual CWnd*			GetCWnd() PURE;
	// Allow controls to force Layout Recalc on strip
	virtual void			RecalcLayout( void ) PURE;
	virtual ULONG			GetContainerNumber() const PURE;		// container number for this site
	virtual int				GetContainerIndex() const PURE;
	virtual void			PopEdit( CProperty* pProp, const CRect& rc, BOOL& bChanged ) PURE;

	// public accessor for adding controls
	// when we did modular prochan and needed to have common preset code, the control site was passed into the class as a member.
	// therfore the helper to add controls needed public access.  Rather than ugly friending, just add access
	virtual void			AddControl( CNWControl* pControl, BOOL bTabStop = TRUE, BOOL bTopZ = FALSE )
	{
		return addControl(pControl, bTabStop, bTopZ);
	}

	// ditto for removing controls
	virtual void			RemoveControl( CNWControl* pControl )
	{
		return removeControl( pControl );
	}

	// ditto for layout
	void					PlaceControlAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor )
	{
		return placeControlAndOffset(pC, pt, nXFactor, nYFactor);
	}

	// These two really should be PURE, but
	// it was easier to get it to compiler quicker this way!
	virtual BOOL			EnterQuickGangMode() { return FALSE; };
	virtual void			LeaveQuickGangMode() {};
	virtual BOOL			GetIsQuickGanged() { return m_bIsQuickGanged; }

	// Translate a Raw keyboard character to a CNWControl::EKeyboard enum
	virtual CNWControl::EKeyboard		TranslateKey( UINT nChar );

	// Callbacks from Control UI actions
	virtual void			DoCtrlContextMenu( CNWControl* pSource, UINT nFlags, CMenu& rMenu ){}
	virtual void			DoCtrlActivate( CNWControl* pSource, UINT nFlags ){}
	virtual void			RefreshControl( CNWControl* pSource ) {};	// request to be refreshed from the App
	virtual void			OnValueChange( CNWControl* pSource ) {};
	virtual void			OnFirstPunchIn( CNWControl* pSource ) {};
	virtual void			OnFinalPunchOut( CNWControl* pSource ) {};
	virtual void			OnDragOver( CNWControl* pCtrl ) {}
	virtual void			OnDropComplete( CNWControl* pOriginal ) {};
	virtual void			OnRequestValueString( CNWControl* pSource, CString* pstr, double* pdVal = NULL ) {};
	virtual void			UpdateAccessibility( CNWControl* pSource ) {}

	virtual void			NotifyChange( LONG_PTR lhint, LONG_PTR ldata = -1 ) {}
	virtual void			MarkDocDirty() {}

	virtual void			GetControlsRect( CRect* pRect ) const;
	virtual void			RefreshAllControls();			// cause controls to obtain a new state from the seq.
	virtual void			PaintAllControls( CDC* pDC );
	virtual void			PaintAllControlsCWG( CWGraphicsCtx * pcwg ); // parallel version for graphics lib.

	virtual BOOL			IsCurrent() const { return FALSE;}		// does this site represent the "current" track
	virtual BOOL			IsSelected() const { return FALSE; }	// does this site represent a "selected" track

	virtual BOOL			IsBus() const { return FALSE; }			// does this site represent a bus

	// Microsoft UI Automation support
	virtual CNWControlSiteUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );
	CWnd* GetMainFrame() { return m_pMainFrm; }
	BOOL IsAccessibilityEnabled() { return (m_nAccessibilityMethod > 0); }
	static void SetAccessibilityMethod( int nMethod ) { m_nAccessibilityMethod = nMethod; }

	// Tabbing / Keyboard Support
	virtual void			TabNext();
	virtual void			TabPrev();
	virtual void			TabTo( int ix );
	void						TabFirst();
	void						TabLast();
	virtual int				GetTabIx();
	virtual BOOL			IsTabStart();
	virtual BOOL			IsTabEnd();
	virtual void			InvalidateTabList( ) { m_bTabValid = FALSE; }
	virtual BOOL			OnKeyboardDN( CNWControl::EKeyboard kbd );	// pass thru to Active Control
	virtual BOOL			OnKeyboardUP( CNWControl::EKeyboard kbd );	// pass thru to Active Control

	virtual NWControlList& GetControlList() { return m_listControls; }
	virtual NWControlList& GetTabList() { return m_tabList; }

	void						SetTopmostControl( CNWControl* pCtrl ) { m_pCtrlTopMost = pCtrl; }

	// Support for "similar" controls. 
	// NOTE: this will not work if the app does not set up the controls'
	// typeId and type info (CNWControl::SetTypeInfo). Project 5 does NOT do this
	virtual CNWControl*	FindSimilarControl( const TYPEID& typeId, const TYPEID& typeGeneric, long lExtra, const CLSID * pclsidFilter );
	virtual BOOL			TabToSimilarControl( const TYPEID& typeId, const TYPEID& typeGeneric, long lExtra, const CLSID * pclsidFilter );


	// Tool Tip Support
	virtual	COurToolTipCtrl*	GetToolTipCtrl();
	virtual	COurToolTipCtrl*	RawGetToolTipCtrl() { return m_pTt; }
	virtual	CCtrlMouseTip*		GetDynamicMouseTipCtrl();
	// this method allows for things like, say you have a view that's not a control site
	// with strips that are control sites.  Well, if you had 127 strips you wouldn't want 127 windows!
	// instead, you could have the view create a control and then each strip would set it's site to the same tool
	// tip control.  One big caveat is however, that you must insure your strips parameter ids are unique.
	// one way to accomplish that with DWORDs is by doing a low word, hi word kind of thing.
	// That would give you 65,535 strips with 65,535 controls each.
	// NOTE: to do this successfully, you must override
	// GetNextUId() and have your site return the correct id as the default behavior simply bumps up a count which wouldn't work in the sharing case.
	virtual bool				SetToolTipCtrl(COurToolTipCtrl* pTt);

	// Cursor Hiding Support
	virtual void			ShowCursor( BOOL bShow );
	virtual void			LostCapture();  // Should be called by base classes when the cursor looses mouse capture
	
	virtual void			OnNameChange( CString& strName ) {}	// A name control has been changed by user

	virtual void			RedrawControl( CNWControl* pControl, CRect* pRectToInvalidate = NULL );
								
	virtual					CNWControl* GetActiveControl( void ) const { return m_pActiveControl; }
	virtual void			SetActiveControl( CNWControl* pActiveControl, BOOL bForceVisible = FALSE, BOOL bSetActiveTrack = FALSE );

	virtual CNWControl*	FindControlAtPt( const CPoint& pt ){ return findControl(pt); }
								
	virtual UINT			GetNextUId() { return m_uNextId++; }

	// Aggregate show/hide
	virtual void			HideAllControls();
	virtual void			ShowAllControls();

	// COleDropTarget Overrides...
   virtual BOOL			OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point );
   virtual DROPEFFECT	OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
   virtual DROPEFFECT	OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
   virtual void			OnDragLeave( CWnd* pWnd = NULL );

#if _DEBUG
	void						TweakControlPos( CNWControl* pC, int xDelta, int yDelta );
	void						GetPosTweak( CNWControl* pC, int* pxDelta, int* pyDelta );
#endif // _DEBUG

protected:
	CNWControlSite();
	virtual void			sortTabList( );
	virtual void			addControl( CNWControl* pControl, BOOL bTabStop = TRUE, BOOL bTopZ = FALSE );
	virtual void			cleanUpControlList();
	virtual void			removeControl( CNWControl* pControl );	

	virtual BOOL			findControlAndSetActive( const CPoint& pt );
	virtual CNWControl*	findControl( const CPoint& pt );
	virtual CNWControl*	findControl( UINT uID ); // uses tip id
	virtual bool		isSameControl( CNWControl * p1, CNWControl * p2 );

	// layout helper
	void						placeControlAndOffset( CNWControl* pC, CPoint& pt, int nXFactor, int nYFactor );

	// hover():  This function checks to see if the mouse is over a different control
	// and calls HandleMouseEnter() and HandleMouseLeave() as appropriate.  It also 
	// calls _TrackMouseEvent() which will cause a WM_MOUSELEAVE message to be posted
	// to the host CWnd when the mouse leaves.  That message's handler should call
	// handleLeaveStrip() below.
	virtual void			hover( const CPoint& pt );
	
	// handleLeaveStrip():  This function could be implemented here but it is left PURE
	// so the host CWnd is forced to implement it.  It should be called in the
	// CWnd's WM_MOUSELEAVE message handler.  If you don't care about hover effects, you
	// can skip the WM_MOUSELEAVE handler and just implement this as a No-Op
	virtual void			handleMouseLeave() PURE;

	virtual void			assignFocus( BOOL bState = TRUE );
	CString					getGUIDName( const GUID& guid );

	BOOL						canTabTo( CNWControl* pCtrl );
	CNWControl*				getFirstTab();
	CNWControl*				getLastTab();

	HRESULT					subclassWindow();
	LRESULT					onUIAutomationProc( LONG_PTR wpOld,
										HWND hwnd,
										UINT uMsg,
										WPARAM wParam,
										LPARAM lParam );

	static LRESULT CALLBACK HandleUIAutomationProc( HWND hwnd,
											UINT uMsg,
											WPARAM wParam,
											LPARAM lParam );
	struct ProcAndSite
	{
		LONG_PTR wpOld;
		CNWControlSite* pSite;
	};

	static std::map<HWND, ProcAndSite> s_mapWndProcOld;

	/////////////////////////////////////////////////////////////////////////////
	// Useful CNWControlSite functors
	// Compare the "logical" tab order based on the XY coords of
	// the origin of two controls
	struct foGreaterXY :public std::binary_function<CNWControl*,CNWControl*,bool>
	{
		bool operator()( CNWControl* p1, CNWControl* p2 )
		{
			const CPoint& pt1 = p1->GetOrigin();
			const CPoint& pt2 = p2->GetOrigin();
			return pt2.y > pt1.y || (pt2.y == pt1.y && pt2.x > pt1.x);
		}
	};


	struct fo_AccumRects : public std::unary_function<CNWControl*,void>
	{
		fo_AccumRects( CRect* prc ) : std::unary_function<CNWControl*,void>(), m_prc(prc) {}
		inline void	operator() ( CNWControl* pControl ) const
		{
			CRect rc( pControl->GetControlRect() );
			if ( m_prc->IsRectEmpty() )
				*m_prc = rc;
			else
				*m_prc |= rc;
		}
		CRect* m_prc;
	};

	struct fo_RefreshControls : public std::unary_function<CNWControl*,void>
	{
		inline void operator() ( CNWControl* pControl ) const { pControl->RefreshFromApp(); }
	};


	struct fo_PaintControls : public std::unary_function<CNWControl*,void>
	{
		fo_PaintControls( CDC* pDC, CNWControl* pExclude = NULL ) : 
							std::unary_function<CNWControl*,void>(), 
							m_pDC(pDC),
							m_pCtrlExclude( pExclude ) {}
		inline void operator() ( CNWControl* pControl ) const
		{
			// Only repaint this control if its in the Clip Region
			if ( m_pCtrlExclude != pControl && pControl->GetIsVisible() )
			{
				const CRect rcControl = pControl->GetControlRect();
				if ( m_pDC->RectVisible( &rcControl ) )
					pControl->Render( m_pDC );
			}
		}
		CDC* m_pDC;
		CNWControl* m_pCtrlExclude;
	};

	// fo_PaintControlsCWG
	// inner struct.
	struct fo_PaintControlsCWG : public std::unary_function<CNWControl*,void>
	{			
		fo_PaintControlsCWG( CWGraphicsCtx * pcwg, CNWControl* pExclude = NULL ) : 
							std::unary_function<CNWControl*,void>(), 
							m_pCWG(pcwg),
							m_pCtrlExclude( pExclude ) {}
		void DoOperator( CNWControl * pControl ) const;	
		
		void operator() ( CNWControl* pControl ) const
		{
			DoOperator(pControl);
		}	
	
		CWGraphicsCtx * m_pCWG;
		CNWControl* m_pCtrlExclude;
	};

	struct fo_AssignFocus : public std::unary_function<CNWControl*,void>
	{
		fo_AssignFocus( CNWControl* pC, BOOL bF ) : std::unary_function<CNWControl*,void>(),m_pC(pC), m_b(bF){}
		inline void operator()( CNWControl* p ) const
		{
			if (p == m_pC)
				p->SetFocus( m_b );
			else
			{
				if (p->IsFocus())
					p->SetFocus( FALSE );
			}
		}

		CNWControl* m_pC;
		BOOL m_b;
	};

	struct fo_IsAtPoint : public std::unary_function<CNWControl*,bool>
	{
		fo_IsAtPoint( const CPoint& pt ) : std::unary_function<CNWControl*,bool>(), m_pt(pt) {}
		inline bool operator() ( CNWControl* pC )const { return TRUE == pC->HitTest( m_pt ); }

		CPoint m_pt;
	};

	struct fo_IsID : public std::unary_function<CNWControl*,bool>
	{
		fo_IsID( UINT id ) : std::unary_function<CNWControl*,bool>(), m_id(id) {}
		inline bool operator() ( CNWControl* pC ) const { return pC->GetTipId( ) == m_id; }
		UINT m_id;
	};

private:
	CCtrlMouseTip*			m_pMt;
	COurToolTipCtrl*		m_pTt;

protected:
	
	UINT						m_uNextId;
	CNWControl*				m_pActiveControl;				// the one we're wiggling
	CNWControl*				m_pLastMouseEnterControl;	// Control that MouseEnter was last sent to.
	CNWControl*				m_pCtrlTopMost;				// a Top Zorder control for painting and hit testing
	NWControlList			m_listControls;				// vector of CNWControl*
	NWControlList			m_tabList;						// sublist for tabbing purposes
	DWORD						m_dwTimeStamp;					// available timestamp ( can be used to throttle updates )
	int						m_nShowCursor;				// Count that keeps cursor state. < 0 means hidden, >= 0 means visible
	BOOL						m_bTabValid;					// tab list needs sorting before use
	BOOL						m_bIsQuickGanged;				// TRUE if this control site is currently part of a QuickGang

	bool						m_bExternalTipCtrl;			// if someone else create tip control we don't want to go deleting their control!

	CNWControlSiteUIAProvider*	m_pCtrlSiteUIAProvider;	// UI Automation provider for this control site
	bool						m_bIsSubclassed;
	static CWnd*			m_pMainFrm;
	static int				m_nAccessibilityMethod;

#if _DEBUG
	public:
	std::map<CNWControl*,CPoint>	m_mapTweakPos;		// position tweaking map
	protected:
#endif // _DEBUG

}; // CNWControlSite



//--------------------------------------------------------------------
/// A sandwitch class for setting up the Fonts and restoring the CD
class CNWControlSetFont
{
public:
	CNWControlSetFont( CNWControl* pCtrl, CDC* pDC );
	~CNWControlSetFont();
private:
	CNWControl*		m_pCtrl;
	CDC*				m_pDC;
	CFont*			m_pfntOld;
	COLORREF			m_crOld;
};

// forwards
class CWGTempLogFont;
class CWGTempTxtColor;

//--------------------------------------------------------------------
/// A sandwitch class for setting up the Fonts and restoring the CD
class CNWControlSetFontCWG
{
public:
	CNWControlSetFontCWG( CNWControl* pCtrl, CWGraphicsCtx* pcwg );
	~CNWControlSetFontCWG();
private:
	CNWControl*		m_pCtrl;
	CWGraphicsCtx*	m_pCWG;
	CWGTempLogFont * m_ptlf;	
	CWGTempTxtColor * m_pttc;	
};



#endif // !defined(AFX_NWCONTROL_H__374E4A75_4689_4097_A83B_9B49FC42787F__INCLUDED_)
