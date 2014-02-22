#include "stdafx.h"

#include "ACTController.h"

/////////////////////////////////////////////////////////////////////////////

static long s_lRefresh = 0;

void CACTController::OnRefreshSurface(DWORD fdwRefresh)
{
	s_lRefresh++;

	if ( s_lRefresh % 2 != 0 )
		return;

	if (fdwRefresh != REFRESH_F_MIXER)
		TRACE("CACTController::OnRefreshSurface(%lu)\n", fdwRefresh);

	if (m_bSelectHighlightsTrack && m_eStripType == MIX_STRIP_TRACK)
		LimitAndSetSelectedTrack(GetParamSelected());

	DWORD dwNumTracks = GetStripCount(MIX_STRIP_TRACK);
	DWORD dwNumBuses  = GetStripCount(MIX_STRIP_BUS);
	DWORD dwNumMains  = GetStripCount(MIX_STRIP_MASTER);

	if (dwNumTracks != m_dwNumTracks ||
		dwNumBuses  != m_dwNumBuses  ||
		dwNumMains  != m_dwNumMains)
	{
		ShiftStripNum(0, true);
		ShiftSelectedTrack(0, true);
	}
	else if (fdwRefresh & (REFRESH_F_TOPOLOGY | REFRESH_F_PLUGIN))
		UpdateBindings();

	SONAR_UI_CONTEXT uiContext = GetCurrentContext();
	if (uiContext != m_uiContext)
	{
		m_uiContext = uiContext;

		if (m_bUseDynamicMappings && m_bACTFollowsContext)
			UpdateBindings();
	}

	UpdateToolbarText();

	m_dwNumTracks = dwNumTracks;
	m_dwNumBuses  = dwNumBuses;
	m_dwNumMains  = dwNumMains;
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::UpdateToolbarText()
{
	CString strText;

	switch (m_eStripType)
	{
		case MIX_STRIP_TRACK:		strText = "Trks";	break;
		case MIX_STRIP_AUX:			strText = "Auxs";	break;
		case MIX_STRIP_BUS:			strText = "Bus";	break;
		case MIX_STRIP_MAIN:		strText = "VMns";	break;
		case MIX_STRIP_MASTER:		strText = "Mstr";	break;
		default:					strText = "Err";	break;
	}

	CString strBuf;
	strBuf.Format(" %u-%u", GetStripNum() + 1, GetStripNum() + NUM_SLIDERS);
	strText += strBuf;

	strBuf.Format(", Sel %u", GetSelectedTrack() + 1);
	strText += strBuf;

	strBuf.Format(", ACT %s", m_bUseDynamicMappings ? m_strOn : m_strOff);
	strText += strBuf;

	strBuf.Format(" %d%d%d", m_iRotaryBank + 1, m_iSliderBank + 1, m_iButtonBank + 1);
	strText += strBuf;

	if (m_strToolbarText != strText)
	{
		m_strToolbarText = strText;

		if (m_pProject)
			m_pProject->OnNewStatus(0);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CACTController::UpdateBindings()
{
	TRACE("CACTController::UpdateBindings()\n");

	int m, n, id;

	BuildDynControlsList();

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		if (GetRotariesACTMode(m))
		{
			for (n = 0; n < NUM_KNOBS; n++)
				BindToACT(&m_SwKnob[m][n], MASK_KNOB(id++));
		}
		else
		{
			if (m_eRotariesMode == MCS_ASSIGNMENT_MUTLI_CHANNEL)
			{
				DWORD dwNumStrips = GetStripCount(m_eStripType);

				for (n = 0; n < NUM_KNOBS; n++)
				{
					DWORD dwStripNum = GetStripNum() + n;

					if (dwStripNum < dwNumStrips)
						BindParamToBinding(&m_SwKnob[m][n], m_eStripType, dwStripNum, m_dwKnobsBinding[m_iRotaryBank]);
					else
						m_SwKnob[m][n].ClearBinding();
				}
			}
			else
			{
				for (n = 0; n < NUM_KNOBS; n++)
					BindNthParam(&m_SwKnob[m][n], m_eStripType, GetSelectedTrack(), n);
			}
		}
	}

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		if (GetSlidersACTMode(m))
		{
			for (n = 0; n < NUM_SLIDERS; n++)
				BindToACT(&m_SwSlider[m][n], MASK_SLIDER(id++));
		}
		else
		{
			DWORD dwNumStrips = GetStripCount(m_eStripType);

			for (n = 0; n < NUM_SLIDERS; n++)
			{
				DWORD dwStripNum = GetStripNum() + n;

				if (dwStripNum < dwNumStrips)
					BindParamToBinding(&m_SwSlider[m][n], m_eStripType, dwStripNum, m_dwSlidersBinding[m_iSliderBank]);
				else
					m_SwSlider[m][n].ClearBinding();
			}
		}
	}

	id = 0;
	for (m = 0; m < NUM_BANKS; m++)
	{
		for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
		{
			if (GetButtonACTMode(m, (VirtualButton)n))
				BindToACT(&m_SwButton[m][n], MASK_BUTTON(id++));
			else
				m_SwButton[m][n].ClearBinding();
		}
	}

	m_dwUpdateCount++;

	OnContextSwitch();
}

/////////////////////////////////////////////////////////////////////////////
