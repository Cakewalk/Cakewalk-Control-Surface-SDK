#include "stdafx.h"

#include "VS100.h"


/////////////////////////////////////////////////////////////////////////////

// Version information for save/load

//static const DWORD PERSISTENCE_VERSION = 1;
static const DWORD PERSISTENCE_VERSION = 2;

/////////////////////////////////////////////////////////////////////////////

HRESULT CVS100::persist(IStream* pStm, bool bSave)
{
	TRACE("CVS100:Persist() %s\n", bSave ? "save" : "load");

	DWORD dwVersion = PERSISTENCE_VERSION;
	ULONG cb = 0;

	if ( bSave )
	{
		CHECK_RET( pStm->Write( &dwVersion, sizeof(dwVersion), &cb ) );

		CHECK_RET( pStm->Write( &m_bACTMode, sizeof(m_bACTMode), &cb ) );
		CHECK_RET( pStm->Write( &m_bEnableMotors, sizeof(m_bEnableMotors), &cb ) );

		CHECK_RET( pStm->Write( &m_ixEncoderParam, sizeof(m_ixEncoderParam), &cb ) );

		// feet switch settings
		ButtonActionDefinition bad;
		MsgIdToBADMapIterator it = m_mapButton2BAD.find( CID_Feetswitch1 );
		if ( it != m_mapButton2BAD.end() )
			bad = it->second;
		CHECK_RET( pStm->Write( &bad.eActionType, sizeof(bad.eActionType), &cb ) );
		CHECK_RET( pStm->Write( &bad.transportState, sizeof(bad.transportState), &cb ) );
		CHECK_RET( pStm->Write( &bad.dwCommandOrKey, sizeof(bad.dwCommandOrKey), &cb ) );
		CHECK_RET( pStm->Write( &bad.wModKeys, sizeof(bad.wModKeys), &cb ) );
		
		it = m_mapButton2BAD.find( CID_Feetswitch2 );
		if ( it != m_mapButton2BAD.end() )
			bad = it->second;
		CHECK_RET( pStm->Write( &bad.eActionType, sizeof(bad.eActionType), &cb ) );
		CHECK_RET( pStm->Write( &bad.transportState, sizeof(bad.transportState), &cb ) );
		CHECK_RET( pStm->Write( &bad.dwCommandOrKey, sizeof(bad.dwCommandOrKey), &cb ) );
		CHECK_RET( pStm->Write( &bad.wModKeys, sizeof(bad.wModKeys), &cb ) );

		///////////////////////////////////////////////
	}
	else	// LOAD ///
	{
		CHECK_RET( pStm->Read( &dwVersion, sizeof(dwVersion), &cb ) );

		if ( dwVersion > 1 )
		{
			CHECK_RET( pStm->Read( &m_bACTMode, sizeof(m_bACTMode), &cb ) );
			CHECK_RET( pStm->Read( &m_bEnableMotors, sizeof(m_bEnableMotors), &cb ) );

			// index of the current encoder param setting
			CHECK_RET( pStm->Read( &m_ixEncoderParam, sizeof(m_ixEncoderParam), &cb ) );
			m_ixEncoderParam = min( m_ixEncoderParam, m_vEncoderParam.size()-1 );

			// feet switch settings
			ButtonActionDefinition bad;
			CHECK_RET( pStm->Read( &bad.eActionType, sizeof(bad.eActionType), &cb ) );
			CHECK_RET( pStm->Read( &bad.transportState, sizeof(bad.transportState), &cb ) );
			CHECK_RET( pStm->Read( &bad.dwCommandOrKey, sizeof(bad.dwCommandOrKey), &cb ) );
			CHECK_RET( pStm->Read( &bad.wModKeys, sizeof(bad.wModKeys), &cb ) );
			m_mapButton2BAD[CID_Feetswitch1] = bad;

			CHECK_RET( pStm->Read( &bad.eActionType, sizeof(bad.eActionType), &cb ) );
			CHECK_RET( pStm->Read( &bad.transportState, sizeof(bad.transportState), &cb ) );
			CHECK_RET( pStm->Read( &bad.dwCommandOrKey, sizeof(bad.dwCommandOrKey), &cb ) );
			CHECK_RET( pStm->Read( &bad.wModKeys, sizeof(bad.wModKeys), &cb ) );
			m_mapButton2BAD[CID_Feetswitch2] = bad;
		}

		m_bLoaded = true;
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
