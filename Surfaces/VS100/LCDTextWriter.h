#pragma once

class CVS100;


class CLCDTextWriter
{
public:
	CLCDTextWriter( CVS100* p );
	void ShowText( LPCSTR psz, int ixRow, int ixLR );

private:

	// buffer [char][row][lr]
	char m_abuf[8*3*2];

	CVS100*	m_pSurface;
};