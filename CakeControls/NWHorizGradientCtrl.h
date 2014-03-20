// NWHorizGradientCtrl.h: interface for the CNWHorizGradientCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NWHORIZGRADIENTCTRL_H__DAFAAEA1_980B_4B44_A803_C23C0018CABA__INCLUDED_)
#define AFX_NWHORIZGRADIENTCTRL_H__DAFAAEA1_980B_4B44_A803_C23C0018CABA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NWControl.h"
#include "BitmapCache.h"



class CNWHorizGradientCtrl : public CNWControl  
{
public:
	// Control type enumeration 
	enum	EType	{
							eLEVEL,				// Level type filled left to right
							ePAN,					// Pan control - bar moves right or left of center.
							ePAN2,				// SONAR3 style dipping in center
							eBALANCE, 			// Balance Control
							eBargraphLevel,
							eBargraphPan,
							eBargraphBalance
					};

	CNWHorizGradientCtrl( CNWControlSite* pSite, UINT idbImg, int cxCap, EType type = eLEVEL );
	virtual ~CNWHorizGradientCtrl();


	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void	HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseDClk( UINT nFlags, const CPoint& pt );
	virtual void	HandleMouseMove( UINT nFlags, const CPoint& pt );
	virtual void	HandleRMouseUP( UINT nFlags, const CPoint& pt);
	virtual BOOL	HandleKeyboardDN( EKeyboard kbd );
	
	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );
	
	void				SetColors( COLORREF rgbBrite, double dDarkenFactor );
	void				SetMargins( int cxLMarg, int cxRMarg, int cyTopMarg = 4, int cyBotMarg = 4);
	void				SetValueMargins( int cxLMarg, int cxRMarg, int cyTopMarg = 4, int cyBotMarg = 4 );

	// Include a percent sign in the value string if it is displayed
	void				SetUsePercent( BOOL bIsPercentValue = TRUE ) { m_bUsePercent = bIsPercentValue; }
	void				SetBarGraph( bool b ) { m_bBarGraph = b; }
	void				SetRounded3dFrame( bool b ) { m_bDoRounded3dFrame = b; }

	void				SetIsLayeredWindowCompatible( bool bIsCompatible = true ) { m_bIsLayeredWindowCompatible = bIsCompatible; }

protected:
	virtual void		cacheRects();
	virtual double		getMouseIncrement() const 
	{
		if ( m_bMouse )
			return 0.45; 
		else
			return 1.;
	}

	virtual void		drawBackground( CDC* pDC );
	virtual void		showGanged( CDC* pDC );
	void					renderBars( CDC* pDC );
	void					drawBars( CDC* pDC, const CRect& rc, bool bAnchorLeft );

// graphics lib versions
	virtual void		drawBackgroundCWG( CWGraphicsCtx* pCWG );
	virtual void		showGangedCWG( CWGraphicsCtx* pCWG );
	void					renderBarsCWG( CWGraphicsCtx* pCWG );
	void					drawBarsCWG( CWGraphicsCtx* pCWG, const CRect& rc, bool bAnchorLeft );

protected:
	UINT					m_idbImg;
	int					m_cxCap;
	int					m_cxTile;
	int					m_cyImg;
	int					m_cxImg;
	double				m_dClickVal;
	int					m_cxGradMarginL;
	int					m_cxGradMarginR;
	int					m_cyGradMarginT;
	int					m_cyGradMarginB;
	int					m_cyValueMarginL;
	int					m_cyValueMarginR;
	int					m_cyValueMarginT;
	int					m_cyValueMarginB;

	CRect					m_rctName;			// The bounding rect of the name
	CRect					m_rctValue;			// The bounding rect of the value
	CRect					m_rctGang;			// the gang indicator rect
													
	COLORREF				m_rgbBeg;			// Starting color of the gradient
	COLORREF				m_rgbEnd;			// ending color of the gradient
	EType					m_eType;				// pan, balance or level
	bool					m_bVertGradient;	// Do the gradient from top to bottom of the band
	BOOL					m_bUsePercent;		// Add a percent sign e.g. "100.0%" to the value string
	bool					m_bBarGraph;
	bool					m_bDoRounded3dFrame; // add a frame
	bool					m_bMouse;				// via touch or mouse?

	UINT					m_nValueAlign;		// Usually DT_RIGHT

	static int			sm_nValueWidth;	// Width of the value string portion of the control

	bool				m_bIsLayeredWindowCompatible;
};


//////////////////////////////////////////////////////////////////////
// CNWBitmapEditControlDraggable: This was originally a special type of edit control, it's been  
// changed to derive from CNWHorizGradientCtrl so we can make them sliders when/if we decided to do so. (JM)
//////////////////////////////////////////////////////////////////////
class CNWBitmapEditControlDraggable : public CNWHorizGradientCtrl
{
public:
	CNWBitmapEditControlDraggable( CNWControlSite* pSite, UINT idBitmap, int cxCap = 0 );
	virtual			~CNWBitmapEditControlDraggable();

	virtual void	HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void	HandleMouseMove( UINT nFlags, const CPoint& pt );
	virtual void	HandleLMouseDClk( UINT nFlags, const CPoint& pt);
	virtual BOOL	HandleKeyboardDN( EKeyboard kbd );

	virtual void	Render( CDC* pDC );	// pure virtual Paint function
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual double	GetMouseIncrement() const { return 0.45; }
	virtual void	SetCtrlDisplayString( const CString& strVal );
	void				SetControlType( CNWHorizGradientCtrl::EType type ) { m_eType = type; }

protected:
	CString			m_strDisplay; //optional custom formated ctrl display value string
	double			m_dClickVal;

	int				m_nPadTop;
	int				m_nPadLeft;
	int				m_nPadBottom;
	int				m_nPadRight;
};


#endif // !defined(AFX_NWHORIZGRADIENTCTRL_H__DAFAAEA1_980B_4B44_A803_C23C0018CABA__INCLUDED_)
