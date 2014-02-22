// TacomaVUMeters.cpp : Functions for Metering
//

#include "stdafx.h"

#include "TacomaSurface.h"
#include "MixParam.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// Use MAYBE_TRACE() for items we'd like to switch off when not actively
// working on this file.
#if 0
	#define MAYBE_TRACE TRACE
#else
	#define MAYBE_TRACE TRUE ? (void)0 : TRACE
#endif


////////////////////////////////////////////////////////////////////
// Obtain meter values for the 8 strips and send them to the surface
void CTacomaSurface::refreshMeters( )
{
	std::vector<float> v;

	for ( DWORD i = 0; i < 8; i++ )
	{
		GetMeterValues( m_e8StripType, i + getCurrentBankOffset(), &v, 1 );
		if ( !v.empty() )
			setMeterLevel( v[0], i );
		else
			setMeterLevel( 0.f, i );
	}
}


////////////////////////////////////////////////////////////////////////////
// Given a value and a strip index, broadcast the VU Meter message
void CTacomaSurface::setMeterLevel( float f01, DWORD dwStrip )
{
	// determine the byte to send using this transform:
	//	Bit	db		f01
	//	4		0		1.00
	//	3		-3		0.95
	//	2		-6		0.90
	//	1		-12	0.80
	//	0		-36	0.40


	// other levels...
	// -18 => .7
	// -24 => .6
	// -48 => .2

	// formula:  1 - (-)l / 60
	// where l is the level you want in db (ie -24db)


	BYTE by = 0;
	if ( f01 >= 1.f)
		by = 0x1f;
	else if ( f01 >= .95f )	// -3
		by = 0x0f;
//	else if ( f01 >= .9f )
	else if ( f01 >= .8f )	// -12
		by = 0x07;
//	else if ( f01 >= .8f )
	else if ( f01 >= .6f )	// -24
		by = 0x03;
//	else if ( f01 >= .4f )
	else if ( f01 >= .2f )	// -48
		by = 0x01;

	// change?
	MapVUSendByteIterator it = m_mapVUSendByte.find( dwStrip );
	if ( m_mapVUSendByte.end() != it )
	{
		if ( it->second == by )
			return;	// no change
	}
	m_mapVUSendByte[dwStrip] = by;

	// send it
	if ( !m_bOnMackie )
	{
		MAYBE_TRACE( "sending VU Strip:%d, by:%x\n", dwStrip, (int)by );

		m_VUMidiMsg.SetCCNum( 0x50 + dwStrip );
		m_VUMidiMsg.Send( (DWORD)by );
	}
}