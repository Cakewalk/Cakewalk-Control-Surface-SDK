#pragma once

#include "MidiSink.h"
#include "MidiSource.h"

#include <PersistDDX.h>
#include <TTSPersistObject.h>


// {96C20727-8715-4c55-B268-7831CE06789E}
static const GUID IID_ITacomaIOBox = 
{ 0x96c20727, 0x8715, 0x4c55, { 0xb2, 0x68, 0x78, 0x31, 0xce, 0x6, 0x78, 0x9e } };


enum TacomaIOBoxParam
{
	TIOP_Phantom = 0,
	TIOP_Phase,
	TIOP_Pad,
	TIOP_Gain,
	TIOP_StereoLink,
	TIOP_LoCut,
	TIOP_CompEnable,
	TIOP_DMixMono,
	TIOP_DMixMute,
	TIOP_DMixSolo,
	TIOP_DMixPan,
	TIOP_DMixVol,

	// comp params (keep together)
	TIOP_Threshold,
	TIOP_Attack,
	TIOP_Release,
	TIOP_Ratio,
	TIOP_MakeupGain,

   // system params (keep together)
   TIOP_UnitNumber,
   TIOP_SampleRate,
   TIOP_SyncSource,     // Internal, Dig1, Dig2, WordClock
   TIOP_DigitalInput,   // coax, AES/EBU

	// direct mixer output
	TIOP_DMOutMain,
	TIOP_DMOutSub,
	TIOP_DMOutDig,
	TIOP_OUT12,
	TIOP_OUT34,
	TIOP_OUT56,
	TIOP_OUT78,
	TIOP_OUT910,
	TIOP_RevType,
	TIOP_RevTime,
	TIOP_RevDelay,
	TIOP_Hiz,
	TIOP_Gate,
	TIOP_DMixSend,
	TIOP_DirectMix,
	TIOP_DMixReturn,
	TIOP_StereoLinkOC,
	TIOP_MasterLinkOC,
	
	TIOP_MAX
};

interface ITacomaIOBox
{
	virtual HRESULT STDMETHODCALLTYPE GetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float* pf01 ) PURE;
	virtual HRESULT STDMETHODCALLTYPE SetIOBoxParam( DWORD dwChan, TacomaIOBoxParam p, float f01 ) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetIOBoxParamName( TacomaIOBoxParam p, CString* pstr ) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetIOBoxParamValueText( TacomaIOBoxParam p, DWORD dwixChan, float f01, CString* pstr ) PURE;
	virtual HRESULT STDMETHODCALLTYPE RefreshFromHardware( ) PURE;
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class CTacomaSurface;

namespace TacomaIOBox
{
	// Struct for one single parameter
	struct IOParam
	{
		IOParam() :
         f01Val(0.f),
         byAddressOffset( 0 ),
         eParam( TIOP_Phantom )
      {
      }

		float		f01Val;
		CString	strName;
		BYTE		byAddressOffset;
		TacomaIOBoxParam eParam;
	};

	struct Channel
	{
		BYTE			abyAddress[4];		// 4 byte sysx start address for this channel
		BYTE			bySize;				// size of data
	};

	struct CompressorChannel : public Channel
	{
		CompressorChannel()
		{
			abyAddress[0] = 0x00;
			abyAddress[1] = 0x03;
			abyAddress[2] = 0x00;
			bySize = 5;
		};
		
		IOParam		paramGate;
		IOParam		paramAttack;
		IOParam		paramRelease;
		IOParam		paramThreshold;
		IOParam		paramRatio;
		IOParam		paramMakeupGain;

	};

	// struct for all IO params on a single channel
	struct TacomaIOChannel : public Channel
	{
		TacomaIOChannel()
		{
			abyAddress[0] = 0x00;
			abyAddress[1] = 0x01;
			bySize = 0x16;
		};
		IOParam		paramPhantom;		// 0/1
		IOParam		paramPhase;			// 0/1
		IOParam		paramPad;			// 0/1
		IOParam		paramHiz;			// 0/1
		IOParam		paramGain;			// 0..127
		IOParam		paramStereLink;	// 0/1
		IOParam		paramLoCut;			// 0/1
		IOParam		paramCompEnable;	// 0/1
		IOParam		paramDMixMono;		// 0/1
		IOParam		paramDMixMute;		// 0/1
		IOParam		paramDMixSolo;		// 0/1
		IOParam		paramDMixPan;		// 0..16
		IOParam		paramDMixVol;		// 0..131072
	};

		// struct for all IO params on a single channel
	struct OctaIOChannel : public Channel
	{
		OctaIOChannel()
		{
			abyAddress[0] = 0x00;
			abyAddress[1] = 0x01;
			bySize = 0x16;
		};
		IOParam		paramDMixMono;		// 0/1
		IOParam		paramDMixMute;		// 0/1
		IOParam		paramDMixSolo;		// 0/1
		IOParam		paramDMixPan;		// 0..16
		IOParam		paramDMixVol;		// 0..131072
		IOParam		paramDMixSend;
		IOParam		paramStereLinkOC;
		IOParam		paramInLink;
		IOParam		paramDMixReturn;
	};

