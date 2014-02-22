/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SfkUtils.h : Utilities for the surface framework
/////////////////////////////////////////////////////////////////////////

#ifndef _SFKUTILS_H_
#define _SFKUTILS_H_

/////////////////////////////////////////////////////////////////////////////

class CCriticalSection
{
	// This is a simple wrapper class for the Win32 CRITICAL_SECTION API.
	// An advantage of using this class (instead of a raw CRITICAL_SECTION
	// data object) is that the ctor/dtor do the Initialize and Delete calls.
	// This is particularly useful when working with a 'static' instance of
	// a CCriticalSection object.
public:
// Ctors
	CCriticalSection();
	~CCriticalSection();

// Operations
	inline void Enter() { ::EnterCriticalSection( m_pcs ); }
	inline void Leave() { ::LeaveCriticalSection( m_pcs ); }

// Implementation
private:
	CRITICAL_SECTION*	m_pcs;

	// Copy prohibited
	CCriticalSection( const CCriticalSection& rhs );
	CCriticalSection& operator=( const CCriticalSection& rhs );
};

inline CCriticalSection::CCriticalSection()
{
	m_pcs = new CRITICAL_SECTION;
	ASSERT( m_pcs );
	::InitializeCriticalSection( m_pcs );
}

inline CCriticalSection::~CCriticalSection()
{
	if (m_pcs)
	{
		::DeleteCriticalSection( m_pcs );
		delete m_pcs;
		m_pcs = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////

class CCriticalSectionAuto
{
	// This is a simple "sandwich class" to use in conjunction with CCriticalSection.
	// Like any sandiwch class, it is intended to be used by declaring an 'auto'
	// instance of the class.
	// - The ctor calls CCriticalSection::Enter().
	// - The dtor calls CCriticalSection::Leave().
	// This way, the CS can be released implicitly when the 'auto' variable
	// goes out of scope.
public:
	inline CCriticalSectionAuto( CCriticalSection& cs ) : m_rcs( cs ) { m_rcs.Enter(); }
	inline ~CCriticalSectionAuto() { m_rcs.Leave(); }

// Implementation
private:
	CCriticalSection&	m_rcs;

	// Copy prohibited
	CCriticalSectionAuto( const CCriticalSectionAuto& rhs );
	CCriticalSectionAuto& operator=( const CCriticalSectionAuto& rhs );
};

/////////////////////////////////////////////////////////////////////////

BOOL MakeExePathName( HINSTANCE hInstance, char* pszPathName, int cbMax, const char* pszBase = NULL );

/////////////////////////////////////////////////////////////////////////
class CHelpAssist
{
public:
	CHelpAssist() :
		m_hWndParent( NULL ),
		m_dwCookie( 0 )
	{}

	~CHelpAssist() 
	{}

	BOOL Init( HINSTANCE hInstance, HWND hWndParent, const char* pszBase );
	void Launch( UINT idWinHelpCmd, DWORD dwData, const char* pcszCustomHelpPath = NULL );
	void SetHWnd( HWND hWnd ) { m_hWndParent = hWnd; }
	void Terminate();
	LPCSTR GetPath() const { return m_strHelpPath; }

private:
	CString m_strHelpPath;	// Customized Help Path
	HWND    m_hWndParent;	// Parent Window
	DWORD   m_dwCookie;		// filled by HtmlHelp
};

/////////////////////////////////////////////////////////////////////////
// Text crunching

void CrunchString( LPCSTR pszString, int cbyString, char *pBuf, int nBudget, char cPad );

#endif // _SFKUTILS_H_
