// $Header: /Src/Include/PersistDDX.h 8     2/05/03 5:14p Rkuper $
// Copyright (c) Twelve Tone Systems, Inc.  All rights reserved.
//
// PersistDDX.h: interface for the CPersistDDX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTDDX_H__FDF253A4_6B26_441D_8438_253AC94CA645__INCLUDED_)
#define AFX_PERSISTDDX_H__FDF253A4_6B26_441D_8438_253AC94CA645__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct IStream;
struct IPersistStreamInit;
class CNullWriteStream;

#if _MFC_VER >= 0x0700
#include <afxstr.h>
#else
class CString;
#endif

////////////////////////////////////////////////////////////////////////////////

class CPersistDDX  
{
public:

	// Ctors
	CPersistDDX( IStream* pStream, BOOL bSave, BOOL bClearDirty = TRUE );
	CPersistDDX( CNullWriteStream* pNullStream, BOOL bSave, BOOL bClearDirty = TRUE );
	~CPersistDDX();

	// Atributes
	BOOL		IsSaving() const { return m_bSaving; }
	IStream*	GetStream() { return m_pStream; }

	// Operations

	HRESULT XferBuf( long cb, void* pv );

	HRESULT Xfer( char* pData )
		{ return XferBuf( sizeof(char), pData ); }
	HRESULT Xfer( BYTE* pData )
		{ return XferBuf( sizeof(BYTE), pData ); }

	HRESULT Xfer( short* pData )
		{ return XferBuf( sizeof(short), pData ); }
	HRESULT Xfer( unsigned short* pData )
		{ return XferBuf( sizeof(unsigned short), pData ); }

	HRESULT Xfer( int* pData )
		{ return XferBuf( sizeof(int), pData ); }
	HRESULT Xfer( unsigned int* pData )
		{ return XferBuf( sizeof(unsigned int), pData ); }

	HRESULT Xfer( long* pData )
		{ return XferBuf( sizeof(long), pData ); }
	HRESULT Xfer( unsigned long* pData )
		{ return XferBuf( sizeof(unsigned long), pData ); }

	HRESULT Xfer( __int64* pData )
		{ return XferBuf( sizeof(__int64), pData ); }
	HRESULT Xfer( unsigned __int64* pData )
		{ return XferBuf( sizeof(unsigned __int64), pData ); }

	HRESULT Xfer( float* pData )
		{ return XferBuf( sizeof(float), pData ); }

	HRESULT Xfer( double* pData )
		{ return XferBuf( sizeof(double), pData ); }

	HRESULT Xfer( GUID* pData )
		{ return XferBuf( sizeof(GUID), pData ); }

	// Serialize an MFC string
	HRESULT Xfer( CString* pStr );

	// Serialze an OLE (Unicode) string
	HRESULT Xfer( LPOLESTR* ppwsz );

	// Serialize an IPersistStreamInit recursively
	HRESULT Xfer( IPersistStreamInit* pPS );

	// Serialize an IPersistStreamInit recursively
	HRESULT XferStream( IPersistStream* pPS );

	// Serialize a BOOL as a single byte
	HRESULT XferBOOL( BOOL* pData );

private:
	IStream*				m_pStream;
	CNullWriteStream*	m_pNullStream;
	BOOL					m_bSaving;
	BOOL					m_bClearDirty;
};

////////////////////////////////////////////////////////////////////////////////

template <class Tobj>
HRESULT XferType( CPersistDDX& ddx, Tobj* pObj )
{
	return ddx.XferBuf( sizeof(Tobj), pObj );
}

template <class Tobj, class Tcast>
HRESULT XferCast( CPersistDDX& ddx, Tobj* pObj )
{
	if (ddx.IsSaving())
	{
		Tcast x = (Tcast)(*pObj);
		return ddx.Xfer( &x );
	}
	else
	{
		Tcast x;
		HRESULT hr = ddx.Xfer( &x );
		*pObj = (Tobj)x;
		return hr;
	}
}

////////////////////////////////////////////////////////////////////////////////

extern const TCHAR* g_pszXferCheckFmt;
extern BOOL g_bPersistErrorRecoveryMode;

#if NDEBUG
#define THIS_FILE __FILE__
#endif

#define CHECK_XFER(_fn) \
	do{	HRESULT __hr = (_fn); \
		if (!SUCCEEDED(__hr)) \
		{ \
			TRACE( g_pszXferCheckFmt, THIS_FILE, __LINE__, __hr ); \
			if ( g_bPersistErrorRecoveryMode )	\
				TRACE( "In persist error recovery mode - ignoring failure code.\r\n" ); \
			else \
				return __hr; \
		} }while (0)


////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_PERSISTDDX_H__FDF253A4_6B26_441D_8438_253AC94CA645__INCLUDED_)
