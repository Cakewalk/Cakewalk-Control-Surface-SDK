#ifndef MackieControlInformation_h
#define MackieControlInformation_h

/////////////////////////////////////////////////////////////////////////////

// Length of the serial number (in bytes)
#define LEN_SERIAL_NUMBER			7

/////////////////////////////////////////////////////////////////////////////

// Class to hold information about the unit
class CMackieControlInformation
{
public:
	CMackieControlInformation();
	virtual ~CMackieControlInformation();

	bool Load(IStream *pStm);
	bool Save(IStream *pStm);

	DWORD m_dwUniqueId;
	DWORD m_dwOffset;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MackieControlInformation_h
