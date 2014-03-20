// FileStream.h: interface for the CFileStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILESTREAM_H__26E20DFE_9492_4B5A_9389_998FA52C856B__INCLUDED_)
#define AFX_FILESTREAM_H__26E20DFE_9492_4B5A_9389_998FA52C856B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TTSPersistTOC.h"

/////////////////////////////////////////////////////////////////////////////
// CFileStream

class CFileStream : public IStream
{
public:

	// Ctors
	CFileStream(CFile& rFile) : m_rFile(rFile) {}

	// IUnknown
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	// IStream
	STDMETHOD(Read)(void*, ULONG, ULONG*);
	STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
	STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
	STDMETHOD(SetSize)(ULARGE_INTEGER);
	STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*);
	STDMETHOD(Commit)(DWORD);
	STDMETHOD(Revert)();
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Stat)(STATSTG*, DWORD);
	STDMETHOD(Clone)(LPSTREAM*);

	// Implementation
private:
	CFile& m_rFile;
};

/////////////////////////////////////////////////////////////////////////////
// CFileStreamTOC
// A file stream that contains a table of contents

class CFileStreamTOC : public CFileStream
{
public:

	// Ctors
	CFileStreamTOC(CFile& rFile, BOOL bIsSaving, WORD wTopLevelChunkID, HRESULT* phr );
	~CFileStreamTOC();

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	// Flush the TOC
	HRESULT Flush();

	HRESULT LoadTOCFromHeader();
	HRESULT ReserveTOCHeader();
	HRESULT FlushTOCAndHeader();

private:
	HRESULT findTOC( WORD wTopLevelChunkID );

private:
	LARGE_INTEGER			m_liHeaderLoc;
	CTTSPersistTOC			m_toc;
	BOOL						m_bIsSaving;
	BOOL						m_bTOCFlushed;
};

////////////////////////////////////////////////////////////////////////////////
// Header to file containing a table of contents

class CPersistFileHeader : public CTTSPersistObject
{
public:
	// Ctor's
	CPersistFileHeader();
	virtual ~CPersistFileHeader();

	// Persistance 
	virtual HRESULT Persist( WORD wSchema, CPersistDDX& ddx );

public:
	struct
	{
		LARGE_INTEGER		liTOCOffsetLoc;

	} m_hdrPersist;

	// Persistance 
	static WORD				m_wPersistChunkID;			// persist chunk
	static WORD				m_wPersistSchema;				// persist schema
};


#endif // !defined(AFX_FILESTREAM_H__26E20DFE_9492_4B5A_9389_998FA52C856B__INCLUDED_)
