// TTSPersistTOC.h: interface for the CTTSPersistTOC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TTSPERSISTTOC_H__6C11C173_D211_4E40_BD0D_E539E6E71BB4__INCLUDED_)
#define AFX_TTSPERSISTTOC_H__6C11C173_D211_4E40_BD0D_E539E6E71BB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <TTSPersistObject.h>

////////////////////////////////////////////////////////////////////////////////
// Interface to the table of contents 
///////////////////////////////////////////////////////////////////////////////

// {1E934B8C-4110-4e8b-AA95-1AB7CD750A5D}
DEFINE_GUID(IID_IPersistTOC, 
0x1e934b8c, 0x4110, 0x4e8b, 0xaa, 0x95, 0x1a, 0xb7, 0xcd, 0x75, 0xa, 0x5d);

// Defines the mode by which chunks are accessed at load time
enum CHUNK_ACCESSMODE
{	
	ACCESS_SEQUENTIAL	= 0,				// chunks are read according to sequential position (TOC not used by default)
	ACCESS_THROUGH_TOC = 1,				// chunks are always accessed via the TOC but only forward seeking is permitted
	ACCESS_RANDOM_THROUGH_TOC = 2		// chunks are always accessed via the TOC using random access
};

DECLARE_INTERFACE_( IPersistTOC, IUnknown )
{
	// Get/set chunk default access mode 
	STDMETHOD_(HRESULT,	SetAccessMode)( CHUNK_ACCESSMODE eAccessMode ) PURE;
	STDMETHOD_(HRESULT,	GetAccessMode)( CHUNK_ACCESSMODE* peAccessMode ) PURE;

	// Manipulate entries in the TOC
	STDMETHOD_(HRESULT,	Lookup)( WORD idChunk, DWORD nInstance, ULARGE_INTEGER* pnOffset ) PURE;
	STDMETHOD_(HRESULT,	Insert)( WORD idChunk, ULARGE_INTEGER nOffset, DWORD* pnInstance ) PURE;
	STDMETHOD_(HRESULT,	Remove)( WORD idChunk, DWORD nInstance  ) PURE;
	STDMETHOD_(HRESULT,	RemoveAll)() PURE;

	// Increment/Reset running chunk instance number(used during TOC save and file load)
	STDMETHOD_(HRESULT,	UpdateChunkInstance)( WORD idChunk, DWORD* pnInstance ) PURE;
	STDMETHOD_(HRESULT,	ResetChunkInstance)( WORD idChunk ) PURE;
};

////////////////////////////////////////////////////////////////////////////////
// A persistable table of contents

class CTTSPersistTOC : public CTTSPersistObject, public IPersistTOC
{
public:
	// Ctor's
	CTTSPersistTOC();
	virtual ~CTTSPersistTOC();

	// IUnknown
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	// IPersistTOC
	STDMETHODIMP SetAccessMode( CHUNK_ACCESSMODE eAccessMode );
	STDMETHODIMP GetAccessMode( CHUNK_ACCESSMODE* peAccessMode );
	STDMETHODIMP Lookup( WORD idChunk, DWORD nInstance, ULARGE_INTEGER* pnOffset );
	STDMETHODIMP Insert( WORD idChunk, ULARGE_INTEGER nOffset, DWORD* pnInstance );
	STDMETHODIMP Remove( WORD idChunk, DWORD nInstance );
	STDMETHODIMP RemoveAll();
	STDMETHODIMP UpdateChunkInstance( WORD idChunk, DWORD* pnInstance );
	STDMETHODIMP ResetChunkInstance( WORD idChunk );

	// Persistance 
	virtual HRESULT Persist( WORD wSchema, CPersistDDX& ddx );


	// Diagnostics
	HRESULT DumpXML();

public:
	// Persistance 
	static WORD				m_wPersistChunkID;			// persist chunk
	static WORD				m_wPersistSchema;				// persist schema

	// Key to look up an entry in the TOC map
	struct TOCKey
	{
		WORD idChunk;
		DWORD nInstance;
	};

private:

	// Map of TOC entries, mapping the TOC key to the chunk's file offset
	typedef std::map<TOCKey, ULARGE_INTEGER> TOCMap;
	typedef TOCMap::iterator TOCMapIterator;
	TOCMap	m_mapTOC;	

	// Map to generate running per chunk instance numbers during TOC save and file load
	typedef std::map<DWORD, DWORD> ChunkInstanceMap;
	typedef ChunkInstanceMap::iterator ChunkInstanceMapIterator;
	ChunkInstanceMap	m_mapChunkInst;

	// Access mode determines whether chunks are accessed by default via the TOC or sequentially
	CHUNK_ACCESSMODE	m_eAccessMode;
};

#endif // !defined(AFX_TTSPERSISTTOC_H__6C11C173_D211_4E40_BD0D_E539E6E71BB4__INCLUDED_)
