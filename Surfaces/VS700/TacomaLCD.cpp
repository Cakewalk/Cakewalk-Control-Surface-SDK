// TacomaLCD.cpp : Functions for LCD display
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


/*
status data byte                                  status
F0H   41H,14H,**H,**H,**H,12H,           F7H
               aaH,bbH,vvH,...vvH,sum       

41H              :Roland ID (TBD)
14H              :Device ID
**H,**H,**H   :Model ID
12H              :Command ID
aaH,bbH         :Adress MSB,LSB
vvH,...vvH       :Data
sum              :Check sum
*/

static const BYTE s_abyPrefix[] = {0xf0, 0x41, 0x10,	// sysx / roland
                                   0x00, 0x00, 0x31,	// Model ID (TBD)
                                   0x12 };				// Command ID
static const size_t s_cbPrefix = sizeof( s_abyPrefix );

// LCD Index
// ACT0	ACT1	ACT2	ACT3	Strp0	Strp1	Strp2	Strp3	Strp4	Strp5	Strp6	Strp7	Master
//		0		1		2		3		4		5		6		7		8		9		10		11		12



//////////////////////////////////////////////////////////////////////////
// For every param in the world, check to see if it is on screen and
// display its state in the proper LCD area if it is.
//	FT: Updated for VS-700 Surface 1.3 Update.
// If new MixerParam filters are added, create new sets to hold and display them.
void CTacomaSurface::initLCDs()
{
	for ( MixParamSetIterator it = m_setEveryMixparam.begin(); it != m_setEveryMixparam.end(); ++it )
	{
		CMixParam* const p = *it;
		int ixLcd = -1;

		//bypass Params with MIX_PARAM_FILTER_PARAM as an eMixerParam.  Display Everything else.
		if ( p->GetMixerParam() == MIX_PARAM_FILTER_PARAM )
		{		
			//Handle MIX_PARAM_FILTER_PARAMs, bypass EQs, handled below
			if ( ( m_eActSectionMode == KSM_PC_COMP ) && (LOWORD(p->GetParamNum()) != MIX_FILTER_COMP))
				continue;
			else if ( ( m_eActSectionMode == KSM_PC_SAT ) && (LOWORD(p->GetParamNum()) != MIX_FILTER_SAT))
				continue;
			else if ( ( m_eActSectionMode == KSM_EQ || m_eActSectionMode == KSM_PC_EQ || m_eActSectionMode == KSM_PC_EQ_P2 ) && LOWORD(p->GetParamNum()) == MIX_FILTER_EQ )
			{
				//No way to differentiate EQs, handle the filters with separate sets, based on SectionMode.
				if ( m_eActSectionMode == KSM_EQ )
				{
					for ( MixParamSetIterator it2 = m_setSonitusEQ.begin(); it2 != m_setSonitusEQ.end(); ++it2 )
					{
						CMixParam* const pSonitus = *it2;
						int ixLcd = -1;
						if ( getLCDIndex( pSonitus, m_bFlipped, &ixLcd ) )
							showParam( pSonitus, ixLcd, PSM_Refresh );
					}
				}
				else if ( m_eActSectionMode == KSM_PC_EQ || m_eActSectionMode == KSM_PC_EQ_P2 )
				{
					for ( MixParamSetIterator it3 = m_setGlossEQ.begin(); it3 != m_setGlossEQ.end(); ++it3 )
					{
						CMixParam* const pGloss = *it3;
						int ixLcd = -1;
						{
							if ( getLCDIndex( pGloss, m_bFlipped, &ixLcd ) )
								showParam( pGloss, ixLcd, PSM_Refresh );
						}
					}
				}
				break; // done processing eq's
			}
		}

		// Catch all
		if ( getLCDIndex( p, m_bFlipped, &ixLcd ) )
			showParam( p, ixLcd, PSM_Refresh );
	}
}

