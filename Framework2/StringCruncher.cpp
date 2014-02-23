#include "stdafx.h"
#include "StringCruncher.h"

/////////////////////////////////////////////////////////////////////////////

// Based on the code in SfkUtils.cpp

/////////////////////////////////////////////////////////////////////////////

CStringCruncher::CStringCruncher()
{
	m_lid = 	::GetSystemDefaultLangID(); 
//	TRACE("CStringCruncher::CStringCruncher()\n");
}

CStringCruncher::~CStringCruncher()
{
//	TRACE("CStringCruncher::~CStringCruncher()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CStringCruncher::CrunchString(LPCSTR pszString, char *pBuf, int nBudget, char cPad)
{
	int cbyString = (int)::strlen(pszString);

	// First pass: find the number of characters in each priority:

	// If on a japanese system, just copy
	if ( 0x0411 == m_lid )
	{
		::strncpy( pBuf, pszString, nBudget-1 );
		pBuf[nBudget-1] = 0;
		return;
	}

	// if we fit in the budget, just copy original
	if ( cbyString <= nBudget)
	{
		::memset(pBuf, cPad, nBudget);
		::memcpy(pBuf, pszString, cbyString);
		pBuf[nBudget] = 0;

		return;
	}

	// If you modify the maximum priority, make sure you change this
	int arPrioCounts[5]; 

	::memset(arPrioCounts, 0, sizeof(arPrioCounts));

	char prevChar = 0;
	char curChar = 0;
	char nextChar = 0;

	int ix = 0;

	// Count the number of characters on each priority level
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];
		nextChar = pszString[ix + 1];
		int nPrio = GetCharPriority(curChar, prevChar, nextChar);
		arPrioCounts[nPrio]++;
		prevChar = curChar;
	}

	// Triage by priority (find the lowest priority to include,
	// and the remaining budget)
	int nBaseBudget = nBudget;

	for (ix = 4; ix >= 0; ix--)
	{
		// If this priority can be included fully
		if (arPrioCounts[ix] <= nBaseBudget)
		{
			nBaseBudget -= arPrioCounts[ix];
		}
		else
		{
			// This priority cannot be included fully, the remaining budget is known
			break;
		}
	}

	int nBasePriority = ix;

	int ixOutBuf = 0;
	prevChar = 0;
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];
		nextChar = pszString[ix + 1];

		int nPrio = GetCharPriority(curChar, prevChar, nextChar);

		if (nPrio > nBasePriority)
		{
			pBuf[ixOutBuf++] = MakeCrunchChar(curChar, prevChar, nextChar);
		}
		else if ((nPrio == nBasePriority) && (nBaseBudget > 0))
		{
			pBuf[ixOutBuf++] = MakeCrunchChar(curChar, prevChar, nextChar);
			nBaseBudget--;
		}

		prevChar = curChar;
	}

	pBuf[nBudget] = 0;
}

/////////////////////////////////////////////////////////////////////////////

char CStringCruncher::MakeCrunchChar(char c, char prev, char next)
{
	// Is it the first character of a word?
	if (::isalpha(c) && !::isalnum(prev) && ::islower(c))
	{
		// Watch out for 'dB'
		if ('d' == c && 'B' == next)
			return c;
		else
			return char(::toupper(c));
	}

	return c;
}

/////////////////////////////////////////////////////////////////////////////

