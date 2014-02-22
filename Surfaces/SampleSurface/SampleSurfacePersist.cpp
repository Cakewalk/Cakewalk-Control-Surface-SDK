#include "stdafx.h"

#include "SampleSurface.h"


/////////////////////////////////////////////////////////////////////////////

// Version informatin for save/load

static const DWORD PERSISTENCE_VERSION = 1;

/////////////////////////////////////////////////////////////////////////////

HRESULT CSampleSurface::persist(IStream* pStm, bool bSave)
{
	TRACE("CSampleSurface:Persist() %s\n", bSave ? "save" : "load");

	DWORD dwVersion = PERSISTENCE_VERSION;
	ULONG cb = 0;

	if ( bSave )
	{
		CHECK_RET( pStm->Write( &dwVersion, sizeof(dwVersion), &cb ) );

		///////////////////////////////////////////////
	}
	else	// LOAD ///
	{
		CHECK_RET( pStm->Read( &dwVersion, sizeof(dwVersion), &cb ) );

		m_bLoaded = true;
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
