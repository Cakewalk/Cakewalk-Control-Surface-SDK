#include "stdafx.h"
#include "IOBoxInterface.h"
#include "Tacomasurface.h"
#include "TacomaSurfacePropPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR s_pszConsole = _T("console (vs-700c)");

// Todo: string match the actual port name
static LPCTSTR s_pszPortMatch1 = _T("io (vs-700)");
static LPCTSTR s_pszPortMatch2 = _T("io (2-vs-700)");
static LPCTSTR s_pszPortMatch3 = _T("ctrl (octa-capture)");
static LPCTSTR s_pszPortMatch4 = _T("ctrl (exp octa-capture)");
  
// friendly String Names to go along with port match
static LPCTSTR s_pszConsolePort = _T("VS-700C");

static LPCTSTR s_pszPort1 = _T("VS-700R");
static LPCTSTR s_pszPort2 = _T("VS-700R2");
static LPCTSTR s_pszPort3 = _T("OCTA-CAPTURE");
static LPCTSTR s_pszPort4 = _T("OCTA-CAPTURE EXP");

static LPCTSTR s_pszPortNo = _T("NO IO PORT");


static bool s_bTraceSysx = false;

using namespace TacomaIOBox;

//////////////////////////////////////////////////////////////////////
// SYSX Prefix String
static BYTE s_abyPrefix[] =
{
0xF0,		//	Exclusive status
0x41,		// ID number (Roland)
0x10,		// dev	Device ID (dev: 00H - 1FH, 7FH)   
0x00,		// model ID 00 (JP1 I/O)
0x00,		// model ID 00 (JP1 I/O)
0x32,		// model ID 32 (JP1 I/O)
};

// SYSX Prefix String
static BYTE s_abyPrefixOC[] =
{
0xF0,		//	Exclusive status
0x41,		// ID number (Roland)
0x10,		// dev	Device ID (dev: 00H - 1FH, 7FH)   
0x00,		// model ID 00 (JP1 I/O)
0x00,		// model ID 00 (JP1 I/O)
0x4D,		// model ID 4D (JP1 I/O)
};

// Data Request 1	RQ1 (11H)  
// This message requests the other device to transmit data. 
// The address and size indicate the type and amount of data that is requested.
// When a Data Request message is received, if the device is in a state in 
// which it is able to transmit data, and if the address and size are appropriate, 
// the requested data is transmitted as a Data Set 1 (DT1) message. 
// If the conditions are not met, nothing is transmitted.




CIOBoxInterface::CIOBoxInterface( CTacomaSurface* pSurface) :
	m_pSurface( pSurface ),
	m_hSysx(NULL ),
	m_persistPreamp(this),
	m_persistDM( this ),
	m_nActiveIOBox(0)
{
	m_aIOBoxPorts[0] = m_aIOBoxPorts[1] = m_aIOBoxPorts[2] = m_aIOBoxPorts[3] =  (UINT)-1;
	//m_viofacestr.push_back( s_pszPortNo );

	m_hSysx = ::CreateEvent( NULL, FALSE, FALSE, _T("sysx") );

	// create the MIDI source object
	m_pMidiSource = new CMidiSource( m_pSurface->GetTimer() );

	// Create the MIDI outs object
	m_pMidiOuts = new CMidiOuts();
	m_pMidiOuts->Initialize();

	// Try to open the special ports we care about
	findPorts();
	// ASSERT( HasMidiIO() );

	m_pMidiSource->Initialize();
	m_pMidiSource->AddSink( this );
	
	initParams();

	GetInitialValues();

	m_byDirectMix = 0x0;

	m_byRevType = 0x0;

	m_bMidiVerb = false;

	m_bMidiPatch = false;
	
	m_bLinked = false;

	m_bConsoleFader = false;
}

CIOBoxInterface::~CIOBoxInterface(void)
{
	m_pMidiSource->RemoveSink( this );
	delete m_pMidiSource;
	delete m_pMidiOuts;
	::CloseHandle( m_hSysx );
	m_hSysx = NULL;
}

//////////////////////////////////////////////////////////
// Find the IO box midi ports by name and add them to
// the MIDI IO Port containers
void CIOBoxInterface::findPorts()
{
	UINT u = 0;
	m_pMidiSource->GetDeviceCount( &u );
	CString str;

	if ( u > 0)
		m_viofacestr.clear();

	bool bFindConsole = false;
	for ( UINT i = 0; i < u; i++ )
	{
		m_pMidiSource->GetDeviceName( i, &str );
		str.MakeLower();
		bool bFind1 = str.Find( s_pszPortMatch1 ) != -1;
		bool bFind2 = str.Find( s_pszPortMatch2 ) != -1;
		bool bFind3 = str.Find( s_pszPortMatch3 ) != -1;
		bool bFind4 = str.Find( s_pszPortMatch4 ) != -1;
		bFindConsole |= (str.Find( s_pszConsole ) != -1);

		if ( bFind1 || bFind2 || bFind3 || bFind4 )
		{
			// found it
			m_pMidiSource->AddInPort( i );
			if ( bFind1 )
			{
				m_aIOBoxPorts[0] = m_pMidiSource->GetPortCount() - 1;
				m_viofacestr.push_back( s_pszPort1 );
			}
			else if ( bFind2 )
			{
				m_aIOBoxPorts[1] = m_pMidiSource->GetPortCount() - 1;
				m_viofacestr.push_back( s_pszPort2 );
			}
			else if ( bFind3 )
			{
				m_aIOBoxPorts[2] = m_pMidiSource->GetPortCount() - 1;
				m_viofacestr.push_back( s_pszPort3 );
			}

			else if ( bFind4 )
			{
				m_aIOBoxPorts[3] = m_pMidiSource->GetPortCount() - 1;
				m_viofacestr.push_back( s_pszPort4 );
			}		
		}
	}

	if ( m_viofacestr.empty() )
	{
		if (bFindConsole)
			m_viofacestr.push_back( s_pszConsolePort );
		else
			m_viofacestr.push_back( s_pszPortNo );
	}

	m_pMidiOuts->GetDeviceCount( &u );
	for ( UINT i = 0; i < u; i++ )
	{
		m_pMidiOuts->GetDeviceName( i, &str );
		str.MakeLower();
		bool bFind1 = str.Find( s_pszPortMatch1 ) != -1;
		bool bFind2 = str.Find( s_pszPortMatch2 ) != -1;
		bool bFind3 = str.Find( s_pszPortMatch3 ) != -1;
		bool bFind4 = str.Find( s_pszPortMatch4 ) != -1;
		if ( bFind1 || bFind2 || bFind3 || bFind4  )
		{
			// found it
			m_pMidiOuts->AddOutPort( i );
		}
	}
}


/////////////////////////////////////////////////////////////////////
//
bool	CIOBoxInterface::HasMidiIO()
{
	return m_pMidiSource->GetPortCount() > 0  &&  m_pMidiOuts->GetPortCount() > 0;
}

/////////////////////////////////////////////////////////////////////
//
bool	CIOBoxInterface::HasConsolePorts()
{
	if (m_viofacestr.empty())
	{
		ASSERT( false );
		return false;
	}
	
	return 0 == m_viofacestr[ 0 ].Compare( s_pszConsolePort );
}

/////////////////////////////////////////////////////////////////////
//
BOOL CIOBoxInterface::OnMidiShortMsg( DWORD dwMsg, DWORD dwTime, UINT nPort )
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
// Crack the sysx and set our internal state variables
BOOL CIOBoxInterface::OnMidiLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg, UINT nPort )
{
	if ( s_bTraceSysx )
	{
		CString str;
		TRACE( _T("\nRecving Sysx\n") );
		for ( DWORD i = 0; i < cbLongMsg; i++ )
		{
			if ( i )
				str.Format( _T(",%02x"), pbLongMsg[i] );
			else
				str.Format( _T("%02x"), pbLongMsg[i] );
			TRACE( str );
		}
	}

	//	int s = 0;
	//	if ( 0 == pbLongMsg[7] && 1 == pbLongMsg[8] && 0 == pbLongMsg[9] && 0 == pbLongMsg[10] )
	//		s = s;	// breakpoint for channel 0

	UINT ixIOBox = IOBoxIndexFromPort( nPort );	// which box is this from?

	::SetEvent( m_hSysx );	// got one

	//	TRACE( _T("OnMidiLong %d Bytes at %d\n"), cbLongMsg, ::GetTickCount() );

	BYTE* pby = new BYTE[cbLongMsg];
	//	BYTE pby[255];

	::memcpy( pby, pbLongMsg, cbLongMsg );

	// determine identity.  What we're expecting here is a chunk of sysx for an entire
	// IO channel or IO Compressor

	// address bytes are in 7,8,9,10 (we assert for this elsewhere)
	BYTE abyAddr[4];
	::memcpy( abyAddr, pby + 7, 4 );
	if ( (m_viofacestr[ ixIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixIOBox ] == (_T("VS-700R2"))))
	{
		// Look for the channel.
		// quick check to determine if Compressor block
		const bool bComp = (abyAddr[1] == 0x03);
		bool bSystem = abyAddr[0] == 0 && (abyAddr[1] == 0x00) && (abyAddr[2] == 0x02) && abyAddr[3] == 0;
		const bool bDMOut = (abyAddr[1] == 0x02);
		if ( bSystem )
			bSystem = bSystem;	// breakpoint

		if ( bComp )
		{
			TacomaIOBox::CompressorChannel* pChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixIOBox][iChan];
				if ( 0 == ::memcmp( cmpc.abyAddress, abyAddr, 4 ) )
				{
					pChan = &cmpc;		// found it
					break;
				}
			}
			if ( pChan )
			{
				// data starts at BYTE 11, for 5 bytes
				BYTE* pbyData = pby + 11;
				size_t c = valueFromBytes( &pChan->paramThreshold, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramRatio, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramAttack, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramRelease, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramMakeupGain, pbyData );
				pbyData += c;
			}
		}
		else if ( bSystem )
		{
			// data starts at BYTE 11, for 4 bytes
			BYTE* pbyData = pby + 11;
			size_t c = valueFromBytes( &m_System[ixIOBox].paramUnitNumber, pbyData );
			pbyData += c;

			c = valueFromBytes( &m_System[ixIOBox].paramSamplingRate, pbyData );
			pbyData += c;

			c = valueFromBytes( &m_System[ixIOBox].paramSyncSource, pbyData );
			pbyData += c;

			c = valueFromBytes( &m_System[ixIOBox].paramDigitalInput, pbyData );
			pbyData += c;
		}
		else if ( bDMOut )
		{
			// data starts at BYTE 11, for 3 bytes
			BYTE* pbyData = pby + 11;
			size_t c = valueFromBytes( &m_DMOutput[ixIOBox].paramMainOut, pbyData );
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramSubOut, pbyData );
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramDigOut, pbyData );
			pbyData += c;
		}
		else
		{
			TacomaIOBox::TacomaIOChannel* pChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixIOBox][iChan];
				if ( 0 == ::memcmp( ioc.abyAddress, abyAddr, 4 ) )
				{
					pChan = &ioc;		// found it
					break;
				}
			}
			if ( pChan )
			{
				// we have a chan, now extract the values out of the sysx.
				// data starts at BYTE 11, for 25 bytes
				BYTE* pbyData = pby + 11;

				size_t c = valueFromBytes( &pChan->paramPhantom, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramLoCut, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramPhase, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramPad, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramGain, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramStereLink, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramCompEnable, pbyData );
				pbyData += c;

				// skip over reserved
				pbyData += 4;
				c = valueFromBytes( &pChan->paramDMixMono, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramDMixSolo, pbyData );
				pbyData += c;

				c = valueFromBytes( &pChan->paramDMixMute, pbyData );
				pbyData += c;

				// skip over reserved
				c = valueFromBytes( &pChan->paramDMixPan, pbyData );
				pbyData += c;

				// skip over reserved
				valueFromBytes( &pChan->paramDMixVol, pbyData );
				pbyData += c;
			}
			else
				ASSERT(0);
		}

		delete [] pby;

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if ( (m_viofacestr[ ixIOBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixIOBox ] == (_T("OCTA-CAPTURE EXP"))))
	{
		// Look for the channel.
		// quick check to determine if Compressor block
		const bool bComp = ( ( abyAddr[1] == 0x05 ) && ( abyAddr[3] > 0x06) );
		const bool bOctaDM = ( ( abyAddr[1] == 0x06 ) || ( abyAddr[1] == 0x07) );
		bool bSystemDump = abyAddr[0] == 0x01 && (abyAddr[1] == 0x00) && (abyAddr[2] == 0x00) && abyAddr[3] == 0x00;
		bool bSystem = abyAddr[0] == 0x00 && (abyAddr[1] == 0x02);
		const bool bDMOut = ((abyAddr[1] == 0x03));// || (( ( abyAddr[1] == 0x08 ) && ( abyAddr[3] == 0x07) )) );
		const bool bReverb = ( abyAddr[1] == 0x04 );
		const bool bMasterIn = ( abyAddr[1] == 0x08 );
		const bool bMasterOut = ( abyAddr[1] == 0x09 );

		if ( bSystemDump )
			bSystemDump = bSystemDump;	// breakpoint

		if ( bSystem )
			bSystem = bSystem;	// breakpoint

		if ( bComp )
		{
			TacomaIOBox::CompressorChannel* pChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixIOBox][iChan];
				//f ( 0 == ::memcmp( cmpc.abyAddress, abyAddr, 4 ) )
				if (cmpc.abyAddress[2] == abyAddr[2] )
				{
					pChan = &cmpc;		// found it
					break;
				}
			}
			if ( pChan )
			{
				// data starts at BYTE 11, for 6 bytes
				BYTE* pbyData = pby + 11;
				size_t c = 0;
				switch(abyAddr[3] )
				{
				case 0x07:
					c = valueFromBytes( &pChan->paramGate, pbyData );
					break;
				case 0x08:
					c = valueFromBytes( &pChan->paramAttack, pbyData );
					break;
				case 0x09:
					c = valueFromBytes( &pChan->paramRelease, pbyData );
					break;
				case 0x0A:
					c = valueFromBytes( &pChan->paramThreshold, pbyData );
					break;
				case 0x0B:
					c = valueFromBytes( &pChan->paramRatio, pbyData );
					break;
				case 0x0C:
					c = valueFromBytes( &pChan->paramMakeupGain, pbyData );
					break;
				default:
					break;
				}


			}
		}
		//-----------------------------------------------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------------------------------------------
		//-----------------------------------------------------------------------------------------------------------------------------------
		else if ( bSystemDump )
		{
			BYTE MSN = 0x00;
			BYTE LSN = 0x00;

			// data starts at BYTE 11
			BYTE* pbyData = pby + 14;
			//size_t c = valueFromBytes( &m_System[ixIOBox].paramUnitNumber, pbyData );
			//pbyData += c;

			//  c = valueFromBytes( &m_System[ixIOBox].paramSamplingRate, pbyData );
			//pbyData += c;

			//SYSTEM

			size_t c = valueFromBytes( &m_System[ixIOBox].paramSyncSource, pbyData );
			m_fSyncVal = m_System[ixIOBox].paramSyncSource.f01Val;
			pbyData +=c;

			c = valueFromBytes( &m_System[ixIOBox].paramDirectMix, pbyData );
			m_byDirectMix =(BYTE)m_System[ixIOBox].paramDirectMix.f01Val;
			pbyData += c;

			//PATCHBAY
			c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut12, pbyData );
			m_iOUT12 =(int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut34, pbyData );
			m_iOUT34 = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut56, pbyData );
			m_iOUT56 = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut78, pbyData );
			m_iOUT78 = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut910, pbyData );
			m_iOUT910 = (int)*pbyData;
			pbyData += c;

			//REVERB

			//TYPE
			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevType, pbyData );
			m_byRevType = (int)*pbyData;
			pbyData += c;

			//Delay
			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevDelay, pbyData );
			m_iOFF_D = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevDelay, pbyData );
			m_iECHO_D = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevDelay, pbyData );
			m_iROOM_D = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevDelay, pbyData );
			m_iSHALL_D = (int)*pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevDelay, pbyData );
			m_iLHALL_D = (int)*pbyData;
			pbyData += c;

			//Time
			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevTime, pbyData );
			m_iOFF_T = *pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevTime, pbyData );
			m_iECHO_T = *pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevTime, pbyData );
			m_iROOM_T = *pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevTime, pbyData );
			m_iSHALL_T = *pbyData;
			pbyData += c;

			c = valueFromBytes( &m_DMOutput[ixIOBox].paramRevTime, pbyData );
			m_iLHALL_T = (int)*pbyData;
			pbyData += c;

			//Reading Sysex dump starting at 21st Position for Current Param states of OCTA-CAPTURE

			/// COMPRESSOR 

			//Read two Bits and combine their nibles into 1BYTE, Place in bitfields Struct for easy readability.
			//Use Array of Bytes to store value in Bytes.
			MSN = *pbyData; //offset of 21 from DATA Byte.
			pbyData++;
			LSN = *pbyData;
			BYTE byPhantomBits =  (LSN << 4) | MSN;
			CHAN_NIBS* pPhantom = (CHAN_NIBS*)&byPhantomBits;
			BYTE byPhatom[] = {pPhantom->chan1, pPhantom->chan2, pPhantom->chan3, pPhantom->chan4 ,pPhantom->chan5, pPhantom->chan6, pPhantom->chan7, pPhantom->chan8 };

			//next two BYTES
			pbyData++; //+23
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLocutBits =  (LSN << 4) | MSN;
			CHAN_NIBS* pLocut = (CHAN_NIBS*)&byLocutBits;
			BYTE byLocut[] = {pLocut->chan1, pLocut->chan2, pLocut->chan3, pLocut->chan4 ,pLocut->chan5, pLocut->chan6, pLocut->chan7, pLocut->chan8 };

			//next two BYTES
			pbyData++;//+25
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byPhaseBits =  (LSN << 4) | MSN;
			CHAN_NIBS* pPhase = (CHAN_NIBS*)&byPhaseBits;
			BYTE byPhase[] = {pPhase->chan1, pPhase->chan2, pPhase->chan3, pPhase->chan4 ,pPhase->chan5, pPhase->chan6, pPhase->chan7, pPhase->chan8 };

			//next two BYTES
			pbyData++;//+27
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byHizBits =  (LSN << 4) | MSN;
			CHAN_NIBS* pHiz = (CHAN_NIBS*)&byHizBits;
			BYTE byHiz[] = {pHiz->chan1, pHiz->chan2 };

			//Special Case: Single BYTE for Input Gain Information in the middle of 
			//all the nible representation for other params.
			pbyData++;//+29

			TacomaIOBox::TacomaIOChannel* pChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixIOBox][iChan];

				pChan = &ioc;

				c = valueFromBytes( &pChan->paramGain, pbyData );
				pbyData += c;

			}

			//Stereo Link Switch; 1 Nible
			MSN = *pbyData; //+37

			BYTE byLinkBits = MSN;
			NIB_CHAN* pLink = (NIB_CHAN*)&byLinkBits;
			BYTE byLink[] = {pLink->chan0, pLink->chan2, pLink->chan4, pLink->chan6};

			//next two BYTEs
			pbyData++;//+38
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byCompEnableBits =  (LSN << 4) | MSN;
			CHAN_NIBS* pCompEnable = (CHAN_NIBS*)&byCompEnableBits;
			BYTE byCompEnable[] = {pCompEnable->chan1, pCompEnable->chan2, pCompEnable->chan3, pCompEnable->chan4 ,pCompEnable->chan5, pCompEnable->chan6, pCompEnable->chan7, pCompEnable->chan8 };

			pbyData++;//+40

			TacomaIOBox::CompressorChannel* pCompChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixIOBox][iChan];
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixIOBox][iChan];

				pChan = &ioc;
				pCompChan = &cmpc;

				switch( iChan )
				{
				case 0: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[0] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[0] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[0] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[0] );
						c = valueFromBytes( &pChan->paramStereLink, &byLink[0] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[0] );

						//Compressor Status Controls/ +40 or offset of (bytes)28, MIDI MAP is wrong, says 39(27).
						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16);
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio,  pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, ( pbyData + 40 ) );
						pbyData += c;

					}
					break;
				case 1: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[1] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[1] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[1] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[1] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[1] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;
					}
					break;
				case 2: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[2] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[2] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[2] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[2] );
						c = valueFromBytes( &pChan->paramStereLink, &byLink[1] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[2] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;
					}	  
					break;
				case 3: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[3] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[3] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[3] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[3] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[3] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;
					}	  
					break;
				case 4: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[4] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[4] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[4] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[4] );
						c = valueFromBytes( &pChan->paramStereLink, &byLink[2] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[4] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;

					}	  
					break;
				case 5: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[5] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[5] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[5] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[5] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[5] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;
					}	  
					break;
				case 6: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[6] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[6] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[6] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[6] );
						c = valueFromBytes( &pChan->paramStereLink, &byLink[3] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[6] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );
						pbyData += c;
					}	  
					break;
				case 7: 
					{
						c = valueFromBytes( &pChan->paramPhantom, &byPhatom[7] );
						c = valueFromBytes( &pChan->paramLoCut, &byLocut[7] );
						c = valueFromBytes( &pChan->paramPhase, &byPhase[7] );
						c = valueFromBytes( &pChan->paramHiz, &byHiz[7] );
						c = valueFromBytes( &pChan->paramCompEnable, &byCompEnable[7] );

						c = valueFromBytes( &pCompChan->paramGate, pbyData );
						c = valueFromBytes( &pCompChan->paramAttack, pbyData + 8 );
						c = valueFromBytes( &pCompChan->paramRelease, pbyData + 16 );
						c = valueFromBytes( &pCompChan->paramThreshold, pbyData + 24 );
						c = valueFromBytes( &pCompChan->paramRatio, pbyData + 32 );
						c = valueFromBytes( &pCompChan->paramMakeupGain, pbyData + 40 );

					}	  
					break;
				default:
					break;

				}//end of switch

			}//end of For Loop

