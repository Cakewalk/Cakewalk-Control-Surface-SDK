///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// DLL Registration and and Factory Implementations
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TacomaSurface.h"
#include "TacomaSurfacePropPage.h"


// OUR GUIDS.  These must be Unique.  If you build another surface from this,
// you must generate new ones!

// {B7903F58-DC5D-46af-823C-6CE5679431FE}
const GUID CLSID_TacomaSurface = 
{ 0xb7903f58, 0xdc5d, 0x46af, { 0x82, 0x3c, 0x6c, 0xe5, 0x67, 0x94, 0x31, 0xfe } };

// {5A97A2E0-3154-47e1-874C-4EE5AE4EF5B6}
const GUID CLSID_TacomaSurfacePropPage = 
{ 0x5a97a2e0, 0x3154, 0x47e1, { 0x87, 0x4c, 0x4e, 0xe5, 0xae, 0x4e, 0xf5, 0xb6 } };

// {841FCF60-7E1D-4228-9BCA-4AF1EAA52806}
const GUID LIBID_TacomaSurface = 
{ 0x841fcf60, 0x7e1d, 0x4228, { 0x9b, 0xca, 0x4a, 0xf1, 0xea, 0xa5, 0x28, 0x6 } };


///////////////////////////////////////////////////////////////////////////////
// Override QI for the special Tacoma IOBox interface
HRESULT CTacomaSurface::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_ITacomaIOBox == riid)
	{
		*ppv = static_cast<ITacomaIOBox*>(this);
		return S_OK;
	}

	return __super::QueryInterface( riid, ppv );
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CTacomaSurface::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_TacomaSurfacePropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CTacomaSurface::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_TacomaSurface;
	return S_OK;
}

HRESULT CTacomaSurface::GetSizeMax(_ULARGE_INTEGER * pcbSize)
{
	return CControlSurface::GetSizeMax( pcbSize );
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CTacomaSurfaceFactory
//
/////////////////////////////////////////////////////////////////////////////

class CTacomaSurfaceFactory : public IClassFactory
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
	CTacomaSurfaceFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CTacomaSurfaceFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CTacomaSurfaceFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CTacomaSurfaceFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CTacomaSurfaceFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CTacomaSurface* const pCTacomaSurface = new CTacomaSurface;
	if (!pCTacomaSurface)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pCTacomaSurface->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pCTacomaSurface->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CTacomaSurfaceFactory::LockServer( BOOL bLock ) 
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
	CTacomaSurfacePropPage* const pPropPage = new CTacomaSurfacePropPage;
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
	if (CLSID_TacomaSurface == clsid)
	{
		pFactory = new CTacomaSurfaceFactory;  // No AddRef in constructor
	}
	else if (CLSID_TacomaSurfacePropPage == clsid)
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

static void CLSIDToString( const CLSID& clsid, TCHAR* pszCLSID, int cb )
{
	LPOLESTR pwszCLSID;
	if (SUCCEEDED( StringFromCLSID( clsid, &pwszCLSID ) ))
	{
		// Covert from wide characters to non-wide.
		::_tcsncpy( pszCLSID, pwszCLSID, cb );

		// Free memory.
		CoTaskMemFree( pwszCLSID );
	}
}


/////////////////////////////////////////////////////////////////////////////

static void recursiveDeleteKey( HKEY hKeyParent, const TCHAR* pcszKeyChild )
{
	// Open the child.
	HKEY hKeyChild;
	if (ERROR_SUCCESS == RegOpenKeyEx( hKeyParent, pcszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild ))
	{
		// Enumerate all of the decendents of this child.
		FILETIME time;
		TCHAR szBuffer[ 256 ];
		DWORD dwSize = _countof( szBuffer );
		while (S_OK == RegEnumKeyEx( hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time ))
		{
			// Delete the decendents of this child.
			recursiveDeleteKey( hKeyChild, szBuffer );
			dwSize = sizeof szBuffer;
		}

		// Close the child.
		RegCloseKey( hKeyChild );
	}

	// Delete this child.
	RegDeleteKey( hKeyParent, pcszKeyChild );
}


/////////////////////////////////////////////////////////////////////////////

static void setKeyAndValue( HKEY hKeyParent, const TCHAR* pcszKey, const TCHAR* pcszSubkey, const TCHAR* pcszValueName, const TCHAR* pcszValue )
{
	// Copy keyname into buffer.
	TCHAR szKeyBuf[ 1024 ];
	::_tcscpy( szKeyBuf, pcszKey );

	// Add subkey name to buffer.
	if (pcszSubkey != NULL)
	{
		::_tcscat( szKeyBuf, _T("\\") );
		::_tcscat( szKeyBuf, pcszSubkey );
	}

	// Create and open key and subkey.
	HKEY hKey;
	if (ERROR_SUCCESS == RegCreateKeyEx( hKeyParent, szKeyBuf,  0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL ))
	{
		if (pcszValue)
			RegSetValueEx( hKey, pcszValueName, 0, REG_SZ, (BYTE*)pcszValue, (DWORD)::_tcslen( pcszValue ) * sizeof(TCHAR) + 1 );
		RegCloseKey( hKey );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void registerFactory( const CLSID& clsid, const TCHAR* pcszFriendlyName )
{
	// Get server location.
	TCHAR szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, _countof( szModule ) );

	// Convert the CLSID into a char.
	TCHAR szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, _countof( szCLSID ) );

	// Add the CLSID to HKEY_CLASSES_ROOT\CLSID
	TCHAR szKey[ 64 ];
	::_tcscpy( szKey, _T("CLSID\\") );
	::_tcscat( szKey, szCLSID );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, NULL, pcszFriendlyName );

	// Add the pathname subkey under the CLSID key.
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, _T("InprocServer32"), NULL, szModule );
}