//////////////////////////////////////////////////////////////////////
// Given a parameter, determine if it is on screen and which LCD index
// it is in
bool  CTacomaSurface::getLCDIndex( CMixParam* pParam, bool bFlipped, int* pixLcd )
{
	DWORD dwParam = pParam->GetParamNum();
	SONAR_MIXER_PARAM param = pParam->GetMixerParam();
	bool bOnScreen = false;
	int ixLcd = -1;

	// determine if this is on screen
	if ( MIX_PARAM_DYN_MAP == param )
	{
		DWORD dwStrip = pParam->GetStripNum();
		if ( ACTKEY_BASE + TBAR_DYN_INDEX == dwStrip ) // tbar
		{
			*pixLcd = 12;
			bOnScreen = true;
			return bOnScreen;
		}
		else
		{
			const int iActIndex = dwStrip - ACTKEY_BASE;		// how far into our act keys
			ixLcd = iActIndex % 4;

			// ACT
			if ( KSM_ACT == m_eActSectionMode || KSM_FLEXIBLE_PC == m_eActSectionMode )
			{
				// figure out if this param is on the active display row
				if ( (int)m_wACTDisplayRow == ( iActIndex / 4 ) )
				{
					// yes it is.
					bOnScreen = true;
				}
			}
		}
	}
	else if ( MIX_PARAM_FILTER_PARAM == param )
	{
		dwParam = (HIWORD(dwParam));
		ixLcd = dwParam % 4;
		if ( KSM_EQ == m_eActSectionMode )
		{
			// EQ
			// determine if this is on the current display row
			switch( m_wACTDisplayRow )
			{
				// yes I know this looks like a simple formula to equate row with param,
				// but I put this switch/case method in because I'm really hoping the
				// silkscreen changes
			case 0:
				bOnScreen = dwParam >= 0 && dwParam < 4;	// gain
				break;
			case 1:
				bOnScreen = dwParam >= 4 && dwParam < 8;	// freq
				break;
			case 2:
				bOnScreen = dwParam >= 8 && dwParam < 12;	// Q
				break;
			case 3:
				bOnScreen = dwParam >= 16 && dwParam < 20;	// enable
				break;
			}
		}

		if ( KSM_PC_COMP == m_eActSectionMode ) 
		{
			// determine if this is on the current display row
			switch( m_wACTDisplayRow )
			{
			case 0:
				bOnScreen = ( dwParam == 2 || dwParam == 3 || dwParam == 4 || dwParam == 6 );	// Input,Attack, Release, Output
				switch ( dwParam )
				{
				case 2:
					ixLcd = 0;
					break;
				case 3:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
				case 6:
					ixLcd = 3;
					break;
					//
				case 10:
					ixLcd = 0;
					break;
				case 8:
					ixLcd = 1;
					break;
				case 5:
					ixLcd = 2;
					break;
					//
				case 7:
					ixLcd = 3;
					break;
				case 9:
					ixLcd = 0;
					break;
				case 0:
					ixLcd = 2;
					break;
				case 1:
					ixLcd = 3;
					break;
					// Blank Params
				case 11:
					ixLcd = 0;
					break;
				case 12:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 14:
					ixLcd = 3;
					break;
				}
				break;
			case 1:
				bOnScreen = ( dwParam == 10 || dwParam == 12 || dwParam == 5 || dwParam == 7 );	// // HPF, not used, Ratio, Dry/Wet 
				switch ( dwParam )
				{
				case 2:
					ixLcd = 0;
					break;
				case 3:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
				case 6:
					ixLcd = 3;
					break;
					//
				case 10:
					ixLcd = 0;
					break;
				case 5:
					ixLcd = 2;
					break;
				case 7:
					ixLcd = 3;
					//
					break;
				case 9:
					ixLcd = 0;
					break;
				case 0:
					ixLcd = 2;
					break;
				case 1:
					ixLcd = 3;
					break;
					// Blank Params
				case 11:
					ixLcd = 0;
					break;
				case 12:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 14:
					ixLcd = 3;
					break;
				}
				break;
			case 2:
				bOnScreen = ( dwParam == 11 || dwParam == 12 || dwParam == 13 || dwParam == 14 );	//
				switch ( dwParam )
				{
				case 2:
					ixLcd = 0;
					break;
				case 3:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
				case 6:
					ixLcd = 3;
					break;
					//
				case 10:
					ixLcd = 0;
					break;
				case 5:
					ixLcd = 2;
					break;
				case 7:
					ixLcd = 3;
					break;
					//
				case 9:
					ixLcd = 0;
					break;
				case 0:
					ixLcd = 2;
					break;
				case 1:
					ixLcd = 3;
					break;
					// Blank Params
				case 11:
					ixLcd = 0;
					break;
				case 12:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 14:
					ixLcd = 3;
					break;

				}
				break;
			case 3:
				bOnScreen = ( dwParam == 9 || dwParam == 12 || dwParam == 0 || dwParam == 1 );	// 
				switch ( dwParam )
				{
				case 2:
					ixLcd = 0;
					break;
				case 3:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
				case 6:
					ixLcd = 3;
					break;
					//
				case 10:
					ixLcd = 0;
					break;
				case 5:
					ixLcd = 2;
					break;
				case 7:
					ixLcd = 3;
					break;
					//
				case 9:
					ixLcd = 0;
					break;
				case 0:
					ixLcd = 2;
					break;
				case 1:
					ixLcd = 3;
					break;
					// Blank Params
				case 11:
					ixLcd = 0;
					break;
				case 12:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 14:
					ixLcd = 3;
					break;
				}
				break;
			}
		}
		if ( KSM_PC_EQ == m_eActSectionMode ) 
		{
			// determine if this is on the current display row
			switch( m_wACTDisplayRow )
			{
			case 0:
				bOnScreen = ( dwParam == 4 || dwParam == 9 || dwParam == 13 || dwParam == 17 );	// LF Gain,LMF Gain, HMF Gain, HF Gain
				switch ( dwParam )
				{
				case 4:
					ixLcd = 0;
					break;
				case 9:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 17:
					ixLcd = 3;
					break;
					//
				case 5:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 14:
					ixLcd = 2;
					break;
				case 18:
					ixLcd = 3;
					break;
					//
				case 6:
					ixLcd = 0;
					break;
				case 11:
					ixLcd = 1;
					break;
				case 15:
					ixLcd = 2;
					break;
				case 19:
					ixLcd = 3;
					break;
					//
				case 3:
					ixLcd = 0;
					break;
				case 8:
					ixLcd = 1;
					break;
				case 12:
					ixLcd = 2;
					break;
				case 16:
					ixLcd = 3;
					break;
				}
				break;
			case 1:
				bOnScreen = ( dwParam == 5 || dwParam == 10 || dwParam == 14 || dwParam == 18 );	// LF freq, LMF freq, HMf freq, HF freq 
				switch ( dwParam )
				{
				case 4:
					ixLcd = 0;
					break;
				case 9:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 17:
					ixLcd = 3;
					break;
					//
				case 5:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 14:
					ixLcd = 2;
					break;
				case 18:
					ixLcd = 3;
					break;
					//
				case 6:
					ixLcd = 0;
					break;
				case 11:
					ixLcd = 1;
					break;
				case 15:
					ixLcd = 2;
					break;
				case 19:
					ixLcd = 3;
					break;
					//
				case 3:
					ixLcd = 0;
					break;
				case 8:
					ixLcd = 1;
					break;
				case 12:
					ixLcd = 2;
					break;
				case 16:
					ixLcd = 3;
					break;
				}
				break;
			case 2:
				bOnScreen = ( dwParam == 6 || dwParam == 11 || dwParam == 15 || dwParam == 19 );	// LF Q, LMF Q, HMf Q, HF Q
				switch ( dwParam )
				{
				case 4:
					ixLcd = 0;
					break;
				case 9:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 17:
					ixLcd = 3;
					break;
					//
				case 5:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 14:
					ixLcd = 2;
					break;
				case 18:
					ixLcd = 3;
					break;
					//
				case 6:
					ixLcd = 0;
					break;
				case 11:
					ixLcd = 1;
					break;
				case 15:
					ixLcd = 2;
					break;
				case 19:
					ixLcd = 3;
					break;
					//
				case 3:
					ixLcd = 0;
					break;
				case 8:
					ixLcd = 1;
					break;
				case 12:
					ixLcd = 2;
					break;
				case 16:
					ixLcd = 3;
					break;
				}
				break;
			case 3:
				bOnScreen = ( dwParam == 3 || dwParam == 8 || dwParam == 12 || dwParam == 16 );		// LF freq On/off, LMF freq On/off, HMf freq On/off, HF freq On/off
				switch ( dwParam )
				{
				case 4:
					ixLcd = 0;
					break;
				case 9:
					ixLcd = 1;
					break;
				case 13:
					ixLcd = 2;
					break;
				case 17:
					ixLcd = 3;
					break;
					//
				case 5:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 14:
					ixLcd = 2;
					break;
				case 18:
					ixLcd = 3;
					break;
					//
				case 6:
					ixLcd = 0;
					break;
				case 11:
					ixLcd = 1;
					break;
				case 15:
					ixLcd = 2;
					break;
				case 19:
					ixLcd = 3;
					break;
					//
				case 3:
					ixLcd = 0;
					break;
				case 8:
					ixLcd = 1;
					break;
				case 12:
					ixLcd = 2;
					break;
				case 16:
					ixLcd = 3;
					break;
				}
				break;
			}
		}
		if ( KSM_PC_EQ_P2 == m_eActSectionMode ) 
		{
			// EQ
			// determine if this is on the current display row
			switch( m_wACTDisplayRow )
			{

			case 0:
				bOnScreen = ( dwParam == 25 || dwParam == 26 || dwParam == 23 || dwParam == 22 );	// // HO freq, HF Shelf,LF Shelf, LP freq
				switch ( dwParam )
				{
				case 25:
					ixLcd = 0;
					break;
				case 26:
					ixLcd = 1;
					break;
				case 23:
					ixLcd = 2;
					break;
				case 22:
					ixLcd = 3;
					break;
					//
				case 7:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 20:
					ixLcd = 3;
					break;
					//
				case 24:
					ixLcd = 0;
					break;
				case 2:
					ixLcd = 1;
					break;
				case 21:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 27:
					ixLcd = 0;
					break;
				case 28:
					ixLcd = 1;
					break;
				case 29:
					ixLcd = 2;
					break;
				case 30:
					ixLcd = 3;
					break;
				}
				break;
			case 1:
				bOnScreen = ( dwParam == 7 || dwParam == 1 || dwParam == 29 || dwParam == 20 );	// HP slope, Type, Not Used , LP Slope
				switch ( dwParam )
				{
				case 25:
					ixLcd = 0;
					break;
				case 26:
					ixLcd = 1;
					break;
				case 23:
					ixLcd = 2;
					break;
				case 22:
					ixLcd = 3;
					break;
					//
				case 7:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 20:
					ixLcd = 3;
					break;
					//
				case 24:
					ixLcd = 0;
					break;
				case 2:
					ixLcd = 1;
					break;
				case 21:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 27:
					ixLcd = 0;
					break;
				case 28:
					ixLcd = 1;
					break;
				case 29:
					ixLcd = 2;
					break;
				case 30:
					ixLcd = 3;
					break;
				}
				break;
			case 2:
				bOnScreen = ( dwParam == 27 || dwParam == 28 || dwParam == 29 || dwParam == 30 );	// 
				switch ( dwParam )
				{
				case 25:
					ixLcd = 0;
					break;
				case 26:
					ixLcd = 1;
					break;
				case 23:
					ixLcd = 2;
					break;
				case 22:
					ixLcd = 3;
					break;
					//
				case 7:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 20:
					ixLcd = 3;
					break;
					//
				case 24:
					ixLcd = 0;
					break;
				case 2:
					ixLcd = 1;
					break;
				case 21:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 27:
					ixLcd = 0;
					break;
				case 28:
					ixLcd = 1;
					break;
				case 29:
					ixLcd = 2;
					break;
				case 30:
					ixLcd = 3;
					break;

				}
				break;

			case 3:
				bOnScreen = ( dwParam == 24 || dwParam == 2 || dwParam == 21 || dwParam == 0 );	// HP On/off , Gloss, On/Off, LPF On/off
				switch ( dwParam )
				{
				case 25:
					ixLcd = 0;
					break;
				case 26:
					ixLcd = 1;
					break;
				case 23:
					ixLcd = 2;
					break;
				case 22:
					ixLcd = 3;
					break;
					//
				case 7:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 20:
					ixLcd = 3;
					break;
					//
				case 24:
					ixLcd = 0;
					break;
				case 2:
					ixLcd = 1;
					break;
				case 21:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 27:
					ixLcd = 0;
					break;
				case 28:
					ixLcd = 1;
					break;
				case 29:
					ixLcd = 2;
					break;
				case 30:
					ixLcd = 3;
					break;
				}
				break;
			}

		}
		
		if ( KSM_PC_SAT == m_eActSectionMode ) 
		{
			// determine if this is on the current display row
			switch( m_wACTDisplayRow )
			{
			case 0:
				bOnScreen = ( dwParam == 3 || dwParam == 1 || dwParam == 4 || dwParam == 12 );
				switch ( dwParam )
				{
				case 3:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
					//
				case 2:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 9:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 11:
					ixLcd = 2;
					break;
				case 12:
					ixLcd = 3;
					break;
				}
				break;
			case 1:
				bOnScreen = ( dwParam == 9 || dwParam == 10 || dwParam == 11 || dwParam == 12 );	// 
				switch ( dwParam )
				{
				case 3:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;

				case 2:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 9:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 11:
					ixLcd = 2;
					break;
				case 12:
					ixLcd = 3;
					break;


				}
				break;
			case 2:
				bOnScreen = ( dwParam == 9 || dwParam == 10 || dwParam == 11 || dwParam == 12 );	// 
				switch ( dwParam )
				{
				case 3:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
					//
				case 2:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 9:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 11:
					ixLcd = 2;
					break;
				case 12:
					ixLcd = 3;
					break;

				}
				break;
			case 3:
				bOnScreen = ( dwParam == 9 || dwParam == 10 || dwParam == 2 || dwParam == 0 );	// enable
				switch ( dwParam )
				{
				case 3:
					ixLcd = 0;
					break;
				case 1:
					ixLcd = 1;
					break;
				case 4:
					ixLcd = 2;
					break;
					//
				case 2:
					ixLcd = 2;
					break;
				case 0:
					ixLcd = 3;
					break;
					// Blank Params
				case 9:
					ixLcd = 0;
					break;
				case 10:
					ixLcd = 1;
					break;
				case 11:
					ixLcd = 2;
					break;
				case 12:
					ixLcd = 3;
					break;
				}
				break;
			}

		}

	}	
	else if ( pParam->IsAlwaysChanging() && // IsAlwaysChanging is set only for the send controls in the ACT section, while an assignable send param has this off
													(param == MIX_PARAM_SEND_VOL || 
													param == MIX_PARAM_SEND_PAN ||
                                       param == MIX_PARAM_SURROUND_SENDANGLE ||
													param == MIX_PARAM_SEND_PREPOST || 
													param == MIX_PARAM_SEND_ENABLE ) )
	{
		if ( KSM_SEND == m_eActSectionMode )
		{
			ixLcd = dwParam % 4;
			switch( m_wACTDisplayRow )
			{
			case 0:
				bOnScreen = param == MIX_PARAM_SEND_VOL;
				break;
			case 1:
				bOnScreen = ((param == MIX_PARAM_SEND_PAN) || (param == MIX_PARAM_SURROUND_SENDANGLE));
				break;
			case 2:
				bOnScreen = param == MIX_PARAM_SEND_PREPOST;
				break;
			case 3:
				bOnScreen = param == MIX_PARAM_SEND_ENABLE;
				break;
			}
		}
	}
	//else if ( SDM_Layers == m_eDisplayMode && ( param == MIX_PARAM_LAYER_MUTE || param == MIX_PARAM_LAYER_SOLO ) )
	else if ( SDM_Layers == m_eDisplayMode )
	{
		if ( param == MIX_PARAM_LAYER_MUTE || param == MIX_PARAM_LAYER_SOLO )
			ixLcd = (int) pParam->GetParamNum() + 4;	// +4 because strips come after the act section
	}
	else if ( SDM_ChannelBranch == m_eDisplayMode && pParam->GetStripPhysicalIndex() != 0 )
	{
		if ( pParam->GetMixerStrip() == MIX_STRIP_BUS )
			ixLcd = (int) pParam->GetStripNum() + 4;
		else
			ixLcd = (int) pParam->GetStripPhysicalIndex() + 4;

		if ( param == MIX_PARAM_SEND_PAN )
			bOnScreen = true;
	}
	else
	{
		// some strip parameter
		ixLcd = (int) pParam->GetStripPhysicalIndex() + 4;	// +4 because strips come after the act section

		// Determine which param is visible
		// Normally we show the Assigned encoder param
		// However if we're flipped, we would show the fader param (Vol).
		// However However, if m_bShowFaderOnLCD is true, we show whatever the fader is controlling

		PSEncoderParams pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, getModeForRotary() );
		SONAR_MIXER_PARAM paramToMatch = pEnc ? pEnc->mixParam : MIX_PARAM_PAN;
		if ( ( m_bShowFaderOnLCD && !bFlipped ) || ( bFlipped && !m_bShowFaderOnLCD ) )
			if ( pParam->GetStripPhysicalIndex() != 9 ) // don't flip master fader
				paramToMatch = MIX_PARAM_VOL;

		if ( param == paramToMatch )
			bOnScreen = true;
	}
	*pixLcd = ixLcd;

	return bOnScreen;
}



