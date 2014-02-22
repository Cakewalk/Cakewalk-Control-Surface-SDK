//////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
//	MotorMixStrip.h:	Definition of subclass from CMixStrip that
// implements the strip functionality for a MotorMix (tm)
//////////////////////////////////////////////////////////////////////
#pragma once

#define BANK_WIDTH 8

// Notice that throughout this sample the letters MM mean MotorMix
//

// forwards:
class CMMRotaryParam;
class CMMFaderParam;

/////////////////////////////////////////////////////////////////////////
// CMMStrip:
//
// This is where most of the work gets done.
//
// The MotorMix Strip (CMMStrip) sets up all the Input and Output MIDI
// messages right in its constructor. It then associates those messages
// with certain kinds of CMixParams. Just for the purpose of
// ilustrating the reader, we actually go ahead and specify a parameter
// identity for those CMixParams right in the constructor.
// However, the other part of the mix strip's job is to remap its
// parameters as the various state changes occur.
//
// First, you may have noticed that after creating each mix param we call
// add( CBaseMixParam *pParam ) right on the strip.
// The reason for this is to put the parameter in an internal list that
// the strip uses for relocating the container of a parameter with the
// help of the template CMixStripBank.
//
// When the base container or the container class are changed, the
// mix strip bank tells the associated CMixStrips to set their container
// to a new one.
//
// Because we have some custom mappings depending on states other than
// container specifiers, it is nice we can put all of the parameter 
// remapping into a separate method (by overriding remapStrip).
// The base implementation of remapStrip would just set the container
// attributes of the parameters.
// However, because of the high complexity of this implementation,
// we decided to use the parameter remap macros.
// Implement the remapping of each parameter in a separate method:
//
//		void OnRemapParamOne();
//		void OnRemapParamTwo();
//
// then hook them up into the framework by writing the following:
//
//		BEGIN_PARAM_REMAP_MAP
//		PARAM_REMAP_MAP(m_pParamOne, OnRemapParamOne)
//		PARAM_REMAP_MAP(m_pParamTwo, OnRemapParamTwo)
//		END_PARAM_REMAP_MAP
//
// this maintains the code a lot cleaner.
/////////////////////////////////////////////////////////////////////////

class CMMStrip	:	public CMixStrip
{
public:
	CMMStrip(
		DWORD dwHWStripNum,
		CControlSurface *pSurface
	);

	virtual HRESULT Initialize();

	~CMMStrip();

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nState );

private:

	// CMixStrip overrides
	void refreshScribble();
	HRESULT getScribbleText( LPSTR pszName, DWORD* pdwLen );

	// param remap macros:
	BEGIN_PARAM_REMAP_MAP
	PARAM_REMAP_MAP(m_pParamFader, OnRemapFader)
	PARAM_REMAP_MAP(m_pParamMute, OnRemapMute)
	PARAM_REMAP_MAP(m_pParamSolo, OnRemapSolo)
	PARAM_REMAP_MAP(m_pParamRecRdy, OnRemapRecRdy)
	PARAM_REMAP_MAP(m_pParamMulti, OnRemapMulti)
	PARAM_REMAP_MAP(m_pParamRotary, OnRemapRotary)
	END_PARAM_REMAP_MAP

	void OnRemapFader();
	void OnRemapMute();
	void OnRemapSolo();
	void OnRemapRecRdy();
	void OnRemapMulti();
	void OnRemapRotary();

	// helpers
	DWORD getCurrentFxParam();
	DWORD getCurrentFx();
	BOOL isMidiTrack();

	CMidiMsg				m_msgFader;
	CMidiMsg				m_msgKnob;
	CMidiMsg				m_msgPushBtn;
	CMidiMsg				m_msgBtnLED;
	CMidiMsg				m_msgScribble;

	CMixParamFloat*			m_pParamFader;
	CMixParamBool*				m_pParamMute;
	CMixParamBool*				m_pParamSolo;
	CMixParamBool*				m_pParamMulti;
	CMixParamBoolEx*			m_pParamRecRdy;
	CMMRotaryParam*			m_pParamRotary;

	SONAR_MIXER_STRIP			m_mixerStripMeter;
	DWORD							m_dwStripMeter;
};

/////////////////////////////////////////////////////////////////////////