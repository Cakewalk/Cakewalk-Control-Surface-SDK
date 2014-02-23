//-----------------------------------------------------------------
// Definition of CMixParam
// This Class encapsulates a Parameter in the host providing convenient
// methods of binding with the value and state of a parameter.


#pragma once

/////////////////////////////////////////////////////////////////////////////

// Disable identifier name too long for debugging warning
#pragma warning(disable: 4786)

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

#define NO_DEFAULT	-1000.0f
#define ILLEGAL_UNIQUE_ID	((DWORD)-1)

/////////////////////////////////////////////////////////////////////////////

class CMixParam
{
public:
	CMixParam();
	virtual ~CMixParam();

	void SetInterfaces(ISonarMixer *pMixer, ISonarMixer2* pMixer2, ISonarTransport *pTransport, DWORD dwUniqueId =ILLEGAL_UNIQUE_ID);

	void SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStrip, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float fDefault = NO_DEFAULT );
	void SetFxParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );
	void SetEqParams( SONAR_MIXER_STRIP mixerStrip, DWORD dwStripNum, WORD wFxParamNum, float fDefault = NO_DEFAULT );

	void SetDisableWhilePlaying(bool bDisable);

	bool HasBinding()					{ return m_bHasBinding; };
	void ClearBinding();

	SONAR_MIXER_STRIP GetMixerStrip()	{ return m_eMixerStrip; };
	SONAR_MIXER_PARAM GetMixerParam()	{ return m_eMixerParam; };
	DWORD GetParamNum()					{ return m_dwParamNum; };

	HRESULT GetCrunchedParamLabel(LPSTR pszText, DWORD dwLen);
	HRESULT GetCrunchedValueText(LPSTR pszText, DWORD dwLen);

	HRESULT GetParamLabel(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetVal(float *fVal);
	HRESULT SetVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_TIMEOUT);
	HRESULT ToggleBooleanParam(SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_TIMEOUT);
	HRESULT GetValueText(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetValueText(float fVal, LPSTR pszText, DWORD *pdwLen);
	HRESULT Touch(bool bTouchState);
	HRESULT GetWrite(bool *pb);
	HRESULT SetWrite(bool b);
	HRESULT GetWriteAll(bool *pb);
	HRESULT SetWriteAll(bool b);
	HRESULT GetRead(bool* pb);
	HRESULT SetRead(bool b );
	HRESULT GetReadAll( bool* pb );
	HRESULT SetReadAll( bool b );

	HRESULT Adjust(float fDelta);
	HRESULT SetToDefaultValue();

	DWORD  GetStripNum() const { return m_dwStripNum; }

	enum CaptureType
	{
		CT_Jump,			// Set to absolute position
		CT_Relative,	// change by relative amount

#if 0	// would be nice.  never got it to work right
		CT_Converge,	// converge differences of physical knob and value
#endif
		
		CT_Match			// must exactly match (nulling)
	};

	void SetCaptureType( CaptureType ct ) { m_eCaptureType = ct; }
	void SetACTLearning( bool b ) { m_bACTLearning = b; }

	// enums for NULLing
	enum VALUE_HISTORY
	{ 
		VT_UNDEF = 0, 
		VT_WASBELOW = 1, 
		VT_WASABOVE = 2, 
		VT_CROSSED = 3 
	};
	
	void	ResetHistory() { m_wValHistory = VT_UNDEF; m_fValCached = -1.f; }
	float GetValCached() const { return m_fValCached; }
	bool	IsTouched() const { return m_bTouched; }

protected:
	bool isPlaying();

	// The Host Interfaces
	ISonarMixer *		m_pMixer;
	ISonarMixer2*		m_pMixer2;
	ISonarTransport *	m_pTransport;
	DWORD					m_dwUniqueId;

	// Value States
	bool 					m_bTouched;
	bool 					m_bAudioMeteringEnabled;
	bool 					m_bACTLearning;
	WORD 					m_wValHistory;
	DWORD 				m_dwSetParamTimeStamp;
	float 				m_fValCached;
	CString				m_strValTextCached;

	CaptureType			m_eCaptureType;

	// The Param Bindings:
	SONAR_MIXER_STRIP m_eMixerStrip;
	SONAR_MIXER_PARAM m_eMixerParam;
	DWORD 				m_dwParamNum;
	DWORD 				m_dwStripNum;
	bool 					m_bHasBinding;

	float 				m_fDefaultValue;
	float 				m_fLastPosition;


	bool					m_bDisableWhilePlaying;
};

