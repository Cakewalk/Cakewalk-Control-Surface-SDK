#include "stdafx.h"

#include "TacomaSurface.h"
#include "ttsdbg.h"
#include <filestream.h>



static LPCTSTR szPreampExt = _T("preamp");
static LPCTSTR szDMExt =  _T("directmix");
static LPCTSTR szOCPreampExt = _T("OCpreamp");
static LPCTSTR szOCDMExt =  _T("OCdirectmix");



WORD CIOBoxInterface::CPreampPersist::m_wPersistChunkID = 1;
WORD CIOBoxInterface::CDMPersist::m_wPersistChunkID = 2;

////////////////////////////////////////////////////////////////////////////////
// Attempts to create a directory, recursively.  Returns TRUE on success (declaration pulled from TTSUtil.h)

HRESULT CreatePath( const TCHAR* pszPathName, int idsPrompt = -1 );

// Moved from TTSUtil.h to obviate need for that LIB andf header
/////////////////////////////////////////////////////////////////////////////

inline BOOL FileExists( const TCHAR* pcszPathName )
{
	// One way to check for existence is to open the file, read-only:
	//
	//		HANDLE const hFile = CreateFile( pcszPathName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	//		if (INVALID_HANDLE_VALUE != hFile)
	//		{
	//			CloseHandle( hFile );
	//			return TRUE;
	//		}
	//		else
	//			return FALSE;
	//
	// A simpler/faster way is to use GetFileAttributes(), and check for error:
	return 0xFFFFFFFF != ::GetFileAttributes( pcszPathName );
}




//---------------------------------------------------------------
// Get (and create if needed) the data path for presets
HRESULT CTacomaSurface::getDataPath( CString* pstrPath )
{
	TCHAR szSettings[MAX_PATH];

	OSVERSIONINFO osvi = { sizeof(osvi) };
	GetVersionEx( &osvi );
	if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
	{
		return E_NOTIMPL;
		/*
		// Win 9x: use application directory
		ASSERT( !IsBadWritePtr( szSettings, _MAX_PATH ) );
		::MakeExePathName( AfxGetInstanceHandle(), szSettings, _MAX_PATH, NULL );
		*/
	}
	// NT, 2K, 2K3, XP, Vista
	else if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
	{
		// Use the application data folder
		if (!SHGetSpecialFolderPath( GetDesktopWindow(), szSettings, CSIDL_APPDATA, TRUE ) )
		{
			ASSERT(0);
			return E_UNEXPECTED;
		}

		// Tag on the appropriate application data path
		if (!PathAppend( szSettings, _T("Cakewalk\\Shared Presets\\VS") ) )
		{
			ASSERT(0);
			return E_UNEXPECTED;
		}

		// create if necessary
		if (!PathIsDirectory( szSettings ))
		{
			if (FAILED( ::CreatePath( szSettings ) ) )
			{
				ASSERT(0);
				return E_FAIL;
			}
		}
	}
	else	// unknown platform
	{
		ASSERT( FALSE );
		return E_FAIL; 
	}

	*pstrPath = szSettings;

	return S_OK;
}



