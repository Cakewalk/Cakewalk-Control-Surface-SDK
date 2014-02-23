#ifndef _CUIOFFSCREENDC_H_
#define _CUIOFFSCREENDC_H_
#pragma once
//
// $Header: /Src/CakeControls/OffscreenDC.h 3     1/08/01 12:53p Bdamiano $
//
// Copyright (C) 1998- Cakewalk Music Software.  All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////

class COffscreenDC : public CDC
{
public:
// Constructors
	COffscreenDC();
	void CreateCompatible( CDC* pDC, int cx, int cy );


// Implementation
	virtual ~COffscreenDC();

private:
// methods
	void releaseBitmap();

private:
// data
	CBitmap	m_bmp;		// the bitmap created and selected into the DC
	HBITMAP	m_hbmpOld;	// the handle of the old bitmap that was selected out
	int		m_cx;			// our bitmap dims
	int		m_cy;
};

////////////////////////////////////////////////////////////////////////////////


class CWMemDC : public CDC
//////////////////////////////////////////////////
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1997, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
{
private:
	CBitmap		m_bitmap;		// Offscreen bitmap
	CBitmap*		m_pbmOld;		// bitmap originally found in CWMemDC
	CDC*			m_pDC;			// Saves CDC passed in constructor
	CRect			m_rect;			// Rectangle of drawing area.

public:

	// pbmpSrc is an optional bitmap that can be passed in. If specified it is used
	// as the offscreen bitmap. Note that this bitmap should be compatible with pDC,
	// and should be of a large enough size for use as an offscreen bitmap.
	// If none is provided, an internal compatible bitmap is created.
	CWMemDC( CDC* pDC, CBitmap* pbmpSrc = NULL ) : 
			CDC(),
			m_pbmOld(NULL),
			m_pDC(pDC)
	{
		ASSERT(m_pDC != NULL); // If you asserted here, you passed in a NULL CDC.
				
		// Create a Memory DC
		VERIFY( CreateCompatibleDC( pDC) );
		
		pDC->GetClipBox( &m_rect );
		
		if ( pbmpSrc == NULL )
		{
			// Create a compatible bitmap the size of the clip box and select it into the memory DC
			m_bitmap.CreateCompatibleBitmap( pDC, m_rect.Width(), m_rect.Height() );
			m_pbmOld = SelectObject( &m_bitmap );
		}
		else
		{
			// Select the source bitmap into the memory DC
			m_pbmOld = SelectObject( pbmpSrc );
		}

		SetWindowOrg( m_rect.left, m_rect.top );
	}
	
	~CWMemDC()
	{
		// Copy the offscreen bitmap onto the screen.
		m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
								this, m_rect.left, m_rect.top, SRCCOPY);

		//Swap back the original bitmap.
		SelectObject(m_pbmOld);
	}
};




#endif // _CUIOFFSCREENDC_H_