//---------------------------------------------------------------------------------------------------------------------------------
//MIXER INPUT (A, B, C, D)
//---------------------------------------------------------------------------------------------------------------------------------
			//LINK Switch A
			//next two BYTES
			pbyData +=41;//+88 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLinkABits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkA = (NIB_LINK*)&byLinkABits;
			BYTE byLinkA[] = {pLinkA->chan0, pLinkA->chan2, pLinkA->chan4, pLinkA->chan6, pLinkA->chan8 };

						//LINK Switch B
			//next two BYTES
			pbyData++;//+90 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLinkBBits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkB = (NIB_LINK*)&byLinkBBits;
			BYTE byLinkB[] = {pLinkB->chan0, pLinkB->chan2, pLinkB->chan4, pLinkB->chan6, pLinkB->chan8 };

						//LINK Switch C
			//next two BYTES
			pbyData++;//+92 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLinkCBits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkC = (NIB_LINK*)&byLinkCBits;
			BYTE byLinkC[] = {pLinkC->chan0, pLinkC->chan2, pLinkC->chan4, pLinkC->chan6, pLinkC->chan8 };


						//LINK Switch D
			//next two BYTES
			pbyData++;//+94 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;


			BYTE byLinkDBits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkD = (NIB_LINK*)&byLinkDBits;
			BYTE byLinkD[] = {pLinkD->chan0, pLinkD->chan2, pLinkD->chan4, pLinkD->chan6, pLinkD->chan8 };
	
			pbyData += 9; //+104 Skip Mono switch---none

			//SOLO
			//next two BYTES
			BYTE DSN = 0x0;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloABits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloABits2 =  LSN;
		    //bySoloABits = (bySoloABits << 4) | MSN;
			CHAN_NIBS* pSoloA = (CHAN_NIBS*)&bySoloABits;
			ONE_NIB* pSoloA2 = (ONE_NIB*)&bySoloABits2;
			BYTE bySoloA[] = {pSoloA->chan1,pSoloA->chan2,pSoloA->chan3,pSoloA->chan4,pSoloA->chan5,pSoloA->chan6,pSoloA->chan7,pSoloA->chan8};
			BYTE bySoloA2[] = {pSoloA2->chan1,pSoloA2->chan2};

			//SOLO B
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloBBits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloBBits2 =  LSN;
			CHAN_NIBS* pSoloB = (CHAN_NIBS*)&bySoloBBits;
			ONE_NIB* pSoloB2 = (ONE_NIB*)&bySoloBBits2;
			BYTE bySoloB[] = {pSoloB->chan1,pSoloB->chan2,pSoloB->chan3,pSoloB->chan4,pSoloB->chan5,pSoloB->chan6,pSoloB->chan7,pSoloB->chan8};
			BYTE bySoloB2[] = {pSoloB2->chan1,pSoloB2->chan2};

			//SOLO C
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloCBits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloCBits2 =  LSN;
			CHAN_NIBS* pSoloC = (CHAN_NIBS*)&bySoloCBits;
			ONE_NIB* pSoloC2 = (ONE_NIB*)&bySoloCBits2;
			BYTE bySoloC[] = {pSoloC->chan1,pSoloC->chan2,pSoloC->chan3,pSoloC->chan4,pSoloC->chan5,pSoloC->chan6,pSoloC->chan7,pSoloC->chan8};
			BYTE bySoloC2[] = {pSoloC2->chan1,pSoloC2->chan2};

			//SOLO D
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloDBits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloDBits2 =  LSN;
		    //bySoloDBits = (bySoloDBits << 4) | MSN;
			CHAN_NIBS* pSoloD = (CHAN_NIBS*)&bySoloDBits;
			ONE_NIB* pSoloD2 = (ONE_NIB*)&bySoloDBits2;
			BYTE bySoloD[] = {pSoloD->chan1,pSoloD->chan2,pSoloD->chan3,pSoloD->chan4,pSoloD->chan5,pSoloD->chan6,pSoloD->chan7,pSoloD->chan8};
			BYTE bySoloD2[] = {pSoloD2->chan1,pSoloD2->chan2};
		
			//MUTE A
			//
			pbyData++;

			//next two BYTES
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteABits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteABits2 =  LSN;
		    //byMuteABits = (byMuteABits << 4) | MSN;
			CHAN_NIBS* pMuteA = (CHAN_NIBS*)&byMuteABits;
			ONE_NIB* pMuteA2 = (ONE_NIB*)&byMuteABits2;
			BYTE byMuteA[] = {pMuteA->chan1,pMuteA->chan2,pMuteA->chan3,pMuteA->chan4,pMuteA->chan5,pMuteA->chan6,pMuteA->chan7,pMuteA->chan8};
			BYTE byMuteA2[] = {pMuteA2->chan1,pMuteA2->chan2};

			//MUTE B
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteBBits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteBBits2 =  LSN;
			CHAN_NIBS* pMuteB = (CHAN_NIBS*)&byMuteBBits;
			ONE_NIB* pMuteB2 = (ONE_NIB*)&byMuteBBits2;
			BYTE byMuteB[] = {pMuteB->chan1,pMuteB->chan2,pMuteB->chan3,pMuteB->chan4,pMuteB->chan5,pMuteB->chan6,pMuteB->chan7,pMuteB->chan8};
			BYTE byMuteB2[] = {pMuteB2->chan1,pMuteB2->chan2};

			//MUTE C
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteCBits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteCBits2 =  LSN;
			CHAN_NIBS* pMuteC = (CHAN_NIBS*)&byMuteCBits;
			ONE_NIB* pMuteC2 = (ONE_NIB*)&byMuteCBits2;
			BYTE byMuteC[] = {pMuteC->chan1,pMuteC->chan2,pMuteC->chan3,pMuteC->chan4,pMuteC->chan5,pMuteC->chan6,pMuteC->chan7,pMuteC->chan8};
			BYTE byMuteC2[] = {pMuteC2->chan1,pMuteC2->chan2};

			//MUTE D
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteDBits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteDBits2 =  LSN;
		    //byMuteDBits = (byMuteDBits << 4) | MSN;
			CHAN_NIBS* pMuteD = (CHAN_NIBS*)&byMuteDBits;
			ONE_NIB* pMuteD2 = (ONE_NIB*)&byMuteDBits2;
			BYTE byMuteD[] = {pMuteD->chan1,pMuteD->chan2,pMuteD->chan3,pMuteD->chan4,pMuteD->chan5,pMuteD->chan6,pMuteD->chan7,pMuteD->chan8};
			BYTE byMuteD2[] = {pMuteD2->chan1,pMuteD2->chan2};
			
			pbyData++;
//---------------------------------------------------------------------------------------------	
			DWORD dwPan = 0;
			WORD W1,W2,W3 = 0;
			DWORD dwAPans[10];
			DWORD dwBPans[10];
			DWORD dwCPans[10];
			DWORD dwDPans[10];
			DWORD dwAVols[10];
			DWORD dwBVols[10];
			DWORD dwCVols[10];
			DWORD dwDVols[10];
			DWORD dwSends[10];


			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN A
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwAPans[iChan] = dwPan;

			pbyData++;

			}

			//pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN B
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwBPans[iChan] = dwPan;

			pbyData++;

			}

	//		pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN C
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwCPans[iChan] = dwPan;

			pbyData++;

			}

	//		pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN D
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwDPans[iChan] = dwPan;

			pbyData++;

			}

//---------------------------------------------------------------------------------------------	
			//VOLUME A Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG (dwVol,W3);
			dwAVols[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME B Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwBVols[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME C Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwCVols[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME D Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwDVols[iChan] = dwVol;

			pbyData++;
			
			}
		
		//SENDS on A
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwSend = MAKELONG (W1,W2);
			dwSend = MAKELONG(dwSend,W3);
			dwSends[iChan] = dwSend;

			pbyData++;
			
			}

//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------

		TacomaIOBox::OctaIOChannel* pChanOC = NULL;
		for ( size_t iChan = 0; iChan < 10 ; iChan++ )
		{
			for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )
			{
				TacomaIOBox::OctaIOChannel &ioc = m_OCChans[ixIOBox][iChan][iMixer];

				pChanOC = &ioc;


				switch( iChan )
				{

				case 0:

					switch( iMixer )
					{
					case DA:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[0]);
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[0]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[0]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[0]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB[0] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[0]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[0]);


						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC[0] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[0]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[0]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD[0] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[0]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[0]);
						break;
					}

					break;

				case 1:
					{
						switch( iMixer )
						{
						case DA:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[1]);
							c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[1]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[1]);
							break;
						case DB:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[1]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[1]);
							break;
						case DC:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[1]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[1]);
							break;
						case DD:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[1]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[1]);
							break;
						}

					}
					break;
				case 2:
					switch( iMixer )
					{
					case DA:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[1]);
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[2]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[2]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[2]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[2]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[2]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB[1] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[2]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[2]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[2]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[2]);
						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC[1] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[2]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[2]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[2]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[2]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD[1] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[2]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[2]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[2]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[2]);
						break;
					}
					break;

				case 3:
					switch( iMixer )
					{
					case DA:

						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[3]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[3]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[3]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[3]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[3]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[3]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[3]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[3]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[3]);
						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[3]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[3]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[3]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[3]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[3]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[3]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[3]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[3]);
						break;

					}
					break;
				case 4:
					switch( iMixer )
					{
					case DA:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[2]);
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[4]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[4]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[4]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[4]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[4]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB[2] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[4]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[4]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[4]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[4]);

						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC[2] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[4]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[4]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[4]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[4]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD[2] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[4]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[4]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[4]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[4]);
						break;
					}
					break;

				case 5:
					{
						switch( iMixer )
						{
						case DA:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[5]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[5]);
							c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[5]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[5]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[5]);
							break;
						case DB:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[5]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[5]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[5]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[5]);
							break;
						case DC:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[5]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[5]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[5]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[5]);
							break;
						case DD:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[5]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[5]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[5]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[5]);
							break;

						}

					}
					break;
				case 6:
					switch( iMixer )
					{
					case DA:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[3]);
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[6]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[6]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[6]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[6]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[6]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB[3] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[6]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[6]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[6]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[6]);
						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC[3] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[6]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[6]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[6]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[6]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD[3] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[6]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[6]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[6]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[6]);
						break;
					};

					break;
				case 7:
					{
						switch( iMixer )
						{
						case DA:

							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA[7]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA[7]);
							c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[7]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[7]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[7]);
							break;
						case DB:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB[7]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB[7]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[7]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[7]);
							break;
						case DC:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC[7]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC[7]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[7]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[7]);
							break;
						case DD:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD[7]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD[7]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[7]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[7]);
							break;
						}

					}
					break;

				case 8:
					switch( iMixer )
					{
					case DA:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[4]);
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA2[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA2[0]);
						c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[8]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[8]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[8]);
						break;
					case DB:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB[4] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB2[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB2[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[8]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[8]);
						break;
					case DC:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC[4] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC2[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC2[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[8]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[8]);
						break;
					case DD:
						c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD[4] );
						c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD2[0]);
						c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD2[0]);
						c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[8]);
						c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[8]);
						break;
					}
					break;
				case 9:
					{
						switch( iMixer )
						{
						case DA:

							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA2[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA2[1]);
							c = valueFromBytes( &pChanOC->paramDMixSend,(BYTE*)&dwSends[9]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans[9]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols[9]);
							break;
						case DB:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB2[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB2[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans[9]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols[9]);

							break;
						case DC:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC2[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC2[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans[9]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols[9]);
							break;
						case DD:
							c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD2[1]);
							c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD2[1]);
							c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans[9]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols[9]);
							break;
						}

					}
					break;					

				default:

					break;

				}

			}

		}

