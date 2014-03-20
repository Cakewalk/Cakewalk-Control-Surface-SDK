// AutoDCRestorer.h: interface for the CAutoDCRestorer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTODCRESTORER_H__ED6769ED_F10D_4BCE_B808_9C7E872BD373__INCLUDED_)
#define AFX_AUTODCRESTORER_H__ED6769ED_F10D_4BCE_B808_9C7E872BD373__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAutoDCRestorer  
{
public:
	// enumerated flags of the attributes and objects you want to 
	// save and restore.  use eALL for everything and eUSUAL for most common stuff
	enum {
			eBK_COLOR =				0x0001,		// usual stuff
			eTXT_COLOR =			0x0002,
			eBK_MODE =				0x0004,
			ePEN =					0x0008,
			eBRUSH =					0x0010,
			eBITMAP =				0x0020,
			eFONT =					0x0040,

			eROP_MODE =				0x0100,		// less common stuff
			eSTRETCH_BLT_MODE =	0x0200,
			eMAP_MODE =				0x0400,

			eALL =					0xFFFF,		// catch-alls
			eUSUAL =					0x00FF
			};

	CAutoDCRestorer( CDC* pDC, UINT nFlags);
	virtual ~CAutoDCRestorer();

protected:
	UINT			m_nFlags;
	CDC*			m_pDC;

	// saved attribs
	COLORREF		m_rgbBkColor;
	COLORREF		m_rgbTxtColor;
	int			m_nBkMode;
	int			m_nRop2;
	int			m_nStretchBltMode;
	int			m_nMapMode;

	CBitmap*		m_pBitmap;
	CPen*			m_pPen;
	CBrush*		m_pBrush;
	CFont*		m_pFont;

};

#endif // !defined(AFX_AUTODCRESTORER_H__ED6769ED_F10D_4BCE_B808_9C7E872BD373__INCLUDED_)
