// MackieControl.cpp : Defines the basic functionality of the "MackieControl".
//

#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

#include "MackieControlLCDDisplay.h"
#include "MackieControlVPotDisplay.h"
#include "MackieControlFader.h"
#include "MackieControlXT.h"
#include "MackieControlXTPropPage.h"

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

#include "MackieControlC4.h"
#include "MackieControlC4PropPage.h"

#include "ControlSurface_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

long g_lServerLocks = 0;
long g_lComponents = 0;

/////////////////////////////////////////////////////////////////////////////
//
// DLL entry points
//
/////////////////////////////////////////////////////////////////////////////

STDAPI DllCanUnloadNow()
{
	//	Should return S_OK if no objects remain and there are no outstanding
	// locks due to the client calling IClassFactory::LockServer.
	
	if ((g_lComponents == 0) && (g_lServerLocks == 0))
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllGetClassObject( REFCLSID clsid, REFIID riid, void** ppv )
{
	IClassFactory* pFactory = NULL;

	if (CLSID_MackieControlXT == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlXTFactory\n");
		pFactory = new CMackieControlXTFactory;				// No AddRef in constructor
	}
	else if (CLSID_MackieControlXTPropPage == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlXTPropPageFactory\n");
		pFactory = new CMackieControlXTPropPageFactory;		// No AddRef in constructor
	}
	else if (CLSID_MackieControlMaster == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlMasterFactory\n");
		pFactory = new CMackieControlMasterFactory;			// No AddRef in constructor
	}
	else if (CLSID_MackieControlMasterPropPage == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlMasterPropPageFactory\n");
		pFactory = new CMackieControlMasterPropPageFactory;  // No AddRef in constructor
	}
	else if (CLSID_MackieControlC4 == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlC4Factory\n");
		pFactory = new CMackieControlC4Factory;				// No AddRef in constructor
	}
	else if (CLSID_MackieControlC4PropPage == clsid)
	{
//		TRACE("DllGetClassObject: CMackieControlC4PropPageFactory\n");
		pFactory = new CMackieControlC4PropPageFactory;		// No AddRef in constructor
	}
	else
	{
//		TRACE("DllGetClassObject: CLASS_E_CLASSNOTAVAILABLE\n");
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	if (!pFactory)
	{
//		TRACE("DllGetClassObject: E_OUTOFMEMORY");
		return E_OUTOFMEMORY;
	}

	// Get requested interface.
	HRESULT const hr = pFactory->QueryInterface( riid, ppv );
	pFactory->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Server registration
//
/////////////////////////////////////////////////////////////////////////////

static void CLSIDToString( const CLSID& clsid, char* pszCLSID, int cb )
{
	LPOLESTR pwszCLSID;
	if (SUCCEEDED( StringFromCLSID( clsid, &pwszCLSID ) ))
	{
		// Covert from wide characters to non-wide.
		wcstombs( pszCLSID, pwszCLSID, cb );

		// Free memory.
		CoTaskMemFree( pwszCLSID );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void recursiveDeleteKey( HKEY hKeyParent, const char* pcszKeyChild )
{
	// Open the child.
	HKEY hKeyChild;
	if (ERROR_SUCCESS == RegOpenKeyExA( hKeyParent, pcszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild ))
	{
		// Enumerate all of the decendents of this child.
		FILETIME time;
		char szBuffer[ 256 ];
		DWORD dwSize = sizeof szBuffer;
		while (S_OK == RegEnumKeyExA( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ))
		{
			// Delete the decendents of this child.
			recursiveDeleteKey( hKeyChild, szBuffer );
			dwSize = sizeof szBuffer;
		}

		// Close the child.
		RegCloseKey( hKeyChild );
	}

	// Delete this child.
	RegDeleteKeyA( hKeyParent, pcszKeyChild );
}


/////////////////////////////////////////////////////////////////////////////

static void setKeyAndValue( HKEY hKeyParent, const char* pcszKey, const char* pcszSubkey, const char* pcszValueName, const char* pcszValue )
{
	// Copy keyname into buffer.
	char szKeyBuf[ 1024 ];
	strlcpy( szKeyBuf, pcszKey, sizeof(szKeyBuf) );

	// Add subkey name to buffer.
	if (pcszSubkey != NULL)
	{
		strlcat( szKeyBuf, "\\", sizeof(szKeyBuf) );
		strlcat( szKeyBuf, pcszSubkey, sizeof(szKeyBuf) );
	}

	// Create and open key and subkey.
	HKEY hKey;
	if (ERROR_SUCCESS == RegCreateKeyExA( hKeyParent, szKeyBuf,  0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL ))
	{
		if (pcszValue)
			RegSetValueExA( hKey, pcszValueName, 0, REG_SZ, (BYTE*)pcszValue, (DWORD)strlen( pcszValue ) + 1 );
		RegCloseKey( hKey );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void registerFactory( const CLSID& clsid, const char* pcszFriendlyName )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileNameA( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	// Add the CLSID to HKEY_CLASSES_ROOT\CLSID
	char szKey[ 64 ];
	strlcpy( szKey, "CLSID\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, NULL, pcszFriendlyName );

	// Add the pathname subkey under the CLSID key.
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", NULL, szModule );
}

/////////////////////////////////////////////////////////////////////////////

static void unregisterFactory( const CLSID& clsid )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileNameA( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	char szKey[ 64 ];
	strlcpy( szKey, "CLSID\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );

	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	registerFactory( CLSID_MackieControlXT, s_szMackieControlXTFriendlyName );
	registerFactory( CLSID_MackieControlXTPropPage, s_szMackieControlXTFriendlyNamePropPage );
	registerFactory( CLSID_MackieControlMaster, s_szMackieControlMasterFriendlyName );
	registerFactory( CLSID_MackieControlMasterPropPage, s_szMackieControlMasterFriendlyNamePropPage );
	registerFactory( CLSID_MackieControlC4, s_szMackieControlC4FriendlyName );
	registerFactory( CLSID_MackieControlC4PropPage, s_szMackieControlC4FriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	char szKey[ 256 ];

	CLSIDToString( CLSID_MackieControlXT, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szMackieControlXTFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );

	CLSIDToString( CLSID_MackieControlMaster, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szMackieControlMasterFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );
		
	CLSIDToString( CLSID_MackieControlC4, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szMackieControlC4FriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_MackieControlXT );
	unregisterFactory( CLSID_MackieControlXTPropPage );
	unregisterFactory( CLSID_MackieControlMaster );
	unregisterFactory( CLSID_MackieControlMasterPropPage );
	unregisterFactory( CLSID_MackieControlC4 );
	unregisterFactory( CLSID_MackieControlC4PropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	char szKey[ 256 ];

	CLSIDToString( CLSID_MackieControlXT, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );

	CLSIDToString( CLSID_MackieControlMaster, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );

	CLSIDToString( CLSID_MackieControlC4, szCLSID, sizeof szCLSID );
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMackieControlApp

BEGIN_MESSAGE_MAP(CMackieControlApp, CWinApp)
	//{{AFX_MSG_MAP(CMackieControlApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMackieControlApp construction

CMackieControlApp::CMackieControlApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMackieControlApp object

CMackieControlApp theApp;

/////////////////////////////////////////////////////////////////////////////