   struct TacomaSystemChannel : public Channel
   {
		TacomaSystemChannel()
		{
			abyAddress[0] = 0x00;
			abyAddress[1] = 0x00;
			abyAddress[2] = 0x02;
			abyAddress[3] = 0x00;
			bySize = 4;
		};

      IOParam     paramUnitNumber;     // 0..3
      IOParam     paramSamplingRate;   // 0..4
      IOParam     paramSyncSource;     // 0..3
      IOParam     paramDigitalInput;   // 0/1
	  IOParam     paramDirectMix;   // 0/1
	 
   };

	struct DMOutputChannel : public Channel
   {
		DMOutputChannel()
		{
			abyAddress[0] = 0x00;
			abyAddress[1] = 0x02;
			abyAddress[2] = 0x00;
			abyAddress[3] = 0x00;
			bySize = 3;
		};

      IOParam     paramMainOut;  // 0/1
      IOParam     paramSubOut;   // 0/1
      IOParam     paramDigOut;	// 0/1
	  IOParam     paramOut12;
	  IOParam     paramOut34;
	  IOParam     paramOut56;
	  IOParam     paramOut78;
	  IOParam     paramOut910;
	  IOParam     paramDirectMixA;
	  IOParam     paramDirectMixB;
	  IOParam     paramDirectMixC;
	  IOParam     paramDirectMixD;
	  IOParam	  paramRevType;
	  IOParam	  paramRevTime;
	  IOParam	  paramRevDelay;

	 

   };


	// channel counts
	enum { NumChannels = 24, NumMicInputChannels = 8, NumDirectMix = 4 };

};	// end namespace



////////////////////////////////////////////////////////////////////////////
// A class to abstract the IO box settings.  This class will represent the
// states of the IO box and handle the details of tranmitting/receiving
// the states from the hardware via Midi
class CIOBoxInterface : public IMidiSink
{
public:
	friend class CDMPersist;
	friend class CPreampPersist;

	CIOBoxInterface( CTacomaSurface* pSurface );
	~CIOBoxInterface(void);

	bool	HasMidiIO();
	bool	HasConsolePorts();

// IMidiSink
	virtual BOOL OnMidiShortMsg( DWORD dwMsg, DWORD dwTime, UINT nPort );
	virtual BOOL OnMidiLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg, UINT nPort );
	virtual void OnPortsChanged(){};

   void	SetParam( DWORD dwChan, TacomaIOBoxParam p, float f01 );
	float	GetParam( DWORD dwChan, TacomaIOBoxParam p );
	CString GetParamName( TacomaIOBoxParam p );
	CString GetParamValueText( TacomaIOBoxParam p, DWORD dwIxChan, float f01 );

	void		GetInitialValues();
	void		SendAllPreampParams();
	void		KillPhantoms();

	UINT		GetActiveInterface() const { return m_nActiveIOBox; }
	void		SetActiveInterface( UINT u );
	UINT		GetInterfaceCount();

    std::vector<CString>	m_viofacestr;
	float					m_fSyncVal;
	BYTE					m_byDirectMix;
	BYTE					m_byRevType;
	BYTE					m_byDelay;
	bool					m_bMidiVerb;
	bool					m_bMidiDelay;
	bool					m_bMidiPatch;
	bool					m_bLinked;
	bool					m_bLinkclick;
	bool					m_bConsoleFader;
	int						m_iOUT12;
	int						m_iOUT34;
	int						m_iOUT56;
	int						m_iOUT78;
	int						m_iOUT910;

	int						m_iECHO_T;
	int						m_iROOM_T;
	int						m_iSHALL_T;
	int						m_iLHALL_T;
	int						m_iOFF_T;

	int						m_iECHO_D;
	int						m_iROOM_D;
	int						m_iSHALL_D;
	int						m_iLHALL_D;
	int						m_iOFF_D;




		typedef struct ThreeNibles
	{
		BYTE chan1:1; // 1bit allocation
		BYTE chan2:1;
		BYTE chan3:1;
		BYTE chan4:1;
		BYTE chan5:1;
		BYTE chan6:1;
		BYTE chan7:1;
		BYTE chan8:1;
		BYTE chan9:1;
		BYTE chan10:1;
	} THREE_NIBS;

	typedef struct TwoNibles
	{
		BYTE chan1:1; // 1bit allocation
		BYTE chan2:1;
		BYTE chan3:1;
		BYTE chan4:1;
		BYTE chan5:1;
		BYTE chan6:1;
		BYTE chan7:1;
		BYTE chan8:1;
	} CHAN_NIBS;

		typedef struct NibleChan
	{
		BYTE chan0:1; // 1bit allocation; Stereo Link Switch
		BYTE chan2:1;
		BYTE chan4:1;
		BYTE chan6:1;
	} NIB_CHAN;

				typedef struct LinkNible
	{
		BYTE chan0:1; // 1bit allocation; Stereo Link Switch
		BYTE chan2:1;
		BYTE chan4:1;
		BYTE chan6:1;
		BYTE chan8:1;
	} NIB_LINK;


	typedef struct OneNible
	{
		BYTE chan1:1; // 1bit allocation; Stereo Link Switch
		BYTE chan2:1;
		BYTE chan3:1;
		BYTE chan4:1;
	} ONE_NIB;