//---------------------------------------------------------------------------------------------------------------------------------
//MIXER OUTPUT (A, B, C, D)
//---------------------------------------------------------------------------------------------------------------------------------
			//LINK Switch A
			//next two BYTES
			//pbyData++;
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE   byLinkA2Bits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkA2 = (NIB_LINK*)&byLinkA2Bits;
			BYTE byLinkA2[] = {pLinkA2->chan0, pLinkA2->chan2, pLinkA2->chan4, pLinkA2->chan6, pLinkA2->chan8 };

						//LINK Switch B
			//next two BYTES
			pbyData++;//+90 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLinkB2Bits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkB2 = (NIB_LINK*)&byLinkB2Bits;
			BYTE byLinkB2[] = {pLinkB2->chan0, pLinkB2->chan2, pLinkB2->chan4, pLinkB2->chan6, pLinkB2->chan8 };

						//LINK Switch C
			//next two BYTES
			pbyData++;//+92 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;

			BYTE byLinkC2Bits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkC2 = (NIB_LINK*)&byLinkC2Bits;
			BYTE byLinkC2[] = {pLinkC2->chan0, pLinkC2->chan2, pLinkC2->chan4, pLinkC2->chan6, pLinkC2->chan8 };


						//LINK Switch D
			//next two BYTES
			pbyData++;//+94 (58 Bytes)
			MSN = *pbyData;
			pbyData++;
			LSN = *pbyData;


			BYTE byLinkD2Bits =  (LSN << 4) | MSN;
			NIB_LINK* pLinkD2 = (NIB_LINK*)&byLinkD2Bits;
			BYTE byLinkD2[] = {pLinkD2->chan0, pLinkD2->chan2, pLinkD2->chan4, pLinkD2->chan6, pLinkD2->chan8 };
	
			pbyData += 9; //+104 Skip Mono switch---none

			//SOLO
			//next two BYTES
		    DSN = 0x0;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloA2Bits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloA2Bits2 =  LSN;
		    //bySoloA2Bits = (bySoloA2Bits << 4) | MSN;
			CHAN_NIBS* pSoloA3 = (CHAN_NIBS*)&bySoloA2Bits;
			ONE_NIB* pSoloA22 = (ONE_NIB*)&bySoloA2Bits2;
			BYTE bySoloA3[] = {pSoloA3->chan1,pSoloA3->chan2,pSoloA3->chan3,pSoloA3->chan4,pSoloA3->chan5,pSoloA3->chan6,pSoloA3->chan7,pSoloA3->chan8};
			BYTE bySoloA22[] = {pSoloA22->chan1,pSoloA22->chan2};

			//SOLO B
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloB2Bits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloB2Bits2 =  LSN;
			CHAN_NIBS* pSoloB3 = (CHAN_NIBS*)&bySoloB2Bits;
			ONE_NIB* pSoloB22 = (ONE_NIB*)&bySoloB2Bits2;
			BYTE bySoloB3[] = {pSoloB3->chan1,pSoloB3->chan2,pSoloB3->chan3,pSoloB3->chan4,pSoloB3->chan5,pSoloB3->chan6,pSoloB3->chan7,pSoloB3->chan8};
			BYTE bySoloB22[] = {pSoloB22->chan1,pSoloB22->chan2};

			//SOLO C
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloC2Bits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloC2Bits2 =  LSN;
			CHAN_NIBS* pSoloC3 = (CHAN_NIBS*)&bySoloC2Bits;
			ONE_NIB* pSoloC22 = (ONE_NIB*)&bySoloC2Bits2;
			BYTE bySoloC3[] = {pSoloC3->chan1,pSoloC3->chan2,pSoloC3->chan3,pSoloC3->chan4,pSoloC3->chan5,pSoloC3->chan6,pSoloC3->chan7,pSoloC3->chan8};
			BYTE bySoloC22[] = {pSoloC22->chan1,pSoloC22->chan2};

			//SOLO D
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE bySoloD2Bits =  (( (DSN << 4) |  MSN ));
			BYTE bySoloD2Bits2 =  LSN;
		    //bySoloD2Bits = (bySoloD2Bits << 4) | MSN;
			CHAN_NIBS* pSoloD3 = (CHAN_NIBS*)&bySoloD2Bits;
			ONE_NIB* pSoloD22 = (ONE_NIB*)&bySoloD2Bits2;
			BYTE bySoloD3[] = { pSoloD3->chan1, pSoloD3->chan2, pSoloD3->chan3, pSoloD3->chan4, pSoloD3->chan5, pSoloD3->chan6, pSoloD3->chan7, pSoloD3->chan8};
			BYTE bySoloD22[] = {pSoloD22->chan1,pSoloD22->chan2};
		
			//MUTE A
			//
			pbyData++;

			//next two BYTES
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteA2Bits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteA2Bits2 =  LSN;
		    //byMuteA2Bits = (byMuteA2Bits << 4) | MSN;
			CHAN_NIBS* pMuteA3 = (CHAN_NIBS*)&byMuteA2Bits;
			ONE_NIB* pMuteA22 = (ONE_NIB*)&byMuteA2Bits2;
			BYTE byMuteA3[] = {pMuteA3->chan1,pMuteA3->chan2,pMuteA3->chan3,pMuteA3->chan4,pMuteA3->chan5,pMuteA3->chan6,pMuteA3->chan7,pMuteA3->chan8};
			BYTE byMuteA22[] = {pMuteA22->chan1,pMuteA22->chan2};

			//MUTE B
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteB2Bits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteB2Bits2 =  LSN;
			CHAN_NIBS* pMuteB3 = (CHAN_NIBS*)&byMuteB2Bits;
			ONE_NIB* pMuteB22 = (ONE_NIB*)&byMuteB2Bits2;
			BYTE byMuteB3[] = {pMuteB3->chan1,pMuteB3->chan2,pMuteB3->chan3,pMuteB3->chan4,pMuteB3->chan5,pMuteB3->chan6,pMuteB3->chan7,pMuteB3->chan8};
			BYTE byMuteB22[] = {pMuteB22->chan1,pMuteB22->chan2};

			//MUTE C
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteC2Bits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteC2Bits2 =  LSN;
			CHAN_NIBS* pMuteC3 = (CHAN_NIBS*)&byMuteC2Bits;
			ONE_NIB* pMuteC22 = (ONE_NIB*)&byMuteC2Bits2;
			BYTE byMuteC3[] = {pMuteC3->chan1,pMuteC3->chan2,pMuteC3->chan3,pMuteC3->chan4,pMuteC3->chan5,pMuteC3->chan6,pMuteC3->chan7,pMuteC3->chan8};
			BYTE byMuteC22[] = {pMuteC22->chan1,pMuteC22->chan2};

			//MUTE D
			pbyData++;
			MSN = *pbyData;
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN =*pbyData;


			BYTE byMuteD2Bits =  (( (DSN << 4) |  MSN ));
			BYTE byMuteD2Bits2 =  LSN;
		    //byMuteD2Bits = (byMuteD2Bits << 4) | MSN;
			CHAN_NIBS* pMuteD3 = (CHAN_NIBS*)&byMuteD2Bits;
			ONE_NIB* pMuteD22 = (ONE_NIB*)&byMuteD2Bits2;
			BYTE byMuteD3[] = {pMuteD3->chan1,pMuteD3->chan2,pMuteD3->chan3,pMuteD3->chan4,pMuteD3->chan5,pMuteD3->chan6,pMuteD3->chan7,pMuteD3->chan8};
			BYTE byMuteD22[] = {pMuteD22->chan1,pMuteD22->chan2};
			
			pbyData++;
//---------------------------------------------------------------------------------------------	
			dwPan = 0;
			W1,W2,W3 = 0;
			DWORD dwAPans2[10];
			DWORD dwBPans2[10];
			DWORD dwCPans2[10];
			DWORD dwDPans2[10];
			DWORD dwAVols2[10];
			DWORD dwBVols2[10];
			DWORD dwCVols2[10];
			DWORD dwDVols2[10];
			DWORD dwAVolMI[2];
			DWORD dwBVolMI[2];
			DWORD dwCVolMI[2];
			DWORD dwDVolMI[2];
			DWORD dwAVolMO[2];
			DWORD dwBVolMO[2];
			DWORD dwCVolMO[2];
			DWORD dwDVolMO[2];


			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN A
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwAPans2[iChan] = dwPan;

			pbyData++;

			}

			//pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN B
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwBPans2[iChan] = dwPan;

			pbyData++;

			}

	//		pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN C
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwCPans2[iChan] = dwPan;

			pbyData++;

			}

	//		pbyData++;

			for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			//PAN D
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
		

			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			dwPan = MAKELONG (W1,W2);
			dwDPans2[iChan] = dwPan;

			pbyData++;

			}

//---------------------------------------------------------------------------------------------	
			//VOLUME A Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG (dwVol,W3);
			dwAVols2[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME B Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwBVols2[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME C Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwCVols2[iChan] = dwVol;

			pbyData++;
			
			}

					//VOLUME D Faders
		for ( size_t iChan = 0; iChan < 10; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwDVols2[iChan] = dwVol;

			pbyData++;
			
			}
//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------

		//	TacomaIOBox::OctaIOChannel* pChanOC = NULL;
			for ( size_t iChan = 10; iChan < 20 ; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )
				{
				TacomaIOBox::OctaIOChannel &ioc = m_OCChans[ixIOBox][iChan][iMixer];

				pChanOC = &ioc;
			

				switch( iChan )
				{
		
					case 10:

						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[0]);
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[0]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[0]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB2[0] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB2[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[0]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[0]);
								

								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC2[0] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC3[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[0]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[0]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD2[0] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[0]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[0]);
								break;
							}
						
						break;
				
					case 11:
						{
							switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[1]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[1]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[1]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[1]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[1]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[1]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[1]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[1]);
								break;
							}
			
						}
						break;
					case 12:
						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[1]);
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[2]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[2]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[2]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[2]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB2[1] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[2]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[2]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[2]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[2]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC2[1] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[2]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC2[2]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[2]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[2]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD2[1] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[2]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[2]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[2]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[2]);
								break;
							}
						break;
						
					case 13:
					switch( iMixer )
							{
							case DA:

								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[3]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[3]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[3]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[3]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[3]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[3]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[3]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[3]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[3]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[3]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[3]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[3]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[3]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[3]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[3]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[3]);
								break;
	
							}
						break;
					case 14:
						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[2]);
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[4]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[4]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[4]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[4]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB2[2] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[4]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[4]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[4]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[4]);

								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC2[2] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[4]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[4]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[4]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[4]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD2[2] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[4]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[4]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[4]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[4]);
								break;
							}
						break;

					case 15:
						{
							switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[5]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[5]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[5]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[5]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[5]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[5]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[5]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[5]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[5]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[5]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[5]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[5]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[5]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[5]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[5]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[5]);
								break;
							
							}
			
						}
						break;
					case 16:
						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[3]);
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[6]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[6]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[6]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[6]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB2[3] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[6]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[6]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[6]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[6]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC2[3] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[6]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[6]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[6]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[6]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD2[3] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[6]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[6]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[6]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[6]);
								break;
							};
	
						break;
					case 17:
						{
							switch( iMixer )
							{
							case DA:
					
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA3[7]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA3[7]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[7]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[7]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[7]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB3[7]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[7]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[7]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB3[7]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC3[7]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[7]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[7]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD3[7]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD3[7]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[7]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[7]);
								break;
							}
			
						}
						break;

					case 18:
						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkA[4]);
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA22[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA22[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[8]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[8]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkB2[4] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB22[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB22[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[8]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[8]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkC2[4] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB22[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC22[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[8]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[8]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkD2[4] );
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD22[0]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD22[0]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[8]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[8]);
								break;
							}
						break;
					case 19:
						{
							switch( iMixer )
							{
							case DA:
							
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloA22[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteA22[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwAPans2[9]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVols2[9]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloB22[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteB22[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwBPans2[9]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVols2[9]);

								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloC22[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteC22[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwCPans2[9]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVols2[9]);
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixSolo,&bySoloD22[1]);
								c = valueFromBytes( &pChanOC->paramDMixMute,&byMuteD22[1]);
								c = valueFromBytes( &pChanOC->paramDMixPan,(BYTE*)&dwDPans2[9]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVols2[9]);
								break;
							}
			
						}
						break;					

					default:

							break;
				
						}

					}

				}



			//MASTER INPUT (A, B, C, D)
		//	pbyData++;

			//Stereo Link Switch; 1 Nible, 4 mixers
			MSN = *pbyData; //
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN = *pbyData;
			pbyData++;
			BYTE RSN = *pbyData;
			BYTE byLinkMI[] = {MSN, DSN, LSN, RSN};

			pbyData++;//

			//Master A In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwAVolMI[iChan] = dwVol;

			pbyData++;
			
			}
			//Master B In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwBVolMI[iChan] = dwVol;

			pbyData++;
			
			}

			///Master C In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwCVolMI[iChan] = dwVol;

			pbyData++;
			
			}

			//Master  D In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwDVolMI[iChan] = dwVol;

			pbyData++;
			
			}

			//Return Level
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;
						
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwReturn = MAKELONG (W1,W2);
			dwReturn = MAKELONG(dwReturn,W3);

			//Master LINK
			pbyData++;

			//MASTER Link Switch; 1 Nible, 4 mixers
			MSN = *pbyData; //
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN = *pbyData;
			pbyData++;
			RSN = *pbyData;
			BYTE byBigLink[] = {MSN, DSN, LSN, RSN};

			pbyData +=5;
			
	//-----------------------------------------------------------------------------------------------
			for ( size_t iChan = 20; iChan < 22 ; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )
				{
				TacomaIOBox::OctaIOChannel &ioc = m_OCChans[ixIOBox][iChan][iMixer];

				pChanOC = &ioc;
			

				switch( iChan )
				{
		
					case 20:

						switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramDMixReturn,(BYTE*)&dwReturn);
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMI[0]);
								c = valueFromBytes( &pChanOC->paramInLink,&byBigLink[0]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVolMI[0]);

								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMI[1]);
								c = valueFromBytes( &pChanOC->paramInLink,&byBigLink[1]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVolMI[0]);
								

								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMI[2]);
								c = valueFromBytes( &pChanOC->paramInLink,&byBigLink[2]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVolMI[0]);;
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMI[3]);
								c = valueFromBytes( &pChanOC->paramInLink,&byBigLink[3]);
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVolMI[0]);;
								break;
							}
						
						break;
				
					case 21:
						{
							switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVolMI[1]);
								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVolMI[1]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVolMI[1]);;
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVolMI[1]);;
								break;
							}


						}//end of Case


				}//end of Switch
			}
		}
		//MASTER OUTPUT(A, B, C, D)