////////////////////////////////////////////////////////////
// Given a Mix param,  determine its LCD index and if it is 
// actively on screen.  Optionally force it on the screen with
// bTouched
void CTacomaSurface::showParam( CMixParam* pParam, ParamShowMode eShowMode )
{
	int ixLcd = -1;
	bool bOnScreen = getLCDIndex( pParam, m_bFlipped, &ixLcd );

	if ( bOnScreen || PSM_Touch == eShowMode || PSM_Revert == eShowMode )
		showParam( pParam, ixLcd, eShowMode );
}


static DWORD s_mtbd = 100;


//////////////////////////////////////////////////////////////////
// Given a mix param and an index, display it in the lcd
void CTacomaSurface::showParam( CMixParam* pParam, int ixLcd, ParamShowMode eShowMode, bool bForceUpdate /* false */ )
{
	// on a param by param basis, throttle the sending of text
	if ( pParam->IsThrottle() )
	{
		DWORD dwTick = ::GetTickCount();
		ParamDisplayTimeStampMap::iterator it = m_mapParamDisplayTimeStamp.find( pParam );
		if ( it != m_mapParamDisplayTimeStamp.end() )
		{
			DWORD dwts = it->second;
			if ( dwTick - dwts < s_mtbd )
				return;	// too soon
		}

		m_mapParamDisplayTimeStamp[pParam] = dwTick;
	}

	// Get the Param name and value
	char szVal[64];	// non-unicode always
	char szName[9];
	*szName = '\0';
	*szVal = '\0';
	DWORD dwLen = _countof(szName) - 1;

	if ( pParam->CrunchSize() )
	{
		dwLen = pParam->CrunchSize();
		ASSERT( dwLen < 64 );
		if ( SUCCEEDED( pParam->GetCrunchedParamLabel( szName, dwLen ) ) )
			if ( pParam->GetCrunchedValueText( szVal, pParam->CrunchSize() ) == E_NOTIMPL )
				if (( pParam->GetMixerParam() == MIX_PARAM_PAN ) || ( pParam->GetMixerParam() == MIX_PARAM_SEND_PAN ))
					sprintf(szVal, "Srrnd");
	}
	else if ( SUCCEEDED( pParam->GetParamLabel( szName, &dwLen ) ) )
	{
		if ( pParam->GetParamNum() == 65536 && ( m_eActSectionMode == KSM_PC_EQ_P2 ) ) //special handling for Pro Channel EQ Styles Param
		{
			float fval = 0.0; 
			dwLen = _countof( szVal ) - 1;
			ASSERT( dwLen < 64 );
			pParam->GetVal( &fval );
			if ( fval < 0.5 )
				sprintf(szVal, "Pure");
			else if ( fval > 0.5  && fval < 1.0)
				sprintf(szVal, "Vintage");
			else if ( fval == 1.0 )
				sprintf(szVal, "Modern");
		}
		else
		{
			dwLen = _countof( szVal ) - 1;
			ASSERT( dwLen < 64 );
			pParam->GetValueText( szVal, &dwLen );
		}
	} 

	// if this is one of the 8strip params, cram it all on one line
	if ( ( ixLcd < 4 || eShowMode == PSM_Touch ) && (pParam->DisplayName() || bForceUpdate))
	{
		// else the eq/act section uses two lines
		showText( szName, ixLcd, 0 );	// top row
	}
	else if ( PSM_Revert == eShowMode )
		showText( "Revert", ixLcd, 0 );

	// value on bottom row
	if ( pParam->DisplayValue() )
		showText( szVal, ixLcd, 1 );	// bottom row
}

