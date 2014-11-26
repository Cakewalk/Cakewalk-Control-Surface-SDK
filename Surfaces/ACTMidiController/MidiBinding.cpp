#include "stdafx.h"

#include "MidiBinding.h"
#include "utils.h"

/////////////////////////////////////////////////////////////////////////////

// Version informatin for Persist()
//#define PERSISTENCE_VERSION		1
//#define PERSISTENCE_VERSION		2	// add m_eMatchType
//#define PERSISTENCE_VERSION		3	// add sysex string
#define PERSISTENCE_VERSION		4	// Add Message Interpretation, hinge, accel


/////////////////////////////////////////////////////////////////////////////

CMidiBinding::CMidiBinding() :
	m_eMessageInterpretation( MI_Literal ),
	m_eMatchType(MT_Message),
	m_dwHinge( 63 ),
	m_bUseAccel( false )
{
	m_bStatus = 0x00;
	m_bD1 = 0x00;
	m_bD2 = 0x00;
}

CMidiBinding::~CMidiBinding()
{
}

/////////////////////////////////////////////////////////////////////////////

void CMidiBinding::SetMessage(BYTE bStatus, BYTE bD1, BYTE bD2)
{
	m_bStatus = bStatus;
	m_bD1 = bD1;
	m_bD2 = bD2;

	m_vSysex.clear();
}

void CMidiBinding::SetMessage(const BYTE* pbSysex, size_t cbBytes )
{
	m_vSysex.clear();
	for ( size_t ix = 0; ix < cbBytes; ix++ )
		m_vSysex.push_back( pbSysex[ix] );

	m_bStatus = 0x00;
	m_bD1 = 0x00;
	m_bD2 = 0x00;
	m_eMatchType = MT_Message;
}

void CMidiBinding::GetSysex( std::vector<BYTE>* pv )
{
	if ( m_vSysex.size() < 3 )
		return;
	pv->clear();
	for ( size_t ix = 1; ix < m_vSysex.size() - 1; ix++ )
		pv->push_back( m_vSysex[ix] );
}
void CMidiBinding::SetSysex( std::vector<BYTE>& v )
{
	m_vSysex.clear();
	m_vSysex.push_back( 0xF0 );
	for ( size_t ix = 0; ix < v.size(); ix++ )
		m_vSysex.push_back( v[ix] );
	m_vSysex.push_back( 0xF7 );
}



/////////////////////////////////////////////////////////////////////////////

void CMidiBinding::SetMatchType(MatchType mt)
{
	m_eMatchType = mt;
}

/////////////////////////////////////////////////////////////////////////////

bool CMidiBinding::IsMatch(BYTE bStatus, BYTE bD1, BYTE bD2)
{
	return IsMatch( m_eMatchType, bStatus, bD1, bD2 );
}

bool CMidiBinding::IsMatch(MatchType mt, BYTE bStatus, BYTE bD1, BYTE bD2)
{
	if ( MT_MessageAndValue == m_eMatchType )
		return m_bStatus == bStatus && m_bD1 == bD1 && m_bD2 == bD2;
	else if ( MT_Message == m_eMatchType )
		return m_bStatus == bStatus && m_bD1 == bD1;
	else if ( MT_MessageAndBool == m_eMatchType )
		return m_bStatus == bStatus && m_bD1 == bD1 && (!m_bD2 == !bD2);
	else
	{
		// ASSERT(0);
		return false;
	}
}


bool CMidiBinding::IsMatch(const BYTE* pbSysex, size_t cbBytes )
{
	if ( m_vSysex.empty() )
		return false;

	if ( cbBytes != m_vSysex.size() )
		return false;

	return 0 == ::memcmp( &(m_vSysex[0]), pbSysex, cbBytes );
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiBinding::Persist(IStream* pStm, bool bSave)
{
	DWORD dwVersion = PERSISTENCE_VERSION;

	// Save/Load the version
	if (FAILED(Persist(pStm, bSave, &dwVersion, sizeof(dwVersion))))
		return E_FAIL;

	// Member variables
	CHECK_PERSIST(Persist(pStm, bSave, &m_bStatus, sizeof(m_bStatus)));
	CHECK_PERSIST(Persist(pStm, bSave, &m_bD1, sizeof(m_bD1)));
	CHECK_PERSIST(Persist(pStm, bSave, &m_bD2, sizeof(m_bD2)));

	bool bDummyExactMatch = false;
	CHECK_PERSIST(Persist(pStm, bSave, &bDummyExactMatch, sizeof(bDummyExactMatch)));

	if ( dwVersion >= 2 )
	{
		CHECK_PERSIST(Persist(pStm, bSave, &m_eMatchType, sizeof(m_eMatchType)));
	}

	if ( dwVersion >= 3 )
	{
		// count of prefix
		int cb = (int)m_vSysex.size();
		CHECK_PERSIST(Persist(pStm, bSave, &cb, sizeof(cb)));

		if ( bSave )
		{
			for ( int ix = 0; ix < cb; ix++ )
			{
				BYTE b = m_vSysex[ix];
				CHECK_PERSIST(Persist( pStm, bSave, &b, 1 ) );
			}
		}
		else
		{
			m_vSysex.clear();
			for ( int ix = 0; ix < cb; ix++ )
			{
				BYTE b = 0;
				CHECK_PERSIST(Persist( pStm, bSave, &b, 1 ) );
				m_vSysex.push_back( b );
			}
		}

		// persist dummy variable section count
		cb = 0;
		CHECK_PERSIST(Persist(pStm, bSave, &cb, sizeof(cb)));

		// persist dummy suffix
		cb = 0;
		CHECK_PERSIST(Persist(pStm, bSave, &cb, sizeof(cb)));
	}

	if ( dwVersion >= 4 )
	{
		CHECK_PERSIST(Persist( pStm, bSave, &m_eMessageInterpretation, sizeof(m_eMessageInterpretation) ) );
		CHECK_PERSIST(Persist( pStm, bSave, &m_dwHinge, sizeof(m_dwHinge) ) );
		CHECK_PERSIST(Persist( pStm, bSave, &m_bUseAccel, sizeof(m_bUseAccel) ) );
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////

HRESULT CMidiBinding::Persist(IStream* pStm, bool bSave, void *pData, ULONG ulCount)
{
//	TRACE("CMidiBinding::Persist() %s count: %u\n", bSave ? "save" : "load", ulCount);

	if (bSave)
	{
		ULONG ulWritten;

		if (pStm->Write(pData, ulCount, &ulWritten) != S_OK || ulWritten != ulCount)
		{
			TRACE("CMidiBinding::Persist(): save failed\n");
			return E_FAIL;
		}
	}
	else
	{
		ULONG ulRead;

		if (pStm->Read(pData, ulCount, &ulRead) != S_OK || ulRead != ulCount)
		{
			TRACE("CMidiBinding::Persist: load failed\n");
			return E_FAIL;
		}
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
