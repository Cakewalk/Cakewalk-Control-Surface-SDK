/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMix.cpp : Implementation of DLL Exports.
/////////////////////////////////////////////////////////////////////////


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f MotorMix.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#include "MotorMix.h"
#include "MotorMix_i.c"
#include "ControlSurface_i.c"
#include "Surface.h"
#include "PropPage.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ControlSurface, CControlSurface)
OBJECT_ENTRY(CLSID_ControlSurfacePropPage, CControlSurfacePropPageWrapper)
END_OBJECT_MAP()

class CSurfaceFrameworkApp : public CWinApp
{
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurfaceFrameworkApp)
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSurfaceFrameworkApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CSurfaceFrameworkApp, CWinApp)
	//{{AFX_MSG_MAP(CSurfaceFrameworkApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSurfaceFrameworkApp theApp;

BOOL CSurfaceFrameworkApp::InitInstance()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    _Module.Init(ObjectMap, m_hInstance, &LIBID_SURFACEFRAMEWORKLib);
    return CWinApp::InitInstance();
}

int CSurfaceFrameworkApp::ExitInstance()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    _Module.Term();
    return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


