

#pragma once

#define CHECK_PERSIST(_x) do {HRESULT _h = _x; if (FAILED(_h)) {ASSERT(0);return _h;} } while(0)

#include <vector>

void SysexToString( const std::vector<BYTE>& v, CString* pstr );
void StringToSysex( const CString& str , std::vector<BYTE>* pv );
