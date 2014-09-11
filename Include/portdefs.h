/* ======================================================================= */
/*
 * $Header: /Src/Include/portdefs.h 7     6/09/03 11:15 Rkuper $
 *
 * PACKAGE: TTS Utility library
 * Macros for Win32 porting
 *
 * DESCRIPTION: External interface header
 *
 * ---
 * See logfile for revision comments.
 *
 * Copyright (C) 1987- by Greg Hendershott. All rights reserved.
 */
/* ======================================================================= */

#pragma once

#undef FNEXPORT
#undef METHODEXPORT
#undef CLASSEXPORT
#undef DATAEXPORT

////////////////////////////////////////////////////////////////////////////////

#if !defined(WIN32) && !defined(_WIN64)

#ifndef HUGE
	#define HUGE _huge
#endif

#define CLASSEXPORT	_export
#define FNEXPORT		_export _loadds
#define METHODEXPORT	_export _loadds
#define DATAEXPORT	_export

#else // WIN32

#ifdef EXPORT_DLL_SYMBOLS
	#define CLASSEXPORT	__declspec(dllexport)
	#define METHODEXPORT	__declspec(dllexport)
	#define FNEXPORT		__declspec(dllexport)
	#define DATAEXPORT	__declspec(dllexport)
#else
	#define CLASSEXPORT	__declspec(dllimport)
	#define METHODEXPORT	__declspec(dllimport)
	#define FNEXPORT		__declspec(dllimport)
	#define DATAEXPORT	__declspec(dllimport)
#endif

/////////////////////////////////////////////////////////////////////////////

#if (TRUE) // Always building static LIB
	#define AUDAPI
	#define AUDCLASS
	#define AUDDATA
	#define AUDMETHOD
#else
#ifdef BUILD_AUD
	#define AUDAPI		__declspec(dllexport)
	#define AUDCLASS	__declspec(dllexport)
	#define AUDDATA	__declspec(dllexport)
	#define AUDMETHOD	__declspec(dllexport)
#else
	#define AUDAPI		__declspec(dllimport)
	#define AUDCLASS	__declspec(dllimport)
	#define AUDDATA	__declspec(dllimport)
	#define AUDMETHOD	__declspec(dllimport)
#endif
#endif

////////////////////////////////////////////////////////////////////////////////

#if (TRUE) // Always building static LIB
	#define SEQAPI
	#define SEQCLASS
	#define SEQDATA
#else
#ifdef _BUILD_SEQ_
	#define SEQAPI		__declspec(dllexport)
	#define SEQCLASS	__declspec(dllexport)
	#define SEQDATA	__declspec(dllexport)
#else
	#define SEQAPI		__declspec(dllimport)
	#define SEQCLASS	__declspec(dllimport)
	#define SEQDATA	__declspec(dllimport)
#endif
#endif

////////////////////////////////////////////////////////////////////////////

#ifndef DEBUG
	#if defined(_DEBUG)
		#define DEBUG (1)
	#endif
#endif

#ifndef PRODUCTION
	#if defined(_DEBUG) || BETA
		#define PRODUCTION (0)
	#else
		#define PRODUCTION (1)
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////

#if !PRODUCTION
	#define _STLP_DEBUG_MESSAGE (1)		// use our own custom debug message function
	#define _STLP_DEBUG_TERMINATE (1)	// use our own special termination function
#endif


////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 900
	// Must disable this for WINNT.H
	#pragma warning( disable:4201 )	// (4) nonstandard extension used : nameless struct/union
#endif

#if _MSC_VER >= 1300
	#pragma warning( disable:4211 )	// (4) nonstandard exception used : redefined extern to static
#endif // _MSC_VER >= 1300

#if _MSC_VER >= 1400
   #pragma warning( disable: 4430 ) // (4) missing type specifier - int assumed.
#endif

////////////////////////////////////////////////////////////////////////////////

