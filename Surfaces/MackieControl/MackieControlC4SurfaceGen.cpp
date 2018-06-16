// Generic Code that should be the same for all control surfaces.
//
// Contains IUnknown routines as well as server registration code, factory code,
// and DLL entrypoint code.
//
// You should not have to modify any of the following.

#include "stdafx.h"

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
#include "MackieControlC4.h"
#include "MackieControlC4PropPage.h"

////////////////////////////////////////////////////////////////////////////////

const GUID CLSID_MackieControlC4 = { 0x73ad3a4d, 0xaadf, 0x460b, { 0xa8, 0x02, 0xe6, 0xc0, 0xdf, 0x45, 0x7a, 0x70 } };
const GUID CLSID_MackieControlC4PropPage = { 0x141e675f, 0x8443, 0x40cd, { 0xa9, 0x25, 0x0a, 0x63, 0xb3, 0xb4, 0x2a, 0xb9 } };
const GUID LIBID_MackieControlC4 = { 0xddf87092, 0x6823, 0x45d9, { 0x8a, 0x90, 0x1a, 0x0e, 0x84, 0x05, 0x3f, 0xcc } };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CMackieControlC4::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_MackieControlC4PropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CMackieControlC4::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_MackieControlC4;
	return S_OK;
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
		if (SUCCEEDED( LoadRegTypeLib( LIBID_MackieControlC4, 1, 0, MAKELCID(0, SORT_DEFAULT), &ptl ) ))
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

HRESULT CMackieControlC4::GetTypeInfoCount( UINT* pctInfo )
{
	*pctInfo = SUCCEEDED( maybeLoadTypeInfo() ) ? 1 : 0;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::GetTypeInfo( UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo )
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

HRESULT CMackieControlC4::GetIDsOfNames( REFIID riid, OLECHAR** rgszNames, UINT cNames,
										  LCID lcid, DISPID* rgDispID )
{
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4::Invoke( DISPID dispID, REFIID riid, LCID lcid,
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
// CMackieControlC4Factory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4Factory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlC4Factory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlC4Factory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4Factory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlC4* const pMackieControlC4 = new CMackieControlC4;
	if (!pMackieControlC4)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pMackieControlC4->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pMackieControlC4->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4Factory::LockServer( BOOL bLock ) 
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
// CMackieControlC4PropPageFactory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPageFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlC4PropPageFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

ULONG CMackieControlC4PropPageFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPageFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlC4PropPage* const pPropPage = new CMackieControlC4PropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pPropPage->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlC4PropPageFactory::LockServer( BOOL bLock ) 
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
// CMackieControlC4PropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMackieControlC4PropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlC4PropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlC4PropPage::Release() 
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

BOOL CMackieControlC4PropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CMackieControlC4PropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CMackieControlC4PropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

