// MackieControlBase.cpp : Defines the basic functionality of the "MackieControlBase".
//

#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CMackieControlBase
//
/////////////////////////////////////////////////////////////////////////////

CMackieControlState CMackieControlBase::m_cState;

bool CMackieControlBase::m_bMsgRefreshAll;
bool CMackieControlBase::m_bMsgForceRefreshAll;
bool CMackieControlBase::m_bMsgSetAllFadersToDefault;
bool CMackieControlBase::m_bMsgSetAllVPotsToDefault;

/////////////////////////////////////////////////////////////////////////////

// Constructors/Deconstructors

CMackieControlBase::CMackieControlBase() :
	m_cRef( 1 ),
	m_bDirty( FALSE ),
	m_dwUniqueId( 0 ),
	m_pMidiOut( NULL ),
	m_pKeyboard( NULL ),
	m_pCommands( NULL ),
	m_pProject( NULL ),
	m_pMixer( NULL ),
	m_pMixer2( NULL ),
	m_pParamMapping( NULL ),
	m_pTransport( NULL ),
	m_pSonarIdentity( NULL ),
	m_pSonarIdentity2( NULL ),
	m_dwSupportedRefreshFlags( 0 ),
	m_hwndApp( NULL ),
	m_FilterLocator()
{
//	TRACE("CMackieControlBase::CMackieControlBase()\n");

	::InterlockedIncrement( &g_lComponents );
	::InitializeCriticalSection( &m_cs );

	m_currentPluginIsTrackCompressor = false;
	m_lastMidiOnNote = 0x00;
	m_bConnected = false;
	m_dwRefreshCount = 0;
	m_bExpectedDeviceType = 0x00;
	m_bDeviceType = 0x00;
	m_bHaveSerialNumber = false;
	::memset(m_bSerialNumber, 0, LEN_SERIAL_NUMBER);
	m_dwUnitStripNumOffset = 0;
	m_bBindMuteSoloArm = false;

	// We don't use all of these, but it makes indexing easier
	for (int n = 0; n < NUM_SWITCH_AND_LED_IDS; n++)
	{
		m_bSwitches[n] = false;
		m_bLEDs[n] = 0xFF;
	}

	m_bRefreshWhenDone = false;
	m_bForceRefreshWhenDone = false;
	m_bRefreshAllWhenDone = false;
	m_bForceRefreshAllWhenDone = false;

	CCriticalSectionAuto csa(m_cState.GetCS());

	m_cState.AddUnit(this);

	// Load the plugin mappings here so that we can locate the .ini file correctly
	m_cState.LoadPluginMappings();

	UpdateToolbarDisplay(true);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::releaseSonarInterfaces()
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
	if (m_pMixer2)
		m_pMixer2->Release(), m_pMixer2 = NULL;
	if (m_pTransport)
		m_pTransport->Release(), m_pTransport = NULL;
	if (m_pSonarIdentity)
		m_pSonarIdentity->Release(), m_pSonarIdentity = NULL;
	if (m_pSonarIdentity2)
		m_pSonarIdentity2->Release(), m_pSonarIdentity2 = NULL;
	if (m_pParamMapping)
		m_pParamMapping->Release(), m_pParamMapping = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CMackieControlBase::~CMackieControlBase()
{
//	TRACE("CMackieControlBase::~CMackieControlBase()\n");

	CCriticalSectionAuto csa(m_cState.GetCS());
	m_cState.RemoveUnit(this);

	releaseSonarInterfaces();
	::DeleteCriticalSection( &m_cs );
	::InterlockedDecrement( &g_lComponents );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// IControlSurface

HRESULT CMackieControlBase::MidiInShortMsg( DWORD dwShortMsg )
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	if (!m_bHaveSerialNumber)
		return S_OK;

	ClearRefreshFlags();

	BYTE bStatus = (BYTE)(dwShortMsg & 0xFF);
	BYTE bD1 = (BYTE)((dwShortMsg >> 8) & 0xFF);
	BYTE bD2 = (BYTE)((dwShortMsg >> 16) & 0xFF);

	if (m_cState.GetSelectDoubleClick())
	{

		if ((bStatus == 0x90) && (bD1 >= 0x18) && (bD1 <= 0x1F) && (bD2 > 0))
		{
			if (bD1 != m_lastMidiOnNote)
			{
				m_lastMidiOnNote = bD1;
				return S_OK;
			}
		}
		else if ((bStatus != 0x90) || ((bStatus == 0x90) && ((bD1 < 0x18) || (bD1 > 0x1F))))
		{
			m_lastMidiOnNote = 0x00;
		}
	}

	ClearRefreshFlags();

	OnMidiInShort(bStatus, bD1, bD2);

	DispatchRefreshRequest();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg )
{
//	TRACE("CMackieControlBase::MidiInLongMsg()\n");

	if (NULL == pbLongMsg)
		return E_POINTER;

	if (0 == cbLongMsg)
		return S_OK; // nothing to do

	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	ClearRefreshFlags();

	// Wake-up message (sent once after power on)
	BYTE pbWUM[] = { 0xF0, 0x00, 0x00, 0x66,
						// device type
						// 0x01
						// s0..s10 unique id (random)
						// 0xF7
					};

	// Response to Universal Device Inquiry
	BYTE pbRUDI[] = { 0xF0, 0x7E, 0x00, 0x06, 0x02, 0x00, 0x00, 0x66,
						0x01, 0x00,	// family == Mackie Control Surface
						// two bytes of device id
						// four bytes of version
						// 0xF7
					};

	// Response to Serial Number Request
	BYTE pbRSNR[] = { 0xF0, 0x00, 0x00, 0x66,
						// device type
						// 0x1B
						// s0..s6 device serial number
						// 0xF7
					};

	if (::memcmp(pbLongMsg, pbWUM, sizeof(pbWUM)) == 0 && 0x01 == pbLongMsg[5])
	{
		TRACE("CMackieControlBase::MidiInLongMsg(): Mackie Wake-up: 0x%02X\n", pbLongMsg[4]);

		// Wakeup message - reset things and reacquire the information
		m_bDeviceType = 0x00;
		m_bHaveSerialNumber = false;

		m_bRefreshWhenDone = true;

	}
	else if (::memcmp(pbLongMsg, pbRUDI, sizeof(pbRUDI)) == 0)
	{
		if (0x00 == m_bDeviceType)
		{
			m_bDeviceType = pbLongMsg[10];

			TRACE("CMackieControlBase::MidiInLongMsg(): Mackie Control ID: 0x%02X\n", m_bDeviceType);

			m_bRefreshWhenDone = true;
		}
	}
	else if (::memcmp(pbLongMsg, pbRSNR, sizeof(pbRSNR)) == 0 && 0x1B == pbLongMsg[5])
	{
		if (!m_bHaveSerialNumber)
		{
			m_bDeviceType = pbLongMsg[4];
			::memcpy(m_bSerialNumber, pbLongMsg + 6, LEN_SERIAL_NUMBER);

			TRACE("CMackieControlBase::MidiInLongMsg(): Type: 0x%02X, Serial Number: %02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
					m_bDeviceType, m_bSerialNumber[0], m_bSerialNumber[1], m_bSerialNumber[2],
					m_bSerialNumber[3], m_bSerialNumber[4], m_bSerialNumber[5], m_bSerialNumber[6]);

			m_bHaveSerialNumber = true;

			m_bForceRefreshWhenDone = true;

			CCriticalSectionAuto csa(m_cState.GetCS());
			m_cState.RestoreUnitOffset(this);

			OnReceivedSerialNumber();
		}
	}
	else if (m_bHaveSerialNumber)
	{
		OnMidiInLong(cbLongMsg, pbLongMsg);
	}

	DispatchRefreshRequest();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::RefreshSurface( DWORD fdwRefresh, DWORD dwCookie )
{
	CCriticalSectionAuto csa( &m_cs );

	if (!m_bConnected)
		return S_OK;

	if (fdwRefresh != 0 ||
		dwCookie == (DWORD)&m_bMsgRefresh ||
		dwCookie == (DWORD)&m_bMsgForceRefresh ||
		dwCookie == (DWORD)&m_bMsgRefreshAll ||
		dwCookie == (DWORD)&m_bMsgForceRefreshAll)
	{
		m_dwRefreshCount++;

		bool bForceSend = (dwCookie == (DWORD)&m_bMsgForceRefresh || dwCookie == (DWORD)&m_bMsgForceRefreshAll);

		// Detected the surface yet?
		if (!m_bHaveSerialNumber)
		{
			if (m_bExpectedDeviceType != 0x00)
				QuerySerialNumber(m_bExpectedDeviceType);
		}
		else
		{
			OnRefreshSurface(fdwRefresh, bForceSend);
		}
	}
	else if (dwCookie == (DWORD)&m_bMsgSetAllFadersToDefault)
	{
		if (m_bHaveSerialNumber)
			SetAllFadersToDefault();
	}
	else if (dwCookie == (DWORD)&m_bMsgSetAllVPotsToDefault)
	{
		if (m_bHaveSerialNumber)
			SetAllVPotsToDefault();
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx )
{
	if (NULL == pwMask || NULL == pbNoEchoSysx)
		return E_POINTER;

	CCriticalSectionAuto csa( &m_cs );

	// since the Mackie is a dedicated Mixer Control Surface, we'll just grab
	// the whole port...  All channels and Sysex

	*pwMask = 0xFFFF;
	*pbNoEchoSysx = TRUE;

	return S_OK;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/// ISurfaceParamMapping

HRESULT CMackieControlBase::GetDynamicControlCount( DWORD *pdwCount)
{
	return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////

HRESULT CMackieControlBase::GetDynamicControlInfo( DWORD dwIndex, DWORD *pdwKey, SURFACE_CONTROL_TYPE *pcontrolType)
{
	return E_NOTIMPL;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetUnitStripNumOffset(DWORD dwOffset)
{
	m_dwUnitStripNumOffset = dwOffset;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SendMidiShort(BYTE bStatus, BYTE bD1, BYTE bD2)
{
	DWORD dwMsg = bD2;
	dwMsg <<= 8;	dwMsg |= bD1;
	dwMsg <<= 8;	dwMsg |= bStatus;

	m_pMidiOut->MidiOutShortMsg(this, dwMsg);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SendMidiLong(DWORD cbLongMsg, const BYTE* pbLongMsg)
{
	m_pMidiOut->MidiOutLongMsg(this, cbLongMsg, pbLongMsg);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetSerialNumber(BYTE *pSerialNumber)
{
	if (!m_bHaveSerialNumber)
		return false;

	::memcpy(pSerialNumber, m_bSerialNumber, LEN_SERIAL_NUMBER);

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::IsSerialNumber(BYTE *pSerialNumber)
{
	if (!pSerialNumber)
		return false;

	return (::memcmp(m_bSerialNumber, pSerialNumber, LEN_SERIAL_NUMBER) == 0);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ClearRefreshFlags()
{
	m_bRefreshWhenDone = false;
	m_bForceRefreshWhenDone = false;
	m_bRefreshAllWhenDone = false;
	m_bForceRefreshAllWhenDone = false;
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::DispatchRefreshRequest()
{
	if (m_bForceRefreshAllWhenDone)
	{
		RequestRefreshAll(true);
	}
	else
	{
		if (m_bForceRefreshWhenDone)
			RequestRefresh(true);

		if (m_bRefreshAllWhenDone)
			RequestRefreshAll(false);
		else if (m_bRefreshWhenDone && !m_bForceRefreshWhenDone)
			RequestRefresh(false);
	}
}

////////////////////////////////////////////////////////////////////////////////

// Request a refresh of just this module
//
void CMackieControlBase::RequestRefresh(bool bForce)
{
	m_pProject->RequestRefresh((DWORD)(bForce ? &m_bMsgForceRefresh : &m_bMsgRefresh));
}

////////////////////////////////////////////////////////////////////////////////

// Request a refresh of all instances
//
void CMackieControlBase::RequestRefreshAll(bool bForce)
{
	m_pProject->RequestRefresh((DWORD)(bForce ? &m_bMsgForceRefreshAll : &m_bMsgRefreshAll));
}


////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::RequestSetAllFadersToDefault()
{
	m_pProject->RequestRefresh((DWORD)&m_bMsgSetAllFadersToDefault);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::RequestSetAllVPotsToDefault()
{
	m_pProject->RequestRefresh((DWORD)&m_bMsgSetAllVPotsToDefault);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SendUniversalDeviceQuery()
{
	TRACE("CMackieControlBase::SendUniversalDeviceQuery()\n");

	BYTE pbSNR[] = { 0xF0, 0x7E, 0x00, 0x06, 0x01, 0xF7 };

	SendMidiLong(sizeof(pbSNR), pbSNR);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SendWakeUp(BYTE bDeviceType)
{
	TRACE("CMackieControlBase::SendWakeUp(): 0x%02X\n", bDeviceType);

	BYTE pbWake[] = { 0xF0, 0x00, 0x00, 0x66, bDeviceType, 0x00, 0xF7 };

	SendMidiLong(sizeof(pbWake), pbWake);
}

////////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::QuerySerialNumber(BYTE bDeviceType)
{
	TRACE("CMackieControlBase::QuerySerialNumber(): 0x%02X\n", bDeviceType);

	// don't attempt query if no project is loaded.  The ISonarProject
	// APIs return E_FAIL if no project is loaded.
	if ( m_pProject && !SUCCEEDED( m_pProject->GetProjectModified() ) )
		return;

	BYTE pbSNR[] = { 0xF0, 0x00, 0x00, 0x66, bDeviceType, 0x1A, 0x00, 0xF7 };

	SendMidiLong(sizeof(pbSNR), pbSNR);
}

/////////////////////////////////////////////////////////////////////////////

// Set LED state, 3 options:
//
//		0 - Off
//		1 - On
//		2 - Blink
//
// Note: this is different from what the hardware expects, but
// allows us to simply pass in a bool for the most common on/off case
//
void CMackieControlBase::SetLED(BYTE bID, BYTE bVal, bool bForceSend)
{
	if (bID >= NUM_SWITCH_AND_LED_IDS)
		return;

	if (UsingHUIProtocol())
	{
		SetHuiLED(bID, bVal, bForceSend);
		return;
	}

	// The hardware accepts:
	//		0x00 - Off
	//		0x01 - Blink
	//		0x7F - On

	if (2 == bVal)
		bVal = 1;
	else if (1 == bVal || bVal > 2)
		bVal = 0x7F;

	if (!bForceSend && m_bLEDs[bID] == bVal)
		return;

	m_bLEDs[bID] = bVal;

	SendMidiShort(0x90, bID, bVal);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::UpdateToolbarDisplay(bool bForce)
{
	if (!bForce && m_cState.GetToolbarUpdateCount() == m_dwToolbarUpdateCount)
		return;

//	TRACE("CMackieControlBase::UpdateToolbarDisplay()\n");

	if (m_pProject)
		m_pProject->OnNewStatus(0);

	m_dwToolbarUpdateCount = m_cState.GetToolbarUpdateCount();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetProjectLoadedState(bool bProjectLoadedState)
{
	m_bProjectLoadedState = bProjectLoadedState;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::UsingHUIProtocol()
{
	return m_cState.GetUseHUIProtocol();
}