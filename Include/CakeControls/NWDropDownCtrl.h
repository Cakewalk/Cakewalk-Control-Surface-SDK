// CropDownCtrl.h: interface for the CCropDownCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CROPDOWNCTRL_H__A3AF233F_6AB3_4749_BE8B_1F55E998117B__INCLUDED_)
#define AFX_CROPDOWNCTRL_H__A3AF233F_6AB3_4749_BE8B_1F55E998117B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NWControl.h"
#include "UIAProviders.h"


#define MULTI_EDIT_MAX_ROWS	1000		// Arbitrary - adjust these upward if you need to
#define MULTI_EDIT_MAX_COLS	100		// Arbitrary - adjust these upward if you need to

class CNWDropDownCtrl : public CNWControl  
{
public:
							CNWDropDownCtrl( CNWControlSite* pSite, bool bEllipsesStyle = false );
	virtual				~CNWDropDownCtrl();

	virtual void		Render( CDC* pDC );	// pure virtual Paint function
	virtual void		RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void		HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void		HandleLMouseDClk( UINT nFlags, const CPoint& pt);
	virtual BOOL		HandleKeyboardDN( EKeyboard kbd );

	// Set padding for the text in this control
	virtual void		SetTextPadding( int top, int left, int bottom, int right );
	virtual void		SetMargins( int top, int left, int bottom, int right );

	// Set the region in which LButtonUp drops down the control.  This allows
	// other regions to respond to double click
	void				SetLimitedHotSpot( BOOL bLimitedHotSpot ) { m_bLimitedHotSpot = bLimitedHotSpot; }
	
	// Set whether or not the dropdown arrow is shown.  The arrow can be turned off 
	// to allow more room for text in cases where space is at a premium
	void				SetNoDropArrow( BOOL bNoDropArrow ) { m_bNoDropArrow = bNoDropArrow; }

	void SetActivateOnClick( BOOL bActivateOnClick ) { m_bActivateOnClick = bActivateOnClick; }

	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );

protected:
	virtual void		cacheRects();

protected:

	BOOL					m_bLimitedHotSpot; // if the control has a hot spot, then
										// only drop down if they click within the 
										// hot spot, m_rctDrop.
	BOOL					m_bNoDropArrow; //don't draw the drop arrow, allowing more room for text
	CRect					m_rctDrop;
	CRect					m_rctLabel;
	BOOL					m_bActivateOnClick;
	BOOL					m_bActUp;

	int					m_cxyArrow;

	int					m_nPadTop;
	int					m_nPadLeft;
	int					m_nPadBottom;
	int					m_nPadRight;

	int					m_nLeftMarg;
	int					m_nTopMarg;
	int					m_nRightMarg;
	int					m_nBottomMarg;

	bool					m_bEllipsesStyle;
};




