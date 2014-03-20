// Generic Code that should be the same for all control surfaces.
//
// Contains IUnknown routines as well as server registration code, factory code,
// and DLL entrypoint code.
//
// You should not have to modify any of the following.

#include "stdafx.h"
#include "ControlSurfaceProbe.h"
#include "ControlSurfaceProbePropPage.h"

#include "strlcpy.h"
#include "strlcat.h"

const GUID CLSID_ControlSurfaceProbe = { 0x7a674054, 0xe334, 0x4836, { 0x8c, 0xf8, 0xe2, 0xea, 0xfa, 0x93, 0xf4, 0xdd } };
const GUID CLSID_ControlSurfaceProbePropPage = { 0x17df8014, 0xe85a, 0x4afd, { 0xb3, 0x92, 0x8c, 0x42, 0x10, 0x38, 0x72, 0xde } };
const GUID LIBID_ControlSurfaceProbe = { 0x6a01187f, 0xa460, 0x4dd8, { 0x9b, 0x4c, 0x1c, 0xbc, 0x69, 0xa1, 0xa1, 0xc9 } };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CControlSurfaceProbe::Connect( IUnknown* pUnk, HWND hwndApp )
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
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarCommands, (void**)&m_pCommands ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarProject, (void**)&m_pProject ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarMixer, (void**)&m_pMixer ) ))
		return hr;
	if (FAILED( hr = pUnk->QueryInterface( IID_ISonarTransport, (void**)&m_pTransport ) ))
		return hr;

	if (FAILED(      pUnk->QueryInterface( IID_ISonarIdentity, (void**)&m_pIdentity ) ))
	{
		// Just in case
		m_pIdentity = NULL;
	}
	else
	{
		m_pIdentity->GetUniqueSurfaceId(this, &m_dwSurfaceID);
	}

	if (FAILED(      pUnk->QueryInterface( IID_ISonarVUMeters, (void**)&m_pVUMeters ) ))
	{
		// Just in case
		m_pVUMeters = NULL;
	}

	if (FAILED(      pUnk->QueryInterface( IID_ISonarProject2, (void**)&m_pProject2 ) ))
	{
		// Just in case
		m_pProject2 = NULL;
	}

	if ( FAILED( pUnk->QueryInterface( IID_ISonarMixer2, (void**)&m_pMixer2 ) ) )
		m_pMixer2 = NULL;

	if ( FAILED( pUnk->QueryInterface( IID_ISonarParamMapping, (void**)&m_pSonarParamMapping ) ) )
		m_pSonarParamMapping = NULL;

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::Disconnect()
{
	// Note: You will probably not need to change this implementation.
	// The wizard has already generated code to release all ISonarXXX
	// interfaces currently held.

	CCriticalSectionAuto csa( &m_cs );

	releaseSonarInterfaces();
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CControlSurfaceProbe::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_ControlSurfaceProbePropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CControlSurfaceProbe::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_ControlSurfaceProbe;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::IsDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CControlSurfaceProbe::QueryInterface( REFIID riid, void** ppv )
{    
	if (IID_IUnknown == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface == riid)
		*ppv = static_cast<IControlSurface*>(this);
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

ULONG CControlSurfaceProbe::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CControlSurfaceProbe::Release() 
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
		if (SUCCEEDED( LoadRegTypeLib( LIBID_ControlSurfaceProbe, 1, 0, MAKELCID(0, SORT_DEFAULT), &ptl ) ))
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

HRESULT CControlSurfaceProbe::GetTypeInfoCount( UINT* pctInfo )
{
	*pctInfo = SUCCEEDED( maybeLoadTypeInfo() ) ? 1 : 0;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::GetTypeInfo( UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo )
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

HRESULT CControlSurfaceProbe::GetIDsOfNames( REFIID riid, OLECHAR** rgszNames, UINT cNames,
										  LCID lcid, DISPID* rgDispID )
{
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::Invoke( DISPID dispID, REFIID riid, LCID lcid,
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
// CControlSurfaceProbeFactory
//
/////////////////////////////////////////////////////////////////////////////

class CControlSurfaceProbeFactory : public IClassFactory
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
	CControlSurfaceProbeFactory() : m_cRef(1) {}

private:
	long m_cRef;
};

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbeFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CControlSurfaceProbeFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CControlSurfaceProbeFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbeFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CControlSurfaceProbe* const pControlSurfaceProbe = new CControlSurfaceProbe;
	if (!pControlSurfaceProbe)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pControlSurfaceProbe->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pControlSurfaceProbe->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbeFactory::LockServer( BOOL bLock ) 
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
	CControlSurfaceProbePropPage* const pPropPage = new CControlSurfaceProbePropPage;
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
	if (CLSID_ControlSurfaceProbe == clsid)
	{
		pFactory = new CControlSurfaceProbeFactory;  // No AddRef in constructor
	}
	else if (CLSID_ControlSurfaceProbePropPage == clsid)
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
	if (ERROR_SUCCESS == RegOpenKeyEx( hKeyParent, pcszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild ))
	{
		// Enumerate all of the decendents of this child.
		FILETIME time;
		char szBuffer[ 256 ];
		DWORD dwSize = sizeof szBuffer;
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

static void setKeyAndValue( HKEY hKeyParent, const char* pcszKey, const char* pcszSubkey, const char* pcszValueName, const char* pcszValue )
{
	// Copy keyname into buffer.
	char szKeyBuf[ 1024 ];
	::strlcpy( szKeyBuf, pcszKey, sizeof(szKeyBuf) );

	// Add subkey name to buffer.
	if (pcszSubkey != NULL)
	{
		::strlcat( szKeyBuf, "\\", sizeof(szKeyBuf) );
		::strlcat( szKeyBuf, pcszSubkey, sizeof(szKeyBuf) );
	}

	// Create and open key and subkey.
	HKEY hKey;
	if (ERROR_SUCCESS == RegCreateKeyEx( hKeyParent, szKeyBuf,  0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,  &hKey, NULL ))
	{
		if (pcszValue)
			RegSetValueEx( hKey, pcszValueName, 0, REG_SZ, (BYTE*)pcszValue, DWORD(strlen( pcszValue ) + 1) );
		RegCloseKey( hKey );
	}
}

/////////////////////////////////////////////////////////////////////////////

static void registerFactory( const CLSID& clsid, const char* pcszFriendlyName )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	// Add the CLSID to HKEY_CLASSES_ROOT\CLSID
	char szKey[ 64 ];
	::strlcpy( szKey, "CLSID\\", sizeof(szKey) );
	::strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, NULL, pcszFriendlyName );

	// Add the pathname subkey under the CLSID key.
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, "InprocServer32", NULL, szModule );
}

/////////////////////////////////////////////////////////////////////////////

static void unregisterFactory( const CLSID& clsid )
{
	// Get server location.
	char szModule[ _MAX_PATH ];
	::GetModuleFileName( theApp.m_hInstance, szModule, sizeof szModule );

	// Convert the CLSID into a char.
	char szCLSID[ 128 ];
	CLSIDToString( clsid, szCLSID, sizeof szCLSID );

	char szKey[ 64 ];
	::strlcpy( szKey, "CLSID\\", sizeof(szKey) );
	::strlcat( szKey, szCLSID, sizeof(szKey) );

	recursiveDeleteKey( HKEY_CLASSES_ROOT, szKey );
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	registerFactory( CLSID_ControlSurfaceProbe, s_szFriendlyName );
	registerFactory( CLSID_ControlSurfaceProbePropPage, s_szFriendlyNamePropPage );

	// Add the CLSID to HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_ControlSurfaceProbe, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	::strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	::strlcat( szKey, szCLSID, sizeof(szKey) );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "Description", s_szFriendlyName );
	setKeyAndValue( HKEY_CLASSES_ROOT, szKey, NULL, "HelpFilePath", "" );
		
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	unregisterFactory( CLSID_ControlSurfaceProbe );
	unregisterFactory( CLSID_ControlSurfaceProbePropPage );

	// Remove the CLSID from HKEY_CLASSES_ROOT\CakewalkControlSurfaces
	char szCLSID[ 128 ];
	CLSIDToString( CLSID_ControlSurfaceProbe, szCLSID, sizeof szCLSID );
	char szKey[ 256 ];
	::strlcpy( szKey, "CakewalkControlSurfaces\\", sizeof(szKey) );
	::strlcat( szKey, szCLSID, sizeof(szKey) );
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
// CControlSurfaceProbeApp

BEGIN_MESSAGE_MAP(CControlSurfaceProbeApp, CWinApp)
	//{{AFX_MSG_MAP(CControlSurfaceProbeApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControlSurfaceProbeApp construction

CControlSurfaceProbeApp::CControlSurfaceProbeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CControlSurfaceProbeApp object

CControlSurfaceProbeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CControlSurfaceProbePropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CControlSurfaceProbePropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CControlSurfaceProbePropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CControlSurfaceProbePropPage::Release() 
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

BOOL CControlSurfaceProbePropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CControlSurfaceProbePropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CControlSurfaceProbePropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

