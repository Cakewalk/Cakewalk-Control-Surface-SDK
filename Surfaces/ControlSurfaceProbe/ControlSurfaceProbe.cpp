// ControlSurfaceProbe.cpp : Defines the basic functionality of the "ControlSurfaceProbe".
//

#include "stdafx.h"
#include "ControlSurfaceProbe.h"
#include "ControlSurfaceProbePropPage.h"
#include "ControlSurface_i.c"

#include "CapabilitiesDialog.h"

#include "strlcpy.h"
#include "strlcat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CControlSurfaceProbe
//
/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CControlSurfaceProbe::CControlSurfaceProbe() :
	m_cRef( 1 ),
	m_bDirty( FALSE ),
	m_pMidiOut( NULL ),
	m_pKeyboard( NULL ),
	m_pCommands( NULL ),
	m_pProject( NULL ),
	m_pProject2( NULL ),
	m_pMixer( NULL ),
	m_pMixer2( NULL ),
	m_pTransport( NULL ),
	m_pIdentity( NULL ),
	m_pVUMeters( NULL ),
	m_pSonarParamMapping( NULL ),
	m_pHostWindow( NULL ),
	m_dwSurfaceID( 0 ),
	m_hwndApp( NULL )
{ 
	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );

	m_eMixerStrip = MIX_STRIP_TRACK;
	m_dwStripNum = 0;
	m_eMixerParam = MIX_PARAM_VOL;
	m_dwParamNum = 0;

	m_fLastValue = 0.0f;

	m_dwUpdateCount = 0;

	// build the map of dynamic controls
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_SWITCH,SCT_SWITCH) ); 

	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_ROTARY,SCT_ROTARY) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_ROTARY2,SCT_ROTARY) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_ROTARY3,SCT_ROTARY) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_ROTARY4,SCT_ROTARY) );

	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_SLIDER,SCT_SLIDER) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_SLIDER2,SCT_SLIDER) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_SLIDER3,SCT_SLIDER) ); 
	m_vDynControls.push_back( DYNCONTROL(IDC_SCT_SLIDER4,SCT_SLIDER) ); 
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::releaseSonarInterfaces()
{
	SAFE_RELEASE( m_pMidiOut );
	SAFE_RELEASE( m_pKeyboard );
	SAFE_RELEASE( m_pCommands );
	SAFE_RELEASE( m_pProject );
	SAFE_RELEASE( m_pProject2);
	SAFE_RELEASE( m_pMixer );
	SAFE_RELEASE( m_pMixer2);
	SAFE_RELEASE( m_pTransport);
	SAFE_RELEASE( m_pIdentity);
	SAFE_RELEASE( m_pVUMeters);
	SAFE_RELEASE( m_pSonarParamMapping );
	SAFE_RELEASE( m_pHostWindow );
}

/////////////////////////////////////////////////////////////////////////////

