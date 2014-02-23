// NWBitmapButton.h: interface for the CNWBitmapButton class.
//
// This file defines the following windowless buttons and other widgets:
//
//   - CNWToggleControl
//   - CNWBitmapButton
//   - CNWMultiBtn
//   - CNWSpinner
//   - CNWLabelButton
//   - CNWBitmap
//   - CNWTooltipArea
//	  - CNWToolbarBtn
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NWBITMAPBUTTON_H__B979A96A_DA40_4C04_92B7_D22110453E28__INCLUDED_)
#define AFX_NWBITMAPBUTTON_H__B979A96A_DA40_4C04_92B7_D22110453E28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NWControl.h"

#include "BitmapCache.h"


///////////////////////////////////////////////////////////////////////////////
/// A Windowless control with toggle action
/// This class handles mouse interaction for a windowless control with toggle
/// action. Derive from this to create buttons with a unique look and feel.
///////////////////////////////////////////////////////////////////////////////
class CNWToggleControl : public CNWControl
{
public:

	enum	EActive {ACTIVEDOWN, ACTIVEUP, ACTIVEBOTH}; // needed both for ffwd/rew buttons that really do a DoCtrlActivate both on up and down!
									CNWToggleControl( CNWControlSite* pSite, EActive eActiveUp = ACTIVEUP );
	virtual						~CNWToggleControl();

	virtual void				HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void				HandleLMouseDClk( UINT nFlags, const CPoint& pt);
	virtual void				HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void				HandleRMouseUP( UINT nFlags, const CPoint& pt);

	// Get/set the check/toggled state 
	BOOL							GetCheck() const;
	void							SetCheck( BOOL bCheck, BOOL bSelf );

	void							SetNoActivate(BOOL b){ m_bDoActivate = !b; }
	void							SetNoToggle(BOOL b){ m_bDoToggle = !b; }

	virtual BOOL				HandleKeyboardDN( EKeyboard kbd );
	
	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );

protected:
	EActive			m_eActiveUp; ///< this says whether to do activate/check on mouse up or down or both
	BOOL				m_bDoActivate; ///< if all you want is a menu toggle, you don't want activate or check
	BOOL				m_bDoToggle; ///< be able to control if auto toggle happens or not.
};


///////////////////////////////////////////////////////////////////////////////
/// Graphical button with semantics similar to MFC's CBitmapButton. Resources
/// are stored in the bitmap cache for maximum efficiency.
///////////////////////////////////////////////////////////////////////////////
class CNWBitmapButton : public CNWToggleControl  
{
public:
						CNWBitmapButton( CNWControlSite* pSite, 
												UINT nIDNormal, 
												UINT nIDSel = 0, 
												UINT nDisable = 0,
												EActive eActiveUp = ACTIVEUP,
												int cxEndCap = 0,
												bool bPinBitmap = false );
	virtual			~CNWBitmapButton();
	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	// Should we draw a capture rect 
	void SetShowCapture( BOOL bShowCapture = TRUE ) { m_bShowCapture = !!bShowCapture; }
	void SetTriState( bool b );

	// Allow the images to be swapped out
	virtual void SetImages( UINT nIDNormal, UINT nIDSel = 0, UINT nDisable = 0 );

protected:

protected:
	enum				{IMG_UNSEL, IMG_DIS_TRI, IMG_SEL, IMG_COUNT};
	UINT				m_aIdBitmaps[IMG_COUNT];	// normal, disable/tri-state, sel

	bool				m_bShowCapture; // Should we draw a capture rect around the control
	bool				m_bPinBitmaps;
	bool				m_bTriState;	// Interpret the m_idDisable as a tristate bitmap

	int				m_cxCap;
	int				m_cxImg;
	int				m_cyImg;
	int				m_cxTile;
};

///////////////////////////////////////////////////////////////////////////////
/// Class CNWMultiBtn
///
/// A bitmap button that can:
/// (1) Have up to seven states: normal, normal down, normal hover, selected,
///     selected down, selected hover, and disabled.
/// (2) Optionally have multiple _T("frames.") Frames can be used either to animate the
///     button (e.g., a Play button that dims and brightens), or to define a large
///     number of same-sized buttons using one convenient bitmap (e.g., to create a
///     button grid like Kinetic's Sequence Picker).
///
/// This is a versatile control that's also a little hard to explain. To get the idea
/// quickly, look at [VSS2002]/$/Src2002/Design/NWControls/CNWMultiBtn Design.jpg.
/// (NOTE (AEB): I'll also put it in TTSNET, so even if VSS gets reorganized,
/// hopefully you'll still be able to find it by searching for keyword CNWMultiBtn.)
//
///////////////////////////////////////////////////////////////////////////////

