// ACTController.cpp : Defines the basic functionality of the "ACTController".
//

#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

#include "ACTController.h"
#include "ACTControllerPropPage.h"
#include "ControlSurface_i.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CACTController
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CACTController::CACTController() :
	m_cRef( 1 ),
	m_bDirty( FALSE ),
	m_pMidiOut( NULL ),
	m_pKeyboard( NULL ),
	m_pCommands( NULL ),
	m_pProject( NULL ),
	m_pMixer( NULL ),
	m_pTransport( NULL ),
	m_pSonarIdentity( NULL ),
	m_pSonarParamMapping( NULL ),
	m_pSonarMixer2( NULL ),
	m_pSonarUIContext( NULL ),
	m_dwSurfaceId( 0 ),
	m_hwndApp( NULL ),
	m_pMidiLearnTarget(NULL),
	m_bInitSent( false )
{ 
	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );
	::InitializeCriticalSection( &m_dyn_cs );

	int n;

	m_bConnected = false;

	m_eStripType = MIX_STRIP_TRACK;
	for (n = 0; n < NUM_STRIP_TYPES; n++)
	{
		m_dwStripNum[n] = 0;
		m_dwSelectedTrack[n] = 0;
	}
	m_eRotariesMode = MCS_ASSIGNMENT_MUTLI_CHANNEL;
	m_bUseDynamicMappings = false;

	m_dwUpdateCount = 0;

	m_strToolbarText = "ACT MIDI Controller";

	m_dwNumTracks = 0;
	m_dwNumBuses = 0;
	m_dwNumMains = 0;
	m_uiContext = UIC_TRACKVIEW;

	for (n = 0; n < NUM_KNOBS; n++)
		m_strRotaryLabel[n].Format("R%d", n + 1);

	for (n = 0; n < NUM_SLIDERS; n++)
		m_strSliderLabel[n].Format("S%d", n + 1);

	for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		m_strButtonLabel[n].Format("%sB%d", (n >= NUM_BUTTONS) ? "Shift " :  "", (n % NUM_BUTTONS) + 1);

	for ( n = 0; n < NUM_BANKS; n++ )
	{
		m_aCTRotary[n] = CMixParam::CT_Jump;
		m_aCTSlider[n] = CMixParam::CT_Jump;
	}

	m_iRotaryBank = 0;
	m_iSliderBank = 0;
	m_iButtonBank = 0;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_strMidiLearn.LoadString(IDS_MIDI_LEARN_DDD);
	m_strOn.LoadString(IDS_ON);
	m_strOff.LoadString(IDS_OFF);
	m_strMultiChannel.LoadString(IDS_MULTI_CHANNEL);
	m_strChannelStrip.LoadString(IDS_CHANNEL_STRIP);
	m_strStripParameters.LoadString(IDS_STRIP_PARAMETERS);

	RestoreDefaultBindings();
	ResetMidiLearn();
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::releaseSonarInterfaces()
{
	if (m_pMidiOut)
		m_pMidiOut->Release(), m_pMidiOut = NULL;
	if (m_pKeyboard)
		m_pKeyboard->Release(), m_pKeyboard = NULL;
	if (m_pCommands)
		m_pCommands->Release(), m_pCommands = NULL;
	if (m_pProject)
		m_pProject->Release(), m_pProject = NULL;
	if (m_pMixer)
		m_pMixer->Release(), m_pMixer = NULL;
	if (m_pTransport)
		m_pTransport->Release(), m_pTransport = NULL;
	if (m_pSonarIdentity)
		m_pSonarIdentity->Release(), m_pSonarIdentity = NULL;
	if (m_pSonarParamMapping)
		m_pSonarParamMapping->Release(), m_pSonarParamMapping = NULL;
	if (m_pSonarMixer2)
		m_pSonarMixer2->Release(), m_pSonarMixer2 = NULL;
	if ( m_pSonarUIContext )
		m_pSonarUIContext->Release(), m_pSonarUIContext = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CACTController::~CACTController() 
{ 
	releaseSonarInterfaces();
	::DeleteCriticalSection( &m_dyn_cs );
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CACTController::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (NULL == pdwLen)
		return E_POINTER;

	// Return results to caller
	if (NULL == pszStatus)
	{
		*pdwLen = m_strToolbarText.GetLength() + 1;
	}
	else
	{
		::strlcpy(pszStatus, m_strToolbarText, *pdwLen);
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::MidiInShortMsg( DWORD dwShortMsg )
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	// Break up the message
	BYTE bStatus = (BYTE)(dwShortMsg & 0xFF);
	BYTE bD1 = (BYTE)((dwShortMsg >> 8) & 0xFF);
	BYTE bD2 = (BYTE)((dwShortMsg >> 16) & 0xFF);

	OnShortMidiIn(bStatus, bD1, bD2);

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
	if (NULL == pbLongMsg)
		return E_POINTER;
	if (0 == cbLongMsg)
		return S_OK; // nothing to do

	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	int n = 0;
	if (m_pMidiLearnTarget)
	{
		// currently, sysex is only used for toggle functions so only learn
		// for a button
		bool bIsButton = false;
		for (n = 0; n < NUM_BUTTONS; n++)
		{
			if (&m_cMidiButton[n] == m_pMidiLearnTarget)
			{
				bIsButton = true;
				break;
			}
		}

		if ( bIsButton )
		{
			m_pMidiLearnTarget->SetMessage( pbLongMsg, cbLongMsg );

			if (m_pMidiLearnTarget == &m_cMidiModifierDown)
				m_pMidiLearnTarget = &m_cMidiModifierUp;
			else
				m_pMidiLearnTarget = NULL;

			OnContextSwitch();

			return S_OK;
		}
	}


/*
	TRACE( "count:%d:\n", cbLongMsg );
	for ( DWORD ix = 0; ix < cbLongMsg; ix++ )
		TRACE( "%0x,", pbLongMsg[ix] );

	TRACE( "\n-------\n");
*/

	for (n = 0; n < NUM_BUTTONS; n++)
	{
		if (m_cMidiButton[n].IsMatch(pbLongMsg, cbLongMsg))
			OnButton(n, 1);
	}

	if (m_cMidiModifierDown.IsMatch(pbLongMsg, cbLongMsg))
	{
		m_bModifierIsDown = true;
	}

	if (m_cMidiModifierUp.IsMatch(pbLongMsg, cbLongMsg))
	{
		m_bModifierIsDown = false;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

static int s_nRefreshCount = 0;

HRESULT CACTController::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	// no motors on this, so we can do less often
	if ( 0 != (s_nRefreshCount++ % 3) )
		return S_OK;

	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	OnRefreshSurface(fdwRefresh);

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	if (NULL == pwMask || NULL == pbNoEchoSysx)
		return E_POINTER;

	CCriticalSectionAuto csa( &m_cs );

	// TODO: Tell SONAR about any channels (or sysx) that do not require echo.
	
	*pwMask = 0;//m_wNoEchoStatus;
	*pbNoEchoSysx = FALSE;

	return S_OK;
}


// IControlSurface3

//-------------------------------------------------------------------------
/// Return a list of status bytes that this surface is listening for.  the
/// host should not echo or record messages with tese status bytes. 
/// This is an extension of the GetNoEchoMask() found in IControlSurface.  
/// That method only has granularity to the Channel level which is  too limited 
/// in today's Keys+knobs type controllers
HRESULT CACTController::GetNoEchoStatusMessages( WORD** ppwStatus, DWORD* pdwCount )
{
	if ( !pdwCount || !ppwStatus )
		return E_POINTER;
	*pdwCount = NUM_KNOBS + NUM_SLIDERS + NUM_BUTTONS + 2;	// +2 for the shift modifier messages

	*ppwStatus = (WORD*)::CoTaskMemAlloc( (size_t)(*pdwCount * sizeof(WORD)) );
	if ( !*ppwStatus )
		return E_OUTOFMEMORY;

	WORD* pbuf = *ppwStatus;

	// all knobs
	for ( int ix = 0; ix < NUM_KNOBS; ix++ )
	{
		CMidiBinding& mb = m_cMidiKnob[ix];
		*pbuf++ = mb.GetMessage();
	}

	// all sliders
	for ( int ix = 0; ix < NUM_SLIDERS; ix++ )
	{
		CMidiBinding& mb = m_cMidiSlider[ix];
		*pbuf++ = mb.GetMessage();
	}

	// all switches
	for ( int ix = 0; ix < NUM_BUTTONS; ix++ )
	{
		CMidiBinding& mb = m_cMidiButton[ix];
		*pbuf++ = mb.GetMessage();
	}

	// lastly the shift modifiers
	*pbuf++ = m_cMidiModifierDown.GetMessage();
	*pbuf++ = m_cMidiModifierUp.GetMessage();

	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// ISurfaceParamMapping

HRESULT CACTController::GetStripRangeCount( DWORD* pdwCount )
{
	if (!pdwCount)
		return E_POINTER;

	*pdwCount = 1;
	
	return S_OK;
}

HRESULT CACTController::GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip )
{
	if (dwIndex != 0)
		return E_INVALIDARG;

	if (!pdwLowStrip || !pdwHighStrip || !pmixerStrip)
		return E_POINTER;

	*pdwLowStrip = GetStripNum();
	*pdwHighStrip = *pdwLowStrip + NUM_SLIDERS - 1;
	*pmixerStrip = m_eStripType;

	return S_OK;
}

HRESULT CACTController::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
{
	if (dwLowStrip == GetStripNum() && mixerStrip == m_eStripType)
		return S_FALSE;	// no change

	// Change this first, so that...
	m_eStripType = mixerStrip;

	// ...this can check the correct strip type
	LimitAndSetStripNum(dwLowStrip, true);

	return S_OK;
}


HRESULT CACTController::GetDynamicControlCount( DWORD* pdwCount )
{
	CCriticalSectionAuto csa( &m_dyn_cs );

	TRACE("CACTController::GetDynamicControlCount()\n");

	if (!pdwCount)
		return E_POINTER;

	*pdwCount = (DWORD)m_vDynControls.size();

	TRACE("  Count = %u\n", *pdwCount);

	return S_OK;
}

HRESULT CACTController::GetDynamicControlInfo( DWORD dwIndex, DWORD* pdwKey, SURFACE_CONTROL_TYPE* pcontrolType )
{
	CCriticalSectionAuto csa( &m_dyn_cs );

//	TRACE("CACTController::GetDynamicControlInfo(%u)\n", dwIndex);

	if (!pdwKey || !pcontrolType)
		return E_POINTER;

	if (dwIndex >= (DWORD)m_vDynControls.size())
		return E_INVALIDARG;

	DYNCONTROL& dc = m_vDynControls[dwIndex];
	*pdwKey = dc.dwKey;
	*pcontrolType = dc.sctType;

	return S_OK;
}

HRESULT CACTController::SetLearnState( BOOL bLearning )
{
	int m = 0, n = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		for (n = 0; n < NUM_KNOBS; n++)
		{
			m_SwKnob[m][n].SetACTLearning( !!bLearning );
		}
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			m_SwSlider[m][n].SetACTLearning( !!bLearning );
		}
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			m_SwButton[m][n].SetACTLearning( !!bLearning );
		}
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_ACTController
// and put that in your ACTController.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CACTController::Save( IStream* pStm, BOOL bClearDirty )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should write any data to pStm which you wish to store.
	// Typically you will write values for all of your properties.
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.

	// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
	// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.

	HRESULT hr = Persist(pStm, true);

	if (FAILED(hr))
		return hr;

	if (bClearDirty)
		m_bDirty = FALSE;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Load( IStream* pStm )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should read the data using the format you used in Save().
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.
	
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
	// E_FAIL. Don't simply return whatever IStream::Read() returned.

	HRESULT hr = Persist(pStm, false);

	UpdateButtonActionStrings();
	UpdateBindings();
	onCaptureModeChange();

	// Optional?   Send init messages?
	// SendInitMessages();

	if (FAILED(hr))
		return hr;

	m_bDirty = FALSE;

	return hr;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( &m_cs );

	TRACE("CACTController::GetSizeMax()\n");

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 4096;
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CACTController::OnConnect()
{
	TRACE("CACTController::OnConnect()\n");

	int m, n;

	for (m = 0; m < NUM_BANKS; m++)
	{
		for (n = 0; n < NUM_KNOBS; n++)
		{
			m_SwKnob[m][n].SetInterfaces(m_pMixer, m_pSonarMixer2, m_pTransport, m_dwSurfaceId);
			m_SwKnob[m][n].SetCaptureType( CMixParam::CT_Match );
		}
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			m_SwSlider[m][n].SetInterfaces(m_pMixer, m_pSonarMixer2, m_pTransport, m_dwSurfaceId);
			m_SwSlider[m][n].SetCaptureType( CMixParam::CT_Match );
		}
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			m_SwButton[m][n].SetInterfaces(m_pMixer, m_pSonarMixer2, m_pTransport, m_dwSurfaceId);
			m_SwButton[m][n].SetCaptureType( CMixParam::CT_Jump );	// switches should be absolute
		}
	}

	m_cParamUtil.SetInterfaces(m_pMixer, m_pSonarMixer2, m_pTransport, m_dwSurfaceId);

	m_vKnobBindings.clear();
	m_vSliderBindings.clear();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	AddItem(&m_vKnobBindings,	IDS_VOL,	MAKELONG(MIX_PARAM_VOL, 0));
	AddItem(&m_vKnobBindings,	IDS_PAN,	MAKELONG(MIX_PARAM_PAN, 0));

	AddItem(&m_vSliderBindings, IDS_VOL,	MAKELONG(MIX_PARAM_VOL, 0));
	AddItem(&m_vSliderBindings, IDS_PAN,	MAKELONG(MIX_PARAM_PAN, 0));

	CString fmt_vol(MAKEINTRESOURCE(IDS_SEND_N_VOL));
	CString fmt_pan(MAKEINTRESOURCE(IDS_SEND_N_PAN));

	for (n = 0; n < 8; n++)
	{
		CString vol, pan;

		vol.Format(fmt_vol, n + 1);
		pan.Format(fmt_pan, n + 1);

		AddItem(&m_vKnobBindings,	(LPCTSTR)vol,	MAKELONG(MIX_PARAM_SEND_VOL, n));
		AddItem(&m_vKnobBindings,	(LPCTSTR)pan,	MAKELONG(MIX_PARAM_SEND_PAN, n));

		AddItem(&m_vSliderBindings, (LPCTSTR)vol,	MAKELONG(MIX_PARAM_SEND_VOL, n));
		AddItem(&m_vSliderBindings, (LPCTSTR)pan,	MAKELONG(MIX_PARAM_SEND_PAN, n));
	}

	AddItem(&m_vKnobBindings,	IDS_INPUT_TRIM,	MAKELONG(MIX_PARAM_VOL_TRIM, 0));

	AddItem(&m_vSliderBindings, IDS_INPUT_TRIM,	MAKELONG(MIX_PARAM_VOL_TRIM, 0));

	m_vButtonActions.clear();

	AddItem(&m_vButtonActions, IDS_CMD_NONE,							CMD_NONE);
	AddItem(&m_vButtonActions, IDS_CMD_ACT_ENABLE,						CMD_ACT_ENABLE);
	AddItem(&m_vButtonActions, IDS_CMD_ACT_LOCK,						CMD_ACT_LOCK);
	AddItem(&m_vButtonActions, IDS_CMD_ACT_LEARN,						CMD_ACT_LEARN);
	AddItem(&m_vButtonActions, IDS_CMD_ROTARIES_MODE,					CMD_ROTARIES_MODE);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_TRACK,						CMD_PREV_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_TRACK,						CMD_NEXT_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_TRACK_BANK,					CMD_PREV_TRACK_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_TRACK_BANK,					CMD_NEXT_TRACK_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_STRIP_TYPE,					CMD_PREV_STRIP_TYPE);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_STRIP_TYPE,					CMD_NEXT_STRIP_TYPE);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_SEL_TRACK,					CMD_PREV_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_SEL_TRACK,					CMD_NEXT_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_MUTE_SEL_TRACK,					CMD_MUTE_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_SOLO_SEL_TRACK,					CMD_SOLO_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_REC_ARM_SEL_TRACK,				CMD_REC_ARM_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_AUTO_READ_SEL_TRACK,				CMD_AUTO_READ_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_AUTO_WRITE_SEL_TRACK,			CMD_AUTO_WRITE_SEL_TRACK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_ROTARIES_BANK,				CMD_PREV_ROTARIES_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_ROTARIES_BANK,				CMD_NEXT_ROTARIES_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_SLIDERS_BANK,				CMD_PREV_SLIDERS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_SLIDERS_BANK,				CMD_NEXT_SLIDERS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_BUTTONS_BANK,				CMD_PREV_BUTTONS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_BUTTONS_BANK,				CMD_NEXT_BUTTONS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_ROTARIES_AND_SLIDERS_BANK,	CMD_PREV_ROTARIES_AND_SLIDERS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_ROTARIES_AND_SLIDERS_BANK,	CMD_NEXT_ROTARIES_AND_SLIDERS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_PREV_CONTROLLERS_BANK,			CMD_PREV_CONTROLLERS_BANK);
	AddItem(&m_vButtonActions, IDS_CMD_NEXT_CONTROLLERS_BANK,			CMD_NEXT_CONTROLLERS_BANK);

	AddItem(&m_vButtonActions, IDS_CMD_PROPS_TOGGLE,		CMD_SURFACE_PROPS_TOGGLE);
	AddItem(&m_vButtonActions, IDS_CMD_PROPS_SHOW,			CMD_SURFACE_PROPS_SHOW);
	AddItem(&m_vButtonActions, IDS_CMD_PROPS_HIDE,			CMD_SURFACE_PROPS_HIDE);
	AddItem(&m_vButtonActions, IDS_CMD_OPEN_CUR_FX,			CMD_OPEN_CUR_FX);
	AddItem(&m_vButtonActions, IDS_CMD_CLOSE_CUR_FX,		CMD_CLOSE_CUR_FX);

	AddItem(&m_vButtonActions, IDS_CMD_FOCUS_NEXT_FX, CMD_FOCUS_NEXT_FX );
	AddItem(&m_vButtonActions, IDS_CMD_FOCUS_PREV_FX, CMD_FOCUS_PREV_FX );
	AddItem(&m_vButtonActions, IDS_CMD_OPEN_NEXT_FX, CMD_OPEN_NEXT_FX );
	AddItem(&m_vButtonActions, IDS_CMD_OPEN_PREV_FX, CMD_OPEN_PREV_FX );

	DWORD dwCount;
	if (SUCCEEDED(m_pCommands->GetCommandCount(&dwCount)))
	{
		for (DWORD n = 0; n < dwCount; n++)
		{
			DWORD dwCmdId;
			DWORD dwSize;

			if (FAILED(m_pCommands->GetCommandInfo(n, &dwCmdId, NULL, &dwSize)))
				continue;

			LPSTR pszName = new char[dwSize];

			if (SUCCEEDED(m_pCommands->GetCommandInfo(n, &dwCmdId, pszName, &dwSize)))
			{
				AddItem(&m_vButtonActions, pszName, dwCmdId);
			}
		}
	}

	m_vBankNames.clear();

	CString fmt(MAKEINTRESOURCE(IDS_BANK_N));

	for (n = 0; n < NUM_BANKS; n++)
	{
		CString str;

		str.Format(fmt, n + 1);

		AddItem(&m_vBankNames, (LPCTSTR)str, n);
	}

	UpdateButtonActionStrings();
	UpdateBindings();
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::OnDisconnect()
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CACTController::OnContextSwitch()
{
	if (m_pSonarParamMapping)
		m_pSonarParamMapping->OnContextSwitch(m_dwSurfaceId);

	// Reset the value history of all continuously variable controls.
	int m = 0, n = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		for (n = 0; n < NUM_KNOBS; n++)
		{
			m_SwKnob[m][n].ResetHistory();
		}
		for (n = 0; n < NUM_SLIDERS; n++)
		{
			m_SwSlider[m][n].ResetHistory();
		}
	}

}

