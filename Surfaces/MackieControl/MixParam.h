#ifndef MixParam_h
#define MixParam_h

/////////////////////////////////////////////////////////////////////////////

// Disable identifier name too long for debugging warning
#pragma warning(disable: 4786)

/////////////////////////////////////////////////////////////////////////////

enum DataType
{
	DT_NO_LEDS,
	DT_BOOL,
	DT_SELECTOR,
	DT_BOOST_CUT,
	DT_SPREAD,
	DT_LEVEL,
	DT_PAN,
	DT_INTERLEAVE,
	NUM_DATA_TYPES
};

/////////////////////////////////////////////////////////////////////////////

#define NO_DEFAULT	-1000.0f
#define ILLEGAL_UNIQUE_ID	((DWORD)-1)

/////////////////////////////////////////////////////////////////////////////

class CMixParam
{
public:
	CMixParam();
	virtual ~CMixParam();

	void Setup(ISonarMixer *pMixer, ISonarTransport *pTransport, FilterLocator *pFilterLocator, DWORD dwUniqueId =ILLEGAL_UNIQUE_ID);

	void SetParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
					SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum);

	void SetAttribs(DataType eDataType, float fDefaultValue =NO_DEFAULT, float fStepSize =1.0f,
										float fMin =0.0f, float fMax =1.0f);

	void SetAlternateLabel(const char *szText);
	void SetDisableWhilePlaying(bool bDisable);
	void SetAllowFineResolution(bool bAllowFineResolution);
	bool GetAllowFineResolution()		{ return m_bAllowFineResolution; };

	bool HasBinding()					{ return m_bHasBinding; };
	void ClearBinding();
	void ClearBindingDisplayDash();

	SONAR_MIXER_STRIP GetMixerStrip()	{ return m_eMixerStrip; };
	DWORD GetStripNum()					{ return m_dwStripNum; };
	SONAR_MIXER_PARAM GetMixerParam()	{ return m_eMixerParam; };
	DWORD GetParamNum();
	DataType GetDataType()				{ return m_eDataType; };
	float GetDefaultValue()				{ return m_fDefaultValue; };
	float GetStepSize()					{ return m_fStepSize; };

	HRESULT GetCrunchedStripName(LPSTR pszText, DWORD dwLen);
	HRESULT GetCrunchedParamLabel(LPSTR pszText, DWORD dwLen);
	HRESULT GetCrunchedValueText(LPSTR pszText, DWORD dwLen);

	HRESULT GetNormalizedVal(float *fVal, bool *bDot =NULL);
	HRESULT SetNormalizedVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);

	HRESULT GetStripName(LPSTR pszText, DWORD *pdwLen);
	HRESULT SetStripName(LPSTR pszText);
	HRESULT GetParamLabel(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetVal(float *fVal);
	HRESULT SetVal(float fVal, SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);
	HRESULT GetValueText(LPSTR pszText, DWORD *pdwLen);
	HRESULT GetValueText(float fVal, LPSTR pszText, DWORD *pdwLen);
	HRESULT Touch(bool bTouchState);
	HRESULT GetArm(bool *pbArm);
	HRESULT SetArm(bool bArm);
	HRESULT GetArmAll(bool *pbArm);
	HRESULT SetArmAll(bool bArm);
	HRESULT Snapshot();

	HRESULT TouchCapture()		{ return Touch(true); };
	HRESULT TouchRelease()		{ return Touch(false); };

	HRESULT ToggleBooleanParam(SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);
	HRESULT Adjust(float fDelta, SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);

	HRESULT ToggleArm();
	HRESULT ToggleArmAll();

	HRESULT ToggleOrSetToDefault(SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);
	HRESULT SetToDefaultValue(SONAR_MIXER_TOUCH eMixerTouch =MIX_TOUCH_NORMAL);

	bool IsMIDITrack();
	bool IsAudioTrack();
	bool StripExists();
	bool IsPlaying();
	bool IsArchived();

	void SetMetering(bool bOn);
	HRESULT ReadMeter(float *fVal);
	bool CheckBinding();

protected:
	ISonarMixer *m_pMixer;
	ISonarTransport *m_pTransport;
	DWORD m_dwUniqueId;

	FilterLocator *m_pFilterLocator;

	bool m_bHasBinding;
	bool m_bTouched;
	bool m_bAudioMeteringEnabled;
	bool m_bWasMIDI;

	SONAR_MIXER_STRIP m_eMixerStrip;
	DWORD m_dwStripNum;
	SONAR_MIXER_PARAM m_eMixerParam;
	DWORD m_dwParamNum;

	DataType m_eDataType;
	float m_fDefaultValue;
	float m_fStepSize;
	float m_fMin;
	float m_fMax;

	char m_szAlternateLabel[64];
	bool m_bDisableWhilePlaying;
	bool m_bAllowFineResolution;
};

/////////////////////////////////////////////////////////////////////////////

#endif // MixParam_h
