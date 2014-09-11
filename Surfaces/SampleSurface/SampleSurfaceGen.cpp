///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// DLL Registration and and Factory Implementations
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "SampleSurface.h"
#include "SampleSurfacePropPage.h"


// OUR GUIDS.  These must be Unique.  If you build another surface from this,
// you must generate new ones!


// {86E2D93C-0B86-4bc1-9717-F170B97E4061}
static const GUID CLSID_SampleSurface = 
{ 0x86e2d93c, 0xb86, 0x4bc1, { 0x97, 0x17, 0xf1, 0x70, 0xb9, 0x7e, 0x40, 0x61 } };

// {86E2D93C-0B86-4bc1-9717-F170B97E4061}
static const GUID CLSID_SampleSurfacePropPage = 
{ 0x86e2d93c, 0xb86, 0x4bc1, { 0x97, 0x17, 0xf1, 0x70, 0xb9, 0x7e, 0x40, 0x61 } };

// {8C917487-54C2-48e7-BA59-743C6C7E5D4D}
static const GUID LIBID_SampleSurface = 
{ 0x8c917487, 0x54c2, 0x48e7, { 0xba, 0x59, 0x74, 0x3c, 0x6c, 0x7e, 0x5d, 0x4d } };


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CSampleSurface::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_SampleSurfacePropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CSampleSurface::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_SampleSurface;
	return S_OK;
}

HRESULT CSampleSurface::GetSizeMax(_ULARGE_INTEGER * pcbSize)
{
	return CControlSurface::GetSizeMax( pcbSize );
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CSampleSurfaceFactory
//
/////////////////////////////////////////////////////////////////////////////

class CSampleSurfaceFactory : public IClassFactory
{
public:
	// IUnknown
	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// Interface IClassFactory
	STDMETHODIMP_(HRESULT) CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP_(HRESULT) LockServer( BOOL bLock ); 

	// Constructor
	CSampleSurfaceFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CSampleSurfaceFactory::QueryInterface( REFIID riid, void** ppv )
{    
	if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this); 
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IClassFactory*>(*ppv)->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CSampleSurfaceFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CSampleSurfaceFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSampleSurfaceFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CSampleSurface* const pCSampleSurface = new CSampleSurface;
	if (!pCSampleSurface)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pCSampleSurface->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pCSampleSurface->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSampleSurfaceFactory::LockServer( BOOL bLock ) 
{
	if (bLock)
	{
		::InterlockedIncrement( &g_lServerLocks ); 
	}
	else
	{
		::InterlockedDecrement( &g_lServerLocks );
	}
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CPropPageFactory
//
/////////////////////////////////////////////////////////////////////////////

class CPropPageFactory : public IClassFactory
{
public:
	// IUnknown
	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	// Interface IClassFactory
	STDMETHODIMP_(HRESULT) CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv );
	STDMETHODIMP_(HRESULT) LockServer( BOOL bLock ); 

	// Constructor
	CPropPageFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::QueryInterface( REFIID riid, void** ppv )
{    
	if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
	{
		*ppv = static_cast<IClassFactory*>(this); 
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IClassFactory*>(*ppv)->AddRef();
	return S_OK;
}

ULONG CPropPageFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

ULONG CPropPageFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CSampleSurfacePropPage* const pPropPage = new CSampleSurfacePropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pPropPage->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPropPageFactory::LockServer( BOOL bLock ) 
{
	if (bLock)
	{
		::InterlockedIncrement( &g_lServerLocks ); 
	}
	else
	{
		::InterlockedDecrement( &g_lServerLocks );
	}
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// DLL entry points
//
/////////////////////////////////////////////////////////////////////////////

long g_lServerLocks = 0;
long g_lComponents = 0;

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
	if (CLSID_SampleSurface == clsid)
	{
		pFactory = new CSampleSurfaceFactory;  // No AddRef in constructor
	}
	else if (CLSID_SampleSurfacePropPage == clsid)
	{
		pFactory = new CPropPageFactory;  // No AddRef in constructor
	}
	else
		return CLASS_E_CLASSNOTAVAILABLE;

	if (!pFactory)
		return E_OUTOFMEMORY;

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
	strcpy( szKeyBuf, pcszKey );

	// Add subkey name to buffer.
	if (pcszSubkey != NULL)
	{
		strcat( szKeyBuf, "\\" );
		strcat( szKeyBuf, pcszSubkey );
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
	strcpy( szKey, "CLSID\\" );
	strcat( szKey, szCLSID );
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
	strcpy( szKey, "CLSID\\" );
	strcat( szKey, szCLSID );

	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	registerFactory( CLSID_SampleSurface, s_szFriendlyName );
	registerFactory( CLSID_SampleSurfacePropPage, s_szFriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_SampleSurface, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strcpy( szKey, "CakewalkControlSurfaces\\" );
	strcat( szKey, szCLSID );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );
		
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_SampleSurface );
	unregisterFactory( CLSID_SampleSurfacePropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_SampleSurface, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strcpy( szKey, "CakewalkControlSurfaces\\");
	strcat( szKey, szCLSID );
	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );

	return S_OK;
}

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CSampleSurfaceApp

BEGIN_MESSAGE_MAP(CSampleSurfaceApp, CWinApp)
	//{{AFX_MSG_MAP(CSampleSurfaceApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSampleSurfaceApp construction

CSampleSurfaceApp::CSampleSurfaceApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSampleSurfaceApp object

CSampleSurfaceApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSampleSurfacePropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CSampleSurfacePropPage::QueryInterface( REFIID riid, void** ppv )
{    
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if (IsEqualIID( riid, IID_IUnknown ))
		*ppv = static_cast<IPropertyPage*>(this);
	else if (IsEqualIID( riid, IID_IPropertyPage ))
		*ppv = static_cast<IPropertyPage*>(this);
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CSampleSurfacePropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CSampleSurfacePropPage::Release() 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IMFCPropertyPage Implementation

BOOL CSampleSurfacePropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CSampleSurfacePropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CSampleSurfacePropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}