//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------
	//Stereo Link Switch; 1 Nible, 4 mixers
			MSN = *pbyData; //
			pbyData++;
			DSN = *pbyData;
			pbyData++;
			LSN = *pbyData;
			pbyData++;
			RSN = *pbyData;
			BYTE byLinkMO[] = {MSN, DSN, LSN, RSN};

			pbyData++;//

			//Master A In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwAVolMO[iChan] = dwVol;

			pbyData++;
			
			}
			//Master B In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwBVolMO[iChan] = dwVol;

			pbyData++;
			
			}

			///Master C In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwCVolMO[iChan] = dwVol;

			pbyData++;
			
			}

			//Master  D In Volume
			for ( size_t iChan = 0; iChan < 2; iChan++ )
			{
			BYTE N1 = *pbyData;
			pbyData++;
			BYTE N2 = *pbyData;
			pbyData++;
			BYTE N3 = *pbyData;
			pbyData++;
			BYTE N4 = *pbyData;
			pbyData++;
			BYTE N5 = *pbyData;
			pbyData++;
			BYTE N6 = *pbyData;

			
			W1 = MAKEWORD(N1,N2);
			W2 = MAKEWORD(N3,N4);
			W3 = MAKEWORD(N5,N6);
			DWORD dwVol = MAKELONG (W1,W2);
			dwVol = MAKELONG(dwVol,W3);
			dwDVolMO[iChan] = dwVol;

			pbyData++;
			
			}

			//-----------------------------------------------------------------------------------------------
			for ( size_t iChan = 22; iChan < 24 ; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )
				{
					TacomaIOBox::OctaIOChannel &ioc = m_OCChans[ixIOBox][iChan][iMixer];

					pChanOC = &ioc;


					switch( iChan )
					{

					case 22:

						switch( iMixer )
						{
						case DA:
							c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMO[0]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVolMO[0]);

							break;
						case DB:
							c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMO[1]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVolMO[0]);
							break;
						case DC:
							c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMO[2]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVolMO[0]);;
							break;
						case DD:
							c = valueFromBytes( &pChanOC->paramStereLinkOC,&byLinkMO[3]);
							c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVolMO[0]);;
							break;
						}

						break;

					case 23:
						{
							switch( iMixer )
							{
							case DA:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwAVolMO[1]);

								break;
							case DB:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwBVolMO[1]);
								break;
							case DC:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwCVolMO[1]);;
								break;
							case DD:
								c = valueFromBytes( &pChanOC->paramDMixVol,(BYTE*)&dwDVolMO[1]);;
								break;
							}


						}//end of Case


					}//end of Switch
				}
			}

	}//end of bSytemDump
//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------
		else if ( bSystem)
		{
			// data starts at BYTE 11, for 6 bytes
			BYTE* pbyData = pby + 11;
			size_t c = 0;
			switch(abyAddr[3] )
			{
			case 0x02:
				{
					size_t c = valueFromBytes( &m_System[ixIOBox].paramSyncSource, pbyData );
					m_fSyncVal = m_System[ixIOBox].paramSyncSource.f01Val;
				}
				break;

			case 0x03:
				{
					size_t c = valueFromBytes( &m_System[ixIOBox].paramDirectMix, pbyData );
					m_fSyncVal = m_System[ixIOBox].paramDirectMix.f01Val;
				}
				break;
			}


		}
//-----------------------------------------------------------------------------------------------------------------------------------
		else if ( bDMOut )

		{
			// data starts at BYTE 11, for 3 bytes
			BYTE* pbyData = pby + 11;
			size_t c = 0;

			m_bMidiPatch = true;

			switch(abyAddr[3] )
			{
			case 0x00:
				{
					c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut12, pbyData );
					m_iOUT12=(int)*pbyData;
				}
				break;

			case 0x01:
				{
					c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut34, pbyData );
					m_iOUT34=(int)*pbyData;
				}
				break;

			case 0x02:
				{
					c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut56, pbyData );
					m_iOUT56=(int)*pbyData;
				}
				break;
			case 0x03:
				{
					c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut78, pbyData );
					m_iOUT78=(int)*pbyData;
				}
				break;
			case 0x04:
				{
					c = valueFromBytes( &m_DMOutput[ixIOBox].paramOut910, pbyData );
					m_iOUT910=(int)*pbyData;
				}
				break;

			}

		}
//-----------------------------------------------------------------------------------------------------------------------------------
		else if (bOctaDM)
		{
			BYTE byChan = ( abyAddr[2] << 4 );
			byChan = ( byChan >> 4 );
			TacomaIOBox::OctaIOChannel* pChan = NULL;

			for ( size_t iChan = 0; iChan <  20; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )

				{
					TacomaIOBox::OctaIOChannel& ioc = m_OCChans[ixIOBox][iChan][iMixer];	

					if ( (ioc.abyAddress[1] == abyAddr[1]) && ((ioc.abyAddress[2] == abyAddr[2])))
					{
						pChan = &ioc;
						break;
					}
				}
			}
			if ( pChan )
			{
				// we have a chan, now extract the values out of the sysx.
				// data starts at BYTE 11
				BYTE* pbyData = pby + 11;
				size_t c = 0;

				//	if ( byMixer == m_byDirectMix )

				switch( abyAddr[3] )
				{

				case 0x0:
					c = valueFromBytes( &pChan->paramStereLinkOC, pbyData );
					m_bLinkclick = true;
					break;
				case 0x2:
					c = valueFromBytes( &pChan->paramDMixSolo, pbyData );
					break;
				case 0x3:
					c = valueFromBytes( &pChan->paramDMixMute, pbyData );
					break;
				case 0x4:
					c = valueFromBytes( &pChan->paramDMixPan, pbyData );
					break;
				case 0x8:
					{
						if (   ( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 1.f) )
						{
							m_bLinked = true;
						}
						else if (( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 0.f))
						{
							m_bLinked = false;
						}

						if (   ( byChan % 2 == 0) && ( m_bLinked == false) )
							c = valueFromBytes( &pChan->paramDMixVol, pbyData );


						if (   ( byChan % 2 != 0) && ( m_bLinked == false))
							c = valueFromBytes( &pChan->paramDMixVol, pbyData );


					}
					break;
				case 0xE:
					pChan->abyAddress[2] = abyAddr[2];
					c = valueFromBytes( &pChan->paramDMixSend, pbyData );
					break;
				default:
					break;
				}
			}

		}
//-----------------------------------------------------------------------------------------------------------------------------------
		else if (bReverb)
		{

			// we have a chan, now extract the values out of the sysx.
			// data starts at BYTE 11
			BYTE* pbyData = pby + 11;
			size_t c = 0;

			switch( abyAddr[3] )
			{

			case 0x0:
				c = valueFromBytes(&m_DMRev[ixIOBox].paramRevType, pbyData );
				m_byRevType = *pbyData;
				m_bMidiVerb = true;
				break;
			case 0x1:  
				{
					//abyAddr[2]=m_byRevType;
					c = valueFromBytes(&m_DMRev[ixIOBox].paramRevDelay, pbyData );
					m_byDelay = *pbyData;
					switch (m_byRevType)
					{
					case OFF:
						m_iOFF_D = (int)*pbyData;
						break;
					case ECHO:
						m_iECHO_D = (int)*pbyData;
						break;
					case ROOM:
						m_iROOM_D = (int)*pbyData;
						break;
					case SMALLH: 
						m_iSHALL_D = (int)*pbyData;
						break;
					case LARGEH:
						m_iLHALL_D= (int)*pbyData;
						break;
					}

					m_bMidiDelay = true;
				}

				break;
			case 0x2:
				//abyAddr[2]=m_byRevType;
				{
					c = valueFromBytes(&m_DMRev[ixIOBox].paramRevTime, pbyData );

					switch (m_byRevType)
					{
					case OFF:
						m_iOFF_T = *pbyData;
						break;
					case ECHO:
						m_iECHO_T = *pbyData;
						break;
					case ROOM:
						m_iROOM_T = *pbyData;
						break;
					case SMALLH: 
						m_iSHALL_T = *pbyData;
						break;
					case LARGEH:
						m_iLHALL_T= *pbyData;
						break;
					}
				}
				break;
			default:
				break;
			}


		}
//-----------------------------------------------------------------------------------------------------------------------------------
		else if (bMasterIn)
		{
			BYTE byChan = ( abyAddr[2] << 4 );
			byChan = ( byChan >> 4 );

			TacomaIOBox::OctaIOChannel* pChan = NULL;

			for ( size_t iChan = 20; iChan < 22; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )

				{
					TacomaIOBox::OctaIOChannel& ioc = m_OCChans[ixIOBox][iChan][iMixer];	


					if ( (ioc.abyAddress[1] == abyAddr[1]) && ((ioc.abyAddress[2] == abyAddr[2])))
					{
						pChan = &ioc;
						break;
					}
				}

			}
			if ( pChan )
			{
				// we have a chan, now extract the values out of the sysx.
				// data starts at BYTE 11
				BYTE* pbyData = pby + 11;
				size_t c = 0;


				switch( abyAddr[3] )
				{

				case 0x0:
					c = valueFromBytes( &pChan->paramStereLinkOC, pbyData );
					break;
				case 0x1:
					if (   ( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 1.f) )
					{
						m_bLinked = true;
					}
					else if (( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 0.f))
					{
						m_bLinked = false;
					}

					if (   ( byChan % 2 == 0) && ( m_bLinked == false) )
						c = valueFromBytes( &pChan->paramDMixVol, pbyData );


					if (   ( byChan % 2 != 0) && ( m_bLinked == false))
						c = valueFromBytes( &pChan->paramDMixVol, pbyData );
					break;
				case 0x7:
					c = valueFromBytes( &pChan->paramDMixReturn, pbyData );
					break;
				case 0xD:
					c = valueFromBytes( &pChan->paramInLink, pbyData );
					break;

				default:
					break;
				}

			}
		}
//-----------------------------------------------------------------------------------------------------------------------------------
		else if (bMasterOut)
		{
			TacomaIOBox::OctaIOChannel* pChan = NULL;
			BYTE byChan = ( abyAddr[2] << 4 );
			BYTE byMixer = ( abyAddr[2] >> 4 );
			byChan = ( byChan >> 4 );

			for ( size_t iChan = 22; iChan < 24; iChan++ )
			{
				for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )

				{
					TacomaIOBox::OctaIOChannel& ioc = m_OCChans[ixIOBox][iChan][iMixer];	


					if ( (ioc.abyAddress[1] == abyAddr[1]) && ((ioc.abyAddress[2] == abyAddr[2])))
					{
						pChan = &ioc;
						break;
					}
				}

			}
			if ( pChan )
			{
				// we have a chan, now extract the values out of the sysx.
				// data starts at BYTE 11
				BYTE* pbyData = pby + 11;
				size_t c = 0;

				//if ( byMixer == m_byDirectMix )
				//{
				switch( abyAddr[3] )
				{

				case 0x0:
					//pChan->abyAddress[2] = ((iMixer << 4) | 0 );
					c = valueFromBytes( &pChan->paramStereLinkOC, pbyData );
					break;
				case 0x1:
					if (   ( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 1.f) )
					{
						m_bLinked = true;
					}
					else if (( byChan % 2 == 0) && ( pChan->paramStereLinkOC.f01Val == 0.f))
					{
						m_bLinked = false;
					}

					if (   ( byChan % 2 == 0) && ( m_bLinked == false) )
						c = valueFromBytes( &pChan->paramDMixVol, pbyData );


					if (   ( byChan % 2 != 0) && ( m_bLinked == false))
						c = valueFromBytes( &pChan->paramDMixVol, pbyData );
					break;

				default:
					break;
				}
				//}
			}
		}

//-----------------------------------------------------------------------------------------------------------------------------------
		else // Eveything else
		{
			TacomaIOBox::TacomaIOChannel* pChan = NULL;
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixIOBox][iChan];

				if (ioc.abyAddress[2] == abyAddr[2] )
				{
					pChan = &ioc;		// found it
					break;
				}

			}
			if ( pChan )
			{
				// we have a chan, now extract the values out of the sysx.
				// data starts at BYTE 11, for 25 bytes
				BYTE* pbyData = pby + 11;
				size_t c = 0;

				switch(abyAddr[3] )
				{
				case 0x0:
					c = valueFromBytes( &pChan->paramPhantom, pbyData );
					break;
				case 0x1:
					c = valueFromBytes( &pChan->paramLoCut, pbyData );
					break;
				case 0x2:
					c = valueFromBytes( &pChan->paramPhase, pbyData );
					break;
				case 0x3:
					c = valueFromBytes( &pChan->paramHiz, pbyData );
					break;
				case 0x4:
					c = valueFromBytes( &pChan->paramGain, pbyData );
					break;
				case 0x5:
					c = valueFromBytes( &pChan->paramStereLink, pbyData );
					break;
				case 0x6:
					c = valueFromBytes( &pChan->paramCompEnable, pbyData );
					break;
				default:
					break;
				}


			}
			else
				ASSERT(0);
		}

		delete [] pby;
	}


	return TRUE;
}



/*
From the Wiki on Q Format Numbers...

Conversion
Float to Q
To convert a number from floating point to Qm.n format:
   1. Multiply the floating point number by 2n
   2. Round to the nearest integer

Q to Float
To convert a number from Qm.n format to floating point:

   1. Convert the number directly to floating point
   2. Divide by 2n
*/


