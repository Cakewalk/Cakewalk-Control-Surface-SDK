//-----------------------------------------------------------------
// Definition of CMixParam
// This Class encapsulates a Parameter in the host providing convenient
// methods of binding with the value and state of a parameter.


#pragma once

/////////////////////////////////////////////////////////////////////////////

// Disable identifier name too long for debugging warning
#pragma warning(disable: 4786)

/////////////////////////////////////////////////////////////////////////////

#include "sfkmidi.h"

/////////////////////////////////////////////////////////////////////////////

#define NO_DEFAULT	-1000.0f
#define ILLEGAL_UNIQUE_ID	((DWORD)-1)

/////////////////////////////////////////////////////////////////////////////

class CMixParam
{
public:
	CMixParam();
	virtual ~CMixParam();

	void SetInterfaces(ISonarMixer *pMixer, ISonarTransport *pTransport, DWORD dwUniqueId =ILLEGAL_UNIQUE_ID);

	void SetAllParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, DWORD dwStripNumOffset, DWORD dwCrunchSize = 0, BOOL bSetCapture = FALSE );
	void SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStrip, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float fDefault = NO_DEFAULT, BOOL bSetCapture = FALSE );
	void SetFxParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );
	void SetEqParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );
	void SetCompParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );
	void SetTubeParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );

	void SetStripNumOffset( DWORD dwOffset );

	void SetDisableWhilePlaying(bool bDisable);

	bool HasBinding()					{ return m_bHasBinding; };
	void ClearBinding();

	SONAR_MIXER_STRIP GetMixerStrip()	{ return m_eMixerStrip; };

	void SetMixerParam( SONAR_MIXER_PARAM param, BOOL bChangeCapture = FALSE );
	SONAR_MIXER_PARAM GetMixerParam()	{ return m_eMixerParam; };
	DWORD GetParamNum()					{ return m_dwParamNum; };
	float GetDefaultValue() const		{ return m_fDefaultValue; }
	void SetDefaultValue( float fVal ) { m_fDefaultValue = fVal; }

	HRESULT GetCrunchedParamLabel(LPSTR pszText, DWORD dwLen);

	HRESULT GetParamLabel(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetVal(float *fVal);
	HRESULT SetVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch = MIX_TOUCH_TIMEOUT, CMidiMsg::ValueChange vc = CMidiMsg::VC_None);
	HRESULT ToggleBooleanParam(SONAR_MIXER_TOUCH eMixerTouch = MIX_TOUCH_TIMEOUT);
	HRESULT Trigger();
	HRESULT GetValueText(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetValueText(float fVal, LPSTR pszText, DWORD *pdwLen);
	HRESULT GetCrunchedValueText(LPSTR pszText, DWORD dwLen);
	HRESULT GetCrunchedValueText(float fVal, LPSTR pszText, DWORD dwLen);
	HRESULT Touch(bool bTouchState);
	HRESULT GetWrite(bool *pb);
	HRESULT SetWrite(bool b);
	HRESULT GetRead(bool* pb);
	HRESULT SetRead(bool b );

	HRESULT Adjust(float fDelta);
	HRESULT SetToDefaultValue();

	DWORD  GetStripNum() const { return m_dwStripNum; }
	void	 GetStripInfo( SONAR_MIXER_STRIP* peType, DWORD* pdwStripNum );

	void	 SetStripNum( DWORD dwStrip );
	void	 SetStripType( SONAR_MIXER_STRIP eStrip );
	void	 SetParamNum( DWORD dwParamNum );
	void	 SetMixParam( SONAR_MIXER_PARAM eparam );

   DWORD  GetStripPhysicalIndex( ) { return m_dwPhysicalStripIndex; };
	void	 SetStripPhysicalIndex( DWORD dwPhysicalIndex );

	HRESULT GetMinMaxStep( float *pfMin, float *pfMax, float *pfStep );

	HRESULT RevertValue();

	enum CaptureType
	{
		CT_Jump,			// Set to absolute position
		CT_Relative,	// change by relative amount

#if 0	// would be nice.  never got it to work right
		CT_Converge,	// converge differences of physical knob and value
#endif
		
		CT_Match,		// must exactly match (nulling)
		CT_Step			// regardless of step size, step by 1 'unit', +/-
	};
	void SetCaptureType( CaptureType ct ) { m_eCaptureType = ct; ResetHistory(); }
	CaptureType GetCaptureType( void ) const { return m_eCaptureType; }
	CaptureType GetDefaultCaptureType( void );
	void SetACTLearning( bool b ) { m_bACTLearning = b; }

	enum TriggerAction
	{
		// what to do in OnTrigger...
		TA_DEFAULT,	// set to default
		TA_TOGGLE,	// toggle boolean range
	};
	void SetTriggerAction( TriggerAction ta ) { m_eTriggerAction = ta; }
	TriggerAction GetTriggerAction() const { return m_eTriggerAction; }

	// enums for NULLing
	enum VALUE_HISTORY
	{ 
		VT_UNDEF = 0, 
		VT_WASBELOW = 1, 
		VT_WASABOVE = 2, 
		VT_CROSSED = 3 
	};
	VALUE_HISTORY	GetNullStatusForValue( float fValue );

	void	ResetHistory();
	float GetValCached() const { return m_fValCached; }
	bool	IsTouched() const { return m_bTouched; }

	DWORD	CrunchSize( void ) { return ( m_dwCrunchSize ); };
	void	SetCrunchSize( DWORD dwSet ) { m_dwCrunchSize = dwSet; };
	bool  DisplayValue( void ) { return ( m_bDisplayValue ); };
	void	SetDisplayValue( bool bSet ) { m_bDisplayValue = bSet; };
	bool  DisplayName( void ) { return ( m_bDisplayName ); };
	void	SetDisplayName( bool bSet ) { m_bDisplayName = bSet; };
	bool  IsAlwaysChanging( void ) { return ( m_bAlwaysChanging ); };
	void	SetAlwaysChanging( bool bSet ) { m_bAlwaysChanging = bSet; };
	bool  IsThrottle( void ) { return ( m_bThrottle ); };
	void	SetThrottle( bool bSet ) { m_bThrottle = bSet; };

protected:
	bool isPlaying();
	void scaleValueToHost( float& fVal );
	void scaleValueFromHost( float& fVal );
	void doSendToHost( SONAR_MIXER_STRIP eStripType, DWORD dwStripNum, float fVal, SONAR_MIXER_TOUCH eMixerTouch );

	// The Param Bindings:
	SONAR_MIXER_STRIP m_eMixerStrip;
	SONAR_MIXER_PARAM m_eMixerParam;
	DWORD 				m_dwParamNum;
	DWORD 				m_dwStripNum;
	DWORD					m_dwPhysicalStripIndex;
	bool 					m_bHasBinding;
	DWORD					m_dwStripNumOffset;

	// Value States
	bool 					m_bTouched;
	bool 					m_bACTLearning;
	WORD 					m_wValHistory;
	float 				m_fValCached;
	CString				m_strValTextCached;
	DWORD 				m_dwSetParamTimeStamp;

	CaptureType			m_eCaptureType;
	TriggerAction		m_eTriggerAction;

	float 				m_fDefaultValue;
	float 				m_fLastPosition;
	float					m_fLastSentToHost;
	bool					m_bDisableWhilePlaying;
	bool					m_bDisplayName;
	bool					m_bDisplayValue;
	DWORD					m_dwCrunchSize;
	bool					m_bAlwaysChanging;
	bool					m_bThrottle;

	// The Host Interfaces
	ISonarMixer *		m_pMixer;
	ISonarMixer3 *		m_pMixer3;
	ISonarTransport *	m_pTransport;
	IHostLockStrip *	m_pLockStrip;
	DWORD					m_dwUniqueId;
};
