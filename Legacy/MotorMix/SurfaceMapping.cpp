/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// SurfaceMapping.cpp : Implementation of ISurfaceParamMapping
/////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Surface.h"
#include "sfkStateDefs.h"

	// ISurfaceparamMapping
HRESULT CControlSurface::GetStripRangeCount(DWORD *pdwCount)
{
	SONAR_MIXER_STRIP mixerStrip = GetCurrentStripKind();

	if ( MIX_STRIP_TRACK == mixerStrip || MIX_STRIP_BUS == mixerStrip )	
		*pdwCount = 1;	// motormix does EITHER tracks or busses
	else
		*pdwCount = 0;

	return S_OK;
}

HRESULT CControlSurface::GetStripRange(DWORD dwIndex, 
													DWORD *pdwLow, 
													DWORD *pdwHigh, 
													SONAR_MIXER_STRIP *pmixerStrip )
{
	CStateShifter *pShifter = NULL;

	if ( dwIndex > 0 )
		return E_FAIL;

	*pmixerStrip = GetCurrentStripKind();
	
	*pdwLow = (DWORD)GetBaseStrip();
	*pdwHigh = *pdwLow + 7;

	return S_OK;
}

//------------------------------------------------
/// Set the low strip of the range from the host
HRESULT CControlSurface::SetStripRange(DWORD dwLow ,SONAR_MIXER_STRIP mixerStrip)
{
	if ( !m_pSonarParamMapping )
		return E_UNEXPECTED;	// a host calling this should support the callback

	SONAR_MIXER_STRIP mixStripCur = GetCurrentStripKind();
	DWORD dwBaseCur = GetBaseStrip();
	bool bChange = (dwBaseCur != dwLow || mixStripCur != mixerStrip);

	if ( !bChange )
		return S_OK;

	if ( MIX_STRIP_TRACK == mixerStrip )
	{
		// if container type is changing
		if ( mixerStrip != mixStripCur )
			GetStateMgr()->PostStateChange( stContainerClass, ccTracks );

		// if number is changing
		if ( dwBaseCur != dwLow )
			GetStateMgr()->PostStateChange( stBaseTrack, dwLow );
	}
	else if ( MIX_STRIP_BUS == mixerStrip )
	{
		// if container type is changing
		if ( mixerStrip != mixStripCur )
			GetStateMgr()->PostStateChange( stContainerClass, ccBus );

		// if number is changing
		if ( dwBaseCur != dwLow )
			GetStateMgr()->PostStateChange( stBaseBus, dwLow );
	}

	return S_OK;
}

//------------------------------------------------
/// Return infor about automapping controls
HRESULT CControlSurface::GetDynamicControlCount(DWORD *pdwCount)
{
	if ( !pdwCount )
		return E_POINTER;
	*pdwCount = 0;

	return S_OK;
}

HRESULT CControlSurface::GetDynamicControlInfo(DWORD dwIndex, DWORD* pdwKey, SURFACE_CONTROL_TYPE *psctControl)
{
	return E_NOTIMPL;
}

HRESULT CControlSurface::SetLearnState(BOOL)
{
	return S_OK;
}