//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class CNWEditControl : public CNWControl
{
public:
							CNWEditControl( CNWControlSite* pSite );
	virtual				~CNWEditControl();

	virtual void		HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void		HandleLMouseDClk( UINT nFlags, const CPoint& pt);
	virtual BOOL		HandleKeyboardDN( EKeyboard kbd );

	virtual void		SetCtrlValue( double d, BOOL bSelf, BOOL bRedraw = TRUE );
	virtual void		Render( CDC* pDC );	// pure virtual Paint function
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	// Set padding for the text in this control
	virtual void		SetTextPadding( int top, int left, int bottom, int right );

	virtual void		SetDoRounded(BOOL b){ m_bDoRounded = b;}

	void SetActivateOnClick( BOOL bActivateOnClick ) { m_bActivateOnClick = bActivateOnClick; }

protected:
	int		m_nPadTop;
	int		m_nPadLeft;
	int		m_nPadBottom;
	int		m_nPadRight;
	BOOL	   m_bDoRounded;
	BOOL	   m_bActivateOnClick;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CNWBitmapEditControl - variation of tCNWEditControl, but contains
// a bitmap for the background
class CNWBitmapEditControl : public CNWEditControl
{
public:
							CNWBitmapEditControl( CNWControlSite* pSite, UINT idBitmap, int cxCap = 0 );
	virtual				~CNWBitmapEditControl();
	void					SetImages( UINT idBitmap );
	
	virtual void		Render( CDC* pDC ); // Paint Function;
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

protected:
	UINT		m_idbImg;
	int		m_cxCap;
	int		m_cxImg;
	int		m_cyImg;
	int		m_cxTile;
};

//////////////////////////////////////////////////////////////////////
// CNWBitmapEditControlLong: derivation of the CNWBitmapEditControl so 
// we can use multi line text and align it vertically.
// The NWEditControl base class only allows horizontal alignment.
//////////////////////////////////////////////////////////////////////
class CNWBitmapEditControlLong : public CNWBitmapEditControl
{
public:
	// c'tor
	CNWBitmapEditControlLong( CNWControlSite* pSite, UINT idBitmap );
	virtual				~CNWBitmapEditControlLong();
	
	virtual void		Render( CDC* pDC ); // Paint Function;
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override
};


//////////////////////////////////////////////////////////////////////
// CNWEditControlLong: derivation of the CNWEditControl so we can 
// use multi line text and align it vertically.
// The NWControl base class only allows horizontal alignment.
//////////////////////////////////////////////////////////////////////
class CNWEditControlLong : public CNWEditControl
{
public:
							CNWEditControlLong( CNWControlSite* pSite );
	virtual				~CNWEditControlLong();

	virtual void		Render( CDC* pDC );	// pure virtual Paint function
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void		GetTipString( CString& rStr );

protected:
};


// variation that allows clicking and rounded bg drawing
class CNWText : public CNWEditControl
{
public:
							CNWText( CNWControlSite* pSite );
	virtual				~CNWText();

	virtual void		HandleLMouseDN( UINT nFlags, const CPoint& pt );
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt );

	virtual void		Render( CDC* pDC );	// pure virtual Paint function
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void		SetUseGDIPlus(BOOL b = TRUE){ m_bUseGDIPlus = TRUE; }
	//virtual void		GetTipString( CString& rStr );

protected:
	BOOL m_bActiveUp;
	BOOL m_bUseGDIPlus;
};



//////////////////////////////////////////////////////////////////////
// CNWDropDownEditCtrl - variation of the drop down, but handles an edit
// like editable combo boxes in Windows do
//////////////////////////////////////////////////////////////////////

class CNWDropDownEditCtrl : public CNWDropDownCtrl
{
public:
// ctor
	CNWDropDownEditCtrl( CNWControlSite* pSite );
	virtual ~CNWDropDownEditCtrl();

	// Overrides
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt);
};

//////////////////////////////////////////////////////////////////////
// CBitmapDropDownControl - variation of the drop down, but contains
// a bitmap for the background
//////////////////////////////////////////////////////////////////////

class CNWBitmapDropDownCtrl : public CNWDropDownCtrl
{
public:

// ctor
	CNWBitmapDropDownCtrl( CNWControlSite* pSite, UINT idBitmap, UINT idBitmapSel = 0, UINT idBitmapDisable = 0, int cxCap = 0 );
	virtual ~CNWBitmapDropDownCtrl();
	virtual void Render( CDC* pDC );
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	void SetImages( UINT nIDNormal, UINT nIDSel = 0, UINT nDisable = 0 );

	virtual void SetIconOverlay(UINT idb, CPoint ptOffset);
	virtual void RemoveIconOverlay();
	
protected:
	int			m_cxCap;
	int			m_cxImg;
	int			m_cyImg;
	int			m_cxTile;

private:

	UINT			m_idbImg;
	UINT			m_idbImgSel;
	UINT			m_idbImgDis;

	UINT		 m_idbImgIcon; // icon overlay.
	CSize		 m_siIcon;
	CPoint	 m_ptIconOffset;
};