class CNWMultiBtn : public CNWToggleControl
{
public:
	// Flags to specify which states you will be specifying in the button art.
	// However many states you specify (up to 7), CNWMultiButton will split your
	// art horizontally into that many equal pieces, one for each state. You can
	// use any/all of the states below by ORing them together, with two rules:
	// (1) NORM must always be one of the states.
	// (2) Whatever states you provide, they must be in the EXACT order below.
	//     For example, if you only want Normal states, the columns in your art
	//     must be [ Normal, Normal Down, Normal Hover ], left to right.
	
	// the storage of the states is in a 26-bit bitfield of the DWORD.
	// that allows up to 3FFFFFF or 67108863.
	// Present max is 255, so, if you need to add more states, consider whether or not you need
	// more storage.  This reduction in size was done for the Step Sequencer where all the individual
	// steps are CNWMultiBtn controls.  This was done to reduce loading time due to mem allocator.
	enum EStates
	{
		// Individual states.
		NORM		= 0x001,				// Normal button.
		NORMDWN	= 0x002,				// Normal button is being pressed down.
		NORMHVR	= 0x004,				// Mouse is hovering over normal button.
		SEL		= 0x008,				// Selected button (usually _T("lit up")).
		SELDWN	= 0x010,				// Selected button is being pressed down.
		SELHVR	= 0x020,				// Mouse is hovering over selected button.
		SELDIS   = 0x040,				// Button is disabled in the on state, can't be clicked.
		DIS		= 0x080,				// Button is disabled, no over effect, can't be clicked.		
		TRI		= 0x100,				// Button is in "indeterminate" state like windows checkbox tristate
		TRIDWN	= 0x200,				// Button is in third state pressed down.
		TRIHVR	= 0x400,				// Mouse is hovering over third state button.
		TRIDIS	= 0x800,				// tri state but disabled.

		// Combined states, for convenience:
		// - 2 states: just normal and selected.
		NS = NORM | SEL,
		// - 3 states: normal, selected, disabled.
		NSD = NORM | SEL | DIS,											
		NS_HVR = NORM | NORMHVR | SEL,
		// - 4 states: normal up/down/hover and disabled.
		NORMONLY = NORM | NORMDWN | NORMHVR | DIS,
		// - 4 states: normal, selected, disabled, disabled selected
		NSD_SD = NORM | SEL | SELDIS | DIS,
		// - 5 states: normal up/down, selected up/down, disabled.
		NSD_NOHVR = NORM | NORMDWN | SEL | SELDWN | DIS,
		// - 5 states: normal up/down/hover, sel, dis
		NSD_HVR = NORM | NORMDWN | NORMHVR | SEL | DIS,
		// - 5 states:
		NSD_DN_SD = NORM | NORMDWN | SEL | SELDIS | DIS,
		// - 6 states: everything but disabled.
		ALLBUTDIS = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR,
		// - All 7 states
		ALL = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR | DIS,
		// - 7 with seldis instead of selhvr
		ALL_SD = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELDIS | DIS,
		// 8 states All 7 + the single TRI
		ALL_1TRI = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR | DIS | TRI,
		// - 8 states no tri
		ALL_NOTRI = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR | SELDIS | DIS,
		// - ALL + tri 10 states
		ALLTRI = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR | DIS | TRI | TRIDWN | TRIHVR,
		// - ALLTRI_SD, 12 STATES
		ALLTRI_SD = NORM | NORMDWN | NORMHVR | SEL | SELDWN | SELHVR | SELDIS | DIS | TRI | TRIDWN | TRIHVR | TRIDIS
	};

	struct MultiButtonMetrics
	{
		int m_nStateCount;				// # of states (derived from the flags passed in).
		// POINTS has two shorts.
		POINTS m_sCell;						// Size of one cell of the art.

