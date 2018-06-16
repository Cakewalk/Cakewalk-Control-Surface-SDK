#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"
#include "MackieControlFader.h"

/////////////////////////////////////////////////////////////////////////////

CMackieControlFader::CMackieControlFader()
{
//	TRACE("CMackieControlFader::CMackieControlFader()\n");

	m_pMackieControlBase = NULL;

	m_bChan = 0xFF;

	m_fVal = -1.0f;
	m_wVal = 0xFFFF;
}

CMackieControlFader::~CMackieControlFader()
{
//	TRACE("CMackieControlFader::~CMackieControlFader()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlFader::Setup(CMackieControlBase *pMackieControlBase, BYTE bChan)
{
	m_pMackieControlBase = pMackieControlBase;
	m_bChan = bChan;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlFader::SetVal(float fVal, bool bForceSend)
{
//	TRACE("CMackieControlFader::SetVal(): %f, %d [%d]\n", fVal, bForceSend, m_bChan);

	if (!bForceSend && m_fVal == fVal)
		return;

	m_fVal = fVal;

	Send(bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlFader::Send(bool bForceSend)
{
//	TRACE("CMackieControlFader::Send(): %.20f, %d\n", m_fVal, bForceSend);

	if (m_bChan > 8 || m_fVal < 0 || m_fVal > 1.0)
		return;

	WORD wVal = ((WORD)(m_fVal * 16383)) & 0xFFF0;

	if (!bForceSend && m_wVal == wVal)
		return;

//	TRACE("CMackieControlFader::Send(): sending 0x%04X\n", wVal);

	m_wVal = wVal;

	BYTE bLow = (BYTE)(wVal & 0x7F);
	BYTE bHigh = (BYTE)(wVal >> 7);

	m_pMackieControlBase->SendMidiShort(0xE0 | m_bChan, bLow, bHigh);
}

/////////////////////////////////////////////////////////////////////////////
