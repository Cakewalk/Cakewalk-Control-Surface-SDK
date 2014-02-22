///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// DLL Registration and and Factory Implementations
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "VS100.h"
#include "VS100PropPage.h"


// OUR GUIDS.  These must be Unique.  If you build another surface from this,
// you must generate new ones!

// {69087565-FF58-4b90-92A6-F1855256341D}
static const GUID CLSID_VS100 = 
{ 0x69087565, 0xFF58, 0x4b90, { 0x92, 0xA6, 0xF1, 0x85, 0x52, 0x56, 0x34, 0x1D } };

// {E27E4EF4-87A2-4672-BB9D-344706B7D613}
static const GUID CLSID_VS100PropPage = 
{ 0xe27e4ef4, 0x87a2, 0x4672, { 0xbb, 0x9d, 0x34, 0x47, 0x6, 0xb7, 0xd6, 0x13 } };

// {1FC5C52A-986A-4b4e-AF47-3AC171572CF9}
static const GUID LIBID_VS100 = 
{ 0x1fc5c52a, 0x986a, 0x4b4e, { 0xaf, 0x47, 0x3a, 0xc1, 0x71, 0x57, 0x2c, 0xf9 } };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CVS100::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_VS100PropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CVS100::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_VS100;
	return S_OK;
}

HRESULT CVS100::GetSizeMax(_ULARGE_INTEGER * pcbSize)
{
	return CControlSurface::GetSizeMax( pcbSize );
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CVS100Factory
//
/////////////////////////////////////////////////////////////////////////////

class CVS100Factory : public IClassFactory
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
	CVS100Factory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100Factory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CVS100Factory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CVS100Factory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100Factory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CVS100* const pCVS100 = new CVS100;
	if (!pCVS100)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pCVS100->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pCVS100->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100Factory::LockServer( BOOL bLock ) 
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
	CVS100PropPage* const pPropPage = new CVS100PropPage;
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
	if (CLSID_VS100 == clsid)
	{
		pFactory = new CVS100Factory;  // No AddRef in constructor
	}
	else if (CLSID_VS100PropPage == clsid)
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
	registerFactory( CLSID_VS100, s_szFriendlyName );
	registerFactory( CLSID_VS100PropPage, s_szFriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	TCHAR szCLSID[ 128 ];
	CLSIDToString( CLSID_VS100, szCLSID, _countof( szCLSID ) );
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
	unregisterFactory( CLSID_VS100 );
	unregisterFactory( CLSID_VS100PropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	TCHAR szCLSID[ 128 ];
	CLSIDToString( CLSID_VS100, szCLSID, _countof( szCLSID ) );
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
// CVS100App

BEGIN_MESSAGE_MAP(CVS100App, CWinApp)
	//{{AFX_MSG_MAP(CVS100App)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVS100App construction

CVS100App::CVS100App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVS100App object

CVS100App theApp;

/////////////////////////////////////////////////////////////////////////////
// CVS100PropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CVS100PropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CVS100PropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CVS100PropPage::Release() 
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

BOOL CVS100PropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CVS100PropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CVS100PropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}