//--------------------------------------------------------------------------------
HRESULT CTacomaSurface::GetPresetsOnDisk( PresetType pt, std::vector<CString>* pvNames )
{
	CString strPath;
	getDataPath( &strPath );

	UINT u = 0;
	u = GetIOBoxInterface()->GetActiveInterface();


	CString strPreset;
	TCHAR szMask[16];
if (( GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
{
	switch (pt)
	{
	case PT_Preamp:
		::_tcscpy( szMask, szPreampExt );
		break;
	case PT_DM:
		::_tcscpy( szMask, szDMExt );
		break;
	default:
		ASSERT(0);
		return E_INVALIDARG;
	}
}

else
	switch (pt)
	{
	case PT_Preamp:
		::_tcscpy( szMask, szOCPreampExt );
		break;
	case PT_DM:
		::_tcscpy( szMask, szOCDMExt );
		break;
	default:
		ASSERT(0);
		return E_INVALIDARG;
	}


	strPreset.Format( _T("%s/*.%s"), strPath, szMask );

	WIN32_FIND_DATA find;
	HANDLE hFind = ::FindFirstFile( strPreset, &find );
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			// None of these attrs should be set
			if (  find.dwFileAttributes &	(  FILE_ATTRIBUTE_DIRECTORY
													|	FILE_ATTRIBUTE_COMPRESSED
													|	FILE_ATTRIBUTE_SYSTEM ) )
			{
				continue;
			}

			CString strFile( find.cFileName );
			int cDot = strFile.ReverseFind( _T('.') );
			if ( -1 == cDot )
			{
				ASSERT(0);	// no extenaion
				continue;
			}
			pvNames->push_back( strFile.Left( cDot ) );
		}
		while ( ::FindNextFile( hFind, &find ) );
		::FindClose( hFind );	// shut down search...
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
HRESULT	CTacomaSurface::LoadIOPreset( PresetType pt, LPCTSTR szName )
{
	CString strPath;
	CHECK_RET( getDataPath( &strPath ) );
	LPCTSTR szExt = NULL;

	UINT u = 0;
	u = GetIOBoxInterface()->GetActiveInterface();

	CString strFile;

	if (( GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
		LPCTSTR szExt = pt == PT_Preamp ? szPreampExt : szDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}
	else
	{
		LPCTSTR szExt = pt == PT_Preamp ? szOCPreampExt : szOCDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}


	CFile file;
	if ( !file.Open( strFile, CFile::modeRead ) )
		return E_FAIL;

	CFileStream fs( file );
	CTTSPersistObject* pObj = pt == PT_Preamp ? m_pIOBoxInterface->GetPreampPersistObject() : m_pIOBoxInterface->GetDMPersistObject();
	CHECK_RET( pObj->Load( &fs ) );

	if ( pt == PT_Preamp )
		m_strPreampPreset = szName;
	else
		m_strDMPreset = szName;

	return S_OK;
}


//-----------------------------------------------------------------------------
HRESULT	CTacomaSurface::SaveIOPreset( PresetType pt, LPCTSTR szName )
{
	CString strPath;
	CHECK_RET( getDataPath( &strPath ) );

	CString strFile;

	UINT u = 0;
	u = GetIOBoxInterface()->GetActiveInterface();


	if (( GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
		LPCTSTR szExt = pt == PT_Preamp ? szPreampExt : szDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}
	else
	{
		LPCTSTR szExt = pt == PT_Preamp ? szOCPreampExt : szOCDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}


	CFile file;
	if ( !file.Open( strFile, CFile::modeCreate|CFile::modeWrite ) )
		return E_FAIL;

	CFileStream fs( file );
	CTTSPersistObject* pObj = pt == PT_Preamp ? m_pIOBoxInterface->GetPreampPersistObject() : m_pIOBoxInterface->GetDMPersistObject();
	CHECK_RET( pObj->Save( &fs,  TRUE ) );

	if ( pt == PT_Preamp )
		m_strPreampPreset = szName;
	else
		m_strDMPreset = szName;

	return S_OK;
}


bool	CTacomaSurface::PresetExists( PresetType pt, LPCTSTR szName )
{
	CString strPath;
	if ( FAILED( getDataPath( &strPath ) ))
		return false;

	CString strFile;

	UINT u = 0;
	u = GetIOBoxInterface()->GetActiveInterface();

	if (( GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
		LPCTSTR szExt = pt == PT_Preamp ? szPreampExt : szDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}
	else
	{
		LPCTSTR szExt = pt == PT_Preamp ? szOCPreampExt : szOCDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}

	return !!::FileExists( strFile );
}


//-----------------------------------------------------------------------------
HRESULT	CTacomaSurface::DeletePreset( PresetType pt, LPCTSTR szName )
{
	CString strPath;
	CHECK_RET( getDataPath( &strPath  ) );
	LPCTSTR szExt = NULL;

	UINT u = 0;
	u = GetIOBoxInterface()->GetActiveInterface();
	CString strFile;

	if (( GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R"))) || (GetIOBoxInterface()->m_viofacestr[u] == (_T("VS-700R2"))))
	{
		LPCTSTR szExt = pt == PT_Preamp ? szPreampExt : szDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}
	else
	{
		LPCTSTR szExt = pt == PT_Preamp ? szOCPreampExt : szOCDMExt;

		strFile.Format( _T("%s\\%s.%s"), strPath, szName, szExt );
	}


	::_tunlink( strFile );

	SetCurrentPresetName( pt, _T("") );

	return S_OK;
}


//-----------------------------------------------------------------------------
CString	CTacomaSurface::GetCurrentPresetName( PresetType pt )
{
	if ( pt == PT_Preamp )
		return m_strPreampPreset;
	return m_strDMPreset;
}



//-----------------------------------------------------------------------------
void CTacomaSurface::SetCurrentPresetName( PresetType pt, LPCTSTR szName )
{
	if ( pt == PT_Preamp )
		m_strPreampPreset = szName;
	else
		m_strDMPreset = szName;
}


//-----------------------------------------------------------------------------
// Persist Imp methods

//WORD CIOBoxInterface::CPreampPersist::m_wPersistSchema = 0;
//WORD CIOBoxInterface::CPreampPersist::m_wPersistSchema = 1;	// digital input source
WORD CIOBoxInterface::CPreampPersist::m_wPersistSchema = 2;	// second box's system stuff

HRESULT CIOBoxInterface::CPreampPersist::Persist( WORD wSchema, CPersistDDX& ddx )
{
	if (!m_pIOBox )
		return E_POINTER;

	UINT ixBox = 0;
	ixBox = m_pIOBox->GetActiveInterface();

	if ( 0 == wSchema )
	{
		int cChannels = TacomaIOBox::NumChannels;
		CHECK_XFER( ddx.Xfer( &cChannels ) );

		if (( m_pIOBox->m_viofacestr[ixBox] == (_T("VS-700R"))) || ( m_pIOBox->m_viofacestr[ixBox] == (_T("VS-700R2"))))
		{
			for ( int iChan = 0; iChan < cChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_pIOBox->m_aChans[ixBox][iChan];
				CHECK_XFER( ddx.Xfer( &ioc.paramGain.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramLoCut.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramPad.f01Val ) );

				// phantom - we may not want to load this
				float fPhantom = ioc.paramPhantom.f01Val;
				CHECK_XFER( ddx.Xfer( &fPhantom ));
				// don't set this during load - ever (loading presets shouldnt blow up a ribbon mic)

				CHECK_XFER( ddx.Xfer( &ioc.paramPhase.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramStereLink.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramCompEnable.f01Val ) );
			}


			// the comps
			int cComps = TacomaIOBox::NumMicInputChannels;
			CHECK_XFER( ddx.Xfer( &cComps ) );

			for ( int iChan = 0; iChan < cComps; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_pIOBox->m_aComps[ixBox][iChan];
				CHECK_XFER( ddx.Xfer( &cmpc.paramAttack.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramMakeupGain.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramRatio.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramThreshold.f01Val ) );
			}

			// sync source
			CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[0].paramSyncSource.f01Val ) );
		}
		else
			//------------------------------------------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------------------------
		{
			for ( int iChan = 0; iChan < cChannels; iChan++ )
			{
				TacomaIOBox::TacomaIOChannel& ioc = m_pIOBox->m_aChans[ixBox][iChan];
				//CHECK_XFER( ddx.Xfer( &ioc.paramGain.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramLoCut.f01Val ) );
				//CHECK_XFER( ddx.Xfer( &ioc.paramPhase.f01Val ) );

				// phantom - we may not want to load this
				//float fPhantom = ioc.paramPhantom.f01Val;
				//CHECK_XFER( ddx.Xfer( &fPhantom ));
				// don't set this during load - ever (loading presets shouldnt blow up a ribbon mic)

				CHECK_XFER( ddx.Xfer( &ioc.paramPhase.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramStereLink.f01Val ) );
				CHECK_XFER( ddx.Xfer( &ioc.paramCompEnable.f01Val ) );
			}


			// the comps
			int cComps = TacomaIOBox::NumMicInputChannels;
			CHECK_XFER( ddx.Xfer( &cComps ) );

			for ( int iChan = 0; iChan < cComps; iChan++ )
			{
				TacomaIOBox::CompressorChannel& cmpc = m_pIOBox->m_aComps[ixBox][iChan];
				CHECK_XFER( ddx.Xfer( &cmpc.paramGate.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramAttack.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramRelease.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramRatio.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramThreshold.f01Val ) );
				CHECK_XFER( ddx.Xfer( &cmpc.paramMakeupGain.f01Val ) );
			}
		}

	}
	else if ( 1 == wSchema )
	{
		CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[0].paramDigitalInput.f01Val ) );
	}
	else if ( 2 == wSchema )
	{
		// sync source
		CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[1].paramSyncSource.f01Val ) );
		CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[1].paramDigitalInput.f01Val ) );
	}
	else
		CHECK_XFER( E_FAIL );


	// We don't get an OnPersistEnd() call so if schemas are added, make sure this happens only once!
	if ( !ddx.IsSaving() )
		m_pIOBox->SendAllPreampParams();


	return S_OK;
}


