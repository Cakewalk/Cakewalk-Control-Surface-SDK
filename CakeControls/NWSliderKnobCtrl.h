#pragma once
#include "nwcontrol.h"

class CNWSliderKnobCtrl :
	public CNWControl
{
public:
	enum SCALETYPE {ST_NONE, SC_AUD, SC_MIDI };

	CNWSliderKnobCtrl( CNWControlSite* pSite, UINT idBack, UINT idFore, int yTopMarg, int yBottomMarg, int xLeftMarg, SCALETYPE scale = SC_AUD, int cStates = 1 );
	virtual ~CNWSliderKnobCtrl(void);

	void	SetArt( UINT, UINT, int, int, int, int cStates = 1 );

	void	Render( CDC* pDC );
	virtual void	RenderCWG( CWGraphicsCtx * pcwg );	// pure virtual Paint function override

	void HandleRMouseUP( UINT nFlags, const CPoint& pt);
	void HandleLMouseDN( UINT nFlags, const CPoint& pt);
	void HandleLMouseUP( UINT nFlags, const CPoint& pt);
	void HandleLMouseDClk( UINT nFlags, const CPoint& pt );
	void HandleMouseMove( UINT nFlags, const CPoint& pt );

	virtual CNWControlUIAProvider* GetUIAutomationProvider( BOOL bCreate = TRUE );

protected:
	void initValues();
	void cacheRects();


protected:
	UINT	m_idbBack;
	UINT	m_idbFore;
	int	m_cyThumb;
	int	m_cxThumb;
	int	m_yTopMarg;
	int	m_yBottomMarg;
	int	m_xLeftMarg;
	int	m_cStates;

	bool  m_bMouse;

	double m_dClickVal;

	SCALETYPE	m_eScaleType;

	struct TIK
	{
		int nPixel;
		CString str;
	};
	
	std::vector<TIK>	m_aTiks;
};