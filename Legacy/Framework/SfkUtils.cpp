/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkUtils.CPP : Various utility functions for the surface framework
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Htmlhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// text crunching routines
/////////////////////////////////////////////////////////////////////////

static int getMaxCharPrio()
{
	return 4;
}

/////////////////////////////////////////////////////////////////////////
static BOOL isVowel( char c )
{
	char *pVowels= "AEIOUaeiou";
	char *pRes = strchr( pVowels, c );
	if (pRes == NULL)
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
static BOOL isConsonant( char c )
{
	char *pConsonants = "BCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz";
	char *pRes = strchr( pConsonants, c );
	if (pRes == NULL)
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
static int getCharPriority( char c, char prev )
{
	_ASSERT( getMaxCharPrio() == 4 );
	if (isdigit( c ))
		return 4; // highest

	// is it the first character of a word?
	if (isalpha( c ) && (isalnum( prev ) == FALSE) )
		return 3;

	if (isConsonant( c ))
		return 2;

	if (isVowel( c ))
		return 1;

	// anything else is lowest
	return 0;
}

/////////////////////////////////////////////////////////////////////////
static char makeCrunchChar( char c, char prev )
{
	// is it the first character of a word?
	if (isalpha( c ) && (isalnum( prev ) == FALSE) )
		if (islower( c ))
			return toupper( c );

	return c;
}

/////////////////////////////////////////////////////////////////////////
void CrunchString( LPCSTR pszString, int cbyString, char *pBuf, int nBudget, char cPad )
{
	// first pass: find the number of characters in each priority:

	if (cbyString <= nBudget)
	{
		// this should never happen.
		memset( pBuf, cPad, nBudget );
		strncpy( pBuf, pszString, cbyString );
		return;
	}

	int arPrioCounts[5]; // if you modify the maximum priority,
	// make sure you change this.

	memset( arPrioCounts, 0, sizeof( arPrioCounts ) );

	char prevChar = 0;
	char curChar = 0;

	int ix = 0;
	// count the number of characters on each priority level
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];

		// do work here
		int nPrio = getCharPriority( curChar, prevChar );
		arPrioCounts[ nPrio ] ++;
		prevChar = curChar;
	}

	// triage by priority (find the lowest priority to include,
	// and the remaining budget)
	int nBaseBudget = nBudget;
	for (ix = 4; ix >= 0; ix--)
	{
		// if this priority can be included fully
		if (arPrioCounts[ ix ] <= nBaseBudget)
		{
			nBaseBudget -= arPrioCounts[ ix ];
		}
		else
			// this priority cannot be included fully, the remaining budget is known
			break;
	}

	int nBasePriority = ix;
	_ASSERT( nBasePriority >= 0 );
	_ASSERT( nBasePriority <= 4 );

	int ixOutBuf = 0;
	prevChar = 0;
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];
		int nPrio = getCharPriority( curChar, prevChar );
		if (nPrio > nBasePriority)
		{
			pBuf[ ixOutBuf++ ] = makeCrunchChar( curChar, prevChar );
		}
		else if ((nPrio == nBasePriority) && (nBaseBudget > 0))
		{
			pBuf[ ixOutBuf++ ] = makeCrunchChar( curChar, prevChar );
			nBaseBudget--;
		}
		_ASSERT( ixOutBuf <= nBudget );
		prevChar = curChar;
	}
}

///////////////////////////////////////////////////////////////////////////////

