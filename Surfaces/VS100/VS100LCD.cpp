// TacomaLCD.cpp : Functions for LCD display
//

#include "stdafx.h"

#include "VS100.h"
#include "MixParam.h"
#include "LCDTextWriter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Use MAYBE_TRACE() for items we'd like to switch off when not actively
// working on this file.
#if 1
	#define MAYBE_TRACE TRACE
#else
	#define MAYBE_TRACE TRUE ? (void)0 : TRACE
#endif




//////////////////////////////////////////////////////////////////////////
// For every param in the world, check to see if it is on screen and
// display its state in the proper LCD area if it is.
void CVS100::initLCDs()
{
}

void CVS100::showACTParamLabel( CMixParam* p )
{
	char	sz[8];
	sz[0] = '\0';
	DWORD cb = sizeof(sz);
	m_pSonarMixer->GetMixParamLabel( MIX_STRIP_ANY, p->GetStripNum(), MIX_PARAM_DYN_MAP, m_dwSurfaceID, sz, &cb );

	m_pLCD->ShowText( sz, 0, 0 );
}


void CVS100::showEncoderParam( CMixParam* p )
{
	char szN[9];
	p->GetCrunchedParamLabel( szN, 8 );

	char szV[9];
	p->GetCrunchedValueText( szV, 8 );

	m_pLCD->ShowText( szN, 2, 0 );
	m_pLCD->ShowText( szV, 2, 1 );
}


////////////////////////////////////////////////////////////
// Given a Mix param,  determine its LCD index and if it is 
// actively on screen.  Optionally force it on the screen with
// bTouched
void CVS100::showParam( CMixParam* pParam, ParamShowMode eShowMode )
{
}



//////////////////////////////////////////////////////////////////
// Given a mix param and an index, display it in the lcd
void CVS100::showParam( CMixParam* pParam, int ixLcd, ParamShowMode eShowMode )
{
}


static DWORD s_mtbd = 100;
static BYTE s_byBuf[128];	// sysx buffer


CLCDTextWriter::CLCDTextWriter( CVS100* pSurf )
: m_pSurface( pSurf )
{
	::memset( m_abuf, 0x20, 8*2*3 );
}

/*
sx		Roland / Model ID		Address		data bytes			chksum	eox
F0		41	10	0	0	31	12		AA	BB			data	…	data		sum		F7
*/

static const BYTE s_abyPrefix[] = {0xf0, 0x41, 0x10,	// sysx / roland
//                                   0x00, 0x00, 0x31,	// Model ID (TBD) (currently same as VS700 - will change!)
                                   0x00, 0x00, 0x3e,	// next version of firmware will be this
                                   0x12 };				// Command ID
static const size_t s_cbPrefix = sizeof( s_abyPrefix );



//----------------------------------------------------------------
void CLCDTextWriter::ShowText( LPCSTR pszIn, int ixRow, int ixLR )
{
	if ( ixLR == 0 && ixRow == 0 )
		ixRow = ixRow;
	if ( ixLR < 0 || ixLR > 1 )
	{
		ASSERT(0);
		return;
	}
	if ( ixRow < 0 || ixRow > 2 )
	{
		ASSERT(0);
		return;
	}

	size_t cMaxChars = 8;

	size_t cChars = ::strlen( pszIn );
	char psz[9];
	::memset(psz, 0, sizeof(psz) );
	cChars = min( cChars, cMaxChars );
	::strncpy( psz, pszIn, cChars );

	// pad out with spaces
	for ( size_t is = cChars; is < cMaxChars; is++ )
		psz[is] = 0x20;

	if ( 0 == ::memcmp( &(m_abuf[8*ixRow*ixLR]), psz, cMaxChars ) )
		return;

	::memcpy( &(m_abuf[8*ixRow*ixLR]), psz, cMaxChars );

	BYTE byCS = 0;

	// Determine the Display Addresses based on the lcd column (index)
	BYTE AA = 0;
	BYTE BB = 0;
	if ( ixLR == 0 )
	{
		if ( ixRow == 1 )
			AA = 0x04;
		if ( ixRow == 2 )
		{
			BB = 0x40;
			AA = 0x04;
		}
	}
	else
	{
		BB = 0x08;
		if ( ixRow == 1 )
		{
			AA = 0x08;
			BB = 0x20;
		}
		else if ( ixRow == 2 )
		{
			AA = 0x08;
			BB = 0x60;
		}
	}

	const size_t cb = s_cbPrefix + 4 + cMaxChars;	// total of the above items

	BYTE* pFill = s_byBuf + s_cbPrefix;

	::memcpy( s_byBuf, s_abyPrefix, s_cbPrefix );

	*pFill++ = AA;		// add the addressing bytes
	byCS += AA;
	*pFill++ = BB;
	byCS += BB;

	// the data itself
	for ( DWORD i = 0; i < cMaxChars; i++ )
	{
		BYTE byChar = psz[i];
		byCS += byChar;
		*pFill++ = byChar;
	}

	// compute checksum.
	const BYTE byMod = byCS % 128;
	byCS = 128 - byMod;
	byCS &= 0x7f;

	*pFill++ = byCS;	// checksum

	*pFill++ = 0xF7;	// EOX

	ASSERT( pFill - s_byBuf == cb );	// if this fires, cb was not computed correctly

	if ( m_pSurface )
		m_pSurface->MidiOutLongMsg( (DWORD)cb, s_byBuf );
}