//-------------------------------------------------------------------
// Given an IOParam and a pointer to 1 or more bytes with a value, 
// set its floating point parameter.
// Return the number of bytes needed for the value
size_t CIOBoxInterface::valueFromBytes( TacomaIOBox::IOParam* pparam, BYTE* pbyVal )
{
	TRACE( _T("valueFromBytes name:%s by:%x\n"), pparam->strName, *pbyVal );

	size_t c = 1;
	float f01 = 0.f;

	UINT ixBox = GetActiveInterface();

	if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
	{

		switch( pparam->eParam )
		{
			// bit controls
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMono:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
			f01 = *pbyVal == 0 ? 0.f : 1.f;
			break;

		case TIOP_Phase:
			// invert
			f01 = *pbyVal == 0 ? 0.f : 1.f;
			break;

		case TIOP_Gain:
			// 0 - 127
			f01 = *pbyVal / 44.f;
			break;
		case TIOP_DMixPan:
			c = 4;
			{
				short sPan = 0;
				BYTE b = *(pbyVal + 0);
				sPan |= (b << 12);
				b = *(pbyVal + 1);
				sPan |= (b << 8);
				b = *(pbyVal + 2);
				sPan |= (b << 4);
				b = *(pbyVal + 3);
				sPan |= b;

				// convert to floating point -100 ... +100
				float fPan = (float)sPan;
				fPan /= 256.f;	// Q7.8
				// normalize
				f01 = (fPan - -100.f) / (200.f);
			}
			break;
		case TIOP_DMixVol:
			c = 4;
			{
				short sVol = 0;
				BYTE b = *(pbyVal + 0);
				sVol |= (b << 12);
				b = *(pbyVal + 1);
				sVol |= (b << 8);
				b = *(pbyVal + 2);
				sVol |= (b << 4);
				b = *(pbyVal + 3);
				sVol |= b;

				// convert to floating point 0..4
				float fGain = (float)sVol;
				fGain /= 8192.f;	// Q2.13

				// to dB
				float fDB = 20 * ::log10( fGain );

				// normalize (we'll use linear dB for now)
				if ( fDB < -96.f )
					f01 = 0.f;
				else
					f01 = (fDB+96) / (6+96);
			}
			break;

		case TIOP_Threshold:
			{
				int idb = *pbyVal - 128;
				if ( -128 == idb )
					f01 = 1.0;
				else
					f01 = (idb+60) / 60.f;
			}
			break;

		case TIOP_MakeupGain:
			{
				int idb = 0;
				if ( *pbyVal > 30 )
					idb = -(128 - *pbyVal)
					;
				else
					idb = *pbyVal;
				f01 = (idb+30) / 60.f;
			}
			break;
		case TIOP_Attack:
		case TIOP_Release:
			f01 = *pbyVal / 100.f;
			break;
		case TIOP_Ratio:
			f01 = *pbyVal / 13.f;
			break;

		case TIOP_SyncSource:
			f01 = *pbyVal / 3.f;
			break;
		case TIOP_DigitalInput:
			f01 = *pbyVal / 1.f;
			break;

		case TIOP_DMOutMain:
		case TIOP_DMOutSub:
		case TIOP_DMOutDig:
			f01 = *pbyVal / 1.f;
			break;

		case TIOP_SampleRate:
			f01 = *pbyVal / 4.f;
			break;
		case TIOP_UnitNumber:
			f01 = *pbyVal / 1.f;
			break;
		}
	}
	//-------------------------------------------------------------------------------------------------------------------
	else //(OCTA-CAPTURE)
	{
		switch( pparam->eParam )
		{
			// bit controls
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMono:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
		case TIOP_StereoLinkOC:
		case TIOP_MasterLinkOC:
			f01 = *pbyVal == 0 ? 0.f : 1.f;
			break;

		case TIOP_Phase:
			// invert
			f01 = *pbyVal == 0 ? 0.f : 1.f;
			break;

		case TIOP_Gain:
			// 0 - 100
			f01 = *pbyVal / 100.f;
			break;
		case TIOP_DMixPan:
			c = 4;
			{
				short sPan = 0;
				BYTE b = *(pbyVal + 0);
				sPan |= (b << 12);
				b = *(pbyVal + 1);
				sPan |= (b << 8);
				b = *(pbyVal + 2);
				sPan |= (b << 4);
				b = *(pbyVal + 3);
				sPan |= b;

				// convert to floating point -100 ... +100
				float fPan = (float)sPan;
				fPan /= 327.f;	// Q7.8
				// normalize
				f01 = (fPan) / (100.f);
			}
			break;
		case TIOP_DMixVol:
			c = 4;
			{
				short sVol = 0;
				BYTE b = *(pbyVal + 0);
				sVol |= (b << 12);
				b = *(pbyVal + 1);
				sVol |= (b << 8);
				b = *(pbyVal + 2);
				sVol |= (b << 4);
				b = *(pbyVal + 3);
				sVol |= b;

				// convert to floating point 0..4
				float fGain = (float)sVol;
				fGain /= 8192.f;	// Q2.13

				// to dB
				float fDB = 20 * ::log10( fGain );

				// normalize (we'll use linear dB for now)
				if ( fDB < -96.f )
					f01 = 0.f;
				else
					f01 = (fDB+96) / (12+96);
			}
			break;

		case TIOP_DMixSend:
			c = 4;
			{
				short sVol = 0;
				BYTE b = *(pbyVal + 0);
				sVol |= (b << 12);
				b = *(pbyVal + 1);
				sVol |= (b << 8);
				b = *(pbyVal + 2);
				sVol |= (b << 4);
				b = *(pbyVal + 3);
				sVol |= b;

				// convert to floating point 0..4
				float fGain = (float)sVol;
				fGain /= 8192.f;	// Q2.13

				// to dB
				float fDB = 20 * ::log10( fGain );

				// normalize (we'll use linear dB for now)
				if ( fDB < -96.f )
					f01 = 0.f;
				else
					f01 = (fDB+96) / (12+96);
			}
			break;
		case TIOP_DMixReturn:
			c = 4;
			{
				short sVol = 0;
				BYTE b = *(pbyVal + 0);
				sVol |= (b << 12);
				b = *(pbyVal + 1);
				sVol |= (b << 8);
				b = *(pbyVal + 2);
				sVol |= (b << 4);
				b = *(pbyVal + 3);
				sVol |= b;

				// convert to floating point 0..4
				float fGain = (float)sVol;
				fGain /= 8192.f;	// Q2.13

				// to dB
				float fDB = 20 * ::log10( fGain );

				// normalize (we'll use linear dB for now)
				if ( fDB < -96.f )
					f01 = 0.f;
				else
					f01 = (fDB+96) / (12+96);
			}
			break;
		case TIOP_RevTime:
			{
				f01 = *pbyVal / 49.f;
			}
			break;

		case TIOP_RevType:
			{
				f01 = *pbyVal;
			}
			break;

		case TIOP_RevDelay:
			{
				f01 = *pbyVal;
			}
			break;

		case TIOP_Threshold:
			{
				int idb = *pbyVal - 50;
				if ( -50 == idb )
					f01 = 0.0;
				else
					f01 = (idb+50) / 50.f;
			}
			break;

		case TIOP_Gate:
			{
				int idb = *pbyVal - 50;
				if ( -50 == idb )
					f01 = 0.0;
				else
					f01 = (idb+50) / 50.f;
			}
			break;
		case TIOP_MakeupGain:
			{
				int idb = *pbyVal - 128;
				if ( -128 == idb )
					f01 = 0.0;
				else
					f01 = (idb+128) / 74.f;
			}
			break;
		case TIOP_Attack:
			f01 = *pbyVal / 25.f;
			break;
		case TIOP_Release:
			f01 = *pbyVal / 45.f;
			break;
		case TIOP_Ratio:
			f01 = *pbyVal / 8.f;
			break;

		case TIOP_SyncSource:
			f01 = *pbyVal / 3.f;
			break;
		case TIOP_DigitalInput:
			f01 = *pbyVal / 1.f;
			break;
		case TIOP_DirectMix:
			f01 = *pbyVal / 3.f;
			break;

		case TIOP_DMOutMain:
		case TIOP_DMOutSub:
		case TIOP_DMOutDig:
			f01 = *pbyVal / 1.f;
			break;

		case TIOP_SampleRate:
			f01 = *pbyVal / 4.f;
			break;
		case TIOP_UnitNumber:
			f01 = *pbyVal / 1.f;
			break;

		case TIOP_OUT12:
		case TIOP_OUT34:
		case TIOP_OUT56:
		case TIOP_OUT78:
		case TIOP_OUT910:
			f01 = *pbyVal;
			break;

	

		}

	}

	if ( f01 < 0.f || f01 > 1.f )
	{
//		ASSERT(0);
		f01 = max( 0.f, f01 );
		f01 = min( 1.f, f01 );
	}

	pparam->f01Val = f01;

	return c;
}


//------------------------------------------------------------------
// Given an IO param, determine the byte representation of its current value
// and stuff in a vector of bytes
void CIOBoxInterface::bytesFromValue( TacomaIOBox::IOParam* pparam, std::vector<BYTE>* pvBytes )
{
	UINT ixBox = GetActiveInterface();

	if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
	{

		switch( pparam->eParam )
		{
			// bit controls
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMono:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
			pvBytes->push_back( (BYTE)(pparam->f01Val >= .5f ? 1 : 0) );
			break;

		case TIOP_Phase:
			// invert from our standard
			pvBytes->push_back( (BYTE)(pparam->f01Val >= .5f ? 1 : 0) );
			break;

		case TIOP_Gain:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 44 ) );
			break;
		case TIOP_DMixPan:
			{
				// first convert to the realworld value
				float fPan = -100.f + pparam->f01Val * 200.f;//**** //VS700******/
				// This is a Q7.8 so multiply by 2^8
				fPan *= 256.f;
				WORD wPan = (WORD)fPan;
				BYTE b = (wPan >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wPan >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wPan >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wPan & 0x0f;
				pvBytes->push_back(b);
			}
			break;
		case TIOP_DMixVol:
			{
				// first convert to the realworld db.
				float fDB = -96.f + pparam->f01Val * (6+96);

				// convert to gain (this is what gets transmitted)
				float fGain = ::pow( 10.0f, (fDB / 20.f) );

				// This is a Q2.13 so multiply by 2^13
				fGain *= 8192.f;
				WORD wVol = (WORD)fGain;
				BYTE b = (wVol >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wVol & 0x0f;
				pvBytes->push_back(b);
			}
			break;

		case TIOP_Threshold:
			{
				BYTE by(0);
				int i = (int)(-60.f + pparam->f01Val * 60.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_Gate:
			{
				BYTE by(0);
				int i = (int)(-128.f + pparam->f01Val * 50.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_MakeupGain:
			{
				BYTE by(0);
				int i = (int)(-30.f + pparam->f01Val * 60.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_Attack:
		case TIOP_Release:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 100 ) );
			break;
		case TIOP_Ratio:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 13 ) );
			break;

		case TIOP_SyncSource:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 3 ) );
			break;
		case TIOP_DigitalInput:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 1 ) );
			break;

		case TIOP_DMOutMain:
		case TIOP_DMOutSub:
		case TIOP_DMOutDig:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 1 ) );
			break;
		}
	}
	//-----------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------
	else //(OCTA-CAPTURE)

	{
		switch( pparam->eParam )
		{
			// bit controls
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMono:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
		case TIOP_StereoLinkOC:
		case TIOP_MasterLinkOC:
			pvBytes->push_back( (BYTE)(pparam->f01Val >= .5f ? 1 : 0) );
			break;

		case TIOP_Phase:
			// invert from our standard
			pvBytes->push_back( (BYTE)(pparam->f01Val >= .5f ? 1 : 0) );
			break;

		case TIOP_Gain:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 100 ) );
			break;
		case TIOP_DMixPan:
			{
				// first convert to the realworld value
				float fPan = pparam->f01Val * 100.f;
				// This is a Q7.8 so multiply by 2^8
				fPan *= 327.f;
				WORD wPan = (WORD)fPan;
				BYTE b = (wPan >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wPan >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wPan >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wPan & 0x0f;
				pvBytes->push_back(b);
			}
			break;
		case TIOP_DMixVol:
			{
				// first convert to the realworld db.
				float fDB = -96.f + pparam->f01Val * (12+96);

				// convert to gain (this is what gets transmitted)
				float fGain = ::pow( 10.0f, (fDB / 20.f) );

				// This is a Q2.13 so multiply by 2^13
				fGain *= 8192.f;
				WORD wVol = (WORD)fGain;
				BYTE b = (wVol >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wVol & 0x0f;
				pvBytes->push_back(b);
			}
			break;

			case TIOP_DMixSend:
			{
				// first convert to the realworld db.
				float fDB = -96.f + pparam->f01Val * (12+96);

				// convert to gain (this is what gets transmitted)
				float fGain = ::pow( 10.0f, (fDB / 20.f) );

				// This is a Q2.13 so multiply by 2^13
				fGain *= 8192.f;
				WORD wVol = (WORD)fGain;
				BYTE b = (wVol >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wVol & 0x0f;
				pvBytes->push_back(b);
			}
			break;
		case TIOP_DMixReturn:
			{
				// first convert to the realworld db.
				float fDB = -96.f + pparam->f01Val * (12+96);

				// convert to gain (this is what gets transmitted)
				float fGain = ::pow( 10.0f, (fDB / 20.f) );

				// This is a Q2.13 so multiply by 2^13
				fGain *= 8192.f;
				WORD wVol = (WORD)fGain;
				BYTE b = (wVol >> 12) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 8) & 0x0f;
				pvBytes->push_back(b);
				b = (wVol >> 4) & 0x0f;
				pvBytes->push_back(b);
				b = wVol & 0x0f;
				pvBytes->push_back(b);
			}
			break;

		case TIOP_RevTime:
			{
				pvBytes->push_back( (BYTE)(pparam->f01Val * 49 ) );
			}
			break;
				case TIOP_RevType:
			{
				pvBytes->push_back( (BYTE)(pparam->f01Val) );
			}
			break;

		case TIOP_RevDelay:
			{
				pvBytes->push_back( (BYTE)(pparam->f01Val ));
			}
			break;
		case TIOP_Threshold:
			{
				BYTE by(0);
				int i = (int)(-128.f + pparam->f01Val * 50.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_Gate:
			{
				BYTE by(0);
				int i = (int)(-128.f + pparam->f01Val * 50.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_MakeupGain:
			{
				BYTE by(0);
				int i = (int)(-128.f + pparam->f01Val * 74.f);
				i = 128+i;
				by = (BYTE) (i & 0x7f );
				pvBytes->push_back( by );
			}
			break;
		case TIOP_Attack:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 25 ) );
			break;
		case TIOP_Release:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 45 ) );
			break;
		case TIOP_Ratio:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 8 ) );
			break;

		case TIOP_SyncSource:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 3 ) );
			break;

		case TIOP_DirectMix:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 3 ) );
			break;
		case TIOP_DigitalInput:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 1 ) );
			break;

		case TIOP_DMOutMain:
		case TIOP_DMOutSub:
		case TIOP_DMOutDig:
			pvBytes->push_back( (BYTE)(pparam->f01Val * 1 ) );
			break;

		case TIOP_OUT12:
		case TIOP_OUT34:
		case TIOP_OUT56:
		case TIOP_OUT78:
		case TIOP_OUT910:
			pvBytes->push_back( (BYTE)(pparam->f01Val) );
		break;


		}



	}

}


static void DumpChannel( TacomaIOChannel& ioc )
{
	TRACE( _T("Address Bytes: %x-%x-%x-%x\n"), ioc.abyAddress[0], ioc.abyAddress[1], ioc.abyAddress[2], ioc.abyAddress[3] );
	TRACE( _T("Size: %xH\n"), ioc.bySize );
	TRACE( _T("Params....\n") );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramPhantom.strName, ioc.paramPhantom.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramPhase.strName, ioc.paramPhase.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramPad.strName, ioc.paramPad.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramGain.strName, ioc.paramGain.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramStereLink.strName, ioc.paramStereLink.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n"),  ioc.paramLoCut.strName, ioc.paramLoCut.byAddressOffset );
	TRACE( _T("Name: %s  Offset: %x\n\n"),  ioc.paramCompEnable.strName, ioc.paramCompEnable.byAddressOffset );
}