		// The X-origins of the columns holding the different states. See States above for info.
		short m_nXNorm;
		short m_nXNormDwn;
		short m_nXNormHvr;
		short m_nXSel;
		short m_nXSelDwn;
		short m_nXSelHvr;
		short m_nXSelDis;
		short m_nXDis;
		short m_nXTri;
		short m_nXTriDwn;
		short m_nXTriHvr;
		short m_nXTriDis;

		MultiButtonMetrics() : 
			m_nXNorm(0),
			m_nXNormDwn(0),
			m_nXNormHvr(0),
			m_nXSel(0),
			m_nXSelDwn(0),
			m_nXSelHvr(0),
			m_nXSelDis(0),
			m_nXDis(0),
			m_nXTri(0),
			m_nXTriDwn(0),
			m_nXTriHvr(0),
			m_nXTriDis(0),
			m_nStateCount(0)
		{
			m_sCell.x = 1;
			m_sCell.y = 1;
		};
	};

	// Ctor. See CPP file for info.
	CNWMultiBtn( CNWControlSite* pSite, DWORD dwStates = NORM|SEL, EActive eActiveUp = ACTIVEUP );

	// Dtor.
	virtual ~CNWMultiBtn();

	// Load in art.
	void SetArt( UINT nIDArt, DWORD dwStates, int nFrameCount = 1, int nUseFrame = 0, MultiButtonMetrics * pMetrics = NULL );
	void SetArt( UINT nIDArt, bool * pbRedrawIfChanged = NULL ); // wacky to have to resort to this, but using bool results in ambiguity with the first version!
	void SetArtPreAllocMode(){ m_bArtPrealloc = true; }

	MultiButtonMetrics & GetMetrics(){ return m_metrics; }
	void SetMetrics(MultiButtonMetrics &m)
	{
		m_metrics.m_nStateCount = m.m_nStateCount;
		m_metrics.m_nXDis = m.m_nXDis;
		m_metrics.m_nXNorm = m.m_nXNorm;
		m_metrics.m_nXNormDwn = m.m_nXNormDwn;
		m_metrics.m_nXNormHvr = m.m_nXNormHvr;
		m_metrics.m_nXSel = m.m_nXSel;
		m_metrics.m_nXSelDwn = m.m_nXSelDwn;
		m_metrics.m_nXSelHvr = m.m_nXSelHvr;
		m_metrics.m_nXSelDis = m.m_nXSelDis;
		m_metrics.m_nXTri = m.m_nXTri;
		m_metrics.m_sCell = m.m_sCell;
		m_metrics.m_nXTriDwn = m.m_nXTriDwn;
		m_metrics.m_nXTriHvr = m.m_nXTriHvr;
		m_metrics.m_nXTriDis = m.m_nXTriDis;
	}	

	void SetColorGroup( UINT nColorGroup );

	// Get and set current frame. (For animating, try Animate() instead.)
	int GetFrame() { return m_nUseFrame; }
	void SetFrame( int nUseFrame );

	// Set a text margins, pixle offsets from source image rect
	void SetTextMargins( int cxLeft, int cyTop, int cxRight, int cyBottom ) 
		{ 
		  VERIFY(cxLeft <= MAXSHORT);
		  VERIFY(cyTop <= MAXSHORT);
		  VERIFY(cxRight <= MAXSHORT);
		  VERIFY(cyBottom <= MAXSHORT);
		  m_TextOffsetL = (short)cxLeft;
		  m_TextOffsetT = (short)cyTop;
		  m_TextOffsetR = (short)cxRight;
		  m_TextOffsetB = (short)cyBottom;
		}

	// Allow same image to be used with cropped edges or full edge with shadow, image pixel margin
	void SetImageMargins( int nLeftMargin , int nRightMargin );

	// Move to the next frame, thus producing an animated effect. Only works when
	// multiple frames are provided, of course.
	void Animate( BOOL bBounce );

	// Draw the button.	
	virtual void Render( CDC* pDC );
	virtual void RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void RenderBtn( CDC* pDC, CRect &rcCtrl );
	virtual void RenderTxt( CDC* pDC, CRect &rcCtrl );

	virtual void RenderBtnCWG( CWGraphicsCtx* pCWG, CRect &rcCtrl );
	virtual void RenderTxtCWG( CWGraphicsCtx* pCWG, CRect &rcCtrl );

	virtual void SetUseGDIPlus(BOOL b = TRUE){ m_bUseGDIPlus = TRUE; }

