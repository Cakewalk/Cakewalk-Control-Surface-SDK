#ifndef MackieControl7SegmentDisplay_h
#define MackieControl7SegmentDisplay_h

/////////////////////////////////////////////////////////////////////////////

class CMackieControl7SegmentDisplay
{
public:
	CMackieControl7SegmentDisplay();
	virtual ~CMackieControl7SegmentDisplay();

	void Setup(CMackieControlBase *pMackieControlBase, BYTE bChan);
	void SetChar(char cVal, bool bDecimalPoint, bool bForceSend); 
	void SetVal(BYTE bVal, bool bDecimalPoint, bool bForceSend);

protected:
	void Send();

	CMackieControlBase *m_pMackieControlBase;

	// Settings
	BYTE m_bChan;

	// Current state
	BYTE m_bVal;
	bool m_bDecimalPoint;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControl7SegmentDisplay_h