#if (_MSC_VER >= 1400)
   #define NCHTRESULT LRESULT	// OnNcHitTest has different return types between VC6/7 and VC8
#else
   #define NCHTRESULT UINT
#endif

#if (_MSC_VER >= 1300)
   #define CFILE_POS ULONGLONG
   #define MFRESULT	 INT_PTR	// Function return result -- incompatible in VC6 <-> VC7
#else 	
	typedef long CFILE_POS;
	#if !defined(PtrToLong)		// if BASETSD.H doesn't define xxx_PTR, then we can define INT_PTR as we like
		typedef UINT UINT_PTR;
		typedef int INT_PTR;
	#endif
	typedef long LONG_PTR;
	typedef unsigned long ULONG_PTR;
	typedef DWORD DWORD_PTR;
	#define MFRESULT int 
#endif

////////////////////////////////////////////////////////////////////////////////

#define MMNODRV
#define MMNOSOUND
#define MMNOAUX
#define MMNOMIXER

////////////////////////////////////////////////////////////////////////////////

#define SAFE_ALLOCA(_cb) (VERIFY(_cb > 0 && _cb <= 0xFFFF) ? _alloca(_cb) : NULL)

////////////////////////////////////////////////////////////////////////////////

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4996)

inline int TCHAR2Unicode( wchar_t* pwszDest, const TCHAR* tsz, int cchDest )
{
#ifdef _UNICODE 
	wcsncpy( pwszDest, tsz, cchDest );
	pwszDest[ cchDest-1 ] = 0;
	return static_cast<int>(wcslen( pwszDest ));
#else
	int const cchConv = MultiByteToWideChar( CP_ACP, 0, tsz, -1, pwszDest, cchDest );
	pwszDest[ cchDest-1 ] = 0;
	return cchConv;
#endif
}

inline int Unicode2TCHAR( TCHAR* pszDest, const wchar_t* pwsz, int cchDest )
{
#ifdef _UNICODE
	wcsncpy( pszDest, pwsz, cchDest );
	pszDest[ cchDest-1 ] = 0;
	return static_cast<int>(wcslen( pszDest ));
#else
	int const cbConv = WideCharToMultiByte( CP_ACP, 0, pwsz, -1, pszDest, cchDest, NULL, NULL );
	pszDest[ cchDest-1 ] = 0;
	return cbConv;
#endif
}

////////////////////////////////////////////////////////////////////////////////

inline int TCHAR2Char( char* pchDest, const TCHAR* tsz, int cbDest )
{
	if ( 0 == cbDest )
		return 0;

#ifndef _UNICODE
	strncpy( pchDest, tsz, cbDest );
	pchDest[ cbDest-1 ] = 0;
	return static_cast<int>(strlen( pchDest ));
#else
	int const cbConv = WideCharToMultiByte( CP_ACP, 0, tsz, -1, pchDest, cbDest, NULL, NULL );
	pchDest[ cbDest-1 ] = 0;
	return cbConv;
#endif
}

inline int Char2TCHAR( TCHAR* pszDest, const char* pch, int cchDest )
{
	if ( 0 == cchDest )
		return 0;

#ifndef _UNICODE
	strncpy( pszDest, pch, cchDest );
	pszDest[ cchDest-1 ] = 0;
	return static_cast<int>(strlen( pszDest ));
#else
	int const cchConv = MultiByteToWideChar( CP_ACP, 0, pch, -1, pszDest, cchDest );
	pszDest[ cchDest-1 ] = 0;
	return cchConv;
#endif
}

/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER <= 1500
inline double _ttof( const TCHAR* pcsz )
{
	double n;
	if (1 == _stscanf( pcsz, _T("%lf"), &n ))
		return n;
	else
		return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// define _STLP_HAS_WCHAR_T for unicode so that STL strings will be wchar_t
#ifdef UNICODE
#define STL_STRING wstring
#else
#define STL_STRING string
#endif

////////////////////////////////////////////////////////////////////////////////

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // WIN32