void CTacomaSurface::showEQParams( ParamShowMode eShowMode )
{
	// on a param by param basis, throttle the sending of text
	DWORD dwTick = ::GetTickCount();
	ParamDisplayTimeStampMap::iterator it = m_mapParamDisplayTimeStamp.find( m_pEqTypeParams[ 0 ] );
	if ( it != m_mapParamDisplayTimeStamp.end() )
	{
		DWORD dwts = it->second;
		if ( dwTick - dwts < s_mtbd )
			return;	// too soon
	}
	m_mapParamDisplayTimeStamp[ m_pEqTypeParams[ 0 ] ] = dwTick;

	// Get the Param name and value
	char szName[9];
	*szName = '\0';
	const DWORD dwLen = _countof(szName) - 1;

	if ( eShowMode == PSM_Refresh )
	{
		float f = 0.0f;
		for ( DWORD dwIndex = 0; dwIndex < 4; dwIndex ++ )
		{
			m_pEqTypeParams[ dwIndex ]->GetVal( &f );
			// EQ - display the shape type instead of "Band 1-4"
			m_pEqTypeParams[ dwIndex ]->GetCrunchedValueText( szName, 7 );
			showText( szName, dwIndex, 0 );	// top row
		}
	}
}

//------------------------------------------------------------------
// Get list of Plug-ins on current sel strip and display them across all LCDs
void CTacomaSurface::showExistingPlugins()
{
	// get the current list for this strip
	std::vector<AnsiString> vec;
   GetPluginListForStrip( m_selectedStrip.stripType, m_selectedStrip.stripIndex, &vec );
	size_t cPlugs = vec.size();
	cPlugs = min( 8, cPlugs );
	int ixRow = 0;
	int ixCol = 0;
	for ( size_t i = 0; i < cPlugs; i++ )
	{
		ixCol = (int(i)/2) * 2;
		ixRow = i % 2;
		char* psz = const_cast<char*>(vec[i].c_str());	// non const alias
		psz[15] = ' ';	// end with a space
		showText(psz, (int)(4 + ixCol), ixRow, NumPluginChar );

      ControlMessageMapIterator it = m_mapControlMsgs.find( (ControlId) ( BID_Arm0 + i ) );
	   if ( it != m_mapControlMsgs.end() )
	   {
		   CMidiMsg* pmsg = it->second;
         stopBlink( (ControlId) ( BID_Arm0 + i ) );
			pmsg->Send( 1.f );
      }
	}

	// now blank out the rest
	for ( size_t i = cPlugs; i < 8; i++ )
	{
		ixCol = (int(i)/2) * 2;
		ixRow = i % 2;
		showText( "", 4 + ixCol , ixRow, NumPluginChar );

      ControlMessageMapIterator it = m_mapControlMsgs.find( (ControlId) ( BID_Arm0 + i ) );
	   if ( it != m_mapControlMsgs.end() )
	   {
		   CMidiMsg* pmsg = it->second;
         if ( i == cPlugs )
            startBlink( (ControlId) ( BID_Arm0 + i ) );
			else
            pmsg->Send( 0.f );
      }
	}

	if ( 0 == cPlugs )
		showText( "No Plugins", 4, 0, NumPluginChar );
}


