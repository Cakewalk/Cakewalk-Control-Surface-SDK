#include "stdafx.h"

#include "FilterLocator.h"

/////////////////////////////////////////////////////////////////////////////

FilterLocator::FilterLocator()
{
	m_pSonarIdentity = NULL;
	m_pSonarMixer = NULL;

	m_bFlexiblePC = false;
}

FilterLocator::~FilterLocator()
{
}

void FilterLocator::OnConnect(ISonarIdentity *pSonarIdentity, ISonarMixer *pSonarMixer)
{
	m_pSonarIdentity = pSonarIdentity;
	m_pSonarMixer = pSonarMixer;

	m_bFlexiblePC = false;

		// the following is from VS-700 source
	if (m_pSonarIdentity && pSonarMixer)  // it make no sense in case the Mixer is not available
	{
		char  szHostName[ _MAX_PATH ] = { 0 };
		DWORD dwLen = _countof( szHostName );
		if (SUCCEEDED( m_pSonarIdentity->GetHostName( szHostName, &dwLen ) ))
		{
			ULONG	nMajor = 0;
			ULONG   nMinor = 0;
			ULONG	nRevision = 0;
			ULONG	nBuild = 0;

			m_pSonarIdentity->GetHostVersion( &nMajor, &nMinor, &nRevision, &nBuild );
			if (0 == _strnicmp( szHostName, "SONAR X1 Producer", 17 ))      // Producer or Producer Expanded
			{
				if (nMajor == 18 && nRevision >= 4)
					m_bFlexiblePC = true;
			}
			else if (0 == _strnicmp( szHostName, "SONAR", 5 )) // future version...
			{
				if (nMajor > 18)
					m_bFlexiblePC = true;
			}
		}
	}
}

DWORD FilterLocator::GetFilterNum(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_FILTER eFilter)
{
	DWORD dwFilterNum = (DWORD)eFilter;

	if (!m_bFlexiblePC)
		return dwFilterNum;

	float	fValue;

	if (!SUCCEEDED( m_pSonarMixer->GetMixParam( MIX_STRIP_ANY, 0, MIX_PARAM_SELECTED, 0, &fValue ) ))
		return dwFilterNum;
	SONAR_MIXER_STRIP eFocusedMixerStrip = static_cast<SONAR_MIXER_STRIP>(static_cast<int>(fValue));

	if (!SUCCEEDED( m_pSonarMixer->GetMixParam( eFocusedMixerStrip, 0, MIX_PARAM_SELECTED, 0, &fValue ) ))
		return dwFilterNum;
	DWORD dwFocusedStripNum = (DWORD)fValue;

	if (eFocusedMixerStrip == eMixerStrip && dwFocusedStripNum == dwStripNum)
	{
		// Current focused -> own counting
		if (SUCCEEDED( m_pSonarMixer->GetMixParam( eMixerStrip, dwStripNum, MIX_PARAM_FILTER_INDEX, dwFilterNum, &fValue ) ))
			return (DWORD)fValue;
	}
	else
	{
		// Not focused -> focused counting
		if (SUCCEEDED( m_pSonarMixer->GetMixParam( eFocusedMixerStrip, dwFocusedStripNum, MIX_PARAM_FILTER_INDEX, dwFilterNum, &fValue ) ))
			return (DWORD)fValue;
		// Not focused, but focused has no PC -> own counting
		if (SUCCEEDED( m_pSonarMixer->GetMixParam( eMixerStrip, dwStripNum, MIX_PARAM_FILTER_INDEX, dwFilterNum, &fValue ) ))
			return (DWORD)fValue;
	}
	return (DWORD)eFilter;
}
