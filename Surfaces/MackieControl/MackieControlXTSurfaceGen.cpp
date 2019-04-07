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
#include "MackieControlXT.h"
#include "MackieControlXTPropPage.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef _MACKIECONTROLC1
const GUID CLSID_MackieControlXT = { 0xf75084a8, 0xbdfd, 0x46dd, { 0xb4, 0xce, 0x83, 0x7d, 0x5e, 0x8d, 0x7f, 0x1d } };
const GUID CLSID_MackieControlXTPropPage = { 0x415c5b6f, 0x67f8, 0x4594, { 0xa9, 0x33, 0xfb, 0x10, 0xb5, 0x50, 0xee, 0x5f } };
const GUID LIBID_MackieControlXT = { 0x6a4f51ec, 0x797f, 0x4618, { 0xaa, 0x9e, 0xc0, 0x89, 0x4b, 0x3f, 0x34, 0x7b } };
#endif
#ifdef _MACKIECONTROLC2
const GUID CLSID_MackieControlXT = { 0xfbcd20bc, 0x5f08, 0x40e0, { 0x9d, 0x9c, 0x28, 0xf0, 0x97, 0xb3, 0x98, 0x7c } };
const GUID CLSID_MackieControlXTPropPage = { 0xfc2b01c4, 0xc212, 0x445b, { 0x9e, 0x33, 0x07, 0xe1, 0xdd, 0xb2, 0x85, 0xe7 } };
const GUID LIBID_MackieControlXT = { 0x93cbde1d, 0x8b32, 0x4967, { 0xbe, 0x04, 0x63, 0x4f, 0x4a, 0x99, 0x28, 0x31 } };
#endif
#ifdef _MACKIECONTROLC3
const GUID CLSID_MackieControlXT = { 0xfd08bb4f, 0xe456, 0x4e28, { 0xb7, 0x0d, 0x1b, 0x13, 0x96, 0x79, 0x21, 0x00 } };
const GUID CLSID_MackieControlXTPropPage = { 0xfd69717f, 0x59aa, 0x426c, { 0xaa, 0xed, 0xb8, 0x8a, 0x7d, 0x00, 0x55, 0x10 } };
const GUID LIBID_MackieControlXT = { 0x881a7705, 0x48b0, 0x4900, { 0x89, 0xf5, 0x65, 0x50, 0xd7, 0xce, 0x52, 0x91 } };
#endif
#ifndef _MACKIECONTROLMMCL
const GUID CLSID_MackieControlXT = { 0xde11fd90, 0xc068, 0x41ff, { 0xb6, 0x13, 0x98, 0xd5, 0x7e, 0xfc, 0xf4, 0xef } };
const GUID CLSID_MackieControlXTPropPage = { 0xd0c1a77b, 0x2502, 0x49f5, { 0x8b, 0xc5, 0x86, 0xbe, 0x17, 0x39, 0x4b, 0xd7 } };
const GUID LIBID_MackieControlXT = { 0x40dc4115, 0x0ff9, 0x4e21, { 0xbe, 0x30, 0x09, 0x13, 0x73, 0xd9, 0x3e, 0xdb } };
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CMackieControlXT::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_MackieControlXTPropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CMackieControlXT::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_MackieControlXT;
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
		if (SUCCEEDED( LoadRegTypeLib( LIBID_MackieControlXT, 1, 0, MAKELCID(0, SORT_DEFAULT), &ptl ) ))
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

HRESULT CMackieControlXT::GetTypeInfoCount( UINT* pctInfo )
{
	*pctInfo = SUCCEEDED( maybeLoadTypeInfo() ) ? 1 : 0;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::GetTypeInfo( UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo )
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

HRESULT CMackieControlXT::GetIDsOfNames( REFIID riid, OLECHAR** rgszNames, UINT cNames,
										  LCID lcid, DISPID* rgDispID )
{
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXT::Invoke( DISPID dispID, REFIID riid, LCID lcid,
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
// CMackieControlXTFactory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlXTFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlXTFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlXT* const pMackieControlXT = new CMackieControlXT;
	if (!pMackieControlXT)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pMackieControlXT->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pMackieControlXT->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTFactory::LockServer( BOOL bLock ) 
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
// CMackieControlXTPropPageFactory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPageFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlXTPropPageFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

ULONG CMackieControlXTPropPageFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPageFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlXTPropPage* const pPropPage = new CMackieControlXTPropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pPropPage->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlXTPropPageFactory::LockServer( BOOL bLock ) 
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
// CMackieControlXTPropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMackieControlXTPropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlXTPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlXTPropPage::Release() 
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

BOOL CMackieControlXTPropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CMackieControlXTPropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CMackieControlXTPropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////
