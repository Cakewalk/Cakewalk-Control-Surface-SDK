#ifndef MackieControlLCDDisplay_h
#define MackieControlLCDDisplay_h

/////////////////////////////////////////////////////////////////////////////

// Note: the display is actually only 55 chars wide, but the second line
// starts at offset 56 so it just makes it easier to pretend there's an
// extra char at the end of the line

#define LCD_HEIGHT	2
#define LCD_WIDTH	56
#define LCD_SIZE	111

/////////////////////////////////////////////////////////////////////////////

class CMackieControlLCDDisplay
{
public:
	CMackieControlLCDDisplay();
	virtual ~CMackieControlLCDDisplay();

	void Setup(CMackieControlBase *pMackieControlBase);
	void SetLCDId(BYTE bId);

	void SetGlobalMeterMode(bool bVertical, bool bForceSend);
	void SetMeterMode(BYTE bChan, bool bSignalPresentLED, bool bLCDBarGraph,
						bool bDisplayPeakHold, bool bForceSend);

	void SendMeterLevel(BYTE bChan, BYTE bLevel, BYTE bRow =0);

	void WriteCentered(BYTE bY, const char *pText, bool bForceSend);
	void WriteToEOL(BYTE bY, const char *pText, bool bForceSend);
	void Write(BYTE bX, BYTE bY, const char *pText, bool bForceSend);

protected:
	void SendText(BYTE bOffset, BYTE bLen);
	void SendGlobalMeterMode();
	void SendMeterModeSelect(BYTE bChan, BYTE bVal);

	CMackieControlBase *m_pMackieControlBase;
	
	bool m_bGlobalMeterMode;
	BYTE m_bMeterMode[8];

	BYTE m_bText[LCD_SIZE + 1];
	BYTE m_bBuffer[7 + LCD_SIZE + 1];
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControlLCDDisplay_h
