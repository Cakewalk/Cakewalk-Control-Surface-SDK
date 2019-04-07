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

#include "MackieControl7SegmentDisplay.h"
#include "MackieControlMaster.h"
#include "MackieControlMasterPropPage.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef _MACKIECONTROLC1
const GUID CLSID_MackieControlMaster = { 0xf746848e, 0x71a1, 0x4bdc, { 0x81, 0x36, 0x05, 0x3f, 0x04, 0x5e, 0x6f, 0x7f } };
const GUID CLSID_MackieControlMasterPropPage = { 0x5779e119, 0xc561, 0x44c6, { 0x82, 0xf9, 0x8a, 0xe7, 0x5f, 0x4c, 0x7b, 0x2c } };
const GUID LIBID_MackieControlMaster = { 0xdac32b99, 0x4b91, 0x41ac, { 0xa3, 0xc4, 0x43, 0x71, 0x2a, 0x0b, 0x63, 0x41 } };
#endif
#ifdef _MACKIECONTROLC2
const GUID CLSID_MackieControlMaster = { 0xfaaeee56, 0x4b0f, 0x4516, { 0xa9, 0x3f, 0xf8, 0xf1, 0x01, 0xa6, 0x23, 0x38 } };
const GUID CLSID_MackieControlMasterPropPage = { 0x3f58edd0, 0x65ba, 0x4c5d, { 0xbe, 0x2b, 0xb1, 0x06, 0xcd, 0x5a, 0xeb, 0xbd } };
const GUID LIBID_MackieControlMaster = { 0x5c854695, 0x2c8a, 0x41fb, { 0xa8, 0x94, 0x5b, 0xea, 0xd4, 0xed, 0xff, 0x86 } };
#endif
#ifdef _MACKIECONTROLC3
const GUID CLSID_MackieControlMaster = { 0xfbf14dc7, 0x16c7, 0x4d3e, { 0x95, 0xfa, 0xfc, 0x8b, 0x92, 0x5c, 0x71, 0x8e } };
const GUID CLSID_MackieControlMasterPropPage = { 0x6d1928a1, 0x33ac, 0x4803, { 0x80, 0xa6, 0x8f, 0xf1, 0x73, 0xe8, 0xda, 0xbd } };
const GUID LIBID_MackieControlMaster = { 0x07a7a952, 0x051f, 0x4692, { 0x9f, 0x52, 0x1b, 0xa0, 0x72, 0x30, 0x96, 0xce } };
#endif
#ifndef _MACKIECONTROLMMCL
const GUID CLSID_MackieControlMaster = { 0xf5cc6567, 0x685b, 0x4fd7, { 0xa7, 0x40, 0xf9, 0x7b, 0x1d, 0x5c, 0x83, 0x86 } };
const GUID CLSID_MackieControlMasterPropPage = { 0xb2a81c09, 0x00e1, 0x4a82, { 0x9a, 0xc6, 0xf5, 0xce, 0x83, 0xe5, 0x26, 0x8e } };
const GUID LIBID_MackieControlMaster = { 0x6d518958, 0x4eaf, 0x47ed, { 0xad, 0x94, 0x27, 0x95, 0xe6, 0x92, 0xcc, 0x6b } };
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ISpecifyPropertyPages

HRESULT CMackieControlMaster::GetPages( CAUUID* pPages )
{
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc( pPages->cElems * sizeof(GUID) );
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;

	pPages->pElems[ 0 ] = CLSID_MackieControlMasterPropPage;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

HRESULT CMackieControlMaster::GetClassID( CLSID* pClsid )
{
	*pClsid = CLSID_MackieControlMaster;
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
		if (SUCCEEDED( LoadRegTypeLib( LIBID_MackieControlMaster, 1, 0, MAKELCID(0, SORT_DEFAULT), &ptl ) ))
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

HRESULT CMackieControlMaster::GetTypeInfoCount( UINT* pctInfo )
{
	*pctInfo = SUCCEEDED( maybeLoadTypeInfo() ) ? 1 : 0;
	return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::GetTypeInfo( UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo )
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

HRESULT CMackieControlMaster::GetIDsOfNames( REFIID riid, OLECHAR** rgszNames, UINT cNames,
										  LCID lcid, DISPID* rgDispID )
{
	if (FAILED( maybeLoadTypeInfo() ))
		return E_NOTIMPL;
	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgDispID );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMaster::Invoke( DISPID dispID, REFIID riid, LCID lcid,
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
// CMackieControlMasterFactory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlMasterFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlMasterFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlMaster* const pMackieControlMaster = new CMackieControlMaster;
	if (!pMackieControlMaster)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pMackieControlMaster->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pMackieControlMaster->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterFactory::LockServer( BOOL bLock ) 
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
// CMackieControlMasterPropPageFactory
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPageFactory::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlMasterPropPageFactory::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

ULONG CMackieControlMasterPropPageFactory::Release() 
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPageFactory::CreateInstance( IUnknown* pUnkOuter, REFIID riid, void** ppv ) 
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
	
	// Cannot aggregate.
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// Create component.
	CMackieControlMasterPropPage* const pPropPage = new CMackieControlMasterPropPage;
	if (!pPropPage)
		return E_OUTOFMEMORY;

	// Get the requested interface.
	HRESULT const hr = pPropPage->QueryInterface( riid, ppv );

	// Release the IUnknown pointer. If QI() failed, component will delete itself.
	pPropPage->Release();
	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlMasterPropPageFactory::LockServer( BOOL bLock ) 
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
// CMackieControlMasterPropPage
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMackieControlMasterPropPage::QueryInterface( REFIID riid, void** ppv )
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

ULONG CMackieControlMasterPropPage::AddRef()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlMasterPropPage::Release() 
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

BOOL CMackieControlMasterPropPage::CreateMFCDialog( HWND hwndParent )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return Create( CMackieControlMasterPropPage::IDD, CWnd::FromHandle( hwndParent ) );
}

///////////////////////////////////////////////////////////////////////////////

BOOL CMackieControlMasterPropPage::DestroyMFCDialog()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	return DestroyWindow();
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

