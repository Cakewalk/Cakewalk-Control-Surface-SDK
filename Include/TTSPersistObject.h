// $Header: /Src/Include/TTSPersistObject.h 3     3/01/04 8:38p Nhaddad $
// Copyright (c) Twelve Tone Systems, Inc.  All rights reserved.
//
// TTSPersistObject.h: interface for the CTTSPersistObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TTSPERSISTOBJECT_H__BCCF3640_7EE7_4D43_9EE9_34013402440E__INCLUDED_)
#define AFX_TTSPERSISTOBJECT_H__BCCF3640_7EE7_4D43_9EE9_34013402440E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable: 4100 ) // unreferenced formal parameter

#include <TTSPersistStream.h>

class CPersistDDX;

////////////////////////////////////////////////////////////////////////////////

struct ITTSPersist
{
	// This interface is meant to be used by lightweight objects that appear
	// in vast quantities, in containers.  Examples of such objects are CSeqEvent,
	// CMarker, CTempo.  These kinds of objects don't carry their own schema
	// or chunk.  Instead, the parent container (map) manages the schema/chunk.
	virtual HRESULT OnPersistBegin( CPersistDDX& ddx ) = 0;
	virtual HRESULT OnPersistEnd( CPersistDDX& ddx ) = 0;
	virtual HRESULT Persist( WORD wSchema, CPersistDDX& ddx ) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class CTTSPersistCOMObject : public CTTSPersistStream, public ITTSPersist
{
public:
	CTTSPersistCOMObject( DWORD dwID ) : CTTSPersistStream(dwID) {}
	CTTSPersistCOMObject( WORD wLastSchemaNum, WORD wID ) : CTTSPersistStream(wLastSchemaNum, wID) {}
	virtual ~CTTSPersistCOMObject();

	CTTSPersistCOMObject& operator=( const CTTSPersistCOMObject& rhs ) 
	{
		if (&rhs != this)
		{
			CTTSPersistStream::operator=( rhs ); // base class persist
		}
		return *this;
	}

	// IUnknown left un-implemented, up to derived classes

	// ITTSPersist: Persist left un-implemented, up to derived classes
	virtual HRESULT OnPersistBegin( CPersistDDX& ddx ) { return S_OK; }
	virtual HRESULT OnPersistEnd( CPersistDDX& ddx ) { return S_OK; }

	// CTTSPersistStream overrides
	HRESULT LoadSchema( DWORD dwID, LPSTREAM pStm, ULARGE_INTEGER cbSize );
	HRESULT SaveSchema( DWORD dwID, LPSTREAM pStm, ULARGE_INTEGER* pcbWritten, BOOL bClearDirty );
	HRESULT GetSizeSchema( DWORD dwID, ULARGE_INTEGER* pcbSize );
};

////////////////////////////////////////////////////////////////////////////////

class CTTSPersistObject : public CTTSPersistCOMObject
{
public:
	CTTSPersistObject( WORD wLastSchemaNum, WORD wID ) :	CTTSPersistCOMObject(wLastSchemaNum, wID), 
																			m_lRefCount(0) {}

	// dwID is the schema num and objectID packed into a DWORD.
	CTTSPersistObject( DWORD dwID ) : CTTSPersistCOMObject(dwID), m_lRefCount(0) {}


	CTTSPersistObject& operator=( const CTTSPersistObject& rhs ) 
	{
		if (&rhs != this)
		{
			CTTSPersistCOMObject::operator=( rhs );
			m_lRefCount = rhs.m_lRefCount;
		}
		return *this;
	}

	// IUnknown
	STDMETHODIMP QueryInterface( REFIID riid, void** ppv )
	{
		if (IsEqualIID( riid, IID_IUnknown ))
			*ppv = static_cast<IUnknown*>( static_cast<IPersistStreamInit*>( this ) );
		else if (IsEqualIID( riid, IID_IPersistStreamInit ))
			*ppv = static_cast<IPersistStreamInit*>( this );
		else if (IsEqualIID( riid, IID_IPersistStream ))
			*ppv = static_cast<IPersistStream*>( this );
		else
			return E_NOTIMPL;
		AddRef();
		return S_OK;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		::InterlockedIncrement( &m_lRefCount );
		return m_lRefCount;
	}

	STDMETHODIMP_(ULONG) Release() 
	{
		::InterlockedDecrement( &m_lRefCount );
		ASSERT( m_lRefCount >= 0 );
		return m_lRefCount;
	}

private:
	long m_lRefCount;
};

////////////////////////////////////////////////////////////////////////////////
// CTTSPersistBaseObject
//
//	Whenever a polymorphically used class hierarchy needs to be extended
// to support persistence, we have the same problem:
// The version control (schema number) cannot be independently changed
// for each of the derived classes. Often the solution employed in this case
// is less than ideal: To have the derived objects duplicate the persistence
// code for the base class members.
//
// CTTSPersistBaseObject can be used to solve this problem.
// Whenever an entire class hierarchy needs to become persistence capable
// all the developer has to do is make the base class derive from
// CTTSPersistBaseObject, instead of CTTSPersistObject, and define
// the schema and ID numbers in the usual way.
// Then, persistence of the base class members is implemented within
// innerPersist( wSchema, ddx );
//
//	Then, each derived class can be made a CTTSPersistObject as needed
// to persist its own attributes that extend the base class.
// In order to persist the base attributes, each derived class simply has to
// invoke BasePersist( ddx ) on the base class.
//
// Notice how this technique can equally be applied to the case when a
// persist object needs to be factored into a hierarchy, since
// the persistence format of a base object is no different
// than the persist format of any object.
//
// CTTSPersistBaseObject has only one caveat: it can only be used
// in a hierarchy that is one level deep.
// However, this design pattern can in principle be extended
// to any depth.
//
// -ARR
//
////////////////////////////////////////////////////////////////////////////////


// forward
class CTTSPersistBaseObject;

////////////////////////////////////////////////////////////////////////////////

class CTTSPersistBaseProxy: public CTTSPersistObject
{
public:
	CTTSPersistBaseProxy( CTTSPersistBaseObject* pOwner, WORD wLastSchemaNum, WORD wID ) :
			CTTSPersistObject(wLastSchemaNum, wID),
			m_pOwner( pOwner )
	{}

	virtual HRESULT Persist( WORD wSchema, CPersistDDX& ddx );

private:
	CTTSPersistBaseObject*		m_pOwner;
};

////////////////////////////////////////////////////////////////////////////////

class CTTSPersistBaseObject
{
public:
	friend class CTTSPersistBaseProxy;

	CTTSPersistBaseObject( WORD wLastSchemaNum, WORD wID );

	HRESULT BasePersist( CPersistDDX& ddx );

protected:

	// override to persist contents of base class
	virtual HRESULT innerPersist( WORD wSchema, CPersistDDX& ddx ) = 0;

private:
	CTTSPersistBaseProxy		m_PersistProxy;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_TTSPERSISTOBJECT_H__BCCF3640_7EE7_4D43_9EE9_34013402440E__INCLUDED_)