	// Set if you want to resize this control.  The middle of the source bitmap
	// will be tiled to fill the required space
	void SetIsResizable( BOOL bDoResize, int cxLeft, int cxRight );



protected:
	// Given the control art and information we know about that art, calculate the
	// metrics we use to draw.
	void calcDrawMetrics();

	int calcXCopyFrom();

	// Draw metrics.	
	MultiButtonMetrics m_metrics;	// 24

	short		m_TextOffsetL;
	short		m_TextOffsetT;
	short		m_TextOffsetR;
	short		m_TextOffsetB;		// 8
	// text offset (optionally passed in via SetTextMargins()).

	// Art.
	UINT		m_nIDArt;			// 4

	// Resizable button
	short		m_cxImgLeft;		// 2
	short		m_cxImgRight;		// 2

	short		m_cxLeftMargin;		// 2
	short		m_cxRightMargin;	// 2

	short		m_nFrameCount;		// 2	// # of frames (passed in explicitly).
	// Current frame.
	short		m_nUseFrame;		// 2


	// Whether the current animation direction is up or down. See Animate().
	BOOL		m_bAnimateUp : 2;
	BOOL		m_bAllowResize : 2;	
	BOOL		m_bArtPrealloc : 2; // specialized mode that does NOT load/free any art because the app has done it.
	BOOL		m_bUseGDIPlus : 2; // Do pretty focus
	// 6 bits
	// States to use.
	DWORD		m_eStates : 24;		// 4, // remaining 24 bits.  // allows for up to 0xFFFFFF value.  Presently max is 255	
};

///////////////////////////////////////////////////////////////////////////////
// Just like MultiBtn but it allows to set several buttons out of which only one is enabled at a time.

class CNWMultiRadioBtn : public CNWMultiBtn
{
public:
	// Ctor. See CPP file for info.
	CNWMultiRadioBtn( CNWControlSite* pSite, DWORD dwStates = NORM|SEL, BOOL bHorizontal = TRUE ) :
		CNWMultiBtn( pSite, dwStates, ACTIVEUP ),
		m_nHoverButton( 0 ),
		m_bHorizontal( bHorizontal )
	{}

	// After calling SetArt for the first button, use this method to add more buttons.
	// All art should have the same dimensions and same states set on the first "set art" call.
	void AppendArt( UINT idbArt );
	
	virtual void RenderBtn( CDC* pDC, CRect &rcCtrl );
	virtual void RenderBtnCWG( CWGraphicsCtx* pCWG, CRect &rcCtrl );
	virtual void HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void HandleLMouseUP( UINT nFlags, const CPoint& pt);

protected:

	UINT m_nHoverButton;
	std::vector<UINT> m_aArtBtns;

	BOOL m_bHorizontal;
};

///////////////////////////////////////////////////////////////////////////////
// Just like MultiBtn, except it operates like a toggle while it is pressed (issues the command on Btn down)
// And then again on Btn Up (needed for Transport controls: FastFwd, FastRev).  So it's active on both down and up.

class CNWMultiToggleBtn : public CNWMultiBtn
{
public:
	// Ctor. See CPP file for info.
	CNWMultiToggleBtn( CNWControlSite* pSite, DWORD dwStates = NORM|SEL, EActive eActiveUp = ACTIVEUP ) :
		CNWMultiBtn( pSite, dwStates, eActiveUp ),
		m_bActivated( false )
	{}

	virtual void HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void HandleLMouseUP( UINT nFlags, const CPoint& pt);

protected:
	bool m_bActivated;

};

///////////////////////////////////////////////////////////////////////////////

class CNWToolbarBtn : public CNWToggleControl
{
public:
	// NOTE unlike a CNWMultiBtn, the art states are in separate strips.
	// toolbar strips have multiple images (cmds)
	// This was created to facilliate moving to CNWControlBar to update the look.
	// The states that it replaces is:
	// UINT idNormal, UINT idDis, UINT idHover, UINT idOn, UINT idClicked
	// It's expected not every button will have an on state.

	// Ctor. See CPP file for info.
	CNWToolbarBtn( CNWControlSite* pSite, UINT id, unsigned short nWidth, unsigned short nIx, UINT idNormal, UINT idDis, UINT idHover, UINT idOn, UINT idClicked, EActive eActiveUp = ACTIVEUP );

	// Dtor.
	virtual ~CNWToolbarBtn();

	void SetColorGroup( UINT nColorGroup );

