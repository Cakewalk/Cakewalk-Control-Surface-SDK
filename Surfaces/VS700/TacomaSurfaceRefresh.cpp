#include "stdafx.h"
#include "TacomaSurface.h"
#include "MixParam.h"
#include "StringCruncher.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Handles Refreshing the surface to match the host states
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------
// The host is asking us to refresh.  Here we should query the host for
// the values we're interested in and set our states
HRESULT CTacomaSurface::onRefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	if ( !m_bConnected )
		return S_FALSE;

	if ( !m_bIOMode )
	{
		refreshMarkers();
		refreshLayers();
		refreshPlugins();
	}

	// use the m_dwRefreshCount member to thin out updates as needed. For example,
	// motors and meters should be updated as often as possible.  However, switches,
	// and text names can be updated less often.  You can also use this to interlace
	// updates (even/odd strips get updated every other refresh)

   const DWORD dwRefreshCountMod2 = ( m_dwRefreshCount % 2 );
   const DWORD dwRefreshCountMod4 = ( (m_dwRefreshCount+1) % 2 ); // frequent but opposite to above
	const int nRefreshMod = ( dwRefreshCountMod2 == 0 ) ? RM_EVEN : RM_ODD;

	// For all of our parameter binding sets,  Query the host for changes and possibly broadcast
	// them out any associated output messages. Also update the input message state so that
	// their current value matches the host

	InputBindingMap*	aMaps[] = { NULL, &m_mapStripMsgs, &m_mapMiscMsgs };	// 0th placeholder is for the act/eq/send map
	
	// select the alternate control map based on the KnobSectionMode (KSM) 
	switch ( m_eActSectionMode )
	{
	case KSM_FLEXIBLE_PC:
	case KSM_ACT:
		aMaps[0] = &m_mapACTControls;
		if ( dwRefreshCountMod2 == 0 && m_pSonarParamMapping )
			updateActContextName( false );
		break;
	case KSM_EQ:
		aMaps[0] = &m_mapEQControls;
		if ( GetSpecialDisplayMode() == SDM_Normal )
			showEQParams( PSM_Refresh );
		if (!IsLegacyEQ())
			m_eActSectionMode = KSM_PC_COMP;
		break;
	case KSM_PC_EQ:
		aMaps[0] = &m_mapPCEQControls;
		break;
	case KSM_PC_EQ_P2:
		aMaps[0] = &m_mapPCEQP2Controls;
		break;
	case KSM_PC_COMP:
		aMaps[0] = &m_mapPCCompControls;
		if (IsLegacyEQ())
			m_eActSectionMode = KSM_EQ;
		break;
	case KSM_PC_SAT:
		aMaps[0] = &m_mapPCSatControls;
		break;
	case KSM_SEND:
		aMaps[0] = &m_mapSendControls;
		break;
	}

	PSEncoderParams pEnc = NULL;
	if ( m_bIOMode )
	{
		pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, Enc_IO );
		if ( !pEnc )
			return ( E_UNEXPECTED );
	}

   for ( size_t iMap = 0; iMap < _countof( aMaps ); iMap++ )
   {
	   InputBindingMap* pMap = aMaps[iMap];
	   if ( !pMap )
		   continue;

	   for ( InputBindingIterator it = pMap->begin(); it != pMap->end(); ++it )
	   {
		   PMBINDING& pmb = it->second;
		   CMidiMsg* pmsgIn = it->first;
		   const DWORD dwId = pmsgIn->GetId();

		   // one special case - skip the T-bar ACT param if not in that mode
		   if ( BID_Tbar == dwId && TBM_ACT != m_eTBM)
			   continue;

		   if ( pmb.nRefreshMod == RM_EVERY || pmb.nRefreshMod == nRefreshMod )
		   {
			   if ( m_bIOMode )
					refreshIoMode( pmb, pmsgIn, pEnc );
			   else
			   {
				   float fVal = 0.f;
				   HRESULT hrChange = pmb.pParam->GetVal( &fVal );
					if ( S_FALSE != hrChange || pmb.pParam->IsAlwaysChanging() )
				   {
					   // if we're in any FX Display mode, do not send anything to
					   // the Solo or Arm
					   if ( ( SDM_ExistingFX == m_eDisplayMode || SDM_FXTree == m_eDisplayMode ) &&
							  ( BID_IN( dwId, BID_Solo0, BID_Solo7 ) || BID_IN( dwId, BID_Arm0, BID_Arm7 ) ) )
						   continue;
						if ( ( SDM_FXTree == m_eDisplayMode ) && BID_IN( dwId, BID_Mute0, BID_Mute7 ) )
							continue;

					   // either a failure or there was a change
					   if ( FAILED( hrChange ) )
						{
							// if getting PAN failed, it could be because we're asking for pan on a surround track
							if ( hrChange == E_NOTIMPL )
								pmb.pParam->ResetHistory();
							else
								fVal = 0.f;
						}

					   // is there an output message?  Broadcast
					   bool bSend = pmb.pMsgOut != NULL;
					   if ( !m_bEnableFaderMotors && BID_IN( dwId, BID_Fader0, BID_FaderMstr ) )
						   bSend = false;

					   // if currently touched don't send
					   if ( pmb.pParam->IsTouched() )
						   bSend = false;

					   if ( bSend )
						   pmb.pMsgOut->Send( fVal );

						// In any case, set input state so that encoders' deltas work
						pmsgIn->SetVal( fVal );

						if ( !m_tcbParamChange.IsActive() )
						{
							if ( ( SDM_Normal == m_eDisplayMode || SDM_ChannelBranch == m_eDisplayMode ) && !pmsgIn->IsTrigger() )
								showParam( pmb.pParam, PSM_Refresh );
						}
				   }
			   }
		   }
	   }
   }

	// Button states
	if ( dwRefreshCountMod4 == 0 )
	{
		for ( ControlMessageMapIterator it = m_mapControlMsgs.begin(); it != m_mapControlMsgs.end(); it++ )
		{
			CMidiMsg* pmsg = it->second;
			ControlId id = it->first;

			if ( BID_Play == id || BID_Stop == id )
			{
				if ( IsScrub() )
					startBlink( id );
				else if ( IsPause() && BID_Play == id )
					startBlink( id );
				else
					stopBlink( id );
			}

			// if not blinking, set the state
			if ( m_setBlinkControls.count( id ) == 0 )
			{
				switch( id )
				{
					case BID_Flip:
						pmsg->Send( m_bFlipped ? 1.f : 0.f );
						break;
					case BID_RudeMute:
						pmsg->Send( GetRudeMute() ? 1.f : 0.f );
						break;
					case BID_RudeSolo:
						pmsg->Send( GetRudeSolo() ? 1.f : 0.f );
						break;
					case BID_RudeArm:
						pmsg->Send( GetRudeArm() ? 1.f : 0.f );
						break;
					case BID_Play:
						pmsg->Send( IsPlaying() ? 1.f : 0.f );
						break;
					case BID_Stop:
						pmsg->Send( IsPlaying() ? 0.f : 1.f );
						break;
					case BID_Record:
						pmsg->Send( IsRecording() ? 1.f : 0.f );
						break;
					case BID_FF:
						pmsg->Send( IsFastForward() ? 1.f : 0.f );
						break;
					case BID_Rew:
						pmsg->Send( IsRewind() ? 1.f : 0.f );
						break;
				}
			}

			// snap
			if ( id == BID_Snap )
			{
				CMidiMsg* pmsg = it->second;
				if ( m_pSonarProject3 )
				{
					BOOL bValue = FALSE;
					float f = 0.f;
					if ( SUCCEEDED( m_pSonarProject3->GetProjectState( EPP_SNAP, &bValue ) ) && bValue )
						f = 1.f;
					pmsg->Send( f );
				}
			}
			// loop & punch
			else if ( id == BID_PunchOn || id == BID_LoopOn )
			{
				if ( m_eJogMode != JM_Loop && m_eJogMode != JM_Punch )
				{
					if ( m_pSonarTransport )
					{
						BOOL bValue = FALSE;
						float f = 0.f;
						if ( SUCCEEDED( m_pSonarTransport->GetTransportState( ( id == BID_PunchOn ) ? TRANSPORT_STATE_AUTOPUNCH : TRANSPORT_STATE_LOOP, &bValue ) ) && bValue )
							f = 1.f;
						pmsg->Send( f );
					}
				}
			}
         else if ( id == BID_Automation && m_pSonarMixer )
         {
            BOOL bOn = FALSE;
            m_pSonarMixer->GetArmMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_ANY, 0, &bOn );
            pmsg->Send( bOn ? 1.f : 0.f );
         }
		}

		// Nulling LEDs for T-bar
		if ( dwRefreshCountMod2 == 0 )
			initOverUnderLeds();

		// Strip Names
		char sz[128];
		char szDest[8];
		DWORD cb = _countof(sz);

		// Check for reasons NOT to display track names...
		if ( !m_bIOMode && !m_tcbParamChange.IsActive() && !m_tctRevert.IsActive() )
		{
			if ( SDM_Normal == m_eDisplayMode )
			{
				CStringCruncher cCruncher;
				for ( DWORD i = 0; i < 8; i++ )
				{
					STRIP_INFO info = getStripInfo( i );

					sz[0] = '\0';
					cb = 127;
					m_pSonarMixer->GetMixStripName( info.stripType, info.stripIndex, sz, &cb );
					if ( info.isStripLocked )
					{
						cCruncher.CrunchString( sz, szDest, 6 );
						size_t iPos = ::strlen(szDest);
						iPos = min( iPos, cb-2 );
						szDest[iPos] = '*';
					}
					else
						cCruncher.CrunchString( sz, szDest, 7 );
					szDest[7] = '\0';

					showText( szDest, i + 4, 0 );	// +4 because 0-3 are Act strips
				}
			}
			else if ( SDM_ChannelBranch == m_eDisplayMode )
			{
				// show the first strip
				CStringCruncher cCruncher;
				sz[0] = '\0';
				cb = 127;
				m_pSonarMixer->GetMixStripName( m_selectedStrip.stripType, m_selectedStrip.stripIndex, sz, &cb );
				cCruncher.CrunchString( sz, szDest, 7 );
				szDest[7] = '\0';
				showText( szDest, 4, 0 );	// +4 because 0-3 are Act strips

				// show the rest
				for ( DWORD i = 1; i < 8; i++ )
				{
					sz[0] = '\0';
					cb = 127;
					m_pSonarMixer->GetMixParamLabel( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_SEND_VOL, i - 1, sz, &cb );
					cCruncher.CrunchString( sz, szDest, 7 );
					szDest[7] = '\0';
					showText( szDest, i + 4, 0 );	// +4 because 0-3 are Act strips
				}
			}
		}

		// Update according to our last type setting
		if (m_eDisplayMode != SDM_ChannelBranch)
		{
			DWORD dwIndex = (m_selectedStrip.stripType == MIX_STRIP_TRACK) ? GetSelectedTrack() : GetSelectedBus();
			DWORD dwStrip = dwIndex - getBankOffset( m_selectedStrip.stripType ); // get hardware strip number (if in range)
			for (int ix = 0; ix < 8; ++ix)
			{
				ControlId id = (ControlId)(ix + BID_Sel0);
				ControlMessageMapIterator it = m_mapControlMsgs.find( id );
				if ( it != m_mapControlMsgs.end() )
				{
					CMidiMsg* pmsg = it->second;
					pmsg->Send( (dwStrip == ix) ? 1.f : 0.f );
				}
			}
			
			if (dwIndex != m_selectedStrip.stripIndex)
			{					
				m_selectedStrip = MapStripInfo( dwIndex ); // populate with current data
			}
		}

		// Todo:  Update any text displays or other LEDs for states as needed.  Remember
		// not all states need to be updated on every refresh.  Be kind to the host and don't
		// ask for everything every time.  Use the m_dwRefreshCount member to thin out as
		// appropriate.
	}

	// Time display
	MFX_TIME time;
	time.timeFormat = m_mfxfPrimary;
	SurfaceTransportTimeUtils::ZeroTime( time );

	if ( SUCCEEDED( m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &time ) ) )
   {
      // project loaded
      if ( m_bProjectClosed )
      {
			onProjectOpen();
         m_bProjectClosed = false;
      }
   }
   else
   {
      // no project loaded
      if ( !m_bProjectClosed )
      {
         m_bProjectClosed = true;
         onProjectClose();
      }
   }

	outputTime( time );

	// Meters
	refreshMeters();

	return S_OK;
}