int CStringCruncher::GetCharPriority(char c, char prev, char next)
{
	if (::isdigit(c))
		return 4;

	if (('-' == c || '.' == c) && ::isdigit(next))
		return 4;

	// Is it the first character of a word?
	if (::isalpha(c) && !::isalnum(prev))
		return 3;

	if (IsConsonant(c))
		return 2;

	if (IsVowel(c))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool CStringCruncher::IsVowel(char c)
{
	char *pVowels= "AEIOUaeiou";

	return (NULL != ::strchr(pVowels, c));
}

/////////////////////////////////////////////////////////////////////////////

bool CStringCruncher::IsConsonant(char c)
{
	char *pConsonants = "BCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz";

	return (NULL != ::strchr(pConsonants, c));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
 // Unicode Version
// Based on the code in SfkUtils.cpp

/////////////////////////////////////////////////////////////////////////////

CStringCruncherU::CStringCruncherU()
{
	m_lid = 	::GetSystemDefaultLangID(); 
//	TRACE("CStringCruncherU::CStringCruncherU()\n");
}

CStringCruncherU::~CStringCruncherU()
{
//	TRACE("CStringCruncherU::~CStringCruncherU()\n");
}

/////////////////////////////////////////////////////////////////////////////

void CStringCruncherU::CrunchString(LPCTSTR pszString, TCHAR *pBuf, int nBudget, TCHAR cPad)
{
	int cbyString = (int)::_tcslen(pszString);

	// First pass: find the number of characters in each priority:

	// If on a japanese system, just copy
	if ( 0x0411 == m_lid )
	{
		::_tcsncpy( pBuf, pszString, nBudget-1 );
		pBuf[nBudget-1] = 0;
		return;
	}

	// if we fit in the budget, just copy original
	if ( cbyString <= nBudget)
	{
		::memset(pBuf, cPad, nBudget);
		::memcpy(pBuf, pszString, cbyString);
		pBuf[nBudget] = 0;

		return;
	}

	// If you modify the maximum priority, make sure you change this
	int arPrioCounts[5]; 

	::memset(arPrioCounts, 0, sizeof(arPrioCounts));

	TCHAR prevChar = 0;
	TCHAR curChar = 0;
	TCHAR nextChar = 0;

	int ix = 0;

	// Count the number of characters on each priority level
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];
		nextChar = pszString[ix + 1];
		int nPrio = GetCharPriority(curChar, prevChar, nextChar);
		arPrioCounts[nPrio]++;
		prevChar = curChar;
	}

	// Triage by priority (find the lowest priority to include,
	// and the remaining budget)
	int nBaseBudget = nBudget;

	for (ix = 4; ix >= 0; ix--)
	{
		// If this priority can be included fully
		if (arPrioCounts[ix] <= nBaseBudget)
		{
			nBaseBudget -= arPrioCounts[ix];
		}
		else
		{
			// This priority cannot be included fully, the remaining budget is known
			break;
		}
	}

	int nBasePriority = ix;

	int ixOutBuf = 0;
	prevChar = 0;
	for (ix = 0; ix < cbyString; ix++)
	{
		curChar = pszString[ix];
		nextChar = pszString[ix + 1];

		int nPrio = GetCharPriority(curChar, prevChar, nextChar);

		if (nPrio > nBasePriority)
		{
			pBuf[ixOutBuf++] = MakeCrunchChar(curChar, prevChar, nextChar);
		}
		else if ((nPrio == nBasePriority) && (nBaseBudget > 0))
		{
			pBuf[ixOutBuf++] = MakeCrunchChar(curChar, prevChar, nextChar);
			nBaseBudget--;
		}

		prevChar = curChar;
	}

	pBuf[nBudget] = 0;
}

/////////////////////////////////////////////////////////////////////////////

TCHAR CStringCruncherU::MakeCrunchChar(TCHAR c, TCHAR prev, TCHAR next)
{
	// Is it the first character of a word?
	if (::isalpha(c) && !::isalnum(prev) && ::islower(c))
	{
		// Watch out for 'dB'
		if ('d' == c && 'B' == next)
			return c;
		else
			return char(::toupper(c));
	}

	return c;
}

/////////////////////////////////////////////////////////////////////////////

int CStringCruncherU::GetCharPriority(TCHAR c, TCHAR prev, TCHAR next)
{
	if (::isdigit(c))
		return 4;

	if ((_T('-') == c || _T('.') == c) && ::isdigit(next))
		return 4;

	// Is it the first character of a word?
	if (::isalpha(c) && !::isalnum(prev))
		return 3;

	if (IsConsonant(c))
		return 2;

	if (IsVowel(c))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

bool CStringCruncherU::IsVowel(TCHAR c)
{
	TCHAR *pVowels= _T("AEIOUaeiou");

	return (NULL != ::_tcschr(pVowels, c));
}

/////////////////////////////////////////////////////////////////////////////

bool CStringCruncherU::IsConsonant(TCHAR c)
{
	TCHAR *pConsonants = _T("BCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz");

	return (NULL != ::_tcschr(pConsonants, c));
}

/////////////////////////////////////////////////////////////////////////////
