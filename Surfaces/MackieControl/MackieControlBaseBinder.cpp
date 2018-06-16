#include "stdafx.h"

#include "strlcpy.h"
#include "strlcat.h"

#include "FilterLocator.h"
#include "MixParam.h"
#include "KeyBinding.h"

#include "MackieControl.h"
#include "MackieControlInformation.h"
#include "MackieControlState.h"
#include "MackieControlBase.h"

/////////////////////////////////////////////////////////////////////////////
//
// Functions to bind VPots and Faders to parameters
//
/////////////////////////////////////////////////////////////////////////////

#define STRIP_HAS_AUX_SEND(s)   (MIX_STRIP_TRACK == (s) || MIX_STRIP_BUS == (s))
#define FILTER_FLAG				(0x80000000)
#define IS_FILTER_PLUGIN(p)     ((p) & FILTER_FLAG)
#define FILTER_NUM(p)			((p) & 0x7FFFFFFF)

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfParameterTrack(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	if (m_bBindMuteSoloArm)		// Include Mute/Solo/Arm?
	{
		// Insert them after Pan

		switch (dwIndex)
		{
			case 2:
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_MUTE, 0);
				pParam->SetAttribs(DT_BOOL);
				return;

			case 3:
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SOLO, 0);
				pParam->SetAttribs(DT_BOOL);
				return;

			case 4:
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_RECORD_ARM, 0);
				pParam->SetAttribs(DT_BOOL);
				pParam->SetAlternateLabel("RecRdy");
				return;

			default:
				break;
		}

		// Fake the indexes for the remaining entries
		if (dwIndex > 4)
			dwIndex -= 3;
	}

	if (IsMIDI(eMixerStrip, dwStripNum))				// MIDI
	{
		switch (dwIndex)
		{
			case IDX_DEFAULT_FADER:
			case 0:										// Volume
				ConfigureVolume(eMixerStrip, dwStripNum, true, pParam);
				break;

			case 1:										// Pan
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
				pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
				break;
			
			case 2:										// Output
				ConfigureParamOutput(eMixerStrip, dwStripNum, false, pParam);
				pParam->SetAlternateLabel("Output");
				break;

			case 3:										// Input
				ConfigureParamInput(eMixerStrip, dwStripNum, true, pParam);
				pParam->SetAlternateLabel("Input");
				break;

			case 4:										// Bank
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_BANK, 0);
				pParam->SetAttribs(DT_SELECTOR, NO_DEFAULT, 1.0f, 0.0f, 127.0f);
				break;

			case 5:										// Patch
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PATCH, 0);
				pParam->SetAttribs(DT_SELECTOR, NO_DEFAULT, 1.0f, 0.0f, 127.0f);
				break;

			case 8:										// Chorus
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, 0);
				pParam->SetAttribs(DT_LEVEL, VOL_MIDI_DEFAULT, VOL_MIDI_STEP_SIZE);
				pParam->SetAlternateLabel("Chorus");
				break;

			case 12:									// Reverb
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, 1);
				pParam->SetAttribs(DT_LEVEL, VOL_MIDI_DEFAULT, VOL_MIDI_STEP_SIZE);
				pParam->SetAlternateLabel("Reverb");
				break;

			default:
				pParam->ClearBinding();
				break;
		}
	}
	else												// Audio
	{
		switch (dwIndex)
		{
			case IDX_DEFAULT_FADER:
			case 0:										// Volume
				ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
				break;

			case 1:										// Pan
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
				pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
				break;

			case 2:										// Output
				ConfigureParamOutput(eMixerStrip, dwStripNum, false, pParam);
				pParam->SetAlternateLabel("Output");
				break;

			case 3:										// Input
				ConfigureParamInput(eMixerStrip, dwStripNum, true, pParam);
				pParam->SetAlternateLabel("Input");
				break;

			case 4:										// Phase
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PHASE, 0);
				pParam->SetAttribs(DT_BOOL, 1.0f);
				break;

			case 5:										// Mono/Stereo
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INTERLEAVE, 0);
				pParam->SetAttribs(DT_INTERLEAVE, 2.0f, 1.0f, 1.0f, 2.0f);
				pParam->SetAlternateLabel("MonStr");
				break;

			case 6:										// Input Echo
				if (m_cState.HaveInputEcho())
				{
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INPUT_ECHO, 0);
					pParam->SetAttribs(DT_BOOL, 0.0f);
				}
				else
				{
					pParam->ClearBinding();
				}
				break;

			default:									// Aux Sends
				ConfigureAuxSends(eMixerStrip, dwStripNum, dwIndex - 7, pParam);
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfParameterAux(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	switch (dwIndex)
	{
		case IDX_DEFAULT_FADER:
		case 0:											// Send Level
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_VOL, 0);
			pParam->SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
			break;

		case 1:											// Send Pan
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
			pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
			break;

		case 2:											// Return Level
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_VOL, 0);
			pParam->SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
			break;

		case 3:											// Return Pan
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
			pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
			break;

		case 4:											// Output
			ConfigureParamOutput(eMixerStrip, dwStripNum, false, pParam);
			break;

		default:
			pParam->ClearBinding();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfParameterMain(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										 DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	switch (dwIndex)
	{
		case IDX_DEFAULT_FADER:
		case 0:											// Volume
			ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
			break;

		case 1:											// Pan
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
			pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
			break;

		case 2:											// Mono/Stereo
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INTERLEAVE, 0);
			pParam->SetAttribs(DT_BOOL, 1.0f);	// Auto is not an option here
			break;

		case 3:											// Output
			ConfigureParamOutput(eMixerStrip, dwStripNum, false, pParam);
			break;

		default:
			pParam->ClearBinding();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfParameterBus(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	switch (dwIndex)
	{
		case IDX_DEFAULT_FADER:
		case 0:										// Volume
			ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
			break;

		case 1:										// Pan
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
			pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
			break;

		case 2:										// Output
			ConfigureParamOutput(eMixerStrip, dwStripNum, true, pParam);
			break;

		case 3:										// Mono/Stereo
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INTERLEAVE, 0);
			pParam->SetAttribs(DT_INTERLEAVE, 2.0f, 1.0f, 1.0f, 2.0f);
			break;

		default:									// Aux Sends
			ConfigureAuxSends(eMixerStrip, dwStripNum, dwIndex - 4, pParam);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfParameterMaster(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	switch (dwIndex)
	{
		case IDX_DEFAULT_FADER:
		case 0:											// Volume
			ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
			break;

		case 1:											// Pan
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
			pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
			break;

		case 2:											// Mono/Stereo
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INTERLEAVE, 0);
			pParam->SetAttribs(DT_INTERLEAVE, 2.0f, 1.0f, 1.0f, 2.0f);
			break;

		default:
			pParam->ClearBinding();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigureVolume(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										 bool bIsMIDI, CMixParam *pParam)
{
	if (bIsMIDI)
	{
		pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_VOL, 0);
		pParam->SetAttribs(DT_LEVEL, VOL_MIDI_DEFAULT, VOL_MIDI_STEP_SIZE);
	}
	else
	{
		pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_VOL, 0);
		pParam->SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigureParamInput(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											 bool bAllowNone, CMixParam *pParam)
{
	DWORD dwNumInputs = GetNumInputs(eMixerStrip, dwStripNum);

	if (0 == dwNumInputs)
	{
		pParam->ClearBinding();
		return;
	}

	float fMin = bAllowNone ? -1.0f : 0.0f;

	pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_INPUT, 0);
	pParam->SetAttribs(DT_SELECTOR, NO_DEFAULT, 1.0f, fMin, dwNumInputs - 1.0f);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigureParamOutput(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
												bool bAllowNone, CMixParam *pParam)
{
	DWORD dwNumOutputs = GetNumOutputs(eMixerStrip, dwStripNum);

	if (0 == dwNumOutputs)
	{
		pParam->ClearBinding();
		return;
	}

	float fMin = bAllowNone ? -1.0f : 0.0f;

	pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_OUTPUT, 0);
	pParam->SetAttribs(DT_SELECTOR, NO_DEFAULT, 1.0f, fMin, dwNumOutputs - 1.0f);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigureAuxSends(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											 DWORD dwIndex, CMixParam *pParam)
{
	if (!STRIP_HAS_AUX_SEND(eMixerStrip))
	{
		pParam->ClearBinding();

		return;
	}

	DWORD dwNumAuxSends = GetNumSends(eMixerStrip, dwStripNum);

	if (dwIndex < (4 * dwNumAuxSends))
	{
		DWORD dwN = dwIndex >> 2;

		switch (dwIndex & 0x03)
		{
			case 0:										// Send Enable
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_ENABLE, dwN);
				pParam->SetAttribs(DT_BOOL, 0.0f);
				return;

			case 1:										// Send Volume
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, dwN);
				pParam->SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
				return;

			case 2:										// Send Pan
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_PAN, dwN);
				pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
				return;

			case 3:										// Send Pre/Post
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_PREPOST, dwN);
				pParam->SetAttribs(DT_BOOL, 0.0f);
				return;

			default:
				TRACE("CMackieControlBase::ConfigureAuxSends(): bad switch statement\n");
				break;
		}
	}