class CNWMultiBitmapDropDownCtrl : public CNWDropDownCtrl
{
public:

	// right now we only have up to 4 states.
	// 5 states, normal, down, hover, disabled, selected
	enum EStates
	{
		// Individual states.
		NORM		= 0x001,				// Normal button.
		NORMDWN	= 0x002,				// Normal button is being pressed down.
		NORMHVR	= 0x004,				// Mouse is hovering over normal button.
		SEL		= 0x008,				// Selected button (usually _T("lit up")).
		//SELDWN	= 0x010,				// Selected button is being pressed down.
		//SELHVR	= 0x020,				// Mouse is hovering over selected button.
		DIS		= 0x040,				// Button is disabled, no over effect, can't be clicked.
		//TRI		= 0x080,				// Button is in "indeterminate" state like windows checkbox tristate
		//TRIDWN	= 0x100,				// Button is in third state pressed down.
		//TRIHVR	= 0x200,				// Mouse is hovering over third state button.
		ND			= NORM | NORMDWN,
		NDHD		= NORM | NORMDWN | NORMHVR | DIS,
	};

	CNWMultiBitmapDropDownCtrl( CNWControlSite* pSite, UINT idBitmapList, int cxCap = 0, EStates e = NDHD );
	virtual ~CNWMultiBitmapDropDownCtrl();
	virtual void Render( CDC* pDC );
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void HandleLMouseUP( UINT nFlags, const CPoint& pt);

	virtual void SetUseGDIPlus(BOOL b = TRUE){m_bUseGDIPlus = b;}
	virtual void SetIconOverlay(UINT idb, CPoint ptOffset);

protected:
	int			m_cxCap;
	int			m_cxImg; // width of art
	int			m_cxFrame; // how wide is one state
	int			m_cyImg;
	int			m_cxTile;
	BOOL			m_bUseGDIPlus;
	EStates		m_eStates;
	std::map<int, int> m_mapImageIndex;		

private:
	UINT		 m_idbImgStrip; // 4 states, normal, down, hover, disabled
	UINT		 m_idbImgIcon; // icon overlay.
	CSize		 m_siIcon;
	CPoint	 m_ptIconOffset;
};

//////////////////////////////////////////////////////////////////////
// CNWMultiBitmapDropDownEditCtrl - variation of the drop down, but handles an edit
// like editable combo boxes in Windows do
//////////////////////////////////////////////////////////////////////

class CNWMultiBitmapDropDownEditCtrl : public CNWMultiBitmapDropDownCtrl 
{
public:
// ctor
	CNWMultiBitmapDropDownEditCtrl( CNWControlSite* pSite, UINT idBitmapList, int cxCap = 0, EStates e = NDHD );
	virtual ~CNWMultiBitmapDropDownEditCtrl();

	// Overrides
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void		HandleLMouseDClk( UINT nFlags, const CPoint& pt );
};



//////////////////////////////////////////////////////////////////////
// CNWMultiEditControl - a spreadsheet-like array of edit controls
//////////////////////////////////////////////////////////////////////
class CNWMultiEditControl : public CNWControl
{

	struct Cell
	{
		double			dValue;
		CString			strValueString;
		double			dMin;
		double			dMax;
	};

	std::vector<std::vector<Cell> > Cells( int cRows, std::vector<Cell>( int cCols ));

public:
							CNWMultiEditControl( CNWControlSite* pSite );
	virtual				~CNWMultiEditControl();

	virtual void		HandleLMouseDN( UINT nFlags, const CPoint& pt );
	virtual void		HandleLMouseUP( UINT nFlags, const CPoint& pt );
	virtual void		HandleLMouseDClk( UINT nFlags, const CPoint& pt);
	virtual void		HandleMouseMove( UINT nFlags, const CPoint& pt );
	virtual BOOL		HandleKeyboardDN( EKeyboard kbd );