HRESULT CTacomaSurface::refreshIoMode( PMBINDING &pmb, CMidiMsg *pmsgIn, PSEncoderParams pEnc )
{
	const DWORD dwId = pmsgIn->GetId();

	if ( BID_IN( dwId, BID_Encoder0, BID_Encoder7 ) )
	{
		// special for encoders when in IO mode
		const DWORD dwChan = dwId - BID_Encoder0;
		char sz[32];
		memset( sz, 0, sizeof ( sz ) );

		// lower line - value of unflipped fader
		float fVal = m_pIOBoxInterface->GetParam( dwChan, TIOP_Gain );
		CString str = m_pIOBoxInterface->GetParamValueText( TIOP_Gain, dwChan, fVal );
		int nBytes = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, (LPCTSTR) str, str.GetLength(), sz, 32, NULL, NULL );
		showText( sz, dwChan + 4, 1, 8 );

		// upper line - value of the unflipped rotaries
		fVal = m_pIOBoxInterface->GetParam( dwChan, pEnc->ioParam );
		str = m_pIOBoxInterface->GetParamValueText( pEnc->ioParam, dwChan, fVal );
		memset( sz, 0, sizeof ( sz ) );
		nBytes = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, (LPCTSTR) str, str.GetLength(), sz, 32, NULL, NULL );
		showText( sz, dwChan + 4, 0 );
	}
	else if ( BID_IN( dwId, BID_Fader0, BID_Fader7 ) )
	{
		const DWORD dwChan = dwId - BID_Fader0;
		const float fVal = m_pIOBoxInterface->GetParam( dwChan, m_bFlipped ? pEnc->ioParam : TIOP_Gain );
		pmsgIn->SetVal( fVal );
		if ( !pmb.pParam->IsTouched() )
			pmb.pMsgOut->Send( fVal );
	}
	else
	{
		const DWORD dwKey[3] = { BID_Mute0, BID_Solo0, BID_Arm0 };
		const DWORD dwCmd[3] = { TIOP_Phase, TIOP_Pad, TIOP_LoCut };
		for ( DWORD dwCntr = 0; dwCntr < 3; ++ dwCntr )
		{
			if ( BID_IN( dwId, dwKey[dwCntr], dwKey[dwCntr] + 7 ) )
			{
				const DWORD dwChan = dwId - dwKey[dwCntr];
				const float fVal = m_pIOBoxInterface->GetParam( dwChan, (TacomaIOBoxParam) dwCmd[dwCntr] );
				pmsgIn->SetVal( fVal );
				ControlMessageMapIterator it = m_mapControlMsgs.find( (ControlId) dwId );
				if ( it != m_mapControlMsgs.end() )
				{
					CMidiMsg* pmsg = it->second;
					pmsg->Send( ( fVal >= 0.5f ) ? 1.f : 0.f );
				}
			}
		}
	}

	return ( S_OK );
}