//-------------------------------------------------------------------
// Display the current child indexes into the Plugin Tree structure
void CTacomaSurface::showAvailablePlugins()
{
	if ( !m_pPluginTree )
		return;

	size_t iLCDSkip = NumPluginChar/8;

	PLUGINS* pNode = m_pPluginTree;

	size_t iLcd = 0;
	for ( iLcd =0; iLcd < NumPluginDesc; iLcd++ )
	{
		int ixChild = m_aPluginChildIndex[iLcd];
		if ( pNode->cChildren == 0 )
			break;
		if ( ixChild >= (int)pNode->cChildren )
		{
			ASSERT(0);
			break;
		}
		pNode = pNode->apChildren[ixChild];
		if ( !pNode )
		{
			ASSERT(0);
			break;
		}
		if ( pNode->cChildren > 0 )
		{
			char sz[NumPluginChar+1];
			::strncpy( sz, pNode->szName, NumPluginChar );
			if ( ::strlen(sz) < NumPluginChar-1 )
				::strcat( sz, ">" );
			else
				sz[15] = '>';
			showText( sz, 4 + int(iLcd * iLCDSkip), 0, NumPluginChar );
		}
		else
			showText( pNode->szName, 4 + int(iLcd * iLCDSkip), 0, NumPluginChar );
	}

	// blank out the rest
	for ( size_t i = iLcd * iLCDSkip; i < 8; i++ )
		showText( "", 4 + int(i), 0, 8, true );

	// and the lower row
	for ( size_t i = 0; i < 8; i++ )
		showText("", 4 + int(i), 1, 8, true );
}




