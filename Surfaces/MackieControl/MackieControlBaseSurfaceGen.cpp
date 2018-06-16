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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CMackieControlBase::Connect( IUnknown* pUnk, HWND hwndApp )
{
//	TRACE("CMackieControlBase::Connect()\n");

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

	if (FAILED(      pUnk->QueryInterface( IID_ISonarIdentity, (void**)&m_pSonarIdentity ) ))
	{
		// Don't worry of this fails, it means it's an old version of SONAR
		TRACE("No ISonarIdentity interface available\n");
		m_pSonarIdentity = NULL; // Just in case

		// In this case, use our internal id generator
		m_dwUniqueId = m_cState.GetNextUniqueId();
	}
	else if ( FAILED ( hr = m_pSonarIdentity->GetUniqueSurfaceId( this, &m_dwUniqueId ) ) )
		return hr;

	if ( FAILED( pUnk->QueryInterface( IID_ISonarMixer2, (void**)&m_pMixer2) ) )
		m_pMixer2 = NULL;

	if ( FAILED( pUnk->QueryInterface( IID_ISonarParamMapping, (void**)&m_pParamMapping ) ) )
		m_pParamMapping = NULL;

	if ( FAILED( pUnk->QueryInterface( IID_ISonarIdentity2, (void**)&m_pSonarIdentity2 ) ) )
		m_pSonarIdentity2 = NULL;
	else if ( FAILED ( hr = m_pSonarIdentity2->GetSupportedRefreshFlags( &m_dwSupportedRefreshFlags ) ) )
		return hr;

	// Initialize filter locator
	m_FilterLocator.OnConnect(m_pSonarIdentity, m_pMixer);

	// Call the child class OnConnect()...
	OnConnect();

	// OK, now we're connected
	m_bConnected = true;

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::Disconnect()
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

/////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::IsDirty( void )
{
	return m_bDirty ? S_OK : S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IUnknown

HRESULT CMackieControlBase::QueryInterface( REFIID riid, void** ppv )
{
	if (IID_IUnknown == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IControlSurface == riid)
		*ppv = static_cast<IControlSurface*>(this);
	else if (IID_IDispatch == riid)
		*ppv = static_cast<IDispatch*>(this);
	else if (IID_ISurfaceParamMapping == riid)
		*ppv = static_cast<ISurfaceParamMapping*>(this);
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

ULONG CMackieControlBase::AddRef()
{
	return ::InterlockedIncrement( &m_cRef );
}

/////////////////////////////////////////////////////////////////////////////

ULONG CMackieControlBase::Release()
{
	ULONG const ulRef = ::InterlockedDecrement( &m_cRef );
	if (0 == ulRef)
		delete this;
	return ulRef;
}

////////////////////////////////////////////////////////////////////////////////
// End of IMFCPropertyPage Implementation
////////////////////////////////////////////////////////////////////////////////