////////////////////////////////////////////////////////////////////
// Set up all the parameter structures
void CIOBoxInterface::initParams()
{
	
	const UINT uActive = GetActiveInterface();

			// Build the sysx addresses for every parameter
for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )	
	
	{
		for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )

		{
			SetActiveInterface(UINT(ixBox));

			if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				ioc.abyAddress[2] = (BYTE)(iChan / 4);
				ioc.abyAddress[3] = (BYTE)((iChan % 4)* 0x20);

				BYTE byOffset = 0;
				ioc.paramPhantom.byAddressOffset = byOffset++;
				ioc.paramPhantom.strName = _T("+48");
				ioc.paramPhantom.eParam = TIOP_Phantom;

				ioc.paramLoCut.byAddressOffset = byOffset++;
				ioc.paramLoCut.strName = _T("Lo Cut");
				ioc.paramLoCut.eParam = TIOP_LoCut;

				ioc.paramPhase.byAddressOffset = byOffset++;
				ioc.paramPhase.strName = _T("Phase");
				ioc.paramPhase.eParam = TIOP_Phase;

				ioc.paramPad.byAddressOffset = byOffset++;
				ioc.paramPad.strName = _T("Pad");
				ioc.paramPad.eParam = TIOP_Pad;

				ioc.paramGain.byAddressOffset = byOffset++;
				ioc.paramGain.strName = _T("Mic Gn");
				ioc.paramGain.eParam = TIOP_Gain;

				ioc.paramStereLink.byAddressOffset = byOffset++;
				ioc.paramStereLink.strName = _T("St Lnk");
				ioc.paramStereLink.eParam = TIOP_StereoLink;

				ioc.paramCompEnable.byAddressOffset = byOffset++;
				ioc.paramCompEnable.strName = _T("Cmp Enab");
				ioc.paramCompEnable.eParam = TIOP_CompEnable;

				byOffset = 0x0b;
				ioc.paramDMixMono.byAddressOffset = byOffset++;
				ioc.paramDMixMono.strName = _T("DMix Mono");
				ioc.paramDMixMono.eParam = TIOP_DMixMono;

				ioc.paramDMixSolo.byAddressOffset = byOffset++;
				ioc.paramDMixSolo.strName = _T("DMix Solo");
				ioc.paramDMixSolo.eParam = TIOP_DMixSolo;

				ioc.paramDMixMute.byAddressOffset = byOffset++;
				ioc.paramDMixMute.strName = _T("DMix Mute");
				ioc.paramDMixMute.eParam = TIOP_DMixMute;

				ioc.paramDMixPan.byAddressOffset = 0x0e;
				ioc.paramDMixPan.strName = _T("DMix Pan");
				ioc.paramDMixPan.eParam = TIOP_DMixPan;

				ioc.paramDMixVol.byAddressOffset = 0x12;
				ioc.paramDMixVol.strName = _T("DMix Vol");
				ioc.paramDMixVol.eParam = TIOP_DMixVol;
			}
			else if ( (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP"))))
			{
				SetActiveInterface(UINT(ixBox));

				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				ioc.abyAddress[1] = (BYTE)(0x05);
				ioc.abyAddress[2] = (BYTE)(iChan);
				ioc.abyAddress[3] = 0x0;

				BYTE byOffset = 0;
				ioc.paramPhantom.byAddressOffset = byOffset++;
				ioc.paramPhantom.strName = _T("+48");
				ioc.paramPhantom.eParam = TIOP_Phantom;

				ioc.paramLoCut.byAddressOffset = byOffset++;
				ioc.paramLoCut.strName = _T("Lo Cut");
				ioc.paramLoCut.eParam = TIOP_LoCut;

				ioc.paramPhase.byAddressOffset = byOffset++;
				ioc.paramPhase.strName = _T("Phase");
				ioc.paramPhase.eParam = TIOP_Phase;

				ioc.paramHiz.byAddressOffset = byOffset++;
				ioc.paramHiz.strName = _T("Hi-Z");
				ioc.paramHiz.eParam = TIOP_Hiz;

				ioc.paramGain.byAddressOffset = byOffset++;
				ioc.paramGain.strName = _T("Mic Gn");
				ioc.paramGain.eParam = TIOP_Gain;

				ioc.paramStereLink.byAddressOffset = byOffset++;
				ioc.paramStereLink.strName = _T("St Lnk");
				ioc.paramStereLink.eParam = TIOP_StereoLink;

				ioc.paramCompEnable.byAddressOffset = byOffset++;
				ioc.paramCompEnable.strName = _T("Cmp Enab");
				ioc.paramCompEnable.eParam = TIOP_CompEnable;

				
			}
			//		DumpChannel( ioc );
		}
	}
	
				// Build the sysx addresses for OCTA IO parameters
	for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )

	{
		for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
		{
			SetActiveInterface(UINT(ixBox));

			for ( size_t iMixer = 0; iMixer < TacomaIOBox::NumDirectMix; iMixer++ )
			{
				if ( (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP"))))
				{

					TacomaIOBox::OctaIOChannel& oioc = m_OCChans[ixBox][iChan][iMixer];
					BYTE byOffset = 0;

					if (iChan <= 9)
					{
						oioc.abyAddress[1] = (BYTE)(0x06);
						oioc.abyAddress[2] = (BYTE)((iMixer<< 4 ) | (iChan));
						oioc.abyAddress[3] = (BYTE)0x0;
					}

					if ((iChan >= 10) && (iChan <= 19) )
					{
						oioc.abyAddress[1] = (BYTE)(0x07);
						oioc.abyAddress[2] = (BYTE)((iMixer<< 4 ) | (iChan - 10));
						oioc.abyAddress[3] = (BYTE)0x0;

					}

					//byOffset = 0x00;
					oioc.paramStereLinkOC.byAddressOffset = 0x00;
					oioc.paramStereLinkOC.strName = _T("OC St Lnk");
					oioc.paramStereLinkOC.eParam = TIOP_StereoLinkOC;

					//byOffset = 0x01;
					oioc.paramDMixMono.byAddressOffset = 0x01;
					oioc.paramDMixMono.strName = _T("OC DMix Mono");
					oioc.paramDMixMono.eParam = TIOP_DMixMono;

					//byOffset = 0x02;
					oioc.paramDMixSolo.byAddressOffset = 0x02;
					oioc.paramDMixSolo.strName = _T("OC DMix Solo");
					oioc.paramDMixSolo.eParam = TIOP_DMixSolo;

					oioc.paramDMixMute.byAddressOffset = 0x03;
					oioc.paramDMixMute.strName = _T("OC DMix Mute");
					oioc.paramDMixMute.eParam = TIOP_DMixMute;

					oioc.paramDMixPan.byAddressOffset = 0x04;
					oioc.paramDMixPan.strName = _T("OC DMix Pan");
					oioc.paramDMixPan.eParam = TIOP_DMixPan;

					//byOffset = 0x08;
					oioc.paramDMixVol.byAddressOffset = 0x08;
					oioc.paramDMixVol.strName = _T("OC DMix Vol");
					oioc.paramDMixVol.eParam = TIOP_DMixVol;

					//byOffset = 0x0E;
					oioc.paramDMixSend.byAddressOffset = 0x0E;
					oioc.paramDMixSend.strName = _T("OC DMix Send");
					oioc.paramDMixSend.eParam = TIOP_DMixSend;

					if ((iChan >= 20) && (iChan <= 21) )
					{
						oioc.abyAddress[1] = (BYTE)(0x08);
						oioc.abyAddress[2] = (BYTE)((iMixer<< 4 ) | (iChan - 20));
						oioc.abyAddress[3] = (BYTE)0x0;

						//byOffset = 0x00;
						oioc.paramStereLinkOC.byAddressOffset = 0x00;
						oioc.paramStereLinkOC.strName = _T("OC St Lnk");
						oioc.paramStereLinkOC.eParam = TIOP_StereoLinkOC;

						//byOffset = 0x01;
						oioc.paramDMixVol.byAddressOffset = 0x01;
						oioc.paramDMixVol.strName = _T("OC Master In Vol");
						oioc.paramDMixVol.eParam = TIOP_DMixVol;

						//byOffset = 0x0D;
						oioc.paramInLink.byAddressOffset = 0x0D;
						oioc.paramInLink.strName = _T("OC In-Out Lnk");
						oioc.paramInLink.eParam = TIOP_MasterLinkOC;


						oioc.paramDMixReturn.byAddressOffset = 0x07;
						oioc.paramDMixReturn.strName = _T("OC DMix Return");
						oioc.paramDMixReturn.eParam = TIOP_DMixReturn;


					}

					if ((iChan >= 22) && (iChan <= 23) )
					{
						oioc.abyAddress[1] = (BYTE)(0x09);
						oioc.abyAddress[2] = (BYTE)((iMixer<< 4 ) | (iChan - 22));
						oioc.abyAddress[3] = (BYTE)0x0;

						//byOffset = 0x00;
						oioc.paramStereLinkOC.byAddressOffset = 0x00;
						oioc.paramStereLinkOC.strName = _T("OC St Lnk");
						oioc.paramStereLinkOC.eParam = TIOP_StereoLinkOC;

						//byOffset = 0x01;
						oioc.paramDMixVol.byAddressOffset = 0x01;
						oioc.paramDMixVol.strName = _T("OC Master In Vol");
						oioc.paramDMixVol.eParam = TIOP_DMixVol;

					}
				}

			}

		}

	}


		

	
	// now the compressors
	for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
	{
		for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
		{
			SetActiveInterface(UINT(ixBox));

			if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				cmpc.abyAddress[3] = (BYTE)(0x10 * iChan);

				BYTE byOffset = 0x0;
				cmpc.paramThreshold.byAddressOffset = byOffset++;
				cmpc.paramThreshold.strName = _T("Thresh");
				cmpc.paramThreshold.eParam = TIOP_Threshold;

				cmpc.paramRatio.byAddressOffset = byOffset++;
				cmpc.paramRatio.strName = _T("Ratio");
				cmpc.paramRatio.eParam = TIOP_Ratio;

				cmpc.paramAttack.byAddressOffset = byOffset++;
				cmpc.paramAttack.strName = _T("Attack");
				cmpc.paramAttack.eParam = TIOP_Attack;

				cmpc.paramRelease.byAddressOffset = byOffset++;
				cmpc.paramRelease.strName = _T("Release");
				cmpc.paramRelease.eParam = TIOP_Release;

				cmpc.paramMakeupGain.byAddressOffset = byOffset++;
				cmpc.paramMakeupGain.strName = _T("Comp Gain");
				cmpc.paramMakeupGain.eParam = TIOP_MakeupGain;
			}
			else if ( (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP"))))
			{
				SetActiveInterface(UINT(ixBox));

				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				cmpc.abyAddress[1] = (BYTE)(0x05);
				cmpc.abyAddress[2] = (BYTE)(iChan);
				cmpc.abyAddress[3] = 0;

				BYTE byOffset = 0x07;

				cmpc.paramGate.byAddressOffset = byOffset++;
				cmpc.paramGate.strName = _T("Gate");
				cmpc.paramGate.eParam = TIOP_Gate;

				cmpc.paramAttack.byAddressOffset = byOffset++;
				cmpc.paramAttack.strName = _T("Attack");
				cmpc.paramAttack.eParam = TIOP_Attack;

				cmpc.paramRelease.byAddressOffset = byOffset++;
				cmpc.paramRelease.strName = _T("Release");
				cmpc.paramRelease.eParam = TIOP_Release;

				cmpc.paramThreshold.byAddressOffset = byOffset++;
				cmpc.paramThreshold.strName = _T("Thresh");
				cmpc.paramThreshold.eParam = TIOP_Threshold;

				cmpc.paramRatio.byAddressOffset = byOffset++;
				cmpc.paramRatio.strName = _T("Ratio");
				cmpc.paramRatio.eParam = TIOP_Ratio;
				
				cmpc.paramMakeupGain.byAddressOffset = byOffset++;
				cmpc.paramMakeupGain.strName = _T("Comp Gain");
				cmpc.paramMakeupGain.eParam = TIOP_MakeupGain;
			}
		}
	}

   // now the system messages
	for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
	{
		SetActiveInterface(UINT(ixBox));

		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
		{
			m_System[ixBox].paramUnitNumber.byAddressOffset = 0;
			m_System[ixBox].paramUnitNumber.eParam = TIOP_UnitNumber;
			m_System[ixBox].paramUnitNumber.strName = _T("Unit Num");

			m_System[ixBox].paramSamplingRate.byAddressOffset = 1;
			m_System[ixBox].paramSamplingRate.eParam = TIOP_SampleRate;
			m_System[ixBox].paramSamplingRate.strName = _T("Sample Rate");

			m_System[ixBox].paramSyncSource.byAddressOffset = 2;
			m_System[ixBox].paramSyncSource.eParam = TIOP_SyncSource;
			m_System[ixBox].paramSyncSource.strName = _T("Sync Source");

			m_System[ixBox].paramDigitalInput.byAddressOffset = 3;
			m_System[ixBox].paramDigitalInput.eParam = TIOP_DigitalInput;
			m_System[ixBox].paramDigitalInput.strName = _T("Dig In Sel");
		}
		else if ( (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP"))))
		{
			SetActiveInterface(UINT(ixBox));

			m_System[ixBox].abyAddress[1] = 0x02;
			m_System[ixBox].abyAddress[2] = 0x00;


			m_System[ixBox].paramUnitNumber.byAddressOffset = 0;
			m_System[ixBox].paramUnitNumber.eParam = TIOP_UnitNumber;
			m_System[ixBox].paramUnitNumber.strName = _T("Unit Num");

			m_System[ixBox].paramSamplingRate.byAddressOffset = 1;
			m_System[ixBox].paramSamplingRate.eParam = TIOP_SampleRate;
			m_System[ixBox].paramSamplingRate.strName = _T("Sample Rate");

			m_System[ixBox].paramSyncSource.byAddressOffset = 2;
			m_System[ixBox].paramSyncSource.eParam = TIOP_SyncSource;
			m_System[ixBox].paramSyncSource.strName = _T("Sync Source");

			m_System[ixBox].paramDirectMix.byAddressOffset = 3;
			m_System[ixBox].paramDirectMix.eParam = TIOP_DirectMix;
			m_System[ixBox].paramDirectMix.strName = _T("DirectMix");

		}

	}

	// now the DM output messages
	for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
	{
		SetActiveInterface(UINT(ixBox));

		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
		{
			m_DMOutput[ixBox].paramMainOut.byAddressOffset = 0;
			m_DMOutput[ixBox].paramMainOut.eParam = TIOP_DMOutMain;
			m_DMOutput[ixBox].paramMainOut.strName = _T("DM Out Main");

			m_DMOutput[ixBox].paramSubOut.byAddressOffset = 1;
			m_DMOutput[ixBox].paramSubOut.eParam = TIOP_DMOutSub;
			m_DMOutput[ixBox].paramSubOut.strName = _T("DM Out Sub");

			m_DMOutput[ixBox].paramDigOut.byAddressOffset = 2;
			m_DMOutput[ixBox].paramDigOut.eParam = TIOP_DMOutDig;
			m_DMOutput[ixBox].paramDigOut.strName = _T("DM Out Dig");
		}

		else if ( (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE"))) || (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP"))))
		{
			SetActiveInterface(UINT(ixBox));
			

			//OCTA Capture Patch Bay			
			TacomaIOBox::DMOutputChannel& dmoc = m_DMOutput[ixBox];

			dmoc.abyAddress[1] = (BYTE)(0x03);
			dmoc.abyAddress[2] = (BYTE)(0x00);
			dmoc.abyAddress[3] = (BYTE)(0x00);
			
			BYTE byOffset = 0x00;

			dmoc.paramOut12.byAddressOffset = byOffset++;
			dmoc.paramOut12.eParam = TIOP_OUT12;
			dmoc.paramOut12.strName = _T("OC OUT 1-2");

			dmoc.paramOut34.byAddressOffset = byOffset++;
			dmoc.paramOut34.eParam = TIOP_OUT34;
			dmoc.paramOut34.strName = _T("OC OUT 3-4");

			dmoc.paramOut56.byAddressOffset = byOffset++;
			dmoc.paramOut56.eParam = TIOP_OUT56;
			dmoc.paramOut56.strName = _T("OC OUT 5-6");

			dmoc.paramOut78.byAddressOffset = byOffset++;
			dmoc.paramOut78.eParam = TIOP_OUT78;
			dmoc.paramOut78.strName = _T("OC OUT 7-8");

			dmoc.paramOut910.byAddressOffset = byOffset++;
			dmoc.paramOut910.eParam = TIOP_OUT910;
			dmoc.paramOut910.strName = _T("OC OUT 9-10");
			

			//OCTA Reverb	
			TacomaIOBox::DMOutputChannel& dmrev = m_DMRev[ixBox];

			//Reverb Controls
			dmrev.abyAddress[1] = (BYTE)(0x04);
			dmrev.abyAddress[2] = (BYTE)(0x00);
			dmrev.abyAddress[3] = (BYTE)(0x00);

			byOffset = 0x0;
			dmrev.paramRevType.byAddressOffset = byOffset++;
			dmrev.paramRevType.strName = _T("OC Rev Type");
			dmrev.paramRevType.eParam = TIOP_RevType;
			
			dmrev.paramRevDelay.byAddressOffset = byOffset++;
			dmrev.paramRevDelay.strName = _T("OC Rev Delay");
			dmrev.paramRevDelay.eParam = TIOP_RevDelay;

			dmrev.paramRevTime.byAddressOffset = byOffset++;
			dmrev.paramRevTime.strName = _T("OC Rev Time");
			dmrev.paramRevTime.eParam = TIOP_RevTime;

		

		}

	}


	SetActiveInterface(uActive);
}


/////////////////////////////////////////////////////////////////////////////////
void	CIOBoxInterface::SetParam( DWORD dwChan, TacomaIOBoxParam p, float f01 )
{
	IOParam* pparam = getIOParam( dwChan, p );

	if ( !pparam )
		return;

	if (( p == TIOP_DMixVol) || ( p == TIOP_DMixSend)|| ( p == TIOP_DMixReturn) )
		f01 = ::pow( f01, 1/4.f );	// curve

	pparam->f01Val = f01;	// store locally in our object

	// get the appropriate channel object
	const bool bComp = (( p >= TIOP_Threshold && p <= TIOP_MakeupGain ) || ( p == TIOP_Gate ) ) ;
   const bool bSystem = (( p >= TIOP_UnitNumber && p <= TIOP_DigitalInput ) || (p == TIOP_DirectMix ));
	const bool bDMOut = ( p >= TIOP_DMOutMain && p <= TIOP_OUT910 );
	const bool bDMRev = ( p >= TIOP_RevType && p <= TIOP_RevDelay );
	const bool bOCDM = ( ( TIOP_DMixVol == p) || (TIOP_DMixSolo == p) || (TIOP_DMixPan == p) || (TIOP_DMixMute == p) ||
		( TIOP_DMixSend == p) || ( TIOP_DMixReturn == p) || ( TIOP_StereoLinkOC == p) || ( TIOP_MasterLinkOC == p) );

	BYTE byChan = 0x0;

	Channel ioc;

	if ( (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R2"))))
	{
		if ( bComp )
			ioc = m_aComps[m_nActiveIOBox][dwChan];
		else if ( bSystem )
			ioc = m_System[m_nActiveIOBox];
		else if ( bDMOut )
			ioc = m_DMOutput[m_nActiveIOBox];
		else
			ioc = m_aChans[m_nActiveIOBox][dwChan];
	}
	else
	{

		if ( bComp )
			ioc = m_aComps[m_nActiveIOBox][dwChan];
		else if ( bSystem )
			ioc = m_System[m_nActiveIOBox];
		else if ( bDMOut )
			ioc = m_DMOutput[m_nActiveIOBox];
		else if ( bDMRev )
		{
			ioc = m_DMRev[m_nActiveIOBox];

			if ( p != TIOP_RevType )
				ioc.abyAddress[2] = m_byRevType;
		}
		else if ( bOCDM )
		{
			ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];

			if ( p != TIOP_DMixReturn )
			{
				if ((dwChan >= 10) && (dwChan <= 19) )
					dwChan = dwChan - 10;
				else if ((dwChan >= 20) && (dwChan <= 21) )
					dwChan = dwChan - 20;
				else if ((dwChan >= 22) && (dwChan <= 23) )
					dwChan = dwChan - 22;

				ioc.abyAddress[2] = ( m_byDirectMix << 4 ) | (BYTE)dwChan;	
			}
		}
		else
			ioc = m_aChans[m_nActiveIOBox][dwChan];

}

	// send the DT sysx msg to the hardware
	sendParamSysx( m_nActiveIOBox, ioc, pparam );
}


//---------------------------------------------------------------
// Given a chan index, and param enum, return a normalized value
float	CIOBoxInterface::GetParam( DWORD dwChan, TacomaIOBoxParam p )
{
	IOParam* pparam = getIOParam( dwChan, p );

	float f = 0.f;
	if ( pparam )
		f = pparam->f01Val;

	if (( p == TIOP_DMixVol) || ( p == TIOP_DMixSend) || (p == TIOP_DMixReturn) )
		f = ::pow( f, 4.f );	// curve

	return f;
}

CString CIOBoxInterface::GetParamName( TacomaIOBoxParam p )
{
	IOParam* pparam = getIOParam( 0, p );
	return pparam->strName;
}

static CString s_aRatio[] = {_T("1.0:1"), _T("1.1:1"), _T("1.2:1"), _T("1.4:1"), _T("1.6:1"), _T("1.8:1"), 
										_T("2.0:1"), _T("2.5:1"), _T("3.2:1"), _T("4.0:1"), _T("5.6:1"), 
										_T("8.0:1"), _T("16.0:1"), _T("INF:1") };

static CString s_aRatioOC[] = {_T("1.0:1"), _T("1.2:1"), _T("1.5:1"), _T("2.0:1"), _T("2.8:1"), _T("4.0:1"), 
										_T("8.0:1"), _T("16:1"), _T("INF:1") };

static CString s_aAttack[] = {_T("0.2"),_T("0.5"),_T("1.0"),_T("2.0"),_T("4.0"),_T("6.0"),_T("8.0"),_T("10"),_T("15"),_T("20"),
	_T("25"),_T("30"),_T("35"),_T("40"),_T("45"),_T("50"),_T("55"),_T("60"),_T("65"),_T("70"),_T("75"),_T("80"),_T("85"),_T("90"),_T("95"),_T("100") };

static CString s_aRelease[] = {_T("10"),_T("12"),_T("15"),_T("18"),_T("20"),_T("25"),_T("30"),_T("35"),_T("40"),_T("45"),_T("50"),_T("55"),_T("60"),_T("65"),_T("70"),_T("75"),_T("80"),
	_T("85"),_T("90"),_T("95"),_T("100"),_T("110"),_T("120"),_T("130"),_T("140"),_T("150"),_T("160"),_T("170"),_T("180"),_T("190"),_T("200"),_T("220"),_T("240"),_T("260"),_T("280"),_T("300"),
	_T("320"),_T("340"),_T("360"),_T("380"),_T("400"),_T("420"),_T("440"),_T("460"),_T("480"),_T("500") };



CString CIOBoxInterface::GetParamValueText( TacomaIOBoxParam p, DWORD dwIxChan, float f01 )
{
	if ( f01 < 0.f || f01 > 1.f )
	{
		ASSERT(0);
		return _T("Range Err");
	}
	CString strVal;
	float fReal = 0.f;

	UINT ixBox = GetActiveInterface();

	if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))

	{
		switch( p )
		{
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
		case TIOP_DMixMono:
			strVal = f01 >= .5f ? _T("On") : _T("Off");
			return strVal;
			break;
		case TIOP_Threshold:
			fReal = -60.f + f01 * 60.f;
			strVal.Format( _T("%.1fdB"), fReal );
			break;
		case TIOP_Gate:
			fReal = -70.f + f01 * 50.f;
			strVal.Format( _T("%.1fdB"), fReal );
			break;
		case TIOP_Attack:
			fReal = .5f + f01 * (100.f - .5f);
			strVal.Format( _T("%.1fmS"), fReal );
			break;
		case TIOP_Release:
			fReal = 10.f + f01 * (500.f - 10.f);
			strVal.Format( _T("%.1fmS"), fReal );
			break;
		case TIOP_Ratio:
			{
				size_t ix = (size_t)(f01 * (_countof(s_aRatio) - 1));
				strVal = s_aRatio[ix];
			}
			break;
		case TIOP_MakeupGain:
			fReal = -30.f + f01 * 60.f;
			strVal.Format( _T("%.1fdB"), fReal );
			break;
		case TIOP_Phase:
			strVal = f01 >= .5f ? _T("Invrt") : _T("Norml");
			break;
		case TIOP_Gain:
			{
				float fAdd = 0.f;
#if 0		// risky so close to RTM
				// this one needs to pay attention to the pad setting for the channel.
				// If pad is off, add an additional 20dB to the display
				if ( dwIxChan < TacomaIOBox::NumMicInputChannels )
				{
					TacomaIOChannel& ioc = m_aChans[m_nActiveIOBox][dwIxChan];
					if ( ioc.paramPad.f01Val < .5f )
						fAdd = 20.f;
				}
#endif
				fAdd = 20.f;	//  just add fixed 20 instead of Pad-dependent value above
				fReal = fAdd + f01 * 44.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;
		case TIOP_DMixPan:
			fReal = -100.f + f01 * 200.f;
			strVal.Format( _T("%.1f%%"), fReal );
			break;
		case TIOP_DMixVol:
			f01 = ::pow( f01, 1/4.f );	// curve
			fReal = -96.f + f01 * (6.f + 96.f);
			if ( fReal <= -96.f )
				strVal = _T("-INF");
			else
			{
				if ( fReal > -.1f && fReal < .1f )
					fReal = 0.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;
		
		}

	}
	else
		//-----------------------------------------------------------------------------------------------------//
	{
		switch( p )
		{
		case TIOP_Phantom:
		case TIOP_Pad:
		case TIOP_Hiz:
		case TIOP_StereoLink:
		case TIOP_LoCut:
		case TIOP_CompEnable:
		case TIOP_DMixMute:
		case TIOP_DMixSolo:
		case TIOP_DMixMono:
		case TIOP_StereoLinkOC:
		case TIOP_MasterLinkOC:
			strVal = f01 >= .5f ? _T("On") : _T("Off");
			return strVal;
			break;
		case TIOP_Threshold:
			{
			fReal = -50.f + f01 * 50.f;
			strVal.Format( _T("%.1fdB"), fReal );
			//	strVal.Format( _T("%d"), int(fReal));
			}
			break;
		case TIOP_Gate:
			{
			fReal = -70.f + f01 * 50.f;
			strVal.Format( _T("%.1fdB"), fReal );
			//	strVal.Format( _T("%d"), int(fReal) );
			break;
			}
		case TIOP_Attack:
			{
				size_t ix = (size_t)(f01 * (_countof(s_aAttack) - 1));
				strVal = s_aAttack[ix];
			//	strVal.Format( _T("%.mS"), s_aAttack[ix] );
			}
			
			break;
		case TIOP_Release:
			{
				size_t ix = (size_t)(f01 * (_countof(s_aRelease) - 1));
				strVal = s_aRelease[ix];
			//	strVal.Format( _T("%.mS"), s_aRelease[ix] );
			}
			break;
		case TIOP_Ratio:
			{
				size_t ix = (size_t)(f01 * (_countof(s_aRatioOC) - 1));
				strVal = s_aRatioOC[ix];
			}
			break;
		case TIOP_MakeupGain:
			fReal = -50.f + f01 * 74.f;
			strVal.Format( _T("%.1fdB"), fReal );
			break;
		case TIOP_Phase:
			strVal = f01 >= .5f ? _T("Invrt") : _T("Norml");
			break;
		case TIOP_Gain:
			{
				float fAdd = 0.f;
#if 0		// risky so close to RTM
				// this one needs to pay attention to the pad setting for the channel.
				// If pad is off, add an additional 20dB to the display
				if ( dwIxChan < TacomaIOBox::NumMicInputChannels )
				{
					TacomaIOChannel& ioc = m_aChans[m_nActiveIOBox][dwIxChan];
					if ( ioc.paramPad.f01Val < .5f )
						fAdd = 20.f;
				}
#endif
				fAdd = 0.f;	//  just add fixed 20 instead of Pad-dependent value above
				fReal = f01 * 50.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;
		case TIOP_DMixPan:
			fReal = -100.f + f01 * 200.f;
			strVal.Format( _T("%.1f%%"), fReal );
			break;
		case TIOP_DMixVol:
			f01 = ::pow( f01, 1/4.f );	// curve
			fReal = -96.f + f01 * (12.f + 96.f);
			if ( fReal <= -96.f )
				strVal = _T("-INF");
			else
			{
				if ( fReal > -.1f && fReal < .1f )
					fReal = 0.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;

		case TIOP_DMixSend:
			f01 = ::pow( f01, 1/4.f );	// curve
			fReal = -96.f + f01 * (12.f + 96.f);
			if ( fReal <= -96.f )
				strVal = _T("-INF");
			else
			{
				if ( fReal > -.1f && fReal < .1f )
					fReal = 0.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;

		case TIOP_DMixReturn:
			f01 = ::pow( f01, 1/4.f );	// curve
			fReal = -96.f + f01 * (12.f + 96.f);
			if ( fReal <= -96.f )
				strVal = _T("-INF");
			else
			{
				if ( fReal > -.1f && fReal < .1f )
					fReal = 0.f;
				strVal.Format( _T("%.1fdB"), fReal );
			}
			break;	
		case TIOP_RevTime:
			{
			fReal = 0.1f + f01 * 4.9f;
			strVal.Format( _T("%.1fs"), fReal );
			break;
			}

		}


	}

	return strVal;
}



//-------------------------------------------------------------------
// Given a channel and a param enum, return the IOParam object
IOParam*	CIOBoxInterface::getIOParam( DWORD dwChan, TacomaIOBoxParam p )
{
//	if ( dwChan == 9 )
//	{
//		ASSERT(0);	// no such thing.  ix 8 is the Aux which is a MONO input (left only of the 8-9 pair)
//		return NULL;
//	}

	IOParam* pparam = NULL;

	const bool bComp = (( p >= TIOP_Threshold && p <= TIOP_MakeupGain ) || ( p == TIOP_Gate ));
   const bool bSystem = (( p >= TIOP_UnitNumber && p <= TIOP_DigitalInput ) || (p == TIOP_DirectMix ));
	const bool bDMOut = ( p >= TIOP_DMOutMain && p <= TIOP_RevDelay );

	if ( bComp )
	{
		if ( dwChan >= TacomaIOBox::NumMicInputChannels )
			return NULL;
		CompressorChannel& ioc = m_aComps[m_nActiveIOBox][dwChan];

		switch( p )
		{
		case TIOP_Threshold:
			pparam = &ioc.paramThreshold;
			break;
		case TIOP_Gate:
			pparam = &ioc.paramGate;
			break;
		case TIOP_Attack:
			pparam = &ioc.paramAttack;
			break;
		case TIOP_Release:
			pparam = &ioc.paramRelease;
			break;
		case TIOP_Ratio:
			pparam = &ioc.paramRatio;
			break;
		case TIOP_MakeupGain:
			pparam = &ioc.paramMakeupGain;
			break;
		}
	}
   else if ( bSystem )
   {
      switch ( p )
      {
         case TIOP_UnitNumber:
            pparam = &m_System[m_nActiveIOBox].paramUnitNumber;
         break;
         case TIOP_SampleRate:
            pparam = &m_System[m_nActiveIOBox].paramSamplingRate;
         break;
         case TIOP_SyncSource:
            pparam = &m_System[m_nActiveIOBox].paramSyncSource;
         break;
         case TIOP_DigitalInput:
            pparam = &m_System[m_nActiveIOBox].paramDigitalInput;
         break;
		 case TIOP_DirectMix:
            pparam = &m_System[m_nActiveIOBox].paramDirectMix;
         break;
      }
   }
	else if ( bDMOut )
	{
		switch ( p )
		{
			case TIOP_DMOutMain:
				pparam = & m_DMOutput[m_nActiveIOBox].paramMainOut;
			break;
			case TIOP_DMOutSub:
				pparam = & m_DMOutput[m_nActiveIOBox].paramSubOut;
			break;
			case TIOP_DMOutDig:
				pparam = & m_DMOutput[m_nActiveIOBox].paramDigOut;
			break;
			case TIOP_OUT12:
				pparam = & m_DMOutput[m_nActiveIOBox].paramOut12;
			break;
			case TIOP_OUT34:
				pparam = & m_DMOutput[m_nActiveIOBox].paramOut34;
			break;
			case TIOP_OUT56:
				pparam = & m_DMOutput[m_nActiveIOBox].paramOut56;
			break;
			case TIOP_OUT78:
				pparam = & m_DMOutput[m_nActiveIOBox].paramOut78;
			break;
			case TIOP_OUT910:
				pparam = & m_DMOutput[m_nActiveIOBox].paramOut910;
			break;
			case TIOP_RevTime:
				pparam = & m_DMRev[m_nActiveIOBox].paramRevTime;
			break;

			case TIOP_RevType:
				pparam = & m_DMRev[m_nActiveIOBox].paramRevType;
			break;

			case TIOP_RevDelay:
				pparam = & m_DMRev[m_nActiveIOBox].paramRevDelay;
			break;
		}
	}
	else
	{
		if ( dwChan >= TacomaIOBox::NumChannels )
			return NULL;
		TacomaIOChannel& ioc = m_aChans[m_nActiveIOBox][dwChan];

		switch( p )
		{
		case TIOP_Phantom:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramPhantom;
			break;
		case TIOP_Pad:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramPad;
			break;
		case TIOP_Hiz:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramHiz;
			break;
		case TIOP_Phase:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramPhase;
			break;
		case TIOP_Gain:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramGain;
			break;
		case TIOP_StereoLink:
			{
			if ( (dwChan % 2) == 0 )
				pparam = &ioc.paramStereLink;
			}
			break;
		case TIOP_LoCut:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramLoCut;
			break;
		case TIOP_CompEnable:
			if ( dwChan < NumMicInputChannels )
				pparam = &ioc.paramCompEnable;
			break;
		case TIOP_DMixMono:
			pparam = &ioc.paramDMixMono;;
			break;
		case TIOP_DMixMute:
			if ( (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R2"))))
			pparam = &ioc.paramDMixMute;
			else
			{
				OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
				pparam = &ioc.paramDMixMute;
			}
			break;
		case TIOP_DMixSolo:
			if ( (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R2"))))
			pparam = &ioc.paramDMixSolo;
			else
			{
				OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
				pparam = &ioc.paramDMixSolo;
			}
			break;
		case TIOP_DMixPan:
			if ( (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R2"))))
			pparam = &ioc.paramDMixPan;
			else
			{
				OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
				pparam = &ioc.paramDMixPan;
			}
			break;
		case TIOP_DMixVol:
			if ( (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R"))) || (m_viofacestr[ m_nActiveIOBox ] == (_T("VS-700R2"))))
			pparam = &ioc.paramDMixVol;
			else
			{
				OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
				pparam = &ioc.paramDMixVol;
			}
			break;
		case TIOP_DMixSend:
			{
			OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
			pparam = &ioc.paramDMixSend;
			}
			break;
		case TIOP_DMixReturn:
			{
			OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
			pparam = &ioc.paramDMixReturn;
			}
			break;
		case TIOP_StereoLinkOC:
			{
			OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
			if ( (dwChan % 2) == 0 )
				pparam = &ioc.paramStereLinkOC;
			}
			break;

		case TIOP_MasterLinkOC:
			{
				OctaIOChannel& ioc = m_OCChans[m_nActiveIOBox][dwChan][m_byDirectMix];
				pparam = &ioc.paramInLink;
			}
			break;
		}
	}

	ASSERT( pparam );
	return pparam;
}


//---------------------------------------------------------------------------------------
// Given an IO channel, Fill an ALLOCATED byte buffer with the initial part of the sysx
// string up to and including the addressing bytes.  Also return the checksum computed so far
// and the count of bytes
void CIOBoxInterface::makeSysxStringForChannel( const TacomaIOBox::Channel& ioc,
															  BYTE byAddressOffset,
															  bool bRQ,
															  BYTE* pbySysx, 
															  BYTE* pbyCheckSum, 
															  size_t* pcLen )
{
	UINT u = GetActiveInterface();

	BYTE* pBuf = pbySysx;
	  
	 if ( (m_viofacestr[ u ] == (_T("VS-700R"))) || (m_viofacestr[ u ] == (_T("VS-700R2"))) )
		 // copy the prefix
	 {
		 size_t cPrefix = sizeof(s_abyPrefix);
		 ::memcpy( pBuf, s_abyPrefix, cPrefix );
		 pBuf += cPrefix;
	 }
	 else
	 {
		 size_t cPrefix = sizeof(s_abyPrefixOC);
		 ::memcpy( pBuf, s_abyPrefixOC, cPrefix );
		 pBuf += cPrefix;
	 }

	// now the DT  byte:  0x12 = DT, 0x11 = RQ
	*pBuf++ = bRQ ? 0x11 : 0x12;

	// copy the address for the channel
	// address bytes are in 7,8,9,10
	ASSERT( pBuf - pbySysx == 7 );
	::memcpy( pBuf, ioc.abyAddress, 4 );


	// Set the address offset
	*(pBuf + 3) += byAddressOffset;

	// checksum
	BYTE byCS = 0;
	for ( size_t iA = 0; iA < 4; iA++ )
		byCS += *(pBuf + iA);

	pBuf += 4;	// to the data 

	*pbyCheckSum = byCS;	// return the checksum so far

	*pcLen = (size_t)(pBuf - pbySysx);
}




void CIOBoxInterface::SendAllPreampParams()
{
	//for ( int ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
	//{
UINT ixBox = m_nActiveIOBox;

		for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
		{
			if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))) )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				sendParamSysx( ixBox, ioc, &ioc.paramGain );
				sendParamSysx( ixBox, ioc, &ioc.paramLoCut );
				sendParamSysx( ixBox, ioc, &ioc.paramPad );
				sendParamSysx( ixBox, ioc, &ioc.paramPhase );
				sendParamSysx( ixBox, ioc, &ioc.paramStereLink );
				sendParamSysx( ixBox, ioc, &ioc.paramCompEnable );
			}
			else
			{
				
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				sendParamSysx( ixBox, ioc, &ioc.paramLoCut );
				sendParamSysx( ixBox, ioc, &ioc.paramPhase );
				sendParamSysx( ixBox, ioc, &ioc.paramStereLink );
				sendParamSysx( ixBox, ioc, &ioc.paramCompEnable );
			}

			
		}
	
		// the comps
		for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
		{
			if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				sendParamSysx( ixBox, cmpc, &cmpc.paramAttack );
				sendParamSysx( ixBox, cmpc, &cmpc.paramMakeupGain );
				sendParamSysx( ixBox, cmpc, &cmpc.paramRatio );
				sendParamSysx( ixBox, cmpc, &cmpc.paramRelease );
				sendParamSysx( ixBox, cmpc, &cmpc.paramThreshold );
			}
			else
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				sendParamSysx( ixBox, cmpc, &cmpc.paramGate );
				sendParamSysx( ixBox, cmpc, &cmpc.paramAttack );
				sendParamSysx( ixBox, cmpc, &cmpc.paramRelease );
				sendParamSysx( ixBox, cmpc, &cmpc.paramThreshold );
				sendParamSysx( ixBox, cmpc, &cmpc.paramRatio );
				sendParamSysx( ixBox, cmpc, &cmpc.paramMakeupGain );
			}
			
		}

	//}
}


void CIOBoxInterface::sendAllDMParams()
{
	//for ( size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
	//{

	UINT ixBox = m_nActiveIOBox;

		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))))
		{
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				sendParamSysx( ixBox, ioc, &ioc.paramDMixVol );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixPan );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixMute );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixSolo );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixMono );
			}

			// DM output
			sendParamSysx( ixBox, m_DMOutput[ixBox], &m_DMOutput[ixBox].paramDigOut );
			sendParamSysx( ixBox, m_DMOutput[ixBox], &m_DMOutput[ixBox].paramMainOut );
			sendParamSysx( ixBox, m_DMOutput[ixBox], &m_DMOutput[ixBox].paramSubOut );
		}
		else
		{
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::OctaIOChannel& ioc = m_OCChans[ixBox][iChan][m_byDirectMix];
				sendParamSysx( ixBox, ioc, &ioc.paramDMixPan );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixMute );
				sendParamSysx( ixBox, ioc, &ioc.paramDMixSolo );
				sendParamSysx( ixBox, ioc, &ioc.paramInLink );
				sendParamSysx( ixBox, ioc, &ioc.paramStereLinkOC );
			}

			
		}


	//}
}