CControlSurfaceProbe::~CControlSurfaceProbe() 
{ 
	releaseSonarInterfaces();
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CControlSurfaceProbe::GetStatusText( LPSTR pszStatus, DWORD* pdwLen )
{
	if (NULL == pdwLen)
		return E_POINTER;

	// TODO: Fill strStatus with a string describing the current
	// status of the surface, e.g., "Tracks 9-16".
	CString strStatus( "OK" );

	// Return results to caller
	if (NULL == pszStatus)
	{
		*pdwLen = strStatus.GetLength() + 1;
	}
	else
	{
		TCHAR2Char(pszStatus, strStatus, *pdwLen);
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::MidiInShortMsg( DWORD dwShortMsg )
{
	// TODO: Handle a MIDI short message which has just been sent
	// *from* the surface.  In other words, the user has just moved
	// a fader or pressed a button; the surface sent a MIDI message;
	// SONAR received the message, and routed back into this DLL.

	// A typical implementation of this would be:

	// [1] "Crack" the MIDI message to determine its type (note, CC, etc.)

	// [2] Confirm that the message did actually come from the surface.
	// (The user may have daisy-chained gear, and you may have been
	//	given data sent from some other device.)

	// [3] Based on the type of message and the state of the surface
	// module, update a SONAR mixer or transport parameter, i.e., make
	// a call to m_pMixer->SetMixParam or m_pTransport->SetTransportState.
	// Or, particularly if your surface has buttons, you might respond by
	// setting some internal state, i.e.,
	//		m_bEffectSelected = TRUE;

	CCriticalSectionAuto csa( &m_cs );

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
	if (NULL == pbLongMsg)
		return E_POINTER;
	if (0 == cbLongMsg)
		return S_OK; // nothing to do

	CCriticalSectionAuto csa( &m_cs );

	// TODO: Handle a MIDI long (SysX) message which has just been sent
	// *from* the surface.  In other words, the user has just moved
	// a fader or pressed a button; the surface sent a MIDI message;
	// SONAR received the message, and routed back into this DLL.

	// See MidiInShortMsg for notes on how this method might be implemented.

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto csa( &m_cs );

	m_dwUpdateCount++;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	if (NULL == pwMask || NULL == pbNoEchoSysx)
		return E_POINTER;

	CCriticalSectionAuto csa( &m_cs );

	// TODO: Tell SONAR about any channels (or sysx) that do not require echo.
	
	*pwMask = 0;
	*pbNoEchoSysx = FALSE;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// ISurfaceParamMapping
HRESULT CControlSurfaceProbe::GetStripRangeCount( DWORD* pdwCount )
{
	if ( !pdwCount )
		return E_POINTER;

	*pdwCount = 1;
	
	return S_OK;
}
HRESULT CControlSurfaceProbe::GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip )
{
	if ( dwIndex != 0 )
		return E_INVALIDARG;
	if ( !pdwLowStrip || !pdwHighStrip || !pmixerStrip )
		return E_POINTER;

	*pdwLowStrip = *pdwHighStrip = GetStripNum();
	*pmixerStrip = GetMixerStrip();

	return S_OK;
}

HRESULT CControlSurfaceProbe::SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip )
{
	SONAR_MIXER_STRIP mixerStripCur = GetMixerStrip();
	DWORD dwCurrentStrip = GetStripNum();

	if ( mixerStrip == mixerStripCur && dwLowStrip == dwCurrentStrip )
		return S_FALSE;	// no change

	SetStripNum( dwLowStrip );
	SetMixerStrip( mixerStrip );

	// notify the host
	m_pSonarParamMapping->OnContextSwitch( m_dwSurfaceID );

	return S_OK;
}


HRESULT CControlSurfaceProbe::GetDynamicControlCount( DWORD* pdwCount )
{
	if ( !pdwCount )
		return E_POINTER;
	
	*pdwCount = (DWORD)m_vDynControls.size();

	return S_OK;
}

HRESULT CControlSurfaceProbe::GetDynamicControlInfo( DWORD dwIndex, DWORD* pdwKey, SURFACE_CONTROL_TYPE* pcontrolType )
{
	if ( !pdwKey || !pcontrolType )
		return E_POINTER;

	if ( dwIndex >= (DWORD)m_vDynControls.size() )
		return E_INVALIDARG;

	DYNCONTROL& dc = m_vDynControls[dwIndex];
	*pdwKey = dc.dwKey;
	*pcontrolType = dc.sctType;

	return S_OK;
}


HRESULT CControlSurfaceProbe::SetLearnState(BOOL)
{
	return S_OK;
}







/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IPersistStream

// WARNING: If you change Save() and Load() and GetSizeMax() to read and write
// different data, you must either generate a new GUID value for CLSID_ControlSurfaceProbe
// and put that in your ControlSurfaceProbe.h file, or you must implement some versioning
// mechanism in your persistence code.

HRESULT CControlSurfaceProbe::Save( IStream* pStm, BOOL bClearDirty )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should write any data to pStm which you wish to store.
	// Typically you will write values for all of your properties.
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.

	// Valid IPersistStream::Write() errors include only STG_E_MEDIUMFULL
	// and STG_E_CANTSAVE. Don't simply return whatever IStream::Write() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::Load( IStream* pStm )
{
	CCriticalSectionAuto csa( &m_cs );

	// Here you should read the data using the format you used in Save().
	// Save() and Load() are used for the host application to provide "Presets"
	// and to persist your effect in application project files.
	
	// Note: IStream::Read() can return S_FALSE, so don't use SUCCEEDED()
	// Valid IPersistStream::Load() errors include only E_OUTOFMEMORY and
	// E_FAIL. Don't simply return whatever IStream::Read() returned.
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CControlSurfaceProbe::GetSizeMax( ULARGE_INTEGER* pcbSize )
{
	CCriticalSectionAuto csa( &m_cs );

	// Assign the size of the data you write to the registry in Save() to pcbSize->LowPart
	pcbSize->LowPart = 0;
	pcbSize->HighPart = 0;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

SONAR_MIXER_STRIP CControlSurfaceProbe::GetMixerStrip()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_eMixerStrip;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetMixerStrip(SONAR_MIXER_STRIP eMixerStrip)
{
	CCriticalSectionAuto csa( &m_cs );

	m_eMixerStrip = eMixerStrip;

	if ( m_pSonarParamMapping )
		m_pSonarParamMapping->OnContextSwitch( m_dwSurfaceID );
}

/////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbe::GetStripNum()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_dwStripNum;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetStripNum(DWORD dwStripNum)
{
	CCriticalSectionAuto csa( &m_cs );

	m_dwStripNum = dwStripNum;

	if ( m_pSonarParamMapping )
		m_pSonarParamMapping->OnContextSwitch( m_dwSurfaceID );
}

/////////////////////////////////////////////////////////////////////////////

SONAR_MIXER_PARAM CControlSurfaceProbe::GetMixerParam()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_eMixerParam;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetMixerParam(SONAR_MIXER_PARAM eMixerParam)
{
	CCriticalSectionAuto csa( &m_cs );

	m_eMixerParam = eMixerParam;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbe::GetParamNum()
{
	CCriticalSectionAuto csa( &m_cs );

	return m_dwParamNum;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetParamNum(DWORD dwParamNum)
{
	CCriticalSectionAuto csa( &m_cs );

	m_dwParamNum = dwParamNum;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetUpdateCount(CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	str->Format(_T("%u"), m_dwUpdateCount);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetStripCount(SONAR_MIXER_STRIP eMixerStrip, CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = _T("m_pMixer is NULL");
		return;
	}

	DWORD dwCount;

	HRESULT hr = m_pMixer->GetMixStripCount(eMixerStrip, &dwCount);

	if (FAILED(hr))
	{
		*str = _T("GetMixStripCount FAILED");
		return;
	}

	str->Format(_T("%u"), dwCount);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetStripName(CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = _T("m_pMixer is NULL");
		return;
	}

	char buf[128];
	DWORD dwLen = sizeof(buf);

	HRESULT hr = m_pMixer->GetMixStripName(m_eMixerStrip, m_dwStripNum, buf, &dwLen);

	if (FAILED(hr))
	{
		*str = _T("GetMixStripName FAILED");
		return;
	}

	*str = buf;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetValueLabel(CString *str)
{
	GetValueLabel(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, str);
}

void CControlSurfaceProbe::GetValueLabel(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = "m_pMixer is NULL";
		return;
	}

	char buf[128];
	DWORD dwLen = sizeof(buf);

	HRESULT hr = m_pMixer->GetMixParamLabel(eMixerStrip, dwStripNum, eMixerParam, dwParamNum, buf, &dwLen);

	if (FAILED(hr))
	{
		*str = "GetMixParamLabel FAILED";
		return;
	}

	*str = buf;
}


/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetValue(CString *str)
{
	GetValue(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, &m_fLastValue, str);
}


void CControlSurfaceProbe::GetValue(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float* pfValue, CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = _T("m_pMixer is NULL");
		return;
	}

	HRESULT hr = m_pMixer->GetMixParam(eMixerStrip, dwStripNum, eMixerParam, dwParamNum, pfValue);

	if (FAILED(hr))
	{
		*str = _T("GetMixParam FAILED");
		return;
	}

	str->Format(_T("%.2f"), *pfValue);
}


/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetValueText(CString *str)
{
	GetValueText(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, m_fLastValue, str);
}

void CControlSurfaceProbe::GetValueText(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float fValue, CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = "m_pMixer is NULL";
		return;
	}

	char buf[128];
	DWORD dwLen = sizeof(buf);

	HRESULT hr = m_pMixer->GetMixParamValueText(eMixerStrip, dwStripNum, eMixerParam, dwParamNum, fValue, buf, &dwLen);

	if (FAILED(hr))
	{
		*str = "GetMixParamValueText FAILED";
		return;
	}

	*str = buf;
}



void CControlSurfaceProbe::GetMappingName( CString* pstr )
{
	if ( !m_pSonarParamMapping )
		*pstr = _T("Not Available");
	else
	{
		char buf[128];
		DWORD dwLen = sizeof(buf);
		if ( SUCCEEDED( m_pSonarParamMapping->GetMapName( m_dwSurfaceID, buf, &dwLen ) ) )
			*pstr = buf;
		else
			*pstr = _T("GetMapName FAILED");
	}
}

void CControlSurfaceProbe::GetUIContext( CString* pstr )
{
	if ( !m_pSonarParamMapping )
		*pstr = _T("Not Available");
	else
	{
		SONAR_UI_CONTEXT uic;
		m_pSonarParamMapping->GetCurrentContext( m_dwSurfaceID, &uic );

		switch( uic )
		{
		case UIC_TRACKVIEW:
			*pstr = "TrackView";
			break;
		case UIC_CONSOLEVIEW:
			*pstr = "ConsoleView";
			break;
		case UIC_PLUGIN:
			*pstr = "Plugin";
			break;
		}
	}
}

bool CControlSurfaceProbe::HasMapping()
{
	if ( !m_pSonarParamMapping )
		return false;

	BOOL b;
	m_pSonarParamMapping->HasMapping( m_dwSurfaceID, &b );
	return !!b;
}


bool	CControlSurfaceProbe::GetContextLocked()
{
	BOOL b;

	if ( m_pSonarParamMapping && SUCCEEDED( m_pSonarParamMapping->GetMapContextLock( m_dwSurfaceID, &b ) ) )
		return !!b;

	return false;
}

void CControlSurfaceProbe::SetContextLocked( bool b )
{
	if ( !m_pSonarParamMapping )
		return;

	m_pSonarParamMapping->SetMapContextLock( m_dwSurfaceID, !!b );
}

bool	CControlSurfaceProbe::GetLearnEnabled()
{
	BOOL b;

	if ( m_pSonarParamMapping && SUCCEEDED( m_pSonarParamMapping->GetActLearnEnable( &b ) ) )
		return !!b;

	return false;
}

void CControlSurfaceProbe::SetLearnEnabled( bool b )
{
	if ( !m_pSonarParamMapping )
		return;

	m_pSonarParamMapping->EnableACTLearnMode( b );
}



/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetArm(CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
	{
		*str = "m_pMixer is NULL";
		return;
	}

	BOOL bVal;

	HRESULT hr = m_pMixer->GetArmMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, &bVal);

	if (FAILED(hr))
	{
		*str = "GetArmMixParam FAILED";
		return;
	}

	*str = (bVal) ? "True" : "False";
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetNumMeters(CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pVUMeters)
	{
		*str = _T("m_pVUMeters is NULL");
		return;
	}

	DWORD dwCount = 0;

	HRESULT hr = m_pVUMeters->GetMeterChannelCount(m_eMixerStrip, m_dwStripNum, &dwCount);

	if (FAILED(hr))
	{
		*str = _T("GetMeterChannelCount FAILED");
		return;
	}

	str->Format(_T("%u"), dwCount);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetMetersValues(CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pVUMeters)
	{
		*str = _T("m_pVUMeters is NULL");
		return;
	}

	DWORD dwCount = 0;

	HRESULT hr = m_pVUMeters->GetMeterChannelCount(m_eMixerStrip, m_dwStripNum, &dwCount);

	if (FAILED(hr))
	{
		*str = _T("GetMeterChannelCount FAILED");
		return;
	}

	if (!m_pIdentity)
	{
		*str = "m_pIdentity is NULL";
		return;
	}

	float *fVals = new float[dwCount];

	hr = m_pVUMeters->GetMeterValues(m_eMixerStrip, m_dwStripNum, m_dwSurfaceID, fVals, &dwCount);

	if (FAILED(hr))
	{
		*str = _T("GetMeterValues FAILED");
	}
	else
	{
		CString tmp;

		str->Empty();

		for (DWORD n = 0; n < dwCount; n++)
		{
			double dB = -60 + 60 * fVals[n];
			tmp.Format(_T("%.2fdB\r\n"), dB );
			*str += tmp;
		}
	}

	delete[] fVals;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetTransportState(SONAR_TRANSPORT_STATE eTransportState, CString *str)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pTransport)
	{
		*str = "m_pTransport is NULL";
		return;
	}

	BOOL bValue;

	HRESULT hr = m_pTransport->GetTransportState(eTransportState, &bValue);
	
	if (FAILED(hr))
	{
		*str = "GetTransportState FAILED";
		return;
	}

	*str = (bValue) ? "1" : "0";
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetTransportState(SONAR_TRANSPORT_STATE eTransportState, bool bEnable)
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pTransport)
		return;

	HRESULT hr = m_pTransport->SetTransportState(eTransportState, (bEnable) ? TRUE : FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::SetValue(float fValue)
{
	SetValue(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, fValue);
}

void CControlSurfaceProbe::SetValue( SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float fValue )
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_pMixer)
		return;

	m_pMixer->SetMixParam(eMixerStrip, dwStripNum, eMixerParam, dwParamNum, fValue, MIX_TOUCH_NORMAL);
}


/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::WriteEnable( bool b )
{
	m_pMixer->SetArmMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, !!b );
}

void CControlSurfaceProbe::ReadEnable( bool b )
{
	if ( !m_pMixer2 )
		return;

	m_pMixer2->SetReadMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, !!b );
}