	// Set a text margins, pixle offsets from source image rect
	void SetTextMargins( int cxLeft, int cyTop, int cxRight, int cyBottom ) 
		{ 
		  VERIFY(cxLeft <= MAXSHORT);
		  VERIFY(cyTop <= MAXSHORT);
		  VERIFY(cxRight <= MAXSHORT);
		  VERIFY(cyBottom <= MAXSHORT);
		  m_TextOffsetL = (short)cxLeft;
		  m_TextOffsetT = (short)cyTop;
		  m_TextOffsetR = (short)cxRight;
		  m_TextOffsetB = (short)cyBottom;
		}

	// Allow same image to be used with cropped edges or full edge with shadow, image pixel margin
	void SetImageMargins( int nLeftMargin , int nRightMargin );

	// Draw the button.
	virtual void Render( CDC* pDC );
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void RenderBtn( CDC* pDC, CRect &rcCtrl );
	virtual void RenderTxt( CDC* pDC, CRect &rcCtrl );

	virtual void RenderBtnCWG( CWGraphicsCtx* pCWG, CRect &rcCtrl );
	virtual void RenderTxtCWG( CWGraphicsCtx* pCWG, CRect &rcCtrl );

	virtual void		SetUseGDIPlus(BOOL b = TRUE){ m_bUseGDIPlus = TRUE; }

protected:

	short		m_TextOffsetL;
	short		m_TextOffsetT;
	short		m_TextOffsetR;
	short		m_TextOffsetB;		// 8
	// text offset (optionally passed in via SetTextMargins()).

	// Art.
	UINT		m_idNormal;
	UINT		m_idDis;
	UINT		m_idHover;
	UINT		m_idOn;
	UINT		m_idClicked;

	unsigned short		m_cxButtonWidth; // specified in construct.
	unsigned short		m_cyButtonHeight; // calculated from art height.

	short		m_cxLeftMargin;		// 2
	short		m_cxRightMargin;	// 2

	// Current frame. (what image in strip are we using?)
	unsigned short		m_nFrame;		// 2

	BOOL		m_bUseGDIPlus : 2; // Do pretty focus
};

///////////////////////////////////////////////////////////////////////////////
/// A Windowless control button with similar semantics as MFC's CButton
/// ie: it will display a label on a flat background
class CNWLabelButton : public CNWToggleControl  
{
public:
						CNWLabelButton( CNWControlSite* pSite, EActive eActiveUp = ACTIVEUP );
	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override
	bool				m_bShowFrame;

	enum EFRAMESTYLE
	{
		ERounded,
		ESquare
	};

	void				SetDeflation( int x, int y ) { m_nXDeflate = x; m_nYDeflate = y; }
	
	// api for backwards compat with s4
	void SetDrawRoundedFrame(COLORREF crFrame, BOOL bOn = TRUE)
	{
		SetDrawFrame( crFrame, bOn, ERounded );
	}

	void SetDrawFrame(COLORREF crFrame, BOOL bOn = TRUE, EFRAMESTYLE eStyle = ERounded );

	virtual void SetDrawEllipsis( bool bDrawForValue );
	virtual void SetDrawEllipsis( bool bDrawForValue, bool bDrawForLabel );

protected:
	virtual void	drawRoundedFrame( CDC* pDC );
	virtual void	drawSquareFrame( CDC* pDC );

	virtual void	drawRoundedFrameCWG( CWGraphicsCtx* pCWG );
	virtual void	drawSquareFrameCWG( CWGraphicsCtx* pCWG );

	EFRAMESTYLE		m_eFrameStyle;
	COLORREF			m_crFrame;

	int				m_nXDeflate;
	int				m_nYDeflate;

private:
	bool				m_bDrawEllipsisForLabel;
};

///////////////////////////////////////////////////////////////////////////////
/// A Windowless control button that acts like a spinner Control
/// 
class CNWSpinner : public CNWControl
{
public:
	CNWSpinner( CNWControlSite* pSite, 
							UINT nIDNormal, 
							UINT nDisable = 0
							);
	virtual ~CNWSpinner();

	virtual void	HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseDClk( UINT nFlags, const CPoint& pt );
	virtual void	HandleRMouseUP( UINT nFlags, const CPoint& pt);
	virtual void	HandleMouseMove( UINT nFlags, const CPoint& pt );
	virtual BOOL	HandleMouseWheel( UINT nFlags, short zDelta, CPoint pt);