void CIOBoxInterface::sendAllDMOutParams()
{
	for ( size_t ixBox = 0; ixBox < 2; ixBox++ )
	{
		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))) )
		{
			sendParamSysx( UINT(ixBox), m_DMOutput[ixBox], &m_DMOutput[ixBox].paramMainOut );
			sendParamSysx( UINT(ixBox), m_DMOutput[ixBox], &m_DMOutput[ixBox].paramSubOut );
			sendParamSysx( UINT(ixBox), m_DMOutput[ixBox], &m_DMOutput[ixBox].paramDigOut );
		}
	}
}


//-----------------------------------------------------------
// Given a Channel object and a IOParam object within the channel, send a DT 
// message to the hardware
void CIOBoxInterface::sendParamSysx( UINT ixBox, const TacomaIOBox::Channel& ioc, IOParam* pparam )
{
	if ( ixBox < GetInterfaceCount() )
	{
		// send the sysx for this param
		BYTE abyMsg[255];
		BYTE byCS = 0;
		size_t cLen = 0;

		makeSysxStringForChannel( ioc, pparam->byAddressOffset, false, abyMsg, &byCS, &cLen );

		BYTE* pBuf = abyMsg + cLen;

		// now the data 
		std::vector<BYTE> vData;
		bytesFromValue( pparam, &vData );
		for ( size_t iD = 0; iD < vData.size(); iD++ )
		{
			BYTE byData = vData[iD];
			*pBuf = byData;
			byCS += *pBuf;
			pBuf++;
		}

		// compute checksum.
		BYTE byMod = byCS % 128;
		byCS = 128 - byMod;
		byCS &= 0x7f;

		*pBuf++ = byCS;

		*pBuf = 0xf7;	// EOX


		// and send
		{
			DWORD dwLen = (DWORD)(pBuf - abyMsg) + 1;
			VERIFY(SUCCEEDED( send( ixBox, abyMsg, dwLen ) ) );
		}
	}
}


