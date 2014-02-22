#ifndef MackieControlFader_h
#define MackieControlFader_h

/////////////////////////////////////////////////////////////////////////////

class CMackieControlFader
{
public:
	CMackieControlFader();
	virtual ~CMackieControlFader();

	void Setup(CMackieControlBase *pMackieControlBase, BYTE bChan);
	void SetVal(float fVal, bool bForceSend);

protected:
	void Send(bool bForceSend);

	CMackieControlBase *m_pMackieControlBase;

	// Settings
	BYTE m_bChan;

	// Current state
	float m_fVal;
	WORD m_wVal;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControlFader_h
