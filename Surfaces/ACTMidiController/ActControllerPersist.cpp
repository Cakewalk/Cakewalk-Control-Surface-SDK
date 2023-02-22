#include "stdafx.h"

#include "ACTController.h"
#include "utils.h"

/////////////////////////////////////////////////////////////////////////////

// Version informatin for save/load
//#define PERSISTENCE_VERSION		6
#define PERSISTENCE_VERSION		7		// add Rotary Slider capture mode and init msgs

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Persist(IStream* pStm, bool bSave)
{
	TRACE("CACTController::Persist() %s\n", bSave ? "save" : "load");

	DWORD dwVersion = PERSISTENCE_VERSION;
	int n, num_banks;

	// Save/Load the version
	if (FAILED(Persist(pStm, bSave, &dwVersion, sizeof(dwVersion))))
		return E_FAIL;

	// Save/Load the first bank
	if (FAILED(PersistBank(pStm, bSave, 0)))
		return E_FAIL;

	// Select highlights track
	if (FAILED(Persist(pStm, bSave, &m_bSelectHighlightsTrack, sizeof(m_bSelectHighlightsTrack))))
		return E_FAIL;

	// Labels
	for (n = 0; n < NUM_KNOBS; n++)
	{
		if (FAILED(Persist(pStm, bSave, &m_strRotaryLabel[n])))
			return E_FAIL;
	}
	for (n = 0; n < NUM_SLIDERS; n++)
	{
		if (FAILED(Persist(pStm, bSave, &m_strSliderLabel[n])))
			return E_FAIL;
	}
	for (n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
	{
		if (FAILED(Persist(pStm, bSave, &m_strButtonLabel[n])))
			return E_FAIL;
	}

	if (dwVersion <= 1)
		return S_OK;

	// ACT follows context
	if (FAILED(Persist(pStm, bSave, &m_bACTFollowsContext, sizeof(m_bACTFollowsContext))))
		return E_FAIL;

	// MIDI Bindings
	for (n = 0; n < NUM_KNOBS; n++)
	{
		if (FAILED(m_cMidiKnob[n].Persist(pStm, bSave)))
			return E_FAIL;
	}
	for (n = 0; n < NUM_SLIDERS; n++)
	{
		if (FAILED(m_cMidiSlider[n].Persist(pStm, bSave)))
			return E_FAIL;
	}
	for (n = 0; n < NUM_BUTTONS; n++)
	{
		if (FAILED(m_cMidiButton[n].Persist(pStm, bSave)))
			return E_FAIL;
	}
	if (FAILED(m_cMidiModifierDown.Persist(pStm, bSave)))
		return E_FAIL;
	if (FAILED(m_cMidiModifierUp.Persist(pStm, bSave)))
		return E_FAIL;

	if (dwVersion <= 2)
		return S_OK;

	// ACT enable
	if (FAILED(Persist(pStm, bSave, &m_bUseDynamicMappings, sizeof(m_bUseDynamicMappings))))
		return E_FAIL;

	// Rotaries mode
	if (FAILED(Persist(pStm, bSave, &m_eRotariesMode, sizeof(m_eRotariesMode))))
		return E_FAIL;

	// Control group
	if (FAILED(Persist(pStm, bSave, &m_eStripType, sizeof(m_eStripType))))
		return E_FAIL;

	if (dwVersion <= 3)
		return S_OK;

	// Number of banks
	num_banks = NUM_BANKS;
	if (FAILED(Persist(pStm, bSave, &num_banks, sizeof(num_banks))))
		return E_FAIL;

	if (num_banks > NUM_BANKS)
		num_banks = NUM_BANKS;

	// Save/Load the rest of the banks
	for (n = 1; n < num_banks; n++)
	{
		if (FAILED(PersistBank(pStm, bSave, n)))
			return E_FAIL;
	}

	if (dwVersion <= 4)
		return S_OK;

	// Comments
	if (FAILED(Persist(pStm, bSave, &m_strComments)))
		return E_FAIL;

	if (dwVersion <= 5)
		return S_OK;

	// Rotary bank
	if (FAILED(Persist(pStm, bSave, &m_iRotaryBank, sizeof(m_iRotaryBank))))
		return E_FAIL;

	// Slider bank
	if (FAILED(Persist(pStm, bSave, &m_iSliderBank, sizeof(m_iSliderBank))))
		return E_FAIL;

	// Button bank
	if (FAILED(Persist(pStm, bSave, &m_iButtonBank, sizeof(m_iButtonBank))))
		return E_FAIL;

	if ( !bSave )
	{
		// if these are out of range, we'll crash
		if ( m_iRotaryBank >= NUM_BANKS || m_iRotaryBank < 0 )
		{
			m_iRotaryBank=0;
			ASSERT(0);
		}
		if ( m_iSliderBank >= NUM_BANKS || m_iSliderBank < 0 )
		{
			m_iSliderBank=0;
			ASSERT(0);
		}
		if ( m_iButtonBank >= NUM_BANKS || m_iButtonBank < 0 )
		{
			m_iButtonBank=0;
			ASSERT(0);
		}
	}

	if ( dwVersion <= 6 )
		return S_OK;

	for ( n = 0; n < NUM_BANKS; n++ )
	{
		if (FAILED(Persist(pStm, bSave, &m_aCTRotary[n], sizeof(m_aCTRotary[n]))))
			return E_FAIL;
		if (FAILED(Persist(pStm, bSave, &m_aCTSlider[n], sizeof(m_aCTSlider[n]))))
			return E_FAIL;
	}

	// Initialization strings
	DWORD cShortMsg = (DWORD)m_vdwInitShortMsg.size();
	CHECK_PERSIST(Persist(pStm, bSave, &cShortMsg, sizeof(cShortMsg)));
	if ( bSave )
	{
		for ( DWORD ix = 0; ix < cShortMsg; ix++ )
		{
			DWORD dwmsg = m_vdwInitShortMsg[ix];
			CHECK_PERSIST( Persist(pStm, bSave, &dwmsg, sizeof(dwmsg)));
		}
	}
	else
	{
		m_vdwInitShortMsg.clear();
		for ( DWORD ix = 0; ix < cShortMsg; ix++ )
		{
			DWORD dwmsg = 0;
			CHECK_PERSIST( Persist(pStm, bSave, &dwmsg, sizeof(dwmsg)));
			m_vdwInitShortMsg.push_back( dwmsg );
		}
	}
	DWORD cLongMsg = (DWORD)m_vdwInitSysexMsg.size();
	CHECK_PERSIST(Persist(pStm, bSave, &cLongMsg, sizeof(cLongMsg)));
	if ( bSave )
	{
		for ( DWORD ix = 0; ix < cLongMsg; ix++ )
		{
			BYTE by = m_vdwInitSysexMsg[ix];
			CHECK_PERSIST( Persist(pStm, bSave, &by, sizeof(by)));
		}
	}
	else
	{
		m_vdwInitSysexMsg.clear();
		for ( DWORD ix = 0; ix < cLongMsg; ix++ )
		{
			BYTE by = 0;
			CHECK_PERSIST( Persist(pStm, bSave, &by, sizeof(by)));
			m_vdwInitSysexMsg.push_back(by);
		}
	}


	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::PersistBank(IStream* pStm, bool bSave, int iBank)
{
	// Rotaries binding
	if (FAILED(Persist(pStm, bSave, &m_dwKnobsBinding[iBank], sizeof(m_dwKnobsBinding[iBank]))))
		return E_FAIL;

	// Exclude rotaries from ACT
	if (FAILED(Persist(pStm, bSave, &m_bExcludeRotariesACT[iBank], sizeof(m_bExcludeRotariesACT[iBank]))))
		return E_FAIL;

	// Sliders binding
	if (FAILED(Persist(pStm, bSave, &m_dwSlidersBinding[iBank], sizeof(m_dwSlidersBinding[iBank]))))
		return E_FAIL;

	// Exclude sliders from ACT
	if (FAILED(Persist(pStm, bSave, &m_bExcludeSlidersACT[iBank], sizeof(m_bExcludeSlidersACT[iBank]))))
		return E_FAIL;

	// Buttons
	for (int n = 0; n < NUM_VIRTUAL_BUTTONS; n++)
	{
		if (FAILED(Persist(pStm, bSave, &m_dwButtonAction[iBank][n], sizeof(m_dwButtonAction[iBank][n]))))
			return E_FAIL;

		if (FAILED(Persist(pStm, bSave, &m_bButtonExcludeACT[iBank][n], sizeof(m_bButtonExcludeACT[iBank][n]))))
			return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Persist(IStream* pStm, bool bSave, CString *pStr)
{
	// string data used to be in char format!
	// CString will be wide in UNICODE build.
	// be explicit here for safety.
	int iLen;

	if (bSave)
	{
		iLen = pStr->GetLength();
		if (FAILED(Persist(pStm, bSave, &iLen, sizeof(iLen))))
			return E_FAIL;
		char sz[512] = { NULL };
		TCHAR2Char(sz, pStr->GetBuffer(), iLen);
		pStr->ReleaseBuffer();
		if (FAILED(Persist(pStm, bSave, sz, iLen)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(Persist(pStm, bSave, &iLen, sizeof(iLen))))
			return E_FAIL;

		char * sz = new char[iLen+1];
		TCHAR * tsz = new TCHAR[iLen+1];
		// shouldn't be trying to read a novel!
		if (iLen < 0 || iLen >= INT_MAX)
		{
			TRACE("CACTController::Persist(): ERROR: bogus iLen %d\n", iLen);
			ASSERT(0);
		}

		HRESULT hr = Persist(pStm, bSave, (void*)sz, iLen);
				
		Char2TCHAR(tsz, sz, iLen+1);
		*pStr = tsz; // assign to dest

		// free allocations
		delete[] sz;
		delete[] tsz;

		if (FAILED(hr))
			return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CACTController::Persist(IStream* pStm, bool bSave, void *pData, ULONG ulCount)
{
//	TRACE("CACTController::Persist() %s count: %u\n", bSave ? "save" : "load", ulCount);

	if (bSave)
	{
		ULONG ulWritten;

		if (pStm->Write(pData, ulCount, &ulWritten) != S_OK || ulWritten != ulCount)
		{
			TRACE("CACTController::Persist(): save failed\n");
			return E_FAIL;
		}
	}
	else
	{
		ULONG ulRead;
		HRESULT hr = pStm->Read(pData, ulCount, &ulRead);
		if (hr != S_OK || ulRead != ulCount)
		{
			TRACE("CACTController::Persist: load failed, hr = %0x, ulRead %d, ulCount %d\n", ulRead, ulCount);
			return E_FAIL;
		}
		//else
		//	TRACE("CACTController::Persist: load succeeded\n");
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
