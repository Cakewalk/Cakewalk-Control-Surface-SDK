/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkUtils.h : Utilities for the surface framework
/////////////////////////////////////////////////////////////////////////

#ifndef _SFKUTILS_H_
#define _SFKUTILS_H_

#include <set>

/////////////////////////////////////////////////////////////////////////////

class CSFKCriticalSection
{
	// This is a simple wrapper class for the Win32 CRITICAL_SECTION API.
	// An advantage of using this class (instead of a raw CRITICAL_SECTION
	// data object) is that the ctor/dtor do the Initialize and Delete calls.
	// This is particularly useful when working with a 'static' instance of
	// a CSFKCriticalSection object.
public:
// Ctors
	CSFKCriticalSection();
	~CSFKCriticalSection();

// Operations
	inline void Enter() { ::EnterCriticalSection( m_pcs ); }
	inline void Leave() { ::LeaveCriticalSection( m_pcs ); }

// Implementation
private:
	CRITICAL_SECTION*	m_pcs;

	// Copy prohibited
	CSFKCriticalSection( const CSFKCriticalSection& rhs );
	CSFKCriticalSection& operator=( const CSFKCriticalSection& rhs );
};

inline CSFKCriticalSection::CSFKCriticalSection()
{
	m_pcs = new CRITICAL_SECTION;
	ASSERT( m_pcs );
	::InitializeCriticalSection( m_pcs );
}

inline CSFKCriticalSection::~CSFKCriticalSection()
{
	if (m_pcs)
	{
		::DeleteCriticalSection( m_pcs );
		delete m_pcs;
		m_pcs = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////

class CSFKCriticalSectionAuto
{
	// This is a simple "sandwich class" to use in conjunction with CSFKCriticalSection.
	// Like any sandiwch class, it is intended to be used by declaring an 'auto'
	// instance of the class.
	// - The ctor calls CSFKCriticalSection::Enter().
	// - The dtor calls CSFKCriticalSection::Leave().
	// This way, the CS can be released implicitly when the 'auto' variable
	// goes out of scope.
public:
	inline CSFKCriticalSectionAuto( CSFKCriticalSection& cs ) : m_rcs( cs ) { m_rcs.Enter(); }
	inline ~CSFKCriticalSectionAuto() { m_rcs.Leave(); }

// Implementation
private:
	CSFKCriticalSection&	m_rcs;

	// Copy prohibited
	CSFKCriticalSectionAuto( const CSFKCriticalSectionAuto& rhs );
	CSFKCriticalSectionAuto& operator=( const CSFKCriticalSectionAuto& rhs );
};

/////////////////////////////////////////////////////////////////////////

BOOL MakeSurfacePathName( HINSTANCE hInstance, TCHAR* pszPathName, int cbMax, const TCHAR* pszBase = NULL );

/////////////////////////////////////////////////////////////////////////
class CSFKHelpAssist
{
public:
	CSFKHelpAssist() :
		m_hWndParent( NULL ),
		m_dwCookie( 0 )
	{}

	~CSFKHelpAssist() 
	{}

	BOOL Init( HINSTANCE hInstance, HWND hWndParent, const TCHAR* pszBase );
	void Launch( UINT idWinHelpCmd, DWORD dwData, const TCHAR* pcszCustomHelpPath = NULL );
	void SetHWnd( HWND hWnd ) { m_hWndParent = hWnd; }
	void Terminate();
	LPCTSTR GetPath() const { return m_strHelpPath; }

private:
	CString m_strHelpPath;	// Customized Help Path
	HWND    m_hWndParent;	// Parent Window
	DWORD   m_dwCookie;		// filled by HtmlHelp
};

/////////////////////////////////////////////////////////////////////////
// Text crunching

void CrunchString( LPCSTR pszString, int cbyString, char *pBuf, int nBudget, char cPad );


class CTimerClient;
class CControlSurface;

/////////////////////////////////////////////////////////////////////////
// CTimer
//
// This object allows any CTimerClient to subscribe for timed / periodic
// notifications at specified intervals. This allows using only one
// timer in the entire surface framework.
/////////////////////////////////////////////////////////////////////////

typedef std::set< CTimerClient* > TimerClientSet;
typedef TimerClientSet::iterator TimerClientSetIt;

class CTimer
{
	friend class CTimerClient;

public:
	CTimer( CControlSurface *pSurface );
	~CTimer();

	HRESULT SetPeriod( CTimerClient *pClient, WORD wPeriod );

	void SetIsActive( BOOL bIsActive );
	BOOL	GetIsActive() const { return m_bIsTimerActive; }

private:
	static void CALLBACK EXPORT timerCallback(
		UINT /*uID*/,
		UINT /*uMsg*/,
		DWORD dwUser,
		DWORD /*dw1*/,
		DWORD /*dw2*/
	);

	void setTimerActive( BOOL bIsActive );

	void onTimer();

	// member variables
	CControlSurface*	m_pSurface;
	UINT					m_uiTimerID;
	WORD					m_wTimerPeriod;
	BOOL					m_bIsTimerActive;
	TimerClientSet		m_setClients;
	CSFKCriticalSection	m_cs;
};

/////////////////////////////////////////////////////////////////////////
// CTimerClient
//
// Base class that can subscribe for timer notifications
/////////////////////////////////////////////////////////////////////////

class CTimerClient
{
	friend class CTimer;

public:
	CTimerClient() :
		m_bOneShot( false )
	{
	}

	virtual ~CTimerClient() {}

	void SetIsOneShot( bool bIsOneShot )
	{
		m_bOneShot = bIsOneShot;
	}

	bool GetIsOneShot() { return m_bOneShot; }

protected:
	virtual void Tick()
	{
		_ASSERT( 0 );
	}



private:
	WORD m_wCountTicks;
	WORD m_wNormalPeriod;
	bool m_bOneShot;
};



#endif // _SFKUTILS_H_