#ifdef _DEBUG
	if (dwIndex < (4 * GetMaxNumSends(eMixerStrip)))
		pParam->ClearBindingDisplayDash();
	else
		pParam->ClearBinding();
#else
	pParam->ClearBinding();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfPan(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
							   DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	switch (eMixerStrip)
	{
		case MIX_STRIP_TRACK:
			switch (dwIndex)
			{
				case IDX_DEFAULT_FADER:					// Volume
					ConfigureVolume(eMixerStrip, dwStripNum, IsMIDI(eMixerStrip, dwStripNum), pParam);
					break;

				case 0:									// Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				default:
					ConfigureAuxSendPans(eMixerStrip, dwStripNum, dwIndex - 1, pParam);
					break;
			}
			break;

		case MIX_STRIP_AUX:
			switch (dwIndex)
			{
				case IDX_DEFAULT_FADER:					// Send Level
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, 0);
					pParam->SetAttribs(DT_LEVEL, VOL_AUDIO_DEFAULT, VOL_AUDIO_STEP_SIZE);
					break;

				case 0:									// Send Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				case 1:									// Return Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				default:
					pParam->ClearBinding();
					break;
			}
			break;

		case MIX_STRIP_MAIN:
			switch (dwIndex)
			{
				case IDX_DEFAULT_FADER:					// Volume
					ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
					break;

				case 0:									// Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				default:
					pParam->ClearBinding();
					break;
			}
			break;

		case MIX_STRIP_BUS:
			switch (dwIndex)
			{
				case IDX_DEFAULT_FADER:					// Volume
					ConfigureVolume(eMixerStrip, dwStripNum, IsMIDI(eMixerStrip, dwStripNum), pParam);
					break;

				case 0:									// Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				default:
					ConfigureAuxSendPans(eMixerStrip, dwStripNum, dwIndex - 1, pParam);
					break;
			}
			break;

		case MIX_STRIP_MASTER:
			switch (dwIndex)
			{
				case IDX_DEFAULT_FADER:					// Volume
					ConfigureVolume(eMixerStrip, dwStripNum, false, pParam);
					break;

				case 0:									// Pan
					pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_PAN, 0);
					pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);
					break;

				default:
					pParam->ClearBinding();
					break;
			}
			break;

		default:
			pParam->ClearBinding();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfSend(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
								DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	bool bIsMIDI = IsMIDI(eMixerStrip, dwStripNum);

	if (IDX_DEFAULT_FADER == dwIndex)					// Volume
	{
		ConfigureVolume(eMixerStrip, dwStripNum, bIsMIDI, pParam);
		return;
	}

	if (bIsMIDI)										// MIDI
	{
		if (MIX_STRIP_TRACK != eMixerStrip)
		{
			pParam->ClearBinding();
			return;
		}

		switch (dwIndex)
		{
			// Align with the Audio send levels

			case 1:										// Chorus
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, 0);
				pParam->SetAttribs(DT_LEVEL, VOL_MIDI_DEFAULT, VOL_MIDI_STEP_SIZE);
				pParam->SetAlternateLabel("Chorus");
				break;

			case 5:										// Reverb
				pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_VOL, 1);
				pParam->SetAttribs(DT_LEVEL, VOL_MIDI_DEFAULT, VOL_MIDI_STEP_SIZE);
				pParam->SetAlternateLabel("Reverb");
				break;

			default:
				pParam->ClearBinding();
				break;
		}
	}
	else												// Audio
	{
														// Aux Sends
		ConfigureAuxSends(eMixerStrip, dwStripNum, dwIndex, pParam);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfPlugin(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
								  DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	ConfigurePlugins(eMixerStrip, dwStripNum, dwPluginNum, dwIndex, PT_ALL, dwModifiers, pParam);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfEQ(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
							  DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	ConfigurePlugins(eMixerStrip, dwStripNum, dwPluginNum, dwIndex, PT_EQ, dwModifiers, pParam);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfDynamics(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	ConfigurePlugins(eMixerStrip, dwStripNum, dwPluginNum, dwIndex, PT_DYNAMICS, dwModifiers, pParam);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfNoBindings(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									  DWORD dwPluginNum, DWORD dwIndex, DWORD dwModifiers, CMixParam *pParam)
{
	pParam->ClearBinding();
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigureAuxSendPans(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											DWORD dwIndex, CMixParam *pParam)
{
	if (!STRIP_HAS_AUX_SEND(eMixerStrip) || IsMIDI(eMixerStrip, dwStripNum))
	{
		pParam->ClearBinding();

		return;
	}

	DWORD dwNumAuxSends = GetNumSends(eMixerStrip, dwStripNum);

	if (dwIndex < dwNumAuxSends)						// Send Pan
	{
		pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_SEND_PAN, dwIndex);
		pParam->SetAttribs(DT_PAN, PAN_CENTER, PAN_STEP_SIZE);

		return;
	}

#ifdef _DEBUG
	if (dwIndex < GetMaxNumSends(eMixerStrip))
		pParam->ClearBindingDisplayDash();
	else
		pParam->ClearBinding();
#else
	pParam->ClearBinding();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::ConfigurePlugins(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										DWORD dwPluginNum, DWORD dwIndex, DWORD dwFilterMask, 
										DWORD dwModifiers, CMixParam *pParam)
{
	bool bIsMIDI = IsMIDI(eMixerStrip, dwStripNum);

	if (IDX_DEFAULT_FADER == dwIndex)					// Volume
	{
		ConfigureVolume(eMixerStrip, dwStripNum, bIsMIDI, pParam);
		return;
	}

	CPluginProperties cPluginProps;

	if (!GetPluginProperties(eMixerStrip, dwStripNum, &dwPluginNum, dwFilterMask, &cPluginProps))
	{
		pParam->ClearBinding();
		return;
	}

	mapParameterProperties *pParamProps;
	bool bAllowFineResolution = true;

	switch (dwModifiers)
	{
		case MCS_MODIFIER_NONE:
			if (cPluginProps.m_mapParamPropsVPot.empty())
			{
				SONAR_MIXER_PARAM eMixerParam = MIX_PARAM_FX_PARAM;

				if (IS_FILTER_PLUGIN(dwPluginNum))
				{
					eMixerParam = MIX_PARAM_FILTER_PARAM;
					dwPluginNum = FILTER_NUM(dwPluginNum);
				}

				pParam->SetParams(eMixerStrip, dwStripNum, eMixerParam, MAKELONG(dwPluginNum, dwIndex));
				pParam->SetAttribs(DT_LEVEL, 0.5f, 0.005f);

				return;
			}
			else
			{
				pParamProps = &cPluginProps.m_mapParamPropsVPot;
			}
			break;

		case MCS_MODIFIER_M1:
			if (cPluginProps.m_mapParamPropsM1VPot.empty())
			{
				if (cPluginProps.m_mapParamPropsVPot.empty())
				{
					SONAR_MIXER_PARAM eMixerParam = MIX_PARAM_FX_PARAM;

					if (IS_FILTER_PLUGIN(dwPluginNum))
					{
						eMixerParam = MIX_PARAM_FILTER_PARAM;
						dwPluginNum = FILTER_NUM(dwPluginNum);
					}

					pParam->SetParams(eMixerStrip, dwStripNum, eMixerParam, MAKELONG(dwPluginNum, dwIndex));
					pParam->SetAttribs(DT_LEVEL, 0.5f, 0.005f);

					return;
				}
				else
				{
					pParamProps = &cPluginProps.m_mapParamPropsVPot;
					bAllowFineResolution = true;
				}
			}
			else
			{
				pParamProps = &cPluginProps.m_mapParamPropsM1VPot;
				bAllowFineResolution = false;
			}
			break;

		case MCS_MODIFIER_M2:
		case MCS_MODIFIER_M1 | MCS_MODIFIER_M2:
			pParamProps = &cPluginProps.m_mapParamPropsM2VPot;
			break;

		case MCS_MODIFIER_M3:
		case MCS_MODIFIER_M1 | MCS_MODIFIER_M3:
			pParamProps = &cPluginProps.m_mapParamPropsM3VPot;
			break;

		case MCS_MODIFIER_M4:
		case MCS_MODIFIER_M1 | MCS_MODIFIER_M4:
			pParamProps = &cPluginProps.m_mapParamPropsM4VPot;
			break;

		default:
			pParam->ClearBinding();
			return;
	}

	SetParamToProperty(eMixerStrip, dwStripNum, dwIndex, dwPluginNum, pParamProps, pParam);
	pParam->SetAllowFineResolution(bAllowFineResolution);
}

/////////////////////////////////////////////////////////////////////////////

void CMackieControlBase::SetParamToProperty(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										  DWORD dwIndex, DWORD dwPluginNum,
										  mapParameterProperties *pParamProps,
										  CMixParam *pParam)
{
	// Is there a parameter mapping table?
	if (pParamProps->empty())							// No
	{
		pParam->ClearBinding();

		return;
	}

	mapParameterProperties::iterator i = pParamProps->find(dwIndex);

	if (i == pParamProps->end())
	{
		pParam->ClearBinding();
	}
	else
	{
		if (i->second.m_dwParamNum == 0x80000000)  // Special case: enable the filter
		{
			pParam->SetParams(eMixerStrip, dwStripNum, MIX_PARAM_FILTER, FILTER_NUM(dwPluginNum));
			pParam->SetAttribs(i->second.m_eDataType, i->second.m_fDefaultValue, i->second.m_fStepSize);
		}
		else
		{
			SONAR_MIXER_PARAM eMixerParam = MIX_PARAM_FX_PARAM;

			if (IS_FILTER_PLUGIN(dwPluginNum))
			{
				eMixerParam = MIX_PARAM_FILTER_PARAM;
				dwPluginNum = FILTER_NUM(dwPluginNum);
			}

			pParam->SetParams(eMixerStrip, dwStripNum, eMixerParam, MAKELONG(dwPluginNum, i->second.m_dwParamNum));
			pParam->SetAttribs(i->second.m_eDataType, i->second.m_fDefaultValue, i->second.m_fStepSize);
		}
#if 0
		float fVal;
		pParam->GetVal(&fVal);
		TRACE("%f\n", fVal);
#endif
	}
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetPluginProperties(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
											  DWORD *dwPluginNum, DWORD dwFilterMask,
											  CPluginProperties *pProps,
											  char *pszText, DWORD *pdwLen)
{
	// Prepend the Track/Bus Eq (filter) to the list, if it exists
	if (GetFilterExists(eMixerStrip, dwStripNum, 0) && (dwFilterMask & PT_EQ) && !m_cState.GetExcludeFiletersFromPlugins())
	{
		// Want to get the details for the filter?
		if (*dwPluginNum == 0)
		{
			char szFilterName[64];
			DWORD dwLen = sizeof(szFilterName);

			GetFilterName(eMixerStrip, dwStripNum, 0, szFilterName, &dwLen);

			// Lookup the plugin
			mapPluginProperties *mapPluginProps = m_cState.GetMapPluginProperties();
			mapPluginProperties::iterator i = mapPluginProps->find(szFilterName);

			// Found it?
			if (i != mapPluginProps->end() && pProps)
				*pProps = i->second;

			// Return the name
			if (pszText && pdwLen)
				::strlcpy(pszText, szFilterName, *pdwLen);

			// Filter "plugins" have the top bit set
			*dwPluginNum |= FILTER_FLAG;

			return true;
		}

		// The filter does exist, but we want to look at another plugin.
		// Decrement the plugin number we want to find to compensate
		// for the filter that's been prepended to the list
		(*dwPluginNum)--;
	}

	// Prepend the Track/Bus Compressor (filter) to the list, if it exists
	if (GetFilterExists(eMixerStrip, dwStripNum, 1) && (dwFilterMask & PT_DYNAMICS) && !m_cState.GetExcludeFiletersFromPlugins())
	{
		// Want to get the details for the filter?
		if (*dwPluginNum == 0)
		{
			char szFilterName[64];
			DWORD dwLen = sizeof(szFilterName);

			GetFilterName(eMixerStrip, dwStripNum, 1, szFilterName, &dwLen);

			// Lookup the plugin
			mapPluginProperties *mapPluginProps = m_cState.GetMapPluginProperties();
			mapPluginProperties::iterator i = mapPluginProps->find(szFilterName);

			// Found it?
			if (i != mapPluginProps->end() && pProps)
				*pProps = i->second;

			// Return the name
			if (pszText && pdwLen)
				::strlcpy(pszText, szFilterName, *pdwLen);

			// Filter "plugins" have the top bit set
			*dwPluginNum = MIX_FILTER_COMP | FILTER_FLAG;

			return true;
		}

		// The filter does exist, but we want to look at another plugin.
		// Decrement the plugin number we want to find to compensate
		// for the filter that's been prepended to the list
		(*dwPluginNum)--;
	}

	DWORD dwPluginCount = GetPluginCount(eMixerStrip, dwStripNum);

	if (*dwPluginNum >= dwPluginCount)
	{
//		TRACE("Plugin offset greater than the number of plugins in this track\n");
		return false;
	}

	DWORD dwCount = 0; 

	// Find the dwPluginNum'th plugin of the type we're looking for
	for (int n = 0; n < (int)dwPluginCount; n++)
	{
		char szParamText[64];
		DWORD dwLen = sizeof(szParamText);

		if (!GetPluginName(eMixerStrip, dwStripNum, n, szParamText, &dwLen))
			continue;

//		TRACE("Plugin name: '%s'\n", szParamText);

		// Lookup the plugin
		mapPluginProperties *mapPluginProps = m_cState.GetMapPluginProperties();
		mapPluginProperties::iterator i = mapPluginProps->find(CString(szParamText));

		BYTE bMatch = 0;

		// Found it?
		if (i != mapPluginProps->end())
		{
			if (dwFilterMask & i->second.m_dwPluginType)
				bMatch = 2;
		}
		else
		{
			if (dwFilterMask & PT_UNKNOWN)				
				bMatch = 1;
		}

		// Yes? Then check if it's the one we're after
		if (bMatch > 0)
		{
			if (dwCount == *dwPluginNum)
			{
				// Return the actual offset of the plugin...
				*dwPluginNum = n;

				// ...and any properties we've found
				if (bMatch > 1 && pProps)
					*pProps = i->second;

				if (pszText && pdwLen)
					::strlcpy(pszText, szParamText, *pdwLen);

				return true;
			}

			dwCount++;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetNumParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
									   DWORD dwPluginNum, Assignment eAssignment)
{
	switch (eAssignment)
	{
		case MCS_ASSIGNMENT_PARAMETER:
			switch (eMixerStrip)
			{
				case MIX_STRIP_TRACK:
					{
						DWORD dwCount = 0;

						if (IsMIDI(eMixerStrip, dwStripNum))	// MIDI
						{
							dwCount = 12;
						}
						else									// Audio
						{
							dwCount = 7 + 4 * GetMaxNumSends(eMixerStrip);
						}

						if (m_bBindMuteSoloArm)
							dwCount += 3;

						return dwCount;
					}

				case MIX_STRIP_AUX:
					return 5;

				case MIX_STRIP_MAIN:
					return 4;

				case MIX_STRIP_BUS:
					return 4 + 4 * GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_MASTER:
					return 3;

				default:
					TRACE("CMackieControlBase::GetNumParams(): Error: unknown strip type!\n");
					break;
			}
			break;

		case MCS_ASSIGNMENT_PAN:
			switch (eMixerStrip)
			{
				case MIX_STRIP_TRACK:
					return 1 + GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_AUX:
					return 2;

				case MIX_STRIP_MAIN:
					return 1;

				case MIX_STRIP_BUS:
					return 1 + GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_MASTER:
					return 1;

				default:
					TRACE("CMackieControlBase::GetNumParams(): Error: unknown strip type!\n");
					break;
			}
			break;


		case MCS_ASSIGNMENT_SEND:
			switch (eMixerStrip)
			{
				case MIX_STRIP_TRACK:
					if (IsMIDI(eMixerStrip, dwStripNum))
						return 6;
					else								// Audio
						return 4 * GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_AUX:
					return 4 * GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_MAIN:
					return 0;

				case MIX_STRIP_BUS:
					return 4 * GetMaxNumSends(eMixerStrip);

				case MIX_STRIP_MASTER:
					return 0;

				default:
					TRACE("CMackieControlBase::GetNumParams(): Error: unknown strip type!\n");
					break;
			}
			break;

		case MCS_ASSIGNMENT_PLUGIN:
			return GetNumPluginParams(eMixerStrip, dwStripNum, dwPluginNum, PT_ALL);

		case MCS_ASSIGNMENT_EQ:
			return GetNumPluginParams(eMixerStrip, dwStripNum, dwPluginNum, PT_EQ);
		
		case MCS_ASSIGNMENT_DYNAMICS:
			return GetNumPluginParams(eMixerStrip, dwStripNum, dwPluginNum, PT_DYNAMICS);

		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:
			{
				CPluginProperties cPluginProps;

				if (GetPluginProperties(eMixerStrip, dwStripNum, &dwPluginNum, PT_EQ, &cPluginProps))
					return cPluginProps.m_dwNumFreqBands;
			}
			return 0;

		default:
			TRACE("CMackieControlBase::GetNumParams(): Error: unknown assignment!\n");
			break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CMackieControlBase::GetNumPluginParams(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum,
										   DWORD dwPluginNum, DWORD dwFilterMask)
{
	CPluginProperties cPluginProps;

	if (GetPluginProperties(eMixerStrip, dwStripNum, &dwPluginNum, dwFilterMask, &cPluginProps) &&
		!cPluginProps.m_mapParamPropsVPot.empty())
	{
		return cPluginProps.m_dwNumVPots;
	}

	if (IS_FILTER_PLUGIN(dwPluginNum))
		return GetFilterParamCount(eMixerStrip, dwStripNum, FILTER_NUM(dwPluginNum));
	else
		return GetPluginParamCount(eMixerStrip, dwStripNum, dwPluginNum);
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlBase::GetCurrentPluginName(Assignment eAssignment, SONAR_MIXER_STRIP eMixerStrip,
											  DWORD dwStripNum, DWORD dwPluginNum,
											  char *pszText, DWORD *pdwLen)
{
	if (!(pszText && pdwLen))
		return false;

	DWORD dwFilterMask;

	switch (eAssignment)
	{
		case MCS_ASSIGNMENT_PARAMETER:
		case MCS_ASSIGNMENT_PAN:
		case MCS_ASSIGNMENT_SEND:
			return false;

		case MCS_ASSIGNMENT_PLUGIN:
			dwFilterMask = PT_ALL;
			break;

		case MCS_ASSIGNMENT_EQ:
		case MCS_ASSIGNMENT_EQ_FREQ_GAIN:
			dwFilterMask = PT_EQ;
			break;
		
		case MCS_ASSIGNMENT_DYNAMICS:
			dwFilterMask = PT_DYNAMICS;
			break;

		default:
			return false;
	}

	return GetPluginProperties(eMixerStrip, dwStripNum, &dwPluginNum,
								dwFilterMask, NULL, pszText, pdwLen);
}

/////////////////////////////////////////////////////////////////////////////
