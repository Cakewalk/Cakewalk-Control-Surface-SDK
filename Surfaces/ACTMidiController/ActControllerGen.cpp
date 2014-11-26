// Generic Code that should be the same for all control surfaces.
//
// Contains IUnknown routines as well as server registration code, factory code,
// and DLL entrypoint code.
//
// You should not have to modify any of the following.

#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

#include "ACTController.h"
#include "ACTControllerPropPage.h"

const GUID CLSID_ACTController = { 0x0a7f7099, 0xe2bc, 0x4869, { 0xb9, 0x43, 0x91, 0x4d, 0x70, 0xfb, 0x0f, 0xe6 } };
const GUID CLSID_ACTControllerPropPage = { 0xb3cf24de, 0x417d, 0x416c, { 0xab, 0xbb, 0x33, 0xa4, 0xc6, 0xbf, 0x12, 0x00 } };
const GUID LIBID_ACTController = { 0xa0c6536c, 0x98d8, 0x4fde, { 0xae, 0xf8, 0x94, 0xd7, 0x13, 0x7a, 0x7d, 0x2a } };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CACTController::Connect( IUnknown* pUnk, HWND hwndApp )
{
	// Note: You will probably not need to change this implementation.
	// The wizard has already generated code to obtain all of the ISonarXXX
	// interfaces that are available.

	CCriticalSectionAuto csa( &m_cs );

	HRESULT hr = S_OK;

	releaseSonarInterfaces();
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarMidiOut, (void**)&m_pMidiOut ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarKeyboard, (void**)&m_pKeyboard ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarCommands2, (void**)&m_pCommands ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarProject, (void**)&m_pProject ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarMixer, (void**)&m_pMixer ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarTransport, (void**)&m_pTransport ) ))
		return hr;

	// try to obtain the optional ISonarIdentity Interface from the host
	hr = pUnk->QueryInterface( IID_ISonarIdentity, (void**)&m_pSonarIdentity );
	if ( SUCCEEDED( hr ))
	{
		// Obtain a unique instance ID.  This ID will be the same for this surface each time
		// the host is restarted as long as the same surfaces are active in the host.
		m_dwSurfaceId = 0;
		hr = m_pSonarIdentity->GetUniqueSurfaceId( reinterpret_cast<IControlSurface*>(this), &m_dwSurfaceId );
		if ( FAILED( hr ))
			return hr;
	}

	if (FAILED( pUnk->QueryInterface( IID_ISonarParamMapping, (void**)&m_pSonarParamMapping ) ))
	{
		// Just in case
		m_pSonarParamMapping = NULL;
	}

	if (FAILED( pUnk->QueryInterface( IID_ISonarMixer2, (void**)&m_pSonarMixer2 ) ))
	{
		// Just in case
		m_pSonarMixer2 = NULL;
	}

	if ( FAILED ( pUnk->QueryInterface( IID_ISonarUIContext2, (void**)&m_pSonarUIContext ) ))
	{
		m_pSonarUIContext = NULL;
	}

	// Call the child class OnConnect()...
	OnConnect();

	// OK, now we're connected
	m_bConnected = true;

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Disconnect()
{
	// Note: You will probably not need to change this implementation.
	// The wizard has already generated code to release all ISonarXXX
	// interfaces currently held.

	CCriticalSectionAuto csa( &m_cs );

	// Call the child class OnDisonnect()...
	OnDisconnect();

	// We're no longer connected
	m_bConnected = false;

	releaseSonarInterfaces();
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CACTController::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_ACTControllerPropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CACTController::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_ACTController;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::IsDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CACTController::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface2 == riid)
		*ppv = static_cast<IControlSurface2*>(this);
	else if (IID_IControlSurface3 == riid)
		*ppv = static_cast<IControlSurface3*>(this);
	else if (IID_ISurfaceParamMapping == riid)
		*ppv = static_cast<ISurfaceParamMapping*>(this);
	else if (IID_IDispatch == riid)
		*ppv = static_cast<IDispatch*>(this);
	else if (IID_ISpecifyPropertyPages == riid)
		*ppv = static_cast<ISpecifyPropertyPages*>(this);
	else if (IID_IPersistStream == riid)
		*ppv = static_cast<IPersistStream*>(this);
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

ULONG CACTController::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CACTController::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IDispatch

static ITypeInfo* m_pTypeInfo = 0;

/////////////////////////////////////////////////////////////////////////////

static HRESULT maybeLoadTypeInfo()
{
	if (!m_pTypeInfo)
	{
		ITypeLib* ptl;
		if (SUCCEEDED( LoadRegTypeLib( LIBID_ACTController, 1, 0, MAKELCID(0, SORT_DEFAULT), &ptl ) ))
		{
			ptl->GetTypeInfoOfGuid( IID_IControlSurface, &m_pTypeInfo );
			ptl->Release();
		}
		else
			return E_NOTIMPL;
	}
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::GetTypeInfoCount( UINT* pctInfo )
{
	*pctInfo = SUCCEEDED( maybeLoadTypeInfo() ) ? 1 : 0;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::GetTypeInfo( UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo )
{
	if (itInfo != 0)
		return E_INVALIDARG;
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	m_pTypeInfo->AddRef();
	*ppITypeInfo = m_pTypeInfo;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::GetIDsOfNames( REFIID riid, OLECHAR** rgszNames, UINT cNames,
										  LCID lcid, DISPID* rgDispID )
{
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Invoke( DISPID dispID, REFIID riid, LCID lcid,
								 WORD wFlags, DISPPARAMS* pDispParams,
								 VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr )
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->Invoke( (IControlSurface*)this, dispID, wFlags,
											pDispParams, pVarResult, 
											pExcepInfo, puArgErr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CACTControllerFactory
//
/////////////////////////////////////////////////////////////////////////////

class CACTControllerFactory : public IClassFactory
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
	CACTControllerFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CACTControllerFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CACTControllerFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CACTController* const pACTController = new CACTController;
	if (!pACTController)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pACTController->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pACTController->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTControllerFactory::LockServer( BOOL bLock ) 
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
	CACTControllerPropPage* const pPropPage = new CACTControllerPropPage;
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
	if (CLSID_ACTController == clsid)
	{
		pFactory = new CACTControllerFactory;  // No AddRef in constructor
	}
	else if (CLSID_ACTControllerPropPage == clsid)
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
	registerFactory( CLSID_ACTController, s_szFriendlyName );
	registerFactory( CLSID_ACTControllerPropPage, s_szFriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_ACTController, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );
		
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_ACTController );
	unregisterFactory( CLSID_ACTControllerPropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_ACTController, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	strlcat( szKey, szCLSID, sizeof(szKey) );
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
// CACTControllerApp

BEGIN_MESSAGE_MAP(CACTControllerApp, CWinApp)
	//{{AFX_MSG_MAP(CACTControllerApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CACTControllerApp construction

CACTControllerApp::CACTControllerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CACTControllerApp object

CACTControllerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CACTControllerPropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CACTControllerPropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CACTControllerPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CACTControllerPropPage::Release() 
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

BOOL CACTControllerPropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CACTControllerPropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CACTControllerPropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