	virtual void		SetSize( const CSize& size, BOOL bRepaint = FALSE );
	virtual void		SetOrigin( const CPoint& ptOrigin, BOOL bRepaint = FALSE );

	virtual void		Render( CDC* pDC );	// pure virtual Paint function
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void		ClearAllCells();
	virtual HRESULT	SetDimensions( int cRows, int cColumns );							// Number of cells - rows & columns
	virtual int			GetColumnWidth() { return m_nColWidth; }							// Width of each cell in pixels - sets all columns to equal width
	virtual void		SetColumnWidth( int nWidth ) { m_nColWidth = nWidth; }		
	virtual int			GetRowHeight() { return m_nRowHeight; }
	virtual void		SetRowHeight( int nHeight ) { m_nRowHeight = nHeight; }
	virtual void		SetTextPadding( int top, int left, int bottom, int right );
	virtual CNWMultiEditControl::Cell& GetCellAt( int ixRow, int ixCol );
	virtual void		SetCellValueAt( int ixRow, int ixColumn, double dValue, BOOL bSelf  );
	virtual void		SetCellValueStrAt( int ixRow, int ixColumn,  const CString& strValue );
	virtual BOOL		GetCellRectangle( CRect* prcCell, int ixRow, int ixColumn );	// Use to locate a specific cell in the viewable region
	virtual int			GetRequestedRow() { return m_nReqRow; }	// For callback methods to site - indicates which row/col is requesting handling
	virtual int			GetRequestedCol(){ return m_nReqCol; }
	virtual void		SetScrollbarColors( COLORREF rgbOutline, COLORREF rgbThumb ) { m_rgbSBLine = rgbOutline; m_rgbSBThumb = rgbThumb; }
	virtual void		SetScrollbarSize( int nWidthHeight ) { m_nSBSize = nWidthHeight; }		// Width/Height of scrollbars in pixels
	virtual void		SetScrollbarCornerRadius( int nRadius ) { m_nSBRadius = max( nRadius, m_nSBSize / 2 ); }		// For rounded corners

	// Implementation
protected:
	int					getFullWidth() { return m_cCols * m_nColWidth; }
	int					getFullHeight() { return m_cRows * m_nRowHeight; }
	BOOL					getIsXScrollerActive() { ASSERT( m_rctLocation.Height() > m_nSBSize ); return ( getFullWidth() > m_rctLocation.Width() ); }
	BOOL					getIsYScrollerActive() { ASSERT( m_rctLocation.Width() > m_nSBSize ); return ( getFullHeight() > m_rctLocation.Height() ); }
	void					updateXThumb( int nStartPos );
	void					updateYThumb( int nStartPos );
	BOOL					getCellAtPt( CPoint pt, int* pnRow, int* pnColumn );

protected:
	int					m_nPadTop;
	int					m_nPadLeft;
	int					m_nPadBottom;
	int					m_nPadRight;
   
	std::vector< std::vector<Cell> > m_cells;
	int					m_cRows;
	int					m_cCols;
	int					m_nReqRow;
	int					m_nReqCol;
	int					m_nRowHeight;
	int					m_nColWidth;

	// Scrollers
	CRect					m_rcXThumb;					// 
	CRect					m_rcYThumb;
	int					m_nSBSize;					// Width/Height of both scrollbars
	int					m_nMinThumbSize;
	int					m_nViewXOfs;				// Start of viewable region of pane
	int					m_nViewYOfs;			
	int					m_nOldViewXOfs;			// Save vars
	int					m_nOldViewYOfs;			
	BOOL					m_bDragXScroller;			// Dragging scrollers
	BOOL					m_bDragYScroller;
	int					m_nSBRadius;				// Shape of scroller handle (round radius)

	COLORREF				m_rgbSBLine;
	COLORREF				m_rgbSBThumb;
};


#endif // !defined(AFX_CROPDOWNCTRL_H__A3AF233F_6AB3_4749_BE8B_1F55E998117B__INCLUDED_)