bool CControlSurfaceProbe::GetWriteEnable()
{
	BOOL b;
	m_pMixer->GetArmMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, &b );
	return !!b;
}
bool CControlSurfaceProbe::GetReadEnable()
{
	if ( !m_pMixer2 )
		return false;

	BOOL b = FALSE;
	m_pMixer2->GetReadMixParam(m_eMixerStrip, m_dwStripNum, m_eMixerParam, m_dwParamNum, &b );
	return !!b;
}


/////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbe::GetCommandCount()
{
	DWORD dwCount;

	if (m_pCommands && SUCCEEDED(m_pCommands->GetCommandCount(&dwCount)))
		return dwCount;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CControlSurfaceProbe::GetCommandInfo(DWORD n, CString *str)
{
	*str = "Err";

	// Command IDs start at 1 (see CommandIDs.h)
	if (!m_pCommands)
		return 0;

	DWORD dwCmdId;
	DWORD dwSize;

	if (FAILED(m_pCommands->GetCommandInfo(n, &dwCmdId, NULL, &dwSize)))
		return 0;

	LPSTR pszName = new char[dwSize];

	if (SUCCEEDED(m_pCommands->GetCommandInfo(n, &dwCmdId, pszName, &dwSize)))
	{
		*str = pszName;
		return dwCmdId;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::DoCommand(DWORD dwCmdId)
{
	if (m_pCommands)
		m_pCommands->DoCommand(dwCmdId);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetUniqueID(CString *str)
{
	if (!m_pIdentity)
	{
		*str = _T("m_pIdentity is NULL");
		return;
	}

	str->Format(_T("0x%08X"), m_dwSurfaceID);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetHostVersion(CString *str)
{
	if (!m_pIdentity)
	{
		*str = _T("m_pIdentity is NULL");
		return;
	}

	ULONG pnMajor, pnMinor, pnRevision, pnBuild;

	HRESULT hr = m_pIdentity->GetHostVersion(&pnMajor, &pnMinor, &pnRevision, &pnBuild);

	if (FAILED(hr))
	{
		*str = _T("GetHostVersion FAILED");
		return;
	}

	str->Format( _T( "%d.%d.%d.%d" ), pnMajor, pnMinor, pnRevision, pnBuild );
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetHostName(CString *str)
{
	if (!m_pIdentity)
	{
		*str = "m_pIdentity is NULL";
		return;
	}

	DWORD dwLen;

	HRESULT hr = m_pIdentity->GetHostName(NULL, &dwLen);

	if (FAILED(hr))
	{
		*str = "GetHostName(NULL) FAILED";
		return;
	}

	LPSTR pszName = new char[dwLen];

	hr = m_pIdentity->GetHostName(pszName, &dwLen);

	*str = (FAILED(hr)) ? "GetHostName FAILED" : pszName;

	delete[] pszName;
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::PopupCapabilities()
{
	CCapabilitiesDialog Dialog;

	Dialog.SetIdentity(m_pIdentity);
	Dialog.DoModal();
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetMarkerCount(CString *str)
{
	if (!m_pProject2)
	{
		*str = _T( "m_pProject2 is NULL" );
		return;
	}

	DWORD dwCount;

	HRESULT hr = m_pProject2->GetMarkerCount(&dwCount);

	if (FAILED(hr))
	{
		*str = _T( "GetMarkerCount FAILED" );
		return;
	}

	str->Format( _T( "%u" ), dwCount );
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetMarkerList(CString *str)
{
	if (!m_pProject2)
	{
		*str = _T("m_pProject2 is NULL");
		return;
	}

	DWORD dwCount;

	HRESULT hr = m_pProject2->GetMarkerCount(&dwCount);

	if (FAILED(hr))
	{
		*str = _T("GetMarkerCount FAILED");
		return;
	}

	str->Empty();

	for (DWORD n = 0; n < dwCount; n++)
	{
		CString tmp;

		GetMarkerByIndex(n, &tmp);

		*str += tmp;
		*str += _T("\r\n");
	}
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetMarkerIndexForTime(MFX_TIME *pTime, CString *str)
{
	if (!m_pProject2)
	{
		*str = _T("m_pProject2 is NULL");
		return;
	}

	DWORD dwIndex;

	HRESULT hr = m_pProject2->GetMarkerIndexForTime(pTime, &dwIndex);

	if (FAILED(hr))
	{
		*str = _T("GetMarkerIndexForTime FAILED");
		return;
	}

	GetMarkerByIndex(dwIndex, str);
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetMarkerByIndex(DWORD dwIndex, CString *str)
{
	if (!m_pProject2)
	{
		*str = _T("m_pProject2 is NULL");
		return;
	}

	MFX_TIME mfxTime;

	mfxTime.timeFormat = TF_MBT;
	HRESULT hr = m_pProject2->GetMarkerTime(dwIndex, &mfxTime);

	if (FAILED(hr))
	{
		*str = _T("GetMarkerTime FAILED");
	}
	else
	{
		DWORD dwSize;

		HRESULT hr = m_pProject2->GetMarkerName(dwIndex, NULL, &dwSize);

		if (FAILED(hr))
		{
			*str = _T("GetMarkerName(NULL) FAILED");
		}
		else
		{
			LPSTR pszName = new char[dwSize];

			HRESULT hr = m_pProject2->GetMarkerName(dwIndex, pszName, &dwSize);

			if (FAILED(hr))
			{
				*str = _T("GetMarkerName FAILED");
			}
			else
			{
				str->Format(_T("[%d:%02d:%03d] %s"),
							mfxTime.mbt.nMeas,
							mfxTime.mbt.nBeat,
							mfxTime.mbt.nTick,
							pszName);
			}

			delete[] pszName;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CControlSurfaceProbe::GetWindowState(WindowType wt, WindowState *pws)
{
	if ( !m_pHostWindow )
		return;

	m_pHostWindow->GetWindowState(wt, pws);
}

void CControlSurfaceProbe::SetWindowState(WindowType wt, WindowState ws)
{
	if ( !m_pHostWindow )
		return;

	m_pHostWindow->SetWindowState(wt, ws);
}

void CControlSurfaceProbe::DoWindowAction(WindowType wt, WindowAction wa, EMoveOperator wo)
{
	if ( !m_pHostWindow )
		return;

	m_pHostWindow->DoWindowAction(wt, wa, wo, 0);
}