#include "stdafx.h"
#include "SampleSurface.h"
#include "MixParam.h"


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Handles Refreshing the surface to match the host states
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------
// The host is asking us to refresh.  Here we should query the host for
// the values we're interested in and set our states
HRESULT CSampleSurface::onRefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	// use the m_dwRefreshCount member to thin out updates as needed. For example,
	// motors and meters should be updated as often as possible.  However, switches,
	// and text names can be updated less often.  You can also use this to interlace
	// updates (even/odd strips get updated every other refresh)

	int nRefreshMod = ( (m_dwRefreshCount % 2 ) == 0 ) ? RM_EVEN : RM_ODD;

	for ( InputBindingIterator it = m_mapStripContinuous.begin(); it != m_mapStripContinuous.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		if ( pmb.pMsgOut && (pmb.nRefreshMod == RM_EVERY || pmb.nRefreshMod == nRefreshMod) )
		{
			float fVal = 0.f;
			HRESULT hrChange = pmb.pParam->GetVal( &fVal );
			if ( S_OK == hrChange )
				pmb.pMsgOut->Send( fVal );
		}
	}

	for ( InputBindingIterator it = m_mapStripTrigger.begin(); it != m_mapStripTrigger.end(); ++it )
	{
		PMBINDING& pmb = it->second;
		if ( pmb.pMsgOut && (pmb.nRefreshMod == RM_EVERY || pmb.nRefreshMod == nRefreshMod) )
		{
			float fVal = 0.f;
			HRESULT hrChange = pmb.pParam->GetVal( &fVal );
			if ( S_OK == hrChange )
				pmb.pMsgOut->Send( fVal );
		}
	}


	// meters
	std::vector<float> vMeters;
	for ( StripRangeMapIterator it = m_mapStripRanges.begin(); it != m_mapStripRanges.end(); ++it )
	{
		STRIPRANGE& sr = it->second;
		SONAR_MIXER_STRIP eStrip = it->first;

		for ( DWORD i = sr.dwL; i <= sr.dwH; i++ )
		{
			GetMeterValues( eStrip, i, &vMeters, 1 );
		}
	}
	return S_OK;
}

