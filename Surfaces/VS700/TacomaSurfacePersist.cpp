#include "stdafx.h"

#include "TacomaSurface.h"


/////////////////////////////////////////////////////////////////////////////

HRESULT CTacomaSurface::persist(IStream* pStm, bool bSave)
{
	TRACE("CTacomaSurface:Persist() %s\n", bSave ? "save" : "load");
	if ( bSave )
		m_persist.Save( pStm, TRUE );
	else
		m_persist.Load( pStm );

	return S_OK;
}



//-------------------------------------------------------------------------------------
WORD CTacomaSurface::CTacomaSurfacePersist::m_wPersistChunkID = 5;
//WORD CTacomaSurface::CTacomaSurfacePersist::m_wPersistSchema = 0;
WORD CTacomaSurface::CTacomaSurfacePersist::m_wPersistSchema = 1;	// add preset names

HRESULT CTacomaSurface::CTacomaSurfacePersist::Persist( WORD wSchema, CPersistDDX& ddx )
{
	if ( 0 == wSchema )
	{
		int cCmds = (int)m_pSurface->m_mapButton2BAD.size();
		CHECK_XFER( ddx.Xfer( &cCmds ) );

		if ( ddx.IsSaving() )
		{
			for ( MsgIdToBADMapIterator it = m_pSurface->m_mapButton2BAD.begin(); 
						it != m_pSurface->m_mapButton2BAD.end(); ++it )
			{
				ButtonActionDefinition& bad = it->second;
				ControlId id = it->first;
				CHECK_XFER( ddx.XferBuf( sizeof(id), &id ) );
				CHECK_XFER( ddx.Xfer( &bad.dwCommandOrKey ) );
				CHECK_XFER( ddx.XferBuf(sizeof(bad.eActionType), &bad.eActionType ) );
				CHECK_XFER( ddx.Xfer( &bad.wModKeys ) );
				CHECK_XFER( ddx.XferBuf( sizeof( bad.transportState ), &bad.transportState ) );
			}
		}
		else
		{
			CTacomaSurface::MsgIdToBADMap mapBAD;

			for ( int i = 0; i < cCmds; i++ )
			{
				ControlId id;
				CHECK_XFER( ddx.XferBuf( sizeof(id), &id ) );

				ButtonActionDefinition bad;
				CHECK_XFER( ddx.Xfer( &bad.dwCommandOrKey ) );
				CHECK_XFER( ddx.XferBuf(sizeof(bad.eActionType), &bad.eActionType ) );
				CHECK_XFER( ddx.Xfer( &bad.wModKeys ) );
				CHECK_XFER( ddx.XferBuf( sizeof( bad.transportState ), &bad.transportState ) );

				mapBAD[id] = bad;
			}

			m_pSurface->m_mapButton2BAD = mapBAD;
		}

		CHECK_XFER( ddx.XferBuf(sizeof(m_pSurface->m_mfxfPrimary), &m_pSurface->m_mfxfPrimary ) );
		CHECK_XFER( ddx.XferBuf(sizeof(bool), &m_pSurface->m_bShowFaderOnLCD ) );
		CHECK_XFER( ddx.XferBuf(sizeof(bool), &m_pSurface->m_bEnableFaderMotors ) );
		CHECK_XFER( ddx.XferBuf(sizeof(bool), &m_pSurface->m_bSoloSelectsChannel ) );
		CHECK_XFER( ddx.XferBuf(sizeof(bool), &m_pSurface->m_bFaderTouchSelectsChannel ) );

		// alt encoder assignments
      //
      int cAssign = (int) m_pSurface->m_vEncoderParamList.size();
      CHECK_XFER( ddx.Xfer( &cAssign ) );
		if ( !ddx.IsSaving() )
			m_pSurface->emptyEncoderParamList();

      PSEncoderParams pParam;
		for ( int i = 0; i < cAssign; i++ )
		{
			if ( ddx.IsSaving() )
				pParam = m_pSurface->m_vEncoderParamList[ i ];
         else
            pParam = new ( SEncoderParams );

         ASSERT( pParam );

			CHECK_XFER( ddx.XferBuf( sizeof( SEncoderParams ), pParam ) );

			if ( !ddx.IsSaving() )
				m_pSurface->m_vEncoderParamList.push_back( pParam );
		}
	}
	else if ( 1 == wSchema )
	{
		// Preamp and DM preset names
		CHECK_XFER( ddx.Xfer( &m_pSurface->m_strPreampPreset ) );
		CHECK_XFER( ddx.Xfer( &m_pSurface->m_strDMPreset) );
	}
	else
		CHECK_XFER( E_FAIL );

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