//---------------------------------------------------------------
HRESULT CTacomaSurface::refreshPlugins( bool bForce /* = false */ )
{
	if ( SDM_ExistingFX != m_eDisplayMode )
		return S_FALSE;

	if ( !bForce && ( (m_dwRefreshCount % 2) != 0 ) )
		return S_FALSE;

	if ( m_tcbParamChange.IsActive() )
		return S_FALSE;

	showExistingPlugins();

	return S_OK;
}


////////////////////////////////////////////////////////////////////
HRESULT	CTacomaSurface::refreshMarkers()
{
	if ( SDM_Markers != m_eDisplayMode )
		return S_FALSE;

	if ( (m_dwRefreshCount % 2) != 0 )
		return S_FALSE;

	if ( m_tcbParamChange.IsActive() )
		return S_FALSE;

	if ( m_tctRevert.IsActive() )
		return S_FALSE;

	if ( !m_pSonarProject2 )
		return S_FALSE;

	DWORD dwCount = 0;
	m_pSonarProject2->GetMarkerCount( &dwCount );

	MFX_TIME time;
	time.timeFormat = TF_SAMPLES;
	DWORD dwCurMkr = 0;
	m_pSonarTransport->GetTransportTime( TRANSPORT_TIME_CURSOR, &time );
	time.llSamples += 10; // because GetMarkerIndexForTime() rounds down...
	m_pSonarProject2->GetMarkerIndexForTime( &time, &dwCurMkr );

	dwCount = min( 12, dwCount );
	char sz[64];
	for ( DWORD iMkr = 0; iMkr < dwCount; iMkr++ )
	{
		DWORD dwLen = sizeof(sz);
		*sz = '\0';
		showText( ( iMkr == dwCurMkr )? "---" : sz, iMkr, 1 );
		m_pSonarProject2->GetMarkerName( iMkr, sz, &dwLen );
		showText( sz, iMkr, 0 );
	}

	// and blank out the rest
	*sz = '\0';
	for ( DWORD iMkr = dwCount; iMkr < 12; iMkr++ )
	{
		showText( sz, iMkr, 0 );
		showText( sz, iMkr, 1 );
	}

	return S_OK;
}