/////////////////////////////////////////////////////////////////////////////

static void unregisterFactory( const CLSID& clsid )
{
	// Get server location.
	TCHAR szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, _countof( szModule ) );

	// Convert the CLSID into a char.
	TCHAR szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, _countof( szCLSID ) );

	TCHAR szKey[ 64 ];
	::_tcscpy( szKey, _T("CLSID\\") );
	::_tcscat( szKey, szCLSID );

	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	registerFactory( CLSID_TacomaSurface, s_szFriendlyName );
	registerFactory( CLSID_TacomaSurfacePropPage, s_szFriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	TCHAR szCLSID[ 128 ];
	CLSIDToString( CLSID_TacomaSurface, szCLSID, _countof( szCLSID ) );
	TCHAR szKey[ 256 ];
	::_tcscpy( szKey, _T("CakewalkControlSurfaces\\") );
	::_tcscat( szKey, szCLSID );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, _T("Description"), s_szFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, _T("HelpFilePath"), _T("") );
		
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_TacomaSurface );
	unregisterFactory( CLSID_TacomaSurfacePropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	TCHAR szCLSID[ 128 ];
	CLSIDToString( CLSID_TacomaSurface, szCLSID, _countof( szCLSID ) );
	TCHAR szKey[ 256 ];
	::_tcscpy( szKey, _T("CakewalkControlSurfaces\\"));
	::_tcscat( szKey, szCLSID );
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
// CTacomaSurfaceApp

BEGIN_MESSAGE_MAP(CTacomaSurfaceApp, CWinApp)
	//{{AFX_MSG_MAP(CTacomaSurfaceApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTacomaSurfaceApp construction

CTacomaSurfaceApp::CTacomaSurfaceApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTacomaSurfaceApp object

CTacomaSurfaceApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTacomaSurfacePropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CTacomaSurfacePropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CTacomaSurfacePropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CTacomaSurfacePropPage::Release() 
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

BOOL CTacomaSurfacePropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CTacomaSurfacePropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CTacomaSurfacePropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}
