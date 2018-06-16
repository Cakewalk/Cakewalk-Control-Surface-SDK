#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"
#include "MackieControlVPotDisplay.h"

/////////////////////////////////////////////////////////////////////////////

CMackieControlVPotDisplay::CMackieControlVPotDisplay()
{
//	TRACE("CMackieControlVPotDisplay::CMackieControlVPotDisplay()\n");

	m_pMackieControlBase = NULL;

	m_bChan = 0xFF;

	m_fVal = -1.0f;
	m_bCenterLED = false;
	m_eDataType = DT_LEVEL;
	m_bVal = 0xFF;
}

CMackieControlVPotDisplay::~CMackieControlVPotDisplay()
{
//	TRACE("CMackieControlVPotDisplay::~CMackieControlVPotDisplay()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlVPotDisplay::Setup(CMackieControlBase *pMackieControlBase, BYTE bChan)
{
	m_pMackieControlBase = pMackieControlBase;
	m_bChan = bChan;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlVPotDisplay::SetVal(float fVal, bool bCenterLED,
									   DataType eDataType, bool bForceSend)
{
	if (!bForceSend && m_fVal == fVal && m_bCenterLED == bCenterLED && m_eDataType == eDataType)
		return;

	m_fVal = fVal;
	m_bCenterLED = bCenterLED;
	m_eDataType = eDataType;

	Send(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlVPotDisplay::SetAllOff(bool bForceSend)
{
	SetVal(0.0f, false, DT_NO_LEDS, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlVPotDisplay::SetOnOff(bool bOn, bool bForceSend)
{
	if (bOn)
		SetVal(0.5f, true, DT_PAN, bForceSend);
	else
		SetVal(0.0f, false, DT_NO_LEDS, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlVPotDisplay::Send(bool bForceSend)
{
	if (m_bChan == 0xFF || m_fVal < 0 || m_fVal > 1.0)
		return;

//	TRACE("CMackieControlVPotDisplay::Send() m_fVal: %f\n", m_fVal);

	BYTE bVal;

	switch (m_eDataType)
	{
		case DT_NO_LEDS:
			bVal = 0;
			break;

		case DT_LEVEL:
			bVal = (BYTE)((m_fVal * 11.0f) + 0.5f);
			break;

		case DT_BOOL:
		case DT_SELECTOR:
		case DT_BOOST_CUT:
		case DT_PAN:
		case DT_INTERLEAVE:
			bVal = 1 + (BYTE)((m_fVal * 10.0f) + 0.5f);
			break;

		case DT_SPREAD:
			bVal = 1 + (BYTE)((m_fVal * 5.0f) + 0.5f);
			break;

		default:
			return;
	}

	switch (m_eDataType)
	{
		case DT_BOOST_CUT:
			bVal |= 0x10;		// Boost/Cut
			break;

		case DT_LEVEL:
			bVal |= 0x20;		// Wrap
			break;

		case DT_SPREAD:
			bVal |= 0x30;		// Spread
			break;

		default:
			break;
	}

	if (m_bCenterLED)
		bVal |= 0x40;

	if (!bForceSend && m_bVal == bVal)
		return;

	m_bVal = bVal;

	m_pMackieControlBase->SendMidiShort(0xB0, m_bChan, bVal);
}

/////////////////////////////////////////////////////////////////////////////