//-------------------------------------------------------------------
HRESULT CTacomaSurface::refreshLayers()
{
	if ( SDM_Layers != m_eDisplayMode )
		return S_OK;

	if ( (m_dwRefreshCount % 2) != 0 )
		return S_FALSE;

	if ( m_tcbParamChange.IsActive() )
		return S_FALSE;

	float fVal = 0.f;
   m_pSonarMixer->GetMixParam( m_selectedStrip.stripType, m_selectedStrip.stripIndex, MIX_PARAM_LAYER_COUNT, 0, &fVal );
	DWORD dwCount = (DWORD)fVal;

	dwCount = min( 8, dwCount );
	char sz[8];

	for ( DWORD iLayer = 0; iLayer < dwCount; iLayer++ )
	{
		::sprintf( sz, "Layer %d", iLayer+1 );
		showText( sz, iLayer + 4, 0 );
		*sz = '\0';
		showText( sz, iLayer + 4, 1 );
	}

	// and blank out the rest
	*sz = '\0';
	for ( DWORD iLayer = dwCount; iLayer < 8; iLayer++ )
	{
		showText( sz, iLayer + 4, 0 );
		showText( sz, iLayer + 4, 1 );
	}

	return S_OK;
}

// [GP]
HRESULT CTacomaSurface::updatePCContextName( bool bForceUpdate )
{
   char szName[32];
   DWORD dwCountof = _countof( szName );
   memset( szName, 0, dwCountof );

	switch ( m_eActSectionMode )
	{
	case KSM_EQ:
		sprintf(szName, "Old EQ");
		break;
	case KSM_PC_COMP:
		sprintf(szName, "ProChannel Comp");
		break;
	case KSM_PC_EQ:
		sprintf(szName, "ProChannel EQ");
		break;
	case KSM_PC_EQ_P2:
		sprintf(szName, "ProChannel EQ2");
		break;
	case KSM_PC_SAT:
		sprintf(szName, "ProChannel Sat");
		break;
	}

    memcpy( m_szACTContext, szName, dwCountof );
	setTimer( m_tcbParamChange, 1000 );

	showText( m_szACTContext, 0, 0, 32 );

	return ( 0 );
}

HRESULT CTacomaSurface::updateActContextName( bool bForceUpdate )
{
   // update the name of the plugin/synth that we are on
   char szName[32];
   DWORD dwCountof = _countof( szName );
   memset( szName, 0, dwCountof );
   m_pSonarParamMapping->GetMapName( m_dwSurfaceID, (LPSTR) &szName, &dwCountof );
	BOOL bDiff = strcmp( m_szACTContext, szName ) != 0;
	bool bChange = false;
	DWORD dwCurIndex = 0;
	SONAR_MIXER_STRIP type = GetStripFocus();

	// Check that the selection has not changed (prochannel has the same name on every strip)
	if (type != m_PCStrip.stripType)
	{
		bDiff = TRUE;
		bChange = true;
		m_PCStrip.stripType = type;
	}

	dwCurIndex = (type == MIX_STRIP_TRACK) ? GetSelectedTrack() : GetSelectedBus();
	if (dwCurIndex  != m_PCStrip.stripIndex)
	{
		bDiff = TRUE;
		bChange = true;
		m_PCStrip.stripIndex = dwCurIndex;
	}

   if ( bForceUpdate || bDiff )
   {
		if ( m_pSonarUIContext )
		{
			SONAR_UI_CONTAINER eContainer;
			m_pSonarUIContext->GetUIContextEx( m_dwSurfaceID, &eContainer );
			if (ProChanLocked() && (eContainer != CNR_PROCHANNEL || bChange))
			{
				m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_PROCHANNEL );
				if (bChange)
					m_pSonarUIContext->SetUIContext( m_PCStrip.stripType, m_PCStrip.stripIndex, MIX_PARAM_FILTER_PARAM, -1, UIA_FOCUS );

				updateActSectionBindings( true );
			}
			else if (m_eActSectionMode == KSM_ACT && eContainer != CNR_USER_BIN)
			{
				m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_USER_BIN );
				updateActSectionBindings( true );
			}
		}

		// ACT context has changed
		if ( bDiff )
		{
			if ( m_wACTPage != 0 )
			{
				m_wACTPage = 0;
				updateActSectionBindings( false );
			}

			resetChannelStrip();
		}

		const bool bLocked = GetLockDynamicMappings();

      memcpy( m_szACTContext, szName, dwCountof );
		setTimer( m_tcbParamChange, bLocked ? 1250 : 1000 );
      showText( m_szACTContext, 0, 0, 32 );

		char sz[8];
		memset( sz, 0, sizeof ( sz ) );
		::sprintf( sz, "Page %d", m_wACTPage + 1 );
		showText( sz, 0, 1, 16 );

		if ( bLocked )
			showText( "     *ACT Locked", 2, 1, 16 );
		else
			showText( "", 2, 1, 16 );
   }

   return S_OK;
}

