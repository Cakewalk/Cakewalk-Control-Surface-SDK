#include "stdafx.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"
#include "MackieControlLCDDisplay.h"

/////////////////////////////////////////////////////////////////////////////

CMackieControlLCDDisplay::CMackieControlLCDDisplay()
{
//	TRACE("CMackieControlLCDDisplay::CMackieControlLCDDisplay()\n");

	m_pMackieControlBase = NULL;

	m_bGlobalMeterMode = false;
	::memset(m_bMeterMode, 0xFF, 8);

	::memset(m_bText, 0, LCD_SIZE + 1);

	m_bBuffer[0] = 0xF0;
	m_bBuffer[1] = 0x00;
	m_bBuffer[2] = 0x00;
	m_bBuffer[3] = 0x66;
	// m_bBuffer[4] = Device Type
	m_bBuffer[5] = 0x12;
	// m_bBuffer[6] = Cursor start position
}

CMackieControlLCDDisplay::~CMackieControlLCDDisplay()
{
//	TRACE("CMackieControlLCDDisplay::~CMackieControlLCDDisplay()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::Setup(CMackieControlBase *pMackieControlBase)
{
	m_pMackieControlBase = pMackieControlBase;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SetLCDId(BYTE bId)
{
	m_bBuffer[5] = bId;
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SetGlobalMeterMode(bool bVertical, bool bForceSend)
{
	if (!bForceSend && m_bGlobalMeterMode == bVertical)
		return;

	m_bGlobalMeterMode = bVertical;

	SendGlobalMeterMode();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SetMeterMode(BYTE bChan, 
											bool bSignalPresentLED,
											bool bLCDBarGraph,
											bool bDisplayPeakHold,
											bool bForceSend)
{
	if (bChan >= 8)
		return;

	BYTE bVal;

	if (m_bBuffer[5] != 0x12) // C4
	{
		bVal  = bLCDBarGraph ? 0x02 : 0x00;
		bVal |= bDisplayPeakHold ? 0x04 : 0x00;
	}
	else
	{
		// The Mackie documentation gets these wrong:
		bVal  = bSignalPresentLED ? 0x01 : 0x00;
		bVal |= bLCDBarGraph ? 0x02 : 0x00;
		bVal |= bDisplayPeakHold ? 0x04 : 0x00;
	}

	if (!bForceSend && m_bMeterMode[bChan] == bVal)
		return;

	m_bMeterMode[bChan] = bVal;

	BYTE bMeterId = bChan;

	switch (m_bBuffer[5])
	{
		case 0x31: bMeterId +=  8; break;
		case 0x32: bMeterId += 16; break;
		case 0x33: bMeterId += 24; break;
		default: break;
	}

	SendMeterModeSelect(bMeterId, bVal);

	// When we turn meter mode off, invalidate the text data below it
	// so that it gets resent next time

	if (!bLCDBarGraph)
		::memset(m_bText + 56 + (bChan * 7), 0, 7);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SendMeterLevel(BYTE bChan, BYTE bLevel, BYTE bRow)
{
	m_pMackieControlBase->SendMidiShort(0xD0 | bRow, (bChan << 4) | bLevel, 0);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::WriteCentered(BYTE bY, const char *pText, bool bForceSend)
{
	char szLine[LCD_WIDTH + 1];

	::memset(szLine, ' ', LCD_WIDTH);
	szLine[LCD_WIDTH] = 0;

	int len = (int)::strlen(pText);

	if (len > LCD_WIDTH)
		len = LCD_WIDTH;

	::memcpy(szLine + (LCD_WIDTH >> 1) - (len >> 1), pText, len);

	Write(0, bY, szLine, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::WriteToEOL(BYTE bY, const char *pText, bool bForceSend)
{
	char szLine[LCD_WIDTH + 1];

	::memset(szLine, ' ', LCD_WIDTH);
	szLine[LCD_WIDTH] = 0;

	int len = (int)::strlen(pText);

	if (len > LCD_WIDTH)
		len = LCD_WIDTH;

	::memcpy(szLine, pText, len);

	Write(0, bY, szLine, bForceSend);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::Write(BYTE bX, BYTE bY, const char *pText, bool bForceSend)
{
	BYTE bOffset = (bY * LCD_WIDTH) + bX;

	if (bOffset >= LCD_SIZE)
		return;

	int iLen = (int)::strlen(pText);

	if ((int)bOffset + iLen >= LCD_SIZE)
		iLen = LCD_SIZE - bOffset;

	if (!bForceSend)
	{
		// Skip leading chars that haven't changed
		while (iLen > 0)
		{
			if (m_bText[bOffset] == *pText)
			{
				bOffset++;
				pText++;
				iLen--;
			}
			else
			{
				break;
			}
		}

		// Skip trailing chars that haven't changed
		while (iLen > 0)
		{
			if (m_bText[bOffset + iLen - 1] == pText[iLen - 1])
				iLen--;
			else
				break;
		}

		// Anything left?
		if (0 == iLen)
			return;
	}

	::memcpy(m_bText + bOffset, (const void *)pText, iLen);
	
	SendText(bOffset, iLen);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SendText(BYTE bOffset, BYTE bLen)
{
//	TRACE("CMackieControlLCDDisplay::SendText(%d, %d)\n", bOffset, bLen);

	if (bOffset >= LCD_SIZE)
		return;

	if (bOffset + bLen >= LCD_SIZE)
		bLen = LCD_SIZE - bOffset;

#if 0
	char szTemp[128];
	::memcpy(szTemp, m_bText + bOffset, bLen);
	szTemp[bLen] = 0;
	TRACE("SendText(): %2d, %2d, '%s'\n", bOffset, bLen, szTemp);
#endif

	m_bBuffer[4] = m_pMackieControlBase->GetDeviceType();
	m_bBuffer[6] = bOffset;
	::memcpy(m_bBuffer + 7, m_bText + bOffset, bLen);
	m_bBuffer[7 + bLen] = 0xF7;

	m_pMackieControlBase->SendMidiLong(8 + bLen, m_bBuffer);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SendGlobalMeterMode()
{
//	TRACE("CMackieControlLCDDisplay::SendGlobalMeterMode()\n");

	BYTE pbMode[] = { 0xF0, 0x00, 0x00, 0x66,
						m_pMackieControlBase->GetDeviceType(),
						0x21, (BYTE)m_bGlobalMeterMode, 0xF7 };

	m_pMackieControlBase->SendMidiLong(sizeof(pbMode), pbMode);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlLCDDisplay::SendMeterModeSelect(BYTE bChan, BYTE bVal)
{
//	TRACE("CMackieControlLCDDisplay::SendMeterModeSelect(%d, %d)\n", bChan, bVal);

	BYTE pbMode[] = { 0xF0, 0x00, 0x00, 0x66,
						m_pMackieControlBase->GetDeviceType(),
						0x20, bChan, bVal, 0xF7 };

	m_pMackieControlBase->SendMidiLong(sizeof(pbMode), pbMode);
}

/////////////////////////////////////////////////////////////////////////////
