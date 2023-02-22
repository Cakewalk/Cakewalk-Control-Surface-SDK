

#pragma once

#define CHECK_PERSIST(_x) do {HRESULT _h = _x; if (FAILED(_h)) {ASSERT(0);return _h;} } while(0)

#include <vector>

void SysexToString( const std::vector<BYTE>& v, CString* pstr );
void StringToSysex( const CString& str , std::vector<BYTE>* pv );

inline int TCHAR2Char(char* pchDest, const TCHAR* tsz, int cbDest)
{
	if (0 == cbDest)
		return 0;

#ifndef _UNICODE
	strncpy(pchDest, tsz, cbDest);
	pchDest[cbDest - 1] = 0;
	return static_cast<int>(strlen(pchDest));
#else
	int const cbConv = WideCharToMultiByte(CP_ACP, 0, tsz, -1, pchDest, cbDest, NULL, NULL);
	pchDest[cbDest - 1] = 0;
	return cbConv;
#endif
}

inline int Char2TCHAR(TCHAR* pszDest, const char* pch, int cchDest)
{
	if (0 == cchDest)
		return 0;

#ifndef _UNICODE
	strncpy(pszDest, pch, cchDest);
	pszDest[cchDest - 1] = 0;
	return static_cast<int>(strlen(pszDest));
#else
	int const cchConv = MultiByteToWideChar(CP_ACP, 0, pch, -1, pszDest, cchDest);
	pszDest[cchDest - 1] = 0;
	return cchConv;
#endif
}