HRESULT CIOBoxInterface::CPreampPersist::OnPersistEnd( CPersistDDX& ddx )
{
	return S_OK;
}



//WORD CIOBoxInterface::CDMPersist::m_wPersistSchema = 0;
WORD CIOBoxInterface::CDMPersist::m_wPersistSchema = 1;	// system stuff

HRESULT CIOBoxInterface::CDMPersist::Persist(WORD wSchema, CPersistDDX &ddx)
{
	if (!m_pIOBox )
		return E_POINTER;

	UINT u = 0;
	u = m_pIOBox->GetActiveInterface();

	if ( 0 == wSchema )

		if (( m_pIOBox->m_viofacestr[u] == (_T("VS-700R"))) || ( m_pIOBox->m_viofacestr[u] == (_T("VS-700R2"))))
		{
			{
				int cChannels = TacomaIOBox::NumChannels;
				CHECK_XFER( ddx.Xfer( &cChannels ) );

				for ( size_t ixBox = 0; ixBox < 2; ixBox++ )
				{
					for ( int iChan = 0; iChan < cChannels; iChan++ )
					{
						TacomaIOBox::TacomaIOChannel& ioc = m_pIOBox->m_aChans[ixBox][iChan];
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixVol.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixPan.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixMute.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixSolo.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixMono.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramStereLink.f01Val ) );
					}

					// DM outputs
					CHECK_XFER( ddx.Xfer( &m_pIOBox->m_DMOutput[ixBox].paramDigOut.f01Val ) );
					CHECK_XFER( ddx.Xfer( &m_pIOBox->m_DMOutput[ixBox].paramMainOut.f01Val ) );
					CHECK_XFER( ddx.Xfer( &m_pIOBox->m_DMOutput[ixBox].paramSubOut.f01Val ) );
				}
			}
		}
		else
		{
			{
				int cChannels = TacomaIOBox::NumChannels;
				CHECK_XFER( ddx.Xfer( &cChannels ) );

				for ( size_t ixBox = 0; ixBox < 2; ixBox++ )
				{
					for ( int iChan = 0; iChan < cChannels; iChan++ )
					{
						TacomaIOBox::OctaIOChannel& ioc = m_pIOBox->m_OCChans[ixBox][iChan][m_pIOBox->m_byDirectMix];

						CHECK_XFER( ddx.Xfer( &ioc.paramDMixPan.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixMute.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramDMixSolo.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramInLink.f01Val ) );
						CHECK_XFER( ddx.Xfer( &ioc.paramStereLinkOC.f01Val ) );
					}

				}
			}


		}
	else if ( 1 == wSchema )
	{
		CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[0].paramDigitalInput.f01Val ) );
		CHECK_XFER( ddx.Xfer( &m_pIOBox->m_System[1].paramDigitalInput.f01Val ) );
	}
	else
		CHECK_XFER( E_FAIL );

	// We don't get an OnPersistEnd() call so if schemas are added, make sure this happens only once!
	if ( !ddx.IsSaving() )
		m_pIOBox->sendAllDMParams();

	return S_OK;
}


HRESULT CIOBoxInterface::CDMPersist::OnPersistEnd( CPersistDDX& ddx )
{
	return S_OK;
}