private:
	void		findPorts();

	void		initParams();
	void		requestChannel( UINT ixBox, TacomaIOBox::Channel& ioc );
	TacomaIOBox::IOParam*	getIOParam( DWORD dwChan, TacomaIOBoxParam p );
	void		sendAllDMParams();
	void		sendAllDMOutParams();
	void		sendParamSysx( UINT ixBox, const TacomaIOBox::Channel&, TacomaIOBox::IOParam* pparam );
	void		makeSysxStringForChannel( const TacomaIOBox::Channel& ioc, BYTE byAddrOffset, bool, BYTE* pbySysx, BYTE* pbyCheckSum, size_t* pcLen );
	size_t	valueFromBytes( TacomaIOBox::IOParam* pparam, BYTE* pbyVal );
	void		bytesFromValue( TacomaIOBox::IOParam* pparam, std::vector<BYTE>* pvBytes );

	HRESULT	send( UINT ixBox, BYTE abyMsg[], DWORD dwLen );

	// Array of Channel structs
	TacomaIOBox::TacomaIOChannel			m_aChans[2][TacomaIOBox::NumChannels];
	TacomaIOBox::CompressorChannel			m_aComps[2][TacomaIOBox::NumMicInputChannels];
	TacomaIOBox::TacomaSystemChannel		m_System[2];
	TacomaIOBox::DMOutputChannel			m_DMRev[2];
	TacomaIOBox::DMOutputChannel			m_DMOutput[2];
	TacomaIOBox::DMOutputChannel			m_OCOutput[2][TacomaIOBox::NumDirectMix];
	TacomaIOBox::OctaIOChannel				m_OCChans[2][TacomaIOBox::NumChannels][TacomaIOBox::NumDirectMix];

	UINT			m_nActiveIOBox;
	UINT			m_aIOBoxPorts[3];
	UINT			getPortForBox( UINT ixBox );
	UINT			IOBoxIndexFromPort( UINT nPort );


	// version info
	WORD			m_wMainVersion;
	WORD			m_wMainBuild;
	WORD			m_wSubVersion;
	WORD			m_wSubBuild;
	WORD			m_wDSPVersion;
	WORD			m_wDSPBuild;
	WORD			m_wSSCVersion;
	WORD			m_wSSCBuild;

	// system info
	enum { DI_Coaxial = 0, DI_AESEBU };
	enum { SS_Internal = 0, SS_Digital1, SS_Digital2, SS_WordClock };
	enum { TS_LTC = 0, TS_MTCIn, TS_MTCVid };
	enum { SR_44 = 0, SR_48, SR_88, SR_96, SR_192 };

	//Mixer Tabs
	enum { DA=0, DB, DC, DD };
	//Reverb Types
	enum { OFF=0, ECHO, ROOM, SMALLH, LARGEH };

	CTacomaSurface*		m_pSurface;
	CMidiSource*		m_pMidiSource;
	CMidiOuts*			m_pMidiOuts;


private:
	HANDLE				m_hSysx;



// TTS Persist stuff
public:
	class CPreampPersist : public CTTSPersistObject
	{
	public:
		CPreampPersist( CIOBoxInterface* p ):
			CTTSPersistObject( m_wPersistSchema, m_wPersistChunkID )
			,m_pIOBox( p )
		{
		}
		// CTTSPersistObject Override ************************************
		HRESULT Persist( WORD wSchema, CPersistDDX& ddx );
		HRESULT OnPersistEnd( CPersistDDX& ddx );
		HRESULT NeedTOCEntry() { return S_OK; }

		// CTTSPersistObject schema and chunk id
		static WORD m_wPersistSchema;
		static WORD m_wPersistChunkID;

	private:
		CIOBoxInterface* m_pIOBox;

	}	m_persistPreamp;

public:
	class CDMPersist : public CTTSPersistObject
	{
	public:
		CDMPersist( CIOBoxInterface* p ):
			CTTSPersistObject( m_wPersistSchema, m_wPersistChunkID )
			,m_pIOBox( p )				
		{
		}
		// CTTSPersistObject Override ************************************
		HRESULT Persist( WORD wSchema, CPersistDDX& ddx );
		HRESULT OnPersistEnd( CPersistDDX& ddx );
		HRESULT NeedTOCEntry() { return S_OK; }

		// CTTSPersistObject schema and chunk id
		static WORD m_wPersistSchema;
		static WORD m_wPersistChunkID;

	private:
		CIOBoxInterface* m_pIOBox;

	}	m_persistDM;

	CTTSPersistObject* GetPreampPersistObject() { return &m_persistPreamp; }
	CTTSPersistObject* GetDMPersistObject() { return &m_persistDM; }

};
