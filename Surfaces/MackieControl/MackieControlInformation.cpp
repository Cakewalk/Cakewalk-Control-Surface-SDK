#include "stdafx.h"

#include "MackieControlInformation.h"

/////////////////////////////////////////////////////////////////////////////

CMackieControlInformation::CMackieControlInformation()
{
	m_dwUniqueId = 0;
	m_dwOffset = 0;
}

CMackieControlInformation::~CMackieControlInformation()
{
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlInformation::Load(IStream *pStm)
{
	ULONG ulWritten;

	if (pStm->Read(&m_dwUniqueId, sizeof(m_dwUniqueId), &ulWritten) != S_OK ||
		ulWritten != sizeof(m_dwUniqueId))
	{
		TRACE("CMackieControlInformation::Load(): m_dwUniqueId failed\n");
		return false;
	}

	if (pStm->Read(&m_dwOffset, sizeof(m_dwOffset), &ulWritten) != S_OK ||
		ulWritten != sizeof(m_dwOffset))
	{
		TRACE("CMackieControlInformation::Load(): m_dwOffset failed\n");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CMackieControlInformation::Save(IStream *pStm)
{
	ULONG ulWritten;

	if (pStm->Write(&m_dwUniqueId, sizeof(m_dwUniqueId), &ulWritten) != S_OK ||
		ulWritten != sizeof(m_dwUniqueId))
	{
		TRACE("CMackieControlInformation::Save(): m_dwUniqueId failed\n");
		return false;
	}

	if (pStm->Write(&m_dwOffset, sizeof(m_dwOffset), &ulWritten) != S_OK ||
		ulWritten != sizeof(m_dwOffset))
	{
		TRACE("CMackieControlInformation::Save(): m_dwOffset failed\n");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
