// $$Safe_root$$.h : main header file for the $$Safe_root$$ DLL
//

#if !defined(AFX_$$Safe_root$$_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_$$Safe_root$$_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

extern const GUID CLSID_$$Safe_root$$;
extern const GUID CLSID_$$Safe_root$$PropPage;
extern const GUID LIBID_$$Safe_root$$;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

static const char s_szFriendlyName[] = "$$Root$$";
static const char s_szFriendlyNamePropPage[] = "$$Root$$ Property Page";

/////////////////////////////////////////////////////////////////////////////
extern long g_lComponents;	// Count of active components
extern long g_lServerLocks;	// Count of locks

/////////////////////////////////////////////////////////////////////////////
// C$$Safe_root$$

class C$$Safe_root$$ :
	public IControlSurface,
	public IPersistStream,
	public ISpecifyPropertyPages
{
public:
	// Ctors
	C$$Safe_root$$();
	virtual ~C$$Safe_root$$();

	// *** IUnknown methods ***
	// Handled in $$Safe_root$$Gen.cpp
	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef();
	STDMETHODIMP_(ULONG)		Release();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "$$Safe_root$$"
	// Note: Skeleton methods are provided in $$Safe_root$$.cpp.
	// Basic dev caps
	STDMETHODIMP GetStatusText( LPSTR pszStatus, DWORD* pdwLen );
	STDMETHODIMP MidiInShortMsg( DWORD dwShortMsg );
	STDMETHODIMP MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );
	STDMETHODIMP RefreshSurface( DWORD fdwRefresh, DWORD dwCookie );
	STDMETHODIMP GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );
	// ($$Safe_root$$Gen.cpp)
	STDMETHODIMP Connect( IUnknown* pUnknown, HWND hwndApp );
	STDMETHODIMP Disconnect();

	// *** IPersistStream methods ***
	STDMETHODIMP Load( IStream* pStm );
	STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty );
	STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize );
	// ($$Safe_root$$Gen.cpp)
	STDMETHODIMP GetClassID( CLSID* pClsid );
	STDMETHODIMP IsDirty();

	// *** ISpecifyPropertyPages methods ($$Safe_root$$Gen.cpp) ***
	STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods ($$Safe_root$$Gen.cpp) ***
	STDMETHODIMP GetTypeInfoCount( UINT* );
	STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );
	
// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.
	
protected:
	ISonarMidiOut*		m_pMidiOut;
	ISonarKeyboard*	m_pKeyboard;
	ISonarCommands*	m_pCommands;
	ISonarProject*		m_pProject;
	ISonarMixer*		m_pMixer;
	ISonarTransport*	m_pTransport;
	ISonarIdentity*	m_pSonarIdentity;
	HWND					m_hwndApp;
	
	void releaseSonarInterfaces();

// Implementation
private:
	LONG					m_cRef;
	BOOL					m_bDirty;
	DWORD					m_dwSurfaceId;

	CRITICAL_SECTION	m_cs;		// critical section to guard properties
};


/////////////////////////////////////////////////////////////////////////////
// CCriticalSectionAuto

class CCriticalSectionAuto
{
public:
	CCriticalSectionAuto( CRITICAL_SECTION* pcs ) : m_pcs(pcs)
	{
		::EnterCriticalSection( m_pcs );
	}
	virtual ~CCriticalSectionAuto()
	{
		::LeaveCriticalSection( m_pcs );
	}
private:
	CRITICAL_SECTION* m_pcs;
};

/////////////////////////////////////////////////////////////////////////////
// C$$Safe_root$$App
// See $$Safe_root$$Gen.cpp for the implementation of this class
// This class should need to be modified when creating your plugin
//

class C$$Safe_root$$App : public CWinApp
{
public:
	C$$Safe_root$$App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C$$Safe_root$$App)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(C$$Safe_root$$App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern C$$Safe_root$$App theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_$$Safe_root$$_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