BOOL MakeExePathName( HINSTANCE hInstance, char* pszPathName, int cbMax, const char* pszBase /* = NULL */ )
{
	// Call us with pszBase NULL or not.
	//
	// If NULL, then we simply return the .EXE path, sans the trailing '\\'.
	// If not NULL, then we append pszBase to the .EXE path to form a
	// complete pathname.

	// Warn about buffer too small under Win32!
	ASSERT( _MAX_PATH <= cbMax );
	// pszBase must NOT contain a leading '\\'!
	ASSERT( pszBase == NULL || pszBase[ 0 ] != '\\' );

	// Set up pathname to be same path as .EXE
	GetModuleFileName( hInstance, pszPathName, cbMax );

	// Guarantee that the base name and the path name together doesn't exceed cbMax!
	if (NULL != pszBase)
		if ((int)strlen( pszPathName ) + (int)strlen( pszBase ) + 1 > cbMax)
			return FALSE;

	char* p = strrchr( pszPathName, '\\' );
	if (p)
	{
		if (pszBase != NULL)
			// Copy their pszBase over the \\PROGNAME.EXE portion.
			strcpy( p + 1, pszBase );
		else
		{
			// Simply zap the trailing '\\'
			if ((p - pszPathName) == 2 && pszPathName[ 1 ] == ':')
				++p;		// If at the root (ex: c:\) leave the '\\'
			*p = '\0';
		}
	}
	else
	{
		if (pszBase != NULL)
			strcpy( pszPathName, pszBase );
		else
			*pszPathName = '\0';
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// These functions and class add a wrapper around HTML help
// An application should call HelpInit during Application startup and 
// HelpTerminate during shutdown (MS Warns: not during process detach)

DWORD HelpInit( HINSTANCE hInstance /* = NULL*/, char* pszPathName /*= NULL*/, int cbMax /*= 0*/, const char* pszBase /*= NULL*/ )
{
	// Initialize HtmlHelp and optionally build a Help file pathname for caller.
	// Returns a "cookie" that must be passed to the Terminate function later

	if (hInstance && pszPathName && cbMax && pszBase)
	{
		MakeExePathName( hInstance, pszPathName, cbMax, pszBase );
	}

	DWORD dwCookie = 0;
	HtmlHelp( NULL, NULL, HH_INITIALIZE, (DWORD)&dwCookie ); // Cookie returned by Hhctrl.ocx.
	
	return dwCookie;
}

///////////////////////////////////////////////////////////////////////////////

UINT MapWHToHtmlCmd( UINT idWinHelpCmd )
{
	// Map a WinHelp command ID to the equivalent HTML Help ID
	
	switch (idWinHelpCmd)
	{
		case HELP_CONTEXT:      // 0x0001L  /* Display topic in ulTopic */
			return HH_HELP_CONTEXT;

		case HELP_QUIT:			// 0x0002L  /* Terminate help */
			return HH_CLOSE_ALL;

		case HELP_INDEX:			// 0x0003L  /* Display index */ HELP_CONTENTS // 0x0003L
			return HH_DISPLAY_INDEX;

		case  HELP_CONTEXTMENU:	//  0x000a
			return HH_TP_HELP_CONTEXTMENU;

		case  HELP_FINDER:		//  0x000b
			return HH_HELP_FINDER;

		case  HELP_WM_HELP:     //  0x000c
			return HH_TP_HELP_WM_HELP;

		case  HELP_CONTEXTPOPUP:	// 0x0008L (this is a guess mapping)
			return HH_DISPLAY_TEXT_POPUP;	
	}

	// Default: (HH_HELP_FINDER = 0)
	
	return HH_HELP_FINDER;
}

///////////////////////////////////////////////////////////////////////////////

HWND HelpLaunch( HWND hWndParent, LPCSTR pszHelpFilePath, UINT idWinHelpCmd, DWORD dwData )
{
	// Call HTML help to launch an instance with a numberic topic or HELP_FINDER
	// We are going to use HTML Help instead of WinHelp now
	UINT const nHHCmd = MapWHToHtmlCmd( idWinHelpCmd );

	return HtmlHelp( hWndParent, pszHelpFilePath, nHHCmd, dwData );
}

///////////////////////////////////////////////////////////////////////////////

void HelpTerminate( DWORD dwCookie )
{
	HtmlHelp( NULL, NULL, HH_UNINITIALIZE, dwCookie ); // Pass in cookie.
}

///////////////////////////////////////////////////////////////////////////////
// Class CHelpAssist

BOOL CHelpAssist::Init( HINSTANCE hInstance, HWND hWndParent, const char* pszBase )
{
	// is this specified as a full path?
	CString strBase( pszBase );
	if (-1 == strBase.Find( "\\" ) && -1 == strBase.Find( "/" ))
	{
		char szPath[ _MAX_PATH ];

		if (!( hInstance && pszBase && *pszBase ))
		{
			_ASSERT( 0 );
			return FALSE;
		}
		m_dwCookie		= HelpInit( hInstance, szPath, sizeof( szPath ), pszBase );
		m_strHelpPath	= szPath;
	}
	else
	{
		HtmlHelp( NULL, NULL, HH_INITIALIZE, (DWORD)&m_dwCookie );
		m_strHelpPath = strBase;
	}

	m_hWndParent	= hWndParent;

	return m_dwCookie != 0;
}

///////////////////////////////////////////////////////////////////////////////

void CHelpAssist::Launch( UINT idWinHelpCmd, DWORD dwData, const char* pcszCustomHelpPath /*= NULL*/ )
{
	if (m_dwCookie == 0) // HTML Help not installed
		return;

	CString strHelp;
	if (pcszCustomHelpPath && *pcszCustomHelpPath)
	{
		// Caller is using a particular help file
		strHelp = pcszCustomHelpPath;
	}
	else
	{
		if (m_strHelpPath.IsEmpty())
		{
			_ASSERT( 0 );
			return;
		}

		// Caller is using the default help file
		strHelp = m_strHelpPath;
	}

  	HelpLaunch( m_hWndParent, strHelp, idWinHelpCmd, dwData );
}

///////////////////////////////////////////////////////////////////////////////

void CHelpAssist::Terminate() 
{
	if (m_dwCookie)
		HelpTerminate( m_dwCookie ); 

	m_dwCookie = 0;
}
