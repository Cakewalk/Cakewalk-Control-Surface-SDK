// BitmapCache.h: interface for the CBitmapCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPCACHE_H__2CA52156_8EB9_45D5_8AB2_6F669C9144F1__INCLUDED_)
#define AFX_BITMAPCACHE_H__2CA52156_8EB9_45D5_8AB2_6F669C9144F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Forward declarations.
struct CWGRGBA;

namespace COLORGROUPID
{
	enum { CG_UNKNOWN = -1, CG_NONE = -2 };
};


////////////////////////////////////////////
// A single entry in the Bitmap Cache
struct BitmapUsage
{
	BitmapUsage() : pBmpRaw(0), pBmpColor(0), nClients(0), crTint((COLORREF)0xffffffff), 
		crPureTint(0), idGroup( (UINT)COLORGROUPID::CG_UNKNOWN ), bHasAlpha(false) {}

	CBitmap*		pBmpRaw;		// Allocated pointer to CBitmap
	CBitmap*		pBmpColor;	// Allocated pointer to Colorized CBitmap
	int			nClients;	// Number of clients currently using
	COLORREF		crTint;		// most recent Tint applied
	COLORREF		crPureTint;	// transparent color ( pure tint )
	UINT			idGroup;		// what group do we belong to if any?
	bool			bHasAlpha;	// has any alpha channel data
};

#if _MSC_VER > 1500
#define USE_WIC4CACHE 1
#include <wincodec.h> // for wic support
#else
#undef USE_WIC4CACHE
#endif

////////////////////////////////////////////////////////////////////
// A cache of bitmaps which may be used repeatedly throught the application
// Once Loaded, only a single copy of the bitmap is kept in the cache and
// is looked up using the bitmap's IDB
class CBitmapCache  
{
	// describe a color group.  This is the set of IDBs in the group and the current color
	struct ColorGroup
	{
		ColorGroup() : crCurrent( (COLORREF)0xffffffff ) {}

		std::set<UINT>		idbMembers;
		COLORREF				crCurrent;
	};

	typedef std::set<UINT>::iterator			ColorGroupIt;		// an iterator into the group's set of IDBs

	typedef std::map<UINT, BitmapUsage>			BmpCacheMap;		// map of IDB to BitmapUsage
	typedef BmpCacheMap::iterator				BmpCacheMapIt;		// and its iterator

	typedef std::map<UINT,ColorGroup>			ColorGroupMap;		// map of Color Group IDs to ColorGroup
	typedef ColorGroupMap::iterator				ColorGroupMapIt;	// its iterator
	typedef ColorGroupMap::value_type			ColorGroupMapKV;	// its insert value type

public:
	static CBitmapCache& GetBitmapCache();
	static void Destroy(); 

	BOOL				ColorizeGroup( UINT uID, COLORREF cr, HDC hdc );

	BOOL				LoadBitmap( UINT uID, COLORREF crXparent = RGB(0,0,0), bool bPin = false );
	UINT				CreateTintedBitmap( UINT uID, CWGRGBA clrTint );

	UINT				AddHBITMAP( HBITMAP hbm, BOOL bDoPreMult = FALSE );
	
	BOOL				FreeBitmap( UINT uID );	
	CBitmap*			GetBitmap( UINT uID, BOOL * pHasAlpha = NULL, CSize *pSize = NULL );
	
	BOOL           GetBitmap( UINT uID, BITMAP* pbm ); // like the Windows function

	BOOL				HasAlpha( UINT uID );	
	BOOL				DrawBitmap( UINT uID, CDC* pDCDest, int xDest, int yDest, int cxSrc, int cySrc, int xSrc = 0, int ySrc = 0, CDC* pDCSrc = NULL, int cxDest = -1, int cyDest = -1 );

	BOOL				SetColorGroup( UINT uIDB, UINT uColorGroup, COLORREF crGroup );
	BOOL				SetColorGroup( std::vector<UINT>& vIds, UINT uColorGroup, COLORREF crGroup );

	BOOL				GetGroupIDs( std::vector<UINT>* pvect );

protected :
	BOOL				colorizeBitmap( UINT idb, COLORREF cr, HDC hdc );

	UINT				colorGroupFromIdb( UINT idb );

	bool				getBits( CBitmap* pBm, HDC hdc, void** pvBits, int* pcBytes );

	bool				premultiplyAlpha( UINT idb, HDC hdc );

#if defined (USE_WIC4CACHE)
	HBITMAP			loadImageFromResource( HINSTANCE hinst, UINT nID, HRSRC hImage );
#endif

	CBitmapCache();
	virtual ~CBitmapCache();

private:
	static CBitmapCache* sm_pSelf;		// Singleton 

	static BITMAPINFO*	sm_pbmi;			// bitmap info to use for colorizing

	BmpCacheMap				m_cache;			// Map of BitmapUsage
	ColorGroupMap			m_colorGroups;		// Map of sets of bitmap IDs

#if defined (USE_WIC4CACHE)
	static IWICImagingFactory*  sm_pWICFactory;
#endif

	CANNOT_COPY_CLASS( CBitmapCache );
};


#endif // !defined(AFX_BITMAPCACHE_H__2CA52156_8EB9_45D5_8AB2_6F669C9144F1__INCLUDED_)
