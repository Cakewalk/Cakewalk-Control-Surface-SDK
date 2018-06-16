#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"
#include "MackieControl7SegmentDisplay.h"

/////////////////////////////////////////////////////////////////////////////

CMackieControl7SegmentDisplay::CMackieControl7SegmentDisplay()
{
//	TRACE("CMackieControl7SegmentDisplay::CMackieControl7SegmentDisplay()\n");

	m_pMackieControlBase = NULL;

	m_bChan = 0xFF;

	m_bVal = 0xFF;
	m_bDecimalPoint = false;
}

CMackieControl7SegmentDisplay::~CMackieControl7SegmentDisplay()
{
//	TRACE("CMackieControl7SegmentDisplay::~CMackieControl7SegmentDisplay()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControl7SegmentDisplay::Setup(CMackieControlBase *pMackieControlBase, BYTE bChan)
{
	m_pMackieControlBase = pMackieControlBase;
	m_bChan = bChan;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControl7SegmentDisplay::SetChar(char cVal, bool bDecimalPoint, bool bForceSend)
{
	BYTE bVal;

	cVal = ::toupper(cVal);

	if (cVal >= 0x21 && cVal <= 0x3F)
		bVal = cVal;
	else if (cVal >= 0x40 && cVal <= 0x5F)
		bVal = cVal - 0x40;
	else
		bVal = 0x20;	// Space

	SetVal(bVal, bDecimalPoint, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControl7SegmentDisplay::SetVal(BYTE bVal, bool bDecimalPoint, bool bForceSend)
{
	if (!bForceSend && m_bVal == bVal && m_bDecimalPoint == bDecimalPoint)
		return;

	m_bVal = bVal;
	m_bDecimalPoint = bDecimalPoint;

	Send();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControl7SegmentDisplay::Send()
{
	if (m_bChan > 0x0B || m_bVal > 0x3F)
		return;

	BYTE bVal = m_bDecimalPoint ? 0x40 : 0x00;
	bVal |= m_bVal;

	m_pMackieControlBase->SendMidiShort(0xB0, 0x40 | m_bChan, bVal);
}

/////////////////////////////////////////////////////////////////////////////
