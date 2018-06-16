#ifndef FilterLocator_h
#define FilterLocator_h

/////////////////////////////////////////////////////////////////////////////
// Possible optimizations:
//   Cache results till REFRESH_F_TOPOLOGY or REFRESH_F_PLUGIN
//   Check current focus only once per processing (Refresh/Incoming MIDI)
/////////////////////////////////////////////////////////////////////////////

class FilterLocator
{
public:
	FilterLocator();
	virtual ~FilterLocator();

	void OnConnect(ISonarIdentity *pSonarIdentity, ISonarMixer *pSonarMixer);

	// It converts eFilter to dwFilterNum for specified strip,
	//  the result can be invalid if the filter does not exist or in case of other failures.
	DWORD GetFilterNum(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_FILTER eFilter);

	bool IsFlexiblePC()	{ return m_bFlexiblePC; }

private:
	ISonarIdentity*		m_pSonarIdentity;
	ISonarMixer*		m_pSonarMixer;

	bool			m_bFlexiblePC;
};

#endif // FilterLocator_h
