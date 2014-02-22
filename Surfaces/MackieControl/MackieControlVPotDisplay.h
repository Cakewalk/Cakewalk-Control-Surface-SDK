#ifndef MackieControlVPotDisplay_h
#define MackieControlVPotDisplay_h

/////////////////////////////////////////////////////////////////////////////

class CMackieControlVPotDisplay
{
public:
	CMackieControlVPotDisplay();
	virtual ~CMackieControlVPotDisplay();

	void Setup(CMackieControlBase *pMackieControlBase, BYTE bChan);

	void SetVal(float fVal, bool bCenterLED, DataType eDataType, bool bForceSend);
	void SetAllOff(bool bForceSend);
	void SetOnOff(bool bOn, bool bForceSend);

protected:
	void Send(bool bForceSend);

	CMackieControlBase *m_pMackieControlBase;

	// Settings
	BYTE m_bChan;
	
	// Current state
	float m_fVal;
	bool m_bCenterLED;
	DataType m_eDataType;
	BYTE m_bVal;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControlVPotDisplay_h
