#ifndef MidiBinding_h
#define MidiBinding_h

#include <vector>

/////////////////////////////////////////////////////////////////////////////

// Disable identifier name too long for debugging warning
#pragma warning(disable: 4786)

/////////////////////////////////////////////////////////////////////////////

class CMidiBinding
{
public:
	CMidiBinding();
	virtual ~CMidiBinding();

	enum MatchType { MT_Message, MT_MessageAndValue, MT_MessageAndBool };
	void SetMessage(BYTE bStatus, BYTE bD1, BYTE bD2 =0x00);
	void SetMessage(const BYTE* pbSysex, size_t cbBytes );
	void SetMatchType(MatchType );


	bool IsMatch(BYTE bStatus, BYTE bD1, BYTE bD2);
	bool IsMatch( MatchType mt, BYTE bStatus, BYTE bD1, BYTE bD2 );
	bool IsMatch(const BYTE* pbSysex, size_t cbBytes);
	BYTE GetStatus() const { return m_bStatus; }
	WORD GetMessage() const { return m_bD1 << 8 | m_bStatus; }
	void GetSysex( std::vector<BYTE>* pv );
	void SetSysex( std::vector<BYTE>& v );

	HRESULT Persist(IStream* pStm, bool bSave);

	enum MessageInterpretation
	{
		MI_Literal = 0,
		MI_Increment,
	};

	DWORD							m_dwHinge;
	bool							m_bUseAccel;
	MessageInterpretation	m_eMessageInterpretation;


protected:
	HRESULT CMidiBinding::Persist(IStream* pStm, bool bSave, void *pData, ULONG ulCount);

	BYTE m_bStatus;
	BYTE m_bD1;
	BYTE m_bD2;
	MatchType	m_eMatchType;

	std::vector<BYTE>	m_vSysex;

};

/////////////////////////////////////////////////////////////////////////////

#endif
