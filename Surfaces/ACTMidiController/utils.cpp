#include "stdafx.h"
#include "utils.h"

void SysexToString( const std::vector<BYTE>& v, CString* pstr )
{
	CString strByte;
	size_t s = v.size();
	bool bBig = s > 255;

	for ( size_t ix = 0; ix < s; ix++ )
	{
		if ( bBig && ix > 16 )
			break;
		strByte.Format( _T("%02x"), v[ix] );
		(*pstr) += strByte;
	}
	if ( bBig )
		*pstr += _T("...");
}
void StringToSysex( const CString& str , std::vector<BYTE>* pv )
{
	if ( -1 != str.ReverseFind( _T('.') ) )	// should be no elipses
		return;

	pv->clear();
	CString strPair;

	int cChars = str.GetLength();
	if ( 0 < cChars && 0 == cChars % 2 )
	{
		// convert each pair into a byte
		for ( int iPos = 0; iPos < cChars; iPos += 2 )
		{
			strPair = str.Mid( iPos, 2 );
			BYTE byPair = (BYTE)::_tcstoul( strPair, 0, 16 );
			pv->push_back( byPair );
		}
	}
}

