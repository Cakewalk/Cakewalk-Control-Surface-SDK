// NWBitmapKnob.h: interface for the CNWBitmapKnob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NWBITMAPKNOB_H__71BD31EC_CE25_4963_A6A9_766A939FE33F__INCLUDED_)
#define AFX_NWBITMAPKNOB_H__71BD31EC_CE25_4963_A6A9_766A939FE33F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NWControl.h"

#include "BitmapCache.h"



class CNWBitmapKnob : public CNWControl  
{
public:
	enum EDragMode { Vert, Horiz };

					CNWBitmapKnob( CNWControlSite* pSite, UINT idBmSequence, int nSlides, 
																		int nStates = 1, int cx = -1, int cy = -1, bool bPinBitmaps = false );
	virtual		~CNWBitmapKnob();

	virtual void	Render( CDC* pDC );	// pure virtual Paint function override
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	virtual void	HandleLMouseDN( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseUP( UINT nFlags, const CPoint& pt);
	virtual void	HandleLMouseDClk( UINT nFlags, const CPoint& pt );
	virtual void	HandleMouseMove( UINT nFlags, const CPoint& pt );
	virtual void	HandleRMouseUP( UINT nFlags, const CPoint& pt);

	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );

	// set new images
	virtual void SetImages( UINT idBmSequence, int nSlides,  int nStates = 1 );
	/// get current images id
	virtual UINT GetImagesID() { return m_uIdSequence; }

	virtual double	GetMouseIncrement() const 
	{
		if ( m_bMouse )
			return 0.45; 
		else
			return 1.;
	}

	void				SetDragMode( EDragMode e ){ m_eDragMode = e; }
	void				SetReverse( bool bIsReversed ) { m_bIsReversed = bIsReversed; }

protected:
	virtual void	cacheRects();

protected:
	UINT			m_uIdSequence;

	int			m_nNumSlides;
	int			m_nNumStates;	// 1 == normal, 2 == active, 3 == disable
	int			m_cPixelHt;
	int			m_cPixelWt;
	double		m_dClickVal;

	BOOL			m_bWidthByImage;
	BOOL			m_bHeightByImage;
	bool			m_bPinBitmaps;
	bool			m_bIsReversed;
	bool			m_bMouse;
	bool			m_bSwitchedAxis;

	EDragMode	m_eDragMode;

	CRect			m_rctImg;
	CRect			m_rctLabel;
};

#endif // !defined(AFX_NWBITMAPKNOB_H__71BD31EC_CE25_4963_A6A9_766A939FE33F__INCLUDED_)