static BYTE s_byBuf[128];	// sysx buffer

///////////////////////////////////////////////////////////////////////////////
// Raw primitive to output a particular 8 char area on the LCD.  You provide
// the string, the LCD index (strip index) and the Row Index (0=top, 1=bot)
void CTacomaSurface::showText( LPCSTR pszIn, int ixLcd, int ixRow, 
											size_t cChars, //= 8
											bool bForce )	//= false
{
	if ( ixLcd < 0 || ixLcd > 12 )
		return;
	if ( ixRow < 0 || ixRow > 1 )
		return;

//	CString strDbg(pszIn );
//	TRACE( _T("showing lcd ix %d: %s\n"), ixLcd, strDbg );
//	if ( ixLcd == 12 )
//		ixLcd = ixLcd;	// breakpointable no-op

	char psz[128];
	cChars = min( cChars, 128 );
	::strncpy( psz, pszIn, cChars );
	psz[cChars] = '\0';

	// pad out with blanks
	size_t cLen = ::strlen( pszIn );
	for ( size_t ic = cLen; ic < cChars; ic++ )
		psz[ic] = ' ';

	// compare against the cache for this row/col to see if we
	// should send
	if ( !bForce && 0 == ::memcmp( (LPBYTE)&m_aaLCDChars[ixRow][ixLcd*8], (LPBYTE)psz, cChars ) )
		return;

	// set in the cache
	::memcpy( (LPBYTE)&m_aaLCDChars[ixRow][ixLcd*8], (LPBYTE)psz, cChars );

	MAYBE_TRACE( "showText \"%s\" ixLcd:%d ixRow:%d\n", psz, ixLcd, ixRow );

	BYTE byCS = 0;

	// Determine the Display Addresses based on the lcd column (index)
	BYTE byMSB = 0;
	if ( ixLcd > 7 )
		byMSB = 0x08;
	else if ( ixLcd > 3 )
		byMSB = 0x04;

	BYTE byLSB = (ixLcd % 4) * 8;
	// special case
	if ( ixLcd == 12 )	// master
		byLSB = 0x20;

	// offset for bottom row
	if ( 1 == ixRow )
		byLSB += 0x40;		// always offset by this much (thank you)

	cLen = ::strlen( psz );
	cLen = min( cLen, 64 );

	//size_t cb = s_cbPrefix;
	//cb += 2;			// address bytes
	//cb += 1;			// checksum
	//cb += 1;			// the 0xF7
	//cb += cLen;		// the data itself
	const size_t cb = s_cbPrefix + 4 + cLen;	// total of the above items

	BYTE* pFill = s_byBuf + s_cbPrefix;

	::memcpy( s_byBuf, s_abyPrefix, s_cbPrefix );

	*pFill++ = byMSB;		// add the addressing bytes
	byCS += byMSB;
	*pFill++ = byLSB;
	byCS += byLSB;

	// the data itself
	for ( DWORD i = 0; i < cLen; i++ )
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

	// If testing on a mackie, this really makes the hardware angry
	if ( !m_bOnMackie )
		MidiOutLongMsg( (DWORD)cb, s_byBuf );
}