	virtual BOOL	HandleKeyboardDN( EKeyboard kbd );

	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	// size is determined by the supplied artwork only
	virtual void	SetSize( const CSize& size, BOOL bRepaint = FALSE ){ ASSERT( 0 ); }
	virtual void	SetOrigin( const CPoint& pt, BOOL bRepaint = FALSE );
	virtual void	SetImages( UINT nIDNormal, UINT nIDDisable = 0 );
	virtual void	SetSpinnerHorz() { m_bSpinnerVert = FALSE; }
	virtual void	SetSpinnerVert() { m_bSpinnerVert = TRUE; }
	virtual void	SetDragHorz() { m_bDragVert = FALSE; }
	virtual void	SetDragVert() { m_bDragVert = TRUE; }
	
	virtual void	SetSpinArea( float fPercentSpin ) { m_fSpinRectOffset = fPercentSpin; }

	// Set if you want to resize this control.  The middle of the source bitmap
	// will be tiled to fill the required space
	void SetIsResizable( BOOL bDoResize, int cxLeft, int cxSpinRect );

	// Set if you want to draw a bar on this control. The bar will be drawn on top of the
	// bitmap background and under the text, with the specified inner margin and color.
	void SetDrawBar() { m_bDrawBar = TRUE; }
	void SetBarColor( COLORREF crColor ) { m_crBarColor = crColor; }
	void SetBarMargin( CRect rMargin ) { m_rBarMargin = rMargin; }
	void SetBarValue( double dVal ) { m_dBarVal = max( 0.0, min( 1.0, dVal ) ); }

protected:
	void				cacheRects();
	UINT				m_uNormal;
	UINT				m_uDisable;
	double			m_dClickVal;
	CRect				m_rctSpin;
	CRect				m_rctName;
	CRect				m_rctValue;

	// Spinner buttons.
	float				m_fSpinRectOffset;		// How much of the width of this control is given to spinner
														// -/+ buttons. If this is 1.0, it's the entire control. Note
														// that the actual "buttons" need to be authored into the
														// background art.
	BOOL				m_bSpinnerVert;			// Whether these buttons are side by side, or one on top of the other.
	
	// Dragging.
	BOOL				m_bDragging;				// Whether we're dragging.
	BOOL				m_bDragVert;				// Whether to respond to up-down motion, or left-right motion.

	// Resizing control.
	BOOL			m_bAllowResize;
	int			m_cxImgLeft;
	int			m_cxSpinRect;

	// Drawing a bar for the value.
	BOOL			m_bDrawBar;
	COLORREF		m_crBarColor;
	CRect			m_rBarMargin;
	double		m_dBarVal;
};					

///////////////////////////////////////////////////////////////////////////////
/// A Windowless control to draw a bitmap as part of the control site
class CNWBitmap : public CNWControl  
{
public:
					CNWBitmap( CNWControlSite* pSite, 
							   UINT nID );
	virtual			~CNWBitmap();
	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	void				SetImage( UINT idb );

	// size is determined by the supplied artwork only
	virtual void	SetSize( const CSize& size, BOOL bRepaint = FALSE ){ ASSERT( 0 ); }

protected:

	// Format names that contain an astrisks on the end so they will fit in the 
	// the specified rect
	void formatDocumentName( CString& strName, CDC* pDC, const CRect& rcText );
	void formatDocumentNameCWG( CString& strName, CWGraphicsCtx* pCWG, const CRect& rcText );

	UINT				m_uBmpId;	// id of the bitmap
};

///////////////////////////////////////////////////////////////////////////////
/// Class CNWTooltipArea
/// This is for when you don't even want to draw *anything*, you simply want to
/// make a particular area show a tooltip. To use it, just size/position it and
/// handle tooltips as you would with any other NWControl.
///////////////////////////////////////////////////////////////////////////////

class CNWTooltipArea : public CNWControl  
{
public:
	CNWTooltipArea( CNWControlSite* pSite );
	virtual			~CNWTooltipArea();
	virtual void	Render( CDC* pDC );
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override
	virtual void	HandleRMouseUP( UINT nFlags, const CPoint& pt);

protected:
};

#endif // !defined(AFX_NWBITMAPBUTTON_H__B979A96A_DA40_4C04_92B7_D22110453E28__INCLUDED_)
