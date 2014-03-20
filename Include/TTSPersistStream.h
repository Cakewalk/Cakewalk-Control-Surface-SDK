// $Header: /Src/Include/TTSPersistStream

#ifndef _TTSPERSISTSTREAM_H_
#define _TTSPERSISTSTREAM_H_
#pragma once

#ifndef ASSERT
#define ASSERT ATLASSERT
#endif

////////////////////////////////////////////////////////////////////////////////
// Chunk IDs
//
// Every chunk is meant to represent a unique class of object.  For this reason,
// we need chunk IDs to be unique across all the various kinds of objects that
// are going to be persisted through this architecture.  Here is where we divide
// the 64k available chunk IDs by subsystem.

#define CHUNK_ID_BASE_PERSIST		(0x0000)
#define CHUNK_ID_BASE_SEQ			(0x1000)
#define CHUNK_ID_BASE_AUD			(0x2000)
#define CHUNK_ID_BASE_SONAR		(0x3000)
#define CHUNK_ID_BASE_VIDEO		(0x4000)
#define CHUNK_ID_BASE_GLOBAL	(0x5000) // Globally unique chunk IDs for objects used in multiple namespaces.
#define CHUNK_ID_BASE_P5			(0x6000)
#define CHUNK_ID_BASE_RESERVED	(0x8000)

////////////////////////////////////////////////////////////////////////////////

// Should we write zero-length chunks into the stream.
void _cdecl SetPersistEmptyChunks( BOOL bPersistEmptyChunks );

class CTTSPersistStream : public IPersistStream,
								  public IPersistStreamInit
{
public:

	static BOOL sm_bPersistEmptyChunks;

	// Ctors

	// dwID represents the object ID and the current schema number for the object
	// schema numbers grow sequentailly.  The UnChunkedSchemaNum is for loading from streams that
	// did not use the forward compatable sub-chunking.
	CTTSPersistStream( DWORD dwID, CRITICAL_SECTION* pcs = NULL, WORD wUnchunkedSchemaNum = 0 ) :
		m_wObjectID(HIWORD(dwID)),
		m_wCurrentSchemaNum(LOWORD(dwID)),
		m_bIsDirty(false),
		m_wUnchunkedSchemaNum(wUnchunkedSchemaNum),
		m_pcs(pcs)
	{
		ASSERT(m_wCurrentSchemaNum >= m_wUnchunkedSchemaNum);
	}

	// An alternate constructor that breaks out the object ID and the current schema into seperate
	// words for clarity. Apart from that it's the same as above.
	CTTSPersistStream( WORD wLastSchemaNum, WORD wID, CRITICAL_SECTION* pcs = NULL, WORD wUnchunkedSchemaNum = 0 ) :
		m_wObjectID( wID ),
		m_wCurrentSchemaNum( wLastSchemaNum ),
		m_bIsDirty(false),
		m_wUnchunkedSchemaNum(wUnchunkedSchemaNum),
		m_pcs(pcs)
	{
		ASSERT(m_wCurrentSchemaNum >= m_wUnchunkedSchemaNum);
	}

	// Copy Ctor
	CTTSPersistStream& operator=( const CTTSPersistStream& rhs ) 
	{
		if (&rhs != this)
		{
			m_bIsDirty = rhs.m_bIsDirty;
			m_wObjectID = rhs.m_wObjectID;
			m_wUnchunkedSchemaNum = rhs.m_wUnchunkedSchemaNum;
			m_pcs = rhs.m_pcs;
		}
		return *this;
	}

public:

	// IPersistStreamInit
	STDMETHOD(GetClassID)( CLSID* pClsid );
	STDMETHOD(IsDirty)( void );
	STDMETHOD(Load)( LPSTREAM pStm );
	STDMETHOD(Save)( LPSTREAM pStm, BOOL fClearDirty );
	STDMETHOD(GetSizeMax)( ULARGE_INTEGER* pcbSize );
	STDMETHOD(InitNew)( void );

	// Load a stream via its table of contents entry
	STDMETHOD(LoadViaTOC)( LPSTREAM pStm, DWORD nInstance, BOOL bRestoreFilePosition );

	/// Skip Loading this chunk but suck up the chunks size
	STDMETHOD(SkipLoad)( LPSTREAM pStm );
	
	// Overridables

	//		Returns: S_OK if the object requires a TOC entry, S_FALSE otherwise
	//		Override if object requires a TOC entry
	virtual HRESULT NeedTOCEntry();

	// Pure virtuals

	// Args:
	//		dwID:				HIWORD contains the chunk ID, LOWORD contains the schema number.
	//		pStm:				The IStream to read from.
	//		cbSizeToRead:	The number of bytes that are expected to be read.
	virtual HRESULT LoadSchema( DWORD dwID, LPSTREAM pStm, ULARGE_INTEGER cbSizeToRead ) = 0;

	// Args:
	//		dwID:				HIWORD contains the chunk ID, LOWORD contains the schema number.
	//		pStm:				The IStream to read from.
	//		pcbWritten:		The number of bytes that were actually written.
	//		bClearDirty:	TRUE if the object's dirty flag should be cleared.
	virtual HRESULT SaveSchema( DWORD dwID, LPSTREAM pStm, ULARGE_INTEGER* pcbWritten, BOOL bClearDirty ) = 0;

	// Args:
	//		dwID:				HIWORD contains the chunk ID, LOWORD contains the schema number.
	//		pcbSize:			Receives the size for the the specified schema.
	virtual HRESULT GetSizeSchema( DWORD dwID, ULARGE_INTEGER* pcbSize ) = 0;
	
	//
	// Return Type : a word that contains the highest schema number for this chunk
	//			
	virtual WORD	GetLastSchemaNum() { return m_wCurrentSchemaNum; }

private:

	HRESULT internalLoad( LPSTREAM pStm, const ULARGE_INTEGER& liChunkStartPos );
	HRESULT internalSkipLoad( LPSTREAM pStm, const ULARGE_INTEGER& liChunkStartPos );
	HRESULT skipLoadViaTOC( LPSTREAM pStm, DWORD nInstance );
	
	WORD	m_wCurrentSchemaNum;

protected:

	BOOL	SetIsDirty(BOOL bState)
	{
		BOOL bWasDirty = m_bIsDirty;
		m_bIsDirty = bState;
		return bWasDirty;
	}

	BOOL	m_bIsDirty;
	WORD	m_wObjectID;
	WORD	m_wUnchunkedSchemaNum;
	CRITICAL_SECTION* m_pcs;

};

////////////////////////////////////////////////////////////////////////////////

#endif // _TTSPERSISTSTREAM_H_