/////////////////////////////////////////////////////////////////////////////

SONAR_UI_CONTEXT CACTController::GetCurrentContext()
{
	if (m_pSonarParamMapping)
	{
		SONAR_UI_CONTEXT uiContext = UIC_TRACKVIEW;

		if (SUCCEEDED(m_pSonarParamMapping->GetCurrentContext(m_dwSurfaceId, &uiContext)))
			return uiContext;
	}

	return UIC_TRACKVIEW;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::DoCommand(DWORD dwCmdID)
{
	if (m_pCommands)
	{
		TRACE("CACTController::DoCommand(0x%08X)\n", dwCmdID);
		m_pCommands->DoCommand(dwCmdID);
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetStripCount(SONAR_MIXER_STRIP eMixerStrip)
{
	if (!m_pMixer)
		return 0;

	DWORD dwCount;

	HRESULT hr = m_pMixer->GetMixStripCount(eMixerStrip, &dwCount);

	// While loading a new project, dwCount is -1
	if (FAILED(hr) || 0xFFFFFFFF == dwCount)
		return 0;

	return dwCount;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::BindNthParam(CMixParam *pParam, SONAR_MIXER_STRIP eMixerStrip,
									DWORD dwStripNum, int n)
{
	SONAR_MIXER_PARAM eMixerParam = MIX_PARAM_VOL;
	DWORD dwParamNum = 0;

	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:
			if (IsMIDI(eMixerStrip, dwStripNum))
			{
				switch (n)
				{
					case 0: eMixerParam = MIX_PARAM_PAN;		dwParamNum = 0;	break;
					case 1: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 0;	break;	// Chorus
					case 2: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 1;	break;	// Reverb
					default:
						pParam->ClearBinding();
						return;
				}
			}
			else // Audio track
			{
				switch (n)
				{
					case 0: eMixerParam = MIX_PARAM_PAN;		dwParamNum = 0;	break;
					case 1: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 0;	break;
					case 2: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 0;	break;
					case 3: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 1;	break;
					case 4: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 1;	break;
					case 5: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 2;	break;
					case 6: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 2;	break;
					case 7: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 3;	break;
					default:
						pParam->ClearBinding();
						return;
				}
			}
			break;

		case MIX_STRIP_BUS:
			switch (n)
			{
				case 0: eMixerParam = MIX_PARAM_PAN;		dwParamNum = 0;	break;
				case 1: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 0;	break;
				case 2: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 0;	break;
				case 3: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 1;	break;
				case 4: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 1;	break;
				case 5: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 2;	break;
				case 6: eMixerParam = MIX_PARAM_SEND_PAN;	dwParamNum = 2;	break;
				case 7: eMixerParam = MIX_PARAM_SEND_VOL;	dwParamNum = 3;	break;
				default:
					pParam->ClearBinding();
					return;
			}
			break;

		case MIX_STRIP_MASTER:
			pParam->ClearBinding();
			return;

		default:
			pParam->ClearBinding();
			return;
	}

	pParam->SetParams(eMixerStrip, dwStripNum, eMixerParam, dwParamNum);
}

/////////////////////////////////////////////////////////////////////////////

void  CACTController::BindParamToBinding(CMixParam *pParam, SONAR_MIXER_STRIP eMixerStrip,
												DWORD dwStripNum, DWORD dwBinding)
{
	SONAR_MIXER_PARAM eMixerParam;
	DWORD dwParamNum;
	bool clear = false;

	eMixerParam = (SONAR_MIXER_PARAM)LOWORD(dwBinding);
	dwParamNum = HIWORD(dwBinding);

	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:
			if (IsMIDI(eMixerStrip, dwStripNum))
			{
				switch (eMixerParam)
				{
					case MIX_PARAM_VOL:
					case MIX_PARAM_PAN:
						break;

					case MIX_PARAM_SEND_VOL:
						if (dwParamNum > 1) // Reverb
							clear = true;
						break;

					default:
						clear = true;
						break;
				}
			}
			break;

		case MIX_STRIP_BUS:
			if (eMixerParam == MIX_PARAM_VOL_TRIM)
				clear = true;
			break;

		case MIX_STRIP_MASTER:
			if (eMixerParam != MIX_PARAM_VOL)
				clear = true;
			break;
	}

	if (clear)
		pParam->ClearBinding();
	else
		pParam->SetParams(eMixerStrip, dwStripNum, eMixerParam, dwParamNum);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::BindToACT(CMixParam *pParam, DWORD dwStripNum)
{
	pParam->SetParams(MIX_STRIP_ANY, dwStripNum, MIX_PARAM_DYN_MAP, m_dwSurfaceId);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::BuildDynControlsList()
{
	CCriticalSectionAuto csa( &m_dyn_cs );

	int m, n, id;

	m_vDynControls.clear();

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		if (!m_bExcludeRotariesACT[m])
		{
			for (n = 0; n < NUM_KNOBS; n++)
				m_vDynControls.push_back(DYNCONTROL(MASK_KNOB(id++), SCT_ROTARY));
		}
	}

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		if (!m_bExcludeSlidersACT[m])
		{
			for (n = 0; n < NUM_SLIDERS; n++)
				m_vDynControls.push_back(DYNCONTROL(MASK_SLIDER(id++), SCT_SLIDER));
		}
	}

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			if (!m_bButtonExcludeACT[m][n])
				m_vDynControls.push_back(DYNCONTROL(MASK_BUTTON(id++), SCT_SWITCH));
		}
	}

	TRACE("BuildDynControlsList() size = %d\n", m_vDynControls.size());
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::AddItem(vectorDwordCStringPairs *vPair, UINT nID, DWORD num)
{
	CDwordCStringPair data;

	if (data.m_Str.LoadString(nID) != 0)
	{
		data.m_Num = num;

		vPair->push_back(data);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::AddItem(vectorDwordCStringPairs *vPair, const char *str, DWORD num)
{
	CDwordCStringPair data;

	data.m_Str = str;
	data.m_Num = num;

	vPair->push_back(data);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ShiftStripNum(int iShift, bool bForce)
{
	LimitAndSetStripNum((int)GetStripNum() + iShift, bForce);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::LimitAndSetStripNum(int iStripNum, bool bForce )
{
	int iNumStrips = GetStripCount(m_eStripType);
	int iLastFader = NUM_SLIDERS - 1;

	if (iStripNum + iLastFader >= iNumStrips)
		iStripNum = iNumStrips - iLastFader - 1;

	if (iStripNum < 0)
		iStripNum = 0;

	if (bForce || (int)GetStripNum() != iStripNum)
	{
		SetStripNum((DWORD)iStripNum);
		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetStripNum()
{
	if (m_eStripType >= MIX_STRIP_ANY)
		return 0;

	return m_dwStripNum[m_eStripType];
}

/////////////////////////////////////////////////////////////////////////////

void  CACTController::SetStripNum(DWORD dwStripNum)
{
	if (m_eStripType >= MIX_STRIP_ANY)
		return;

	m_dwStripNum[m_eStripType] = dwStripNum;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ShiftStripType(int iShift)
{
	if (iShift != 0)
	{
		switch (m_eStripType)
		{
			case MIX_STRIP_TRACK:
				m_eStripType = (iShift > 0) ? MIX_STRIP_BUS : MIX_STRIP_MASTER;
				break;

			case MIX_STRIP_BUS:
				m_eStripType = (iShift > 0) ? MIX_STRIP_MASTER : MIX_STRIP_TRACK;
				break;

			case MIX_STRIP_MASTER:
				m_eStripType = (iShift > 0) ? MIX_STRIP_TRACK : MIX_STRIP_BUS;
				break;

			default:
				return;
		}
	}

	ShiftStripNum(0, true);
	ShiftSelectedTrack(0, true);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ShiftSelectedTrack(int iShift, bool bForce)
{
	LimitAndSetSelectedTrack((int)GetSelectedTrack() + iShift, bForce);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::LimitAndSetSelectedTrack(int iSelectedTrack, bool bForce)
{
	int iNumStrips = GetStripCount(m_eStripType);

	if (iSelectedTrack >= iNumStrips)
		iSelectedTrack = iNumStrips - 1;

	if (iSelectedTrack < 0)
		iSelectedTrack = 0;

	if (bForce || (int)GetSelectedTrack() != iSelectedTrack)
	{
		SetSelectedTrack(iSelectedTrack);
		UpdateBindings();
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetSelectedTrack()
{
	if (m_eStripType >= MIX_STRIP_ANY)
		return 0;

	return m_dwSelectedTrack[m_eStripType];
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::SetSelectedTrack(DWORD dwStripNum)
{
	if (m_eStripType >= MIX_STRIP_ANY)
		return;

	m_dwSelectedTrack[m_eStripType] = dwStripNum;

	if (m_bSelectHighlightsTrack && m_eStripType == MIX_STRIP_TRACK)
	{
		m_pMixer->SetMixParam(MIX_STRIP_TRACK, 0, MIX_PARAM_SELECTED, 0,
							(float)dwStripNum, MIX_TOUCH_NORMAL);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ToggleSelectedTrackParam(SONAR_MIXER_PARAM eMixerParam)
{
	CMixParam cParam;

	cParam.SetInterfaces(m_pMixer, m_pSonarMixer2, m_pTransport, m_dwSurfaceId);
	cParam.SetParams(m_eStripType, m_dwSelectedTrack[m_eStripType], eMixerParam, 0);
	cParam.ToggleBooleanParam();
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetSelectedTrackParam(SONAR_MIXER_PARAM eMixerParam)
{
	float fVal;

	m_cParamUtil.SetParams(m_eStripType, m_dwSelectedTrack[m_eStripType], eMixerParam, 0);
	HRESULT hr = m_cParamUtil.GetVal(&fVal);

	if (FAILED(hr))
		return false;

	return (fVal >= 0.5f) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ToggleSelectedTrackAutomationRead()
{
	if (!m_pSonarMixer2)
		return;

	BOOL bRead;

	HRESULT hr = m_pSonarMixer2->GetReadMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
													MIX_PARAM_ANY, 0, &bRead);

	if (FAILED(hr))
		return;

	m_pSonarMixer2->SetReadMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
									MIX_PARAM_ANY, 0, !bRead);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetSelectedTrackAutomationRead()
{
	if (!m_pSonarMixer2)
		return false;

	BOOL bRead;

	HRESULT hr = m_pSonarMixer2->GetReadMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
													MIX_PARAM_ANY, 0, &bRead);

	if (FAILED(hr))
		return false;

	return bRead ? true : false;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::ToggleSelectedTrackAutomationWrite()
{
	BOOL bArm;

	m_pMixer->GetArmMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
									MIX_PARAM_ANY, 0,
									&bArm);


	m_pMixer->SetArmMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
									MIX_PARAM_ANY, 0,
									bArm);
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetSelectedTrackAutomationWrite()
{
	BOOL bArm;

	m_pMixer->GetArmMixParam(m_eStripType, m_dwSelectedTrack[m_eStripType],
									MIX_PARAM_ANY, 0,
									&bArm);

	return !!bArm;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::GetStripNameAndParamLabel(CMixParam *pParam, CString *strText)
{
	strText->Empty();

	if (pParam == NULL)
		return;

	CString strLabel;
	DWORD dwLen = 256;

	HRESULT hr = pParam->GetParamLabel(strLabel.GetBuffer(dwLen), &dwLen);
	strLabel.ReleaseBuffer();

	if (SUCCEEDED(hr))
	{
		if (pParam->GetMixerStrip() != MIX_STRIP_ANY) // Not ACT
		{
			char cType;

			switch (pParam->GetMixerStrip())
			{
				case MIX_STRIP_TRACK:		cType = 'T'; break;
				case MIX_STRIP_AUX:
				case MIX_STRIP_BUS:			cType = 'B'; break;
				case MIX_STRIP_MAIN:
				case MIX_STRIP_MASTER:		cType = 'M'; break;
				default:					cType = 'E'; break; // Error
			}

			strText->Format("%c%d ", cType, pParam->GetStripNum() + 1);
		}

		*strText += strLabel;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::UpdateButtonActionStrings()
{
	for (int m = 0; m < NUM_BANKS; m++)
		for (int n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
			UpdateButtonActionString(m, (VirtualButton)n);
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::UpdateButtonActionString(int iBank, VirtualButton bButton)
{
	m_strButtonAction[iBank][bButton].Empty();

	DWORD dwAction = m_dwButtonAction[iBank][bButton];

	vectorDwordCStringPairs::iterator i;

	for (i = m_vButtonActions.begin(); i != m_vButtonActions.end(); ++i)
	{
		if (i->m_Num == dwAction)
		{
			m_strButtonAction[iBank][bButton] = i->m_Str;
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

DWORD CACTController::GetParamSelected()
{
	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(MIX_STRIP_TRACK, 0,
										MIX_PARAM_SELECTED, 0,
										&fVal);

	if (FAILED(hr))
		return 0;

	return (DWORD)fVal;
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::IsMIDI(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum)
{
	bool bIsMIDI = false;

	float fVal;

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum,
										MIX_PARAM_IS_MIDI, 0,
										&fVal);

	if (SUCCEEDED(hr))
		bIsMIDI = (fVal >= 0.5f);

	return bIsMIDI;
}

/////////////////////////////////////////////////////////////////////////////

bool CACTController::GetTransportState(SONAR_TRANSPORT_STATE eState)
{
	BOOL bVal;

	HRESULT hr = m_pTransport->GetTransportState(eState, &bVal);

	if (FAILED(hr))
		return false;

	return bVal ? true : false;
}

void CACTController::SetRotaryCaptureType(int iBank, CMixParam::CaptureType ct)
{
	m_aCTRotary[iBank] = ct;
	onCaptureModeChange();
}

void CACTController::SetSliderCaptureType(int iBank, CMixParam::CaptureType ct)
{ 

	m_aCTSlider[iBank] = ct;
	onCaptureModeChange();
}

void CACTController::onCaptureModeChange()
{
	if ( (unsigned int) m_iSliderBank < NUM_BANKS )
	{
		for ( int ixs = 0; ixs < NUM_SLIDERS; ixs++ )
		{
			CMixParam& mp = m_SwSlider[m_iSliderBank][ixs];
			mp.SetCaptureType( m_aCTSlider[m_iSliderBank] );
		}
	}
	if ( (unsigned int) m_iRotaryBank < NUM_BANKS )
	{
		for ( int ixk = 0; ixk < NUM_KNOBS; ixk++ )
		{
			CMixParam& mp = m_SwKnob[m_iRotaryBank][ixk];
			mp.SetCaptureType( m_aCTRotary[m_iRotaryBank] );
		}
	}
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void FillComboBox(CComboBox *pBox, vectorDwordCStringPairs *pData)
{
	if (NULL == pBox || NULL == pData)
		return;

	size_t iNumButtons = pData->size();

	pBox->SetRedraw(FALSE);
	pBox->ResetContent();

	vectorDwordCStringPairs::iterator i;

	for (i = pData->begin(); i != pData->end(); ++i)
	{
#ifdef _DEBUG
		CString str;

		str.Format("%s (0x%08X)", i->m_Str, i->m_Num);
		int idx = pBox->AddString(str);
#else
		int idx = pBox->AddString(i->m_Str);
#endif
		pBox->SetItemData(idx, i->m_Num);
	}

	pBox->SetRedraw(TRUE);
	pBox->RedrawWindow();
}

////////////////////////////////////////////////////////////////////////////////

void SelectByItemData(CComboBox *pBox, DWORD dwData)
{
	if (NULL == pBox || NULL == pBox->m_hWnd)
		return;

	pBox->Clear();

	int iNumItems = pBox->GetCount();

	for (int n = 0; n < iNumItems; n++)
	{
		if (pBox->GetItemData(n) == dwData)
		{
			pBox->SetCurSel(n);
			break;
		}
	}
}

//-----------------------------------------------------------
// Broadcast 
void CACTController::SendInitMessages()
{
	if ( !m_pMidiOut )
		return;

	// send short messages
	for ( size_t ix = 0; ix < m_vdwInitShortMsg.size(); ix++ )
	{
		if ( S_OK != m_pMidiOut->MidiOutShortMsg( this, m_vdwInitShortMsg[ix] ) )
		{
			m_bInitSent = false;
			return;
		}
	}

	// send sysex string

	if ( !m_vdwInitSysexMsg.empty() )
	{
		if ( S_OK != m_pMidiOut->MidiOutLongMsg( this, (DWORD)m_vdwInitSysexMsg.size(), &(m_vdwInitSysexMsg[0]) ) )
			m_bInitSent = false;
	}

	m_bInitSent = true;
}



void CACTController::ActivateProps( BOOL b )
{
	if ( m_pSonarUIContext )
		m_pSonarUIContext->ActivateSurfaceProperties( m_dwSurfaceId, b );
}

void CACTController::ToggleProps()
{
	if ( m_pSonarUIContext )
		m_pSonarUIContext->ToggleSurfaceProperties( m_dwSurfaceId );
}

void CACTController::ActivateCurrentFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	
	SONAR_UI_CONTEXT uic = GetCurrentContext();
	if ( UIC_PLUGIN != uic )
		return;

	// we need a valid widget ID
	if ( m_vDynControls.empty() )
		return;

	DYNCONTROL& dc = m_vDynControls[0];

	m_pSonarUIContext->SetUIContext(
			MIX_STRIP_TRACK, 
			dc.dwKey,
			MIX_PARAM_DYN_MAP,
			m_dwSurfaceId,
			uia );
}


void CACTController::ActivatePrevFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	m_pSonarUIContext->SetPreviousUIContext( m_dwSurfaceId, uia );
}

void CACTController::ActivateNextFx( SONAR_UI_ACTION uia )
{
	if ( !m_pSonarUIContext )
		return;
	m_pSonarUIContext->SetNextUIContext( m_dwSurfaceId, uia );
}


////////////////////////////////////////////////////////////////////////////////
