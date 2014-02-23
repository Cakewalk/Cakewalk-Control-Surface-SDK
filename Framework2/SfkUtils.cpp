/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkUtils.CPP : Various utility functions for the surface framework
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning(disable:4100)

#include "Htmlhelp.h"
#include "surface.h"
#include "sfkUtils.h"
#include <mmsystem.h>


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
			return char(toupper( c ));

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

BOOL MakeSurfacePathName( HINSTANCE hInstance, TCHAR* pszPathName, int cbMax, const TCHAR* pszBase /* = NULL */ )
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
		if ((int)::_tcslen( pszPathName ) + (int)::_tcslen( pszBase ) + 1 > cbMax)
			return FALSE;

	TCHAR* p = _tcsrchr( pszPathName, '\\' );
	if (p)
	{
		if (pszBase != NULL)
			// Copy their pszBase over the \\PROGNAME.EXE portion.
			::_tcscpy( p + 1, pszBase );
		else
		{
			// Simply zap the trailing '\\'
			if ((p - pszPathName) == 2 && pszPathName[ 1 ] == _T(':') )
				++p;		// If at the root (ex: c:\) leave the '\\'
			*p = _T('\0');
		}
	}
	else
	{
		if (pszBase != NULL)
			::_tcscpy( pszPathName, pszBase );
		else
			*pszPathName = _T('\0');
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// These functions and class add a wrapper around HTML help
// An application should call HelpInit during Application startup and 
// HelpTerminate during shutdown (MS Warns: not during process detach)

DWORD HelpInit( HINSTANCE hInstance /* = NULL*/, TCHAR* pszPathName /*= NULL*/, int cbMax /*= 0*/, const TCHAR* pszBase /*= NULL*/ )
{
	// Initialize HtmlHelp and optionally build a Help file pathname for caller.
	// Returns a "cookie" that must be passed to the Terminate function later

	if (hInstance && pszPathName && cbMax && pszBase)
	{
		MakeSurfacePathName( hInstance, pszPathName, cbMax, pszBase );
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

HWND HelpLaunch( HWND hWndParent, LPCTSTR pszHelpFilePath, UINT idWinHelpCmd, DWORD dwData )
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
// Class CSFKHelpAssist

BOOL CSFKHelpAssist::Init( HINSTANCE hInstance, HWND hWndParent, const TCHAR* pszBase )
{
	// is this specified as a full path?
	CString strBase( pszBase );
	if (-1 == strBase.Find( _T("\\") ) && -1 == strBase.Find( _T("/") ))
	{
		TCHAR szPath[ _MAX_PATH ];

		if (!( hInstance && pszBase && *pszBase ))
		{
			_ASSERT( 0 );
			return FALSE;
		}
		m_dwCookie		= HelpInit( hInstance, szPath, _countof( szPath ), pszBase );
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

void CSFKHelpAssist::Launch( UINT idWinHelpCmd, DWORD dwData, const TCHAR* pcszCustomHelpPath /*= NULL*/ )
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

void CSFKHelpAssist::Terminate() 
{
	if (m_dwCookie)
		HelpTerminate( m_dwCookie ); 

	m_dwCookie = 0;
}




/////////////////////////////////////////////////////////////////////////
// CTimer
/////////////////////////////////////////////////////////////////////////
CTimer::CTimer( CControlSurface *pSurface ):
	m_pSurface( pSurface ),
	m_bIsTimerActive( FALSE )
{
	TIMECAPS timeDevCaps;
	timeGetDevCaps( &timeDevCaps, sizeof( timeDevCaps ) );
	m_wTimerPeriod = WORD(max( timeDevCaps.wPeriodMin, 10 ));
}

/////////////////////////////////////////////////////////////////////////
CTimer::~CTimer()
{
	setTimerActive( FALSE );
}

/////////////////////////////////////////////////////////////////////////
void CTimer::SetIsActive( BOOL bIsActive )
{
	setTimerActive( bIsActive );
}

/////////////////////////////////////////////////////////////////////////
void CALLBACK EXPORT CTimer::timerCallback(
		UINT /*uID*/,
		UINT /*uMsg*/,
		DWORD dwUser,
		DWORD /*dw1*/,
		DWORD /*dw2*/
	)
{
	reinterpret_cast<CControlSurface*>(dwUser)->GetTimer()->onTimer();
}

/////////////////////////////////////////////////////////////////////////
void CTimer::setTimerActive( BOOL bIsActive )
{
	BOOL bWasActive = m_bIsTimerActive;
	if (bIsActive && !bWasActive ) // if it is not active but we want to make it so
	{
		MMRESULT mr = timeBeginPeriod( m_wTimerPeriod );
		_ASSERT( mr != TIMERR_NOCANDO );

		m_uiTimerID = timeSetEvent(
			10,
			m_wTimerPeriod,
			(LPTIMECALLBACK)timerCallback,
			(DWORD)m_pSurface,
			TIME_PERIODIC | TIME_CALLBACK_FUNCTION
		);
 
		if (m_uiTimerID != NULL)
			m_bIsTimerActive = TRUE;
	}
	else if (!bIsActive && bWasActive) // if it is active and we want to make it inactive
	{
		timeKillEvent( m_uiTimerID );
		m_bIsTimerActive = FALSE;

		MMRESULT mr = timeEndPeriod( m_wTimerPeriod );
		_ASSERT( mr != TIMERR_NOCANDO );
	}
}

/////////////////////////////////////////////////////////////////////////
HRESULT CTimer::SetPeriod( CTimerClient *pClient, WORD wPeriod )
{
	CSFKCriticalSectionAuto lock( m_cs );

	if (pClient == NULL)
		return E_POINTER;

	TimerClientSetIt itFind = m_setClients.find( pClient );
	
	if (wPeriod == 0)
	{
		// remove from service
		if (itFind != m_setClients.end())
		{
			m_setClients.erase( itFind );

			if (m_setClients.size() == 0)
			{
				setTimerActive( FALSE );
			}

			return S_OK;
		}
		else
			return S_FALSE;
	}
	else
	{
		// add or update for service
		if (itFind == m_setClients.end())
		{
			m_setClients.insert( TimerClientSet::value_type( pClient ) );
			setTimerActive( TRUE );
		}

		pClient->m_wNormalPeriod = max( wPeriod / m_wTimerPeriod, 1 ) ;
		pClient->m_wCountTicks = 0;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////
void CTimer::onTimer()
{
	// service all clients
	TimerClientSet setRemove;
	TimerClientSet setTick;
	TimerClientSetIt it;
	{
		CSFKCriticalSectionAuto lock( m_cs );
		// check what to do with each timer
		for (it = m_setClients.begin(); it != m_setClients.end(); it++)
		{
			// write each state ID and its state.
			CTimerClient *pClient = (*it);
			pClient->m_wCountTicks++;
			if (pClient->m_wCountTicks >= pClient->m_wNormalPeriod)
			{
				// rollover
				pClient->m_wCountTicks = 0;
				setTick.insert( TimerClientSet::value_type( pClient ) );

				if (pClient->GetIsOneShot())
				{
					setRemove.insert( TimerClientSet::value_type( pClient ) );
				}
			}
		}


		// remove the one shots
		for (it = setRemove.begin(); it != setRemove.end(); it++)
		{
			CTimerClient *pClient = (*it);
			TimerClientSetIt itFind = m_setClients.find( pClient );
			if (itFind != m_setClients.end())
			{
				// if found
				m_setClients.erase( itFind );
			}
		}

		if (m_setClients.size() == 0)
		{
			setTimerActive( FALSE );
		}

	}

	// tick them (outside of the critical section)
	for ( it = setTick.begin(); it != setTick.end(); it++)
	{
		CTimerClient *pClient = (*it);
		pClient->Tick();
	}
}
