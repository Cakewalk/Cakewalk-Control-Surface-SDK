// MackieControl.h : main header file for the MackieControl DLL
//

#if !defined(AFX_MackieControl_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_MackieControl_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////

extern long g_lComponents;	// Count of active components
extern long g_lServerLocks;	// Count of locks

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
// CMackieControlApp

class CMackieControlApp : public CWinApp
{
public:
	CMackieControlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMackieControlApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMackieControlApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CMackieControlApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MackieControl_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