void CTacomaSurface::onProjectClose( void )
{
	// clear the LCD screens
	showText( "", 0, 0, 32 );
	showText( "", 0, 1, 32 );

	showText( "", 4, 0, 32 );
	showText( "", 4, 1, 32 );

	showText( "", 8, 0, 40 );
	showText( "", 8, 1, 40 );

	m_bSelectingOrEditing = false;
}

void CTacomaSurface::onProjectOpen( void )
{
	m_eDisplayMode = SDM_Normal;
	m_eActSectionMode = KSM_FLEXIBLE_PC; // default to Eq button
	SetProChanLock();
	m_eJogMode = JM_Standard;
   m_selectedStrip.clear();
	m_prevSelectedStrip.clear();
	m_PCStrip.clear();

	Set8StripType( MIX_STRIP_TRACK );
	m_selectedStrip = MapStripInfo( GetSelectedTrack() ); // populate with current data

	if (m_pSonarUIContext) // default to Eq button
		m_pSonarUIContext->SetUIContextEx( m_dwSurfaceID, CNR_PROCHANNEL );

	updateParamStateOnLCD();

}

CString CTacomaSurface::getIoParamString( TacomaIOBoxParam ioParam, BOOL bShort )
{
	CString str;

	switch ( ioParam )
	{
		case TIOP_Phantom:
			str = bShort ? _T("Phtm") : _T("Phantom");
		break;
		case TIOP_Phase:
			str = bShort ? _T("Ph") : _T("Phase");
		break;
		case TIOP_Pad:
			str = _T("Pad");
		break;
		case TIOP_Hiz:
			str = _T("Hi-Z");
		break;
		case TIOP_Gain:
			str = bShort ? _T("G") : _T("Mic Gain");
		break;
		case TIOP_StereoLink:
			str = bShort ? _T("Lnk") : _T("Link");
		break;
		case TIOP_StereoLinkOC:
			str = bShort ? _T("OC Lnk") : _T("OC Link");
		break;
		case TIOP_MasterLinkOC:
			str = bShort ? _T("OC M Lnk") : _T("OC Master Link");
		break;
		case TIOP_LoCut:
			str = bShort ? _T("Lo") : _T("Low Cut");
		break;
		case TIOP_CompEnable:
			str = bShort ? _T("Comp") : _T("Compress");
		break;
		case TIOP_DMixMono:
			str = _T("Mono");
		break;
		case TIOP_DMixMute:
			str = _T("Mute");
		break;
		case TIOP_DMixSolo:
			str = _T("Solo");
		break;
		case TIOP_DMixPan:
			str = bShort ? _T("Pn") : _T("Pan");
		break;
		case TIOP_DMixVol:
			str = bShort ? _T("V") : _T("Volume");
		break;

		case TIOP_DMixSend:
			str = bShort ? _T("Snd") : _T("Send");
		break;

		// comp params (keep together)
		case TIOP_Gate:
			str = bShort ? _T("Gt") : _T("Gate");
		case TIOP_Threshold:
			str = bShort ? _T("Th") : _T("Threshld");
		break;
		case TIOP_Attack:
			str = bShort ? _T("At") : _T("Attack");
		break;
		case TIOP_Release:
			str = bShort ? _T("Rl") : _T("Release");
		break;
		case TIOP_Ratio:
			str = bShort ? _T("Rt") : _T("Ratio");
		break;
		case TIOP_MakeupGain:
			str = bShort ? _T("MG") : _T("CompGain");
		break;

		// system params (keep together)
		case TIOP_UnitNumber:
		break;
		case TIOP_SampleRate:
		break;
		case TIOP_SyncSource:     // Internal: Dig1: Dig2: WordClock
		break;
		case TIOP_DigitalInput:   // coax: AES/EBU
		break;
		case TIOP_DirectMix:   // coax: AES/EBU
		break;

		// direct mixer output
		case TIOP_DMOutMain:
		break;
		case TIOP_DMOutSub:
		break;
		case TIOP_DMOutDig:
		break;
		case TIOP_DMixReturn:
		break;
		case TIOP_RevTime:
		break;
		case TIOP_RevType:
		break;
		case TIOP_RevDelay:
		break;

		default:
			ASSERT( 0 ); // shouldn't happen
	}

	return ( str );
}