//--------------------------------------------------------------
// Given an IO channel, send a RQ sysx message for the data
void CIOBoxInterface::requestChannel( UINT ixBox, TacomaIOBox::Channel& ioc )
{
	if ( ixBox < GetInterfaceCount() )
	{
		BYTE abyMsg[255];
		BYTE byCS = 0;
		size_t cLen = 0;

		makeSysxStringForChannel( ioc, 0, true, abyMsg, &byCS, &cLen );
		BYTE* pBuf = abyMsg + cLen;

	 UINT u = GetActiveInterface();
	
	 if ( (m_viofacestr[ u ] == (_T("VS-700R"))) || (m_viofacestr[ u ] == (_T("VS-700R2"))) )
	 {
		 // size of request.  size is 4 bytes
		*pBuf++ = 0x00;
		*pBuf++ = 0x00;
		*pBuf++ = 0x00;
		*pBuf++ = ioc.bySize;

		byCS += ioc.bySize;
	 }
	 else
	 {
		*pBuf++ = 0x00;
		*pBuf++ = 0x00;
		*pBuf++ = 0x08;
		*pBuf++ = 0x7A;
		 byCS += ioc.bySize;
	 }
	 
		

		// compute checksum.
		BYTE byMod = byCS % 128;
		byCS = 128 - byMod;

		*pBuf++ = byCS;

		*pBuf = 0xf7;	// EOX

		// and send
		DWORD dwLen = (DWORD)(pBuf - abyMsg) + 1;
		VERIFY(SUCCEEDED( send( ixBox, abyMsg, dwLen ) ) );

	}
}

HRESULT CIOBoxInterface::send( UINT ixBox, BYTE abyMsg[], DWORD dwLen )
{
	ASSERT( ixBox < GetInterfaceCount() );
	if ( s_bTraceSysx )
	{
		CString str;
		TRACE( _T("\nSending Sysx\n") );
		for ( DWORD i = 0; i < dwLen; i++ )
		{
			if ( i )
				str.Format( _T(",%0x"), abyMsg[i] );
			else
				str.Format( _T("%0x"), abyMsg[i] );
			TRACE( str );
		}
	}

	return m_pMidiOuts->SendMidiLong( getPortForBox( ixBox ), abyMsg, dwLen );
}



UINT CIOBoxInterface::IOBoxIndexFromPort( UINT nPort )
{
	const UINT cIfs = GetInterfaceCount();
	const UINT uActive = GetActiveInterface();

	for ( UINT ixBox = 0; ixBox < cIfs; ixBox++ )
	{
		 if ( getPortForBox( uActive ) == nPort ) 
			return uActive;
	}
	ASSERT(0);
	return 0;
}


UINT CIOBoxInterface::getPortForBox( UINT ixBox )
{
	if  (m_viofacestr[ ixBox ] == (_T("VS-700R")))  
		return m_aIOBoxPorts[0];
	else if (m_viofacestr[ ixBox ] == (_T("VS-700R2")))
		return m_aIOBoxPorts[1];
	else if (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE")))
		return m_aIOBoxPorts[2];
	else if (m_viofacestr[ ixBox ] == (_T("OCTA-CAPTURE EXP")))
		return m_aIOBoxPorts[3];

	ASSERT(0);
	return (UINT)-1;
}

//--------------------------------------------------------------
void CIOBoxInterface::KillPhantoms()
{
	for (size_t ixBox = 0; ixBox <  GetInterfaceCount(); ixBox++ )
	{
		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))) )
		{
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				ioc.paramPhantom.f01Val = 0.f;
				sendParamSysx( UINT(ixBox), ioc, &ioc.paramPhantom );
			}
		}
	}
}


void CIOBoxInterface::SetActiveInterface( UINT u )
{
	if ( !m_pMidiOuts || !m_pMidiSource )
		return;

	if ( u >= m_pMidiOuts->GetPortCount() || u >= m_pMidiSource->GetPortCount() )
	{
		if (!HasMidiIO())
		{
			TRACE( "No MIDI I/O Ports\n" );
			m_nActiveIOBox = 0;
		}
		else
			ASSERT( 0 );

		return;
	}

	m_nActiveIOBox = u;
}


UINT	CIOBoxInterface::GetInterfaceCount()
{
	return ( m_pMidiOuts ? m_pMidiOuts->GetPortCount() : 0 );
}

//--------------------------------------------------------------
// Send sysx requests for every channel
void CIOBoxInterface::GetInitialValues()
{
	if ( !HasMidiIO() )
		return;

	CWaitCursor wc;

	const UINT uActive = GetActiveInterface();

	for (size_t ixBox = 0; ixBox < GetInterfaceCount(); ixBox++ )
	{
		SetActiveInterface(UINT(ixBox));

		if ( (m_viofacestr[ ixBox ] == (_T("VS-700R"))) || (m_viofacestr[ ixBox ] == (_T("VS-700R2"))) )
		{
			for   ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				::ResetEvent( m_hSysx );
				requestChannel( UINT(ixBox), ioc );

				if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 750 ) )
				{
					TRACE( "Time-out waiting for sysx for Channel %d\n", iChan );
				}
		
			}

			// now the compressors
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				::ResetEvent( m_hSysx );
				requestChannel( UINT(ixBox), cmpc );
				if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 700 ) )
				{
					TRACE( "Time-out waiting for sysx for Channel %d\n", iChan );
				}
			}

			// and DM output
			::ResetEvent( m_hSysx );
			requestChannel( UINT(ixBox), m_DMOutput[ixBox] );
			if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 700 ) )
				TRACE( "Time-out waiting for sysx for Direct Mixer values\n" );

			// now system messages
			::ResetEvent( m_hSysx );
			requestChannel( UINT(ixBox), m_System[ixBox] );
			if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 700 ) )
				TRACE( "Time-out waiting for sysx for System values\n" );
		}
		else
		{
			m_System[ixBox].abyAddress[0] =0x01;
			m_System[ixBox].abyAddress[1] =0x00;

			/*
			for   ( size_t iChan = 0; iChan < TacomaIOBox::NumChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_aChans[ixBox][iChan];
				::ResetEvent( m_hSysx );
				requestChannel( ixBox, ioc );

				if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 750 ) )
				{
					TRACE( "Time-out waiting for sysx for Channel %d\n", iChan );
				}
		
			}

			 //now the compressors
			for ( size_t iChan = 0; iChan < TacomaIOBox::NumMicInputChannels; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_aComps[ixBox][iChan];
				::ResetEvent( m_hSysx ); 
				requestChannel( ixBox, cmpc );
				if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 700 ) )
				{
					TRACE( "Time-out waiting for sysx for Channel %d\n", iChan );
				}
			}
			*/
			
			// now system messages
			::ResetEvent( m_hSysx );
			requestChannel( UINT(ixBox), m_System[ixBox] );
			if ( WAIT_TIMEOUT == ::WaitForSingleObject( m_hSysx, 1400 ) )
				TRACE( "Time-out waiting for sysx for System values\n" );
			
		}

	}

	SetActiveInterface(uActive);
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// Implement ITacomaIOBox for the surface
HRESULT	CTacomaSurface::GetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float* pf01 )
{
	if ( !m_pIOBoxInterface )
		return E_NOINTERFACE;

	if ( dwChan >= TacomaIOBox::NumChannels )
		return E_INVALIDARG;
	if ( p >= TacomaIOBox::NumChannels )
		return E_INVALIDARG;

	*pf01 = m_pIOBoxInterface->GetParam( dwChan, p );

	return S_OK;
}

HRESULT CTacomaSurface::SetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float f01 )
{
	if ( !m_pIOBoxInterface )
		return E_NOINTERFACE;

	if ( dwChan >= TacomaIOBox::NumChannels )
		return E_INVALIDARG;
	if ( p >= TacomaIOBox::NumChannels )
		return E_INVALIDARG;

	m_pIOBoxInterface->SetParam( dwChan, p, f01 );

	return S_OK;
}


HRESULT CTacomaSurface::GetIOBoxParamName( TacomaIOBoxParam p, CString* pstr )
{
	if ( !m_pIOBoxInterface )
		return E_NOINTERFACE;
	*pstr = m_pIOBoxInterface->GetParamName( p );

	return S_OK;
}


HRESULT CTacomaSurface::GetIOBoxParamValueText( TacomaIOBoxParam p, DWORD dwIxChan, float f01, CString* pstr )
{
	if ( !m_pIOBoxInterface )
		return E_NOINTERFACE;
	*pstr = m_pIOBoxInterface->GetParamValueText( p, dwIxChan, f01 );

	return S_OK;
}

HRESULT CTacomaSurface::RefreshFromHardware(  )
{
	if ( !m_pIOBoxInterface )
		return E_NOINTERFACE;
	
	m_pIOBoxInterface->GetInitialValues();
	return S_OK;
}