static DWORD s_dwLastLED[10] = {0,0,0,0,0,0,0,0,0,0};

///////////////////////////////////////////////////////////////////
// Given an mfx time, show it on the seven segment led
void	CTacomaSurface::outputTime( MFX_TIME& time )
{
	if ( time.timeFormat != m_mfxfPrimary )
	{
		ASSERT(0);	/// why god? why?
		return;
	}

	CString str;

	if ( time.timeFormat == TF_MBT )
	{
		str.Format( _T("%3d%2d  %03d"), time.mbt.nMeas, time.mbt.nBeat, time.mbt.nTick );
	}
	else if ( time.timeFormat == TF_SMPTE )
	{
		str.Format( _T(" %02d%02d%02d %02d"), time.smpte.nHour, time.smpte.nMin, time.smpte.nSec, time.smpte.nFrame );
	}
	else
	{
		ASSERT(0);
		return;
	}

	for ( DWORD i = 0; i < 10; i++ )
	{
		m_msgSevenSegLed.SetCCNum( 0x49 - i );

		DWORD dwVal = 0;
		TCHAR cVal = str.GetAt( i );
		if (cVal >= 0x21 && cVal <= 0x3F)
			dwVal = cVal;
		else if (cVal >= 0x40 && cVal <= 0x5F)
			dwVal = cVal - 0x40;
		else
			dwVal = 0x20;	// Space

		if ( dwVal == s_dwLastLED[i] )
			continue;

		s_dwLastLED[i] = dwVal;

		m_msgSevenSegLed.Send( dwVal );
	}
}

void CTacomaSurface::showIoCtrlText()
{
	if ( !m_bIOMode )
		return;

	PSEncoderParams const pEnc = GetEncoderListParam( m_dw8EncoderParam, MIX_STRIP_ANY, Enc_IO );
	if ( !pEnc )
		return;

	CString str = getIoParamString( pEnc->ioParam, FALSE );
	char sz[32];
	memset( sz, 0, sizeof ( sz ) );
	int nBytes = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, (LPCTSTR) str, str.GetLength(), sz, 32, NULL, NULL );
	showText( sz, 12, 0 );
	showText( "Mic Gain", 12, 1 );
}
