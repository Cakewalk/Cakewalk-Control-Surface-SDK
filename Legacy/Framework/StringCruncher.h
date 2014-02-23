#ifndef StringCruncher_h
#define StringCruncher_h

/////////////////////////////////////////////////////////////////////////////

class CStringCruncher
{
public:
	CStringCruncher();
	virtual ~CStringCruncher();

	void CrunchString(LPCSTR pszString, char *pBuf, int nBudget, char cPad =' ');

protected:
	char MakeCrunchChar(char c, char prev, char next);
	int GetCharPriority(char c, char prev, char next);
	bool IsVowel(char c);
	bool IsConsonant(char c);

	LANGID	m_lid;
};

/////////////////////////////////////////////////////////////////////////////

#endif // StringCruncher_h
