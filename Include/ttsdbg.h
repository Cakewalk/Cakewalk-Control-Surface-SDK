/* ======================================================================= */
/* $Header: /Src/Include/ttsdbg.h 12    9/01/03 10:18p Bdamiano $
 *
 * PACKAGE: TTS Utility library; debug facilites formerly in ttsutil.h
 *
 * DESCRIPTION: BREAK/TRACE/ASSERT/VERIFY macros for debugging, assertions.
 *					 Logging support is enabled via the ENABLE_DEBUG_LOGGING definition.
 *					 Set ENABLE_DEBUG_LOGGING to TRUE/FALSE to turn on/off logging support.
 *
 * ---
 * See logfile for revision comments.
 *
 * Copyright (C) 1987-1997 by Greg Hendershott.  All rights reserved.
 */
/* ======================================================================= */

#ifndef __TTSDBG_H__
#define __TTSDBG_H__

#pragma warning(disable:4996) // '_snwprintf' warning

#include <TTSLogger.h>

#ifndef TTSAPI
#ifdef _BUILD_TTSUTIL_
	#define TTSAPI		__declspec(dllexport)
	#define TTSCLASS	__declspec(dllexport)
	#define TTSDATA	__declspec(dllexport)
#else
	#define TTSAPI		__declspec(dllimport)
	#define TTSCLASS	__declspec(dllimport)
	#define TTSDATA	__declspec(dllimport)
#endif
#endif

// Turn on debug logging support for debug and beta builds, by default
#if defined(_DEBUG) || defined(BETA) 
	#ifndef ENABLE_DEBUG_LOGGING
		#define ENABLE_DEBUG_LOGGING	1
	#endif
#endif


#ifdef _DEBUG 
	#ifdef __AFX_H__		// define MFC macros only for MFC clients
		inline void DEBUGBREAK() { AfxDebugBreak(); }
	#else		
		inline void DEBUGBREAK() { DebugBreak(); }
	#endif	// no MFC
#else		// _DEBUG
	#define DEBUGBREAK()
#endif	// _DEBUG


#ifdef __AFX_H__		// define MFC macros only for MFC clients

////////////////////////////////////////////////////////////////////////////////
// The BETA preprocessor flag enables ASSERT and VERIFY macros

#if BETA && !defined(_DEBUG)

BOOL TTSAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);

#define THIS_FILE __FILE__

#endif // BETA && !defined(_DEBUG)


////////////////////////////////////////////////////////////////////
// Declare default(no log support) TRACE, ASSERT and VERIFY macros
////////////////////////////////////////////////////////////////////

#ifdef _DEBUG 
	#ifdef DEFAULT_TRACE
		#undef DEFAULT_TRACE
	#endif
	#define DEFAULT_TRACE	::AfxTrace 
#else  
	#ifdef DEFAULT_TRACE
		#undef DEFAULT_TRACE
	#endif
	#define DEFAULT_TRACE   1 ? (void)0 : ::AfxTrace
#endif	// !_DEBUG

#if defined(_DEBUG) || BETA // Do something useful

	#if BETA
		#undef AfxDebugBreak
		#define AfxDebugBreak DebugBreak
	#endif

	inline void BREAK() { AfxDebugBreak(); }

	// MFC's VERIFY() doesn't return a value. The one we had provided with
	// TTSUTIL does return a value -- which is extremely useful. Replace MFC's
	// with ours.
	#ifdef DEFAULT_VERIFY	
		#undef DEFAULT_VERIFY
	#endif
	#define DEFAULT_VERIFY(f) ( (f) ? TRUE : (AfxAssertFailedLine(THIS_FILE, __LINE__) ? (BREAK(), FALSE) : FALSE) )

	#ifndef DEFAULT_ASSERT
		#define DEFAULT_ASSERT(f) \
		do \
		{ \
			if (!(f) && AfxAssertFailedLine(THIS_FILE, __LINE__)) \
				AfxDebugBreak(); \
		} while (0)
	#endif

   #define TRACE_IF( a, b, c ) ((a) ? (TRACE( (b), (c) )) : (void)0)

#else		// _DEBUG

	#define BREAK()

	// See comments above re replacing MFC's VERIFY().
	#ifdef DEFAULT_VERIFY
		#undef DEFAULT_VERIFY
	#endif
	#define DEFAULT_VERIFY(f)	(f)

	#ifdef DEFAULT_ASSERT
		#undef DEFAULT_ASSERT
	#endif
	#define DEFAULT_ASSERT(f)	((void)0)

  #define TRACE_IF( a, b, c ) ((void)0)

#endif	// !_DEBUG

#endif // __AFX_H__


////////////////////////////////////////////////////////////////////
// Declare default(no log support) ATLTRACE, ATLASSERT macros
////////////////////////////////////////////////////////////////////

#ifdef _DEBUG 
	#ifdef DEFAULT_ATLTRACE
		#undef DEFAULT_ATLTRACE
	#endif
	#define DEFAULT_ATLTRACE  ::AtlTrace
#endif	// !_DEBUG

#ifdef DEFAULT_ATLASSERT
	#undef DEFAULT_ATLASSERT
#endif
#define DEFAULT_ATLASSERT(expr) _ASSERTE(expr)


////////////////////////////////////////////////////////////////////////////////
// Helper class to handle channel based logging of TRACE/ASSERT/VERIFY macros,
// used only for routing of debug macros to the logger. Not for application use.
////////////////////////////////////////////////////////////////////////////////

class CDebugLog : ITTSLogClientAdvise
{
public:

	CDebugLog();
	~CDebugLog();

	// *** IUnknown methods ***
  	STDMETHODIMP_(HRESULT)		QueryInterface( REFIID riid, void** ppvObject );
  	STDMETHODIMP_(ULONG)			AddRef( void );
	STDMETHODIMP_(ULONG)			Release( void );

	// *** ITTSLogClientAdvise methods ***
	STDMETHODIMP_(HRESULT)		OnServerShutdown(DWORD dwStatus);

	// Properties
	BOOL IsTraceActive();
	BOOL IsWarningActive();
	BOOL IsFixmeActive();
	BOOL IsAssertActive();
	ITTSLog* GetLog();
	HRESULT GetLogFileName(LPOLESTR *ppwszName);
	inline BOOL IsReleaseBuildEmulation() { return m_bEmulateReleaseBuild; }
	inline void SetReleaseBuildEmulation( BOOL bEmulateReleaseBuild ) { m_bEmulateReleaseBuild = bEmulateReleaseBuild; }
	inline BOOL IsAssertionPopups() { return m_bAssertionPopup; }
	inline void SetAssertionPopups( BOOL bAssertionPopup ) { m_bAssertionPopup = bAssertionPopup; }
	inline void SetSaveAssertMinidumps( BOOL bSaveAssertMinidumps ) { m_bSaveAssertMinidumps = bSaveAssertMinidumps; }
	inline BOOL GetSaveAssertMinidumps() { return m_bSaveAssertMinidumps; }

	// Implementation
	HRESULT InitServer( LPCOLESTR pwszChanName, LPCTSTR pstrLogAppName = NULL );
	HRESULT ShutdownServer();
	void Trace( LPCSTR lpszFormat, ... );
	void Trace( LPCWSTR pwszFormat, ... );
	void Warn( LPCSTR lpszFormat, ... );
	void Warn( LPCWSTR pwszFormat, ... );
	void Fixme( LPCSTR lpszFormat, ... );
	void Fixme( LPCWSTR lpszFormat, ... );
	void OutputTraceV( LPCSTR lpszFormat, va_list argList, int nLogLevel, BOOL bIsLoggingActive, LPCSTR lpszPrefix  );
	void OutputTraceV( LPCWSTR lpszFormat, va_list argList, int nLogLevel, BOOL bIsLoggingActive, LPCWSTR lpszPrefix  );
	void OutputAssert( LPCSTR lpszExpression, LPCSTR lpszFileName, int nLine );
	BOOL OutputVerify( LPCSTR lpszExpression, LPCSTR lpszFileName, int nLine );
	BOOL OutputFileLineMsg( LPCSTR lpszExpression, LPCSTR lpszFileName, int nLine );
	
	BOOL CheckTooManyAsserts();

private:
	ITTSLogServer*		m_pLogServer;		// log server
	ITTSLog*				m_pLog;				// log interface
	int					m_hLog;				// log handle 
	int					m_hChan;				// the channel we log to
	DWORD					m_nTimerPeriod;	// timer period in MS
	int					m_nMaxAssert;		// max number of assertions permitted in timer period 
	int					m_cAssert;			// count of assertions since last timer period
	DWORD					m_nTickCount;		// tick count at start of last timer period
	DWORD					m_dwRefCount;		// reference count (for bookkeeping only since we are a singleton)
	BOOL					m_bAssertionPopup;// pop's up an assertion message box when true
	BOOL					m_bSaveAssertMinidumps; // allows saving minidumps from assert dialogs
	BOOL					m_bEmulateReleaseBuild;	// when true ASSERT and TRACE statements are ignored
};

#undef THE_DEBUG_LOG

// Declare global default static instance of channel log helper used by TRACE/ASSERT/VERIFY macros
#ifdef CUSTOM_GLOBAL_LOG
	extern CDebugLog CUSTOM_GLOBAL_LOG;

	// Point to the custom debug log
	#define THE_DEBUG_LOG CUSTOM_GLOBAL_LOG
#else
	extern CDebugLog g_theDebugLog;
	
	// Point to the debug log
	#define THE_DEBUG_LOG g_theDebugLog
#endif


////////////////////////////////////////////////////////////////////////////////
// Define logging enabled versions of TRACE/ASSERT/VERIFY macros
// These versions all output by default to the GLOBAL application log channel 
////////////////////////////////////////////////////////////////////////////////

#ifdef __AFX_H__		// define MFC macros only for MFC clients

#if ENABLE_DEBUG_LOGGING

	// Declare logging enabled TRACE macros
	#undef TRACE 
	#define TRACE THE_DEBUG_LOG.Trace

	#undef TRACE_CHANNEL
	#define TRACE_CHANNEL _LOG_OUTPUT_CHANNEL_TRACE

	// Declare logging enabled ASSERT macros
	#undef ASSERT
	#define ASSERT(f) \
	do \
	{ \
		if ( THE_DEBUG_LOG.IsReleaseBuildEmulation() ) \
		{ \
		} \
		else if ( THE_DEBUG_LOG.IsAssertActive() ) \
		{ \
			if ( !(f) ) \
			{ \
				if ( THE_DEBUG_LOG.IsAssertionPopups() ) \
					DEFAULT_ASSERT(f); \
				THE_DEBUG_LOG.OutputAssert( #f, THIS_FILE, __LINE__ ); \
			} \
		} \
		else \
		{ \
			DEFAULT_ASSERT(f); \
		} \
	} while (0)

	// Declare logging enabled VERIFY macro
	#undef VERIFY
	#define VERIFY(f) \
		(	(THE_DEBUG_LOG.IsAssertActive() || THE_DEBUG_LOG.IsReleaseBuildEmulation()) ? \
			( (f) ? TRUE : THE_DEBUG_LOG.OutputVerify( #f, THIS_FILE, __LINE__ ) ) \
			: DEFAULT_VERIFY(f) )

	// If a _DEBUG build, ASSERT/VERIFY; otherwise, simply TRACE
	#undef ASSERTD
	#undef VERIFYD
	#if _DEBUG
		#define ASSERTD ASSERT
		#define VERIFYD VERIFY
	#else
		#define ASSERTD(f) \
		do \
		{ \
			if ( !THE_DEBUG_LOG.IsReleaseBuildEmulation() ) \
			{ \
				if ( !(f) ) \
				{ \
					THE_DEBUG_LOG.OutputFileLineMsg( #f, THIS_FILE, __LINE__ ); \
				} \
			} \
		} while (0)
		#define VERIFYD(f) \
			(	THE_DEBUG_LOG.IsReleaseBuildEmulation() \
				? (f) \
				: ( (f) ? TRUE : THE_DEBUG_LOG.OutputFileLineMsg( #f, THIS_FILE, __LINE__ ) ) )
	#endif

#else // No debug logging 

	void DummyTrace( const char*, ... );
	void DummyTrace( const wchar_t*, ... );

	#undef TRACE
	#define TRACE 1 ? (void)0 : ::DummyTrace

	#undef TRACE_CHANNEL
	#define TRACE_CHANNEL 1 ? (void)0 : _LOG_OUTPUT_CHANNEL_TRACE

	#undef WARN
	#define WARN	1 ? (void)0 : ::DummyTrace

	#undef FIXME
	#define FIXME	1 ? (void)0 : ::DummyTrace

	#undef ASSERT
	#define ASSERT DEFAULT_ASSERT

	#undef VERIFY
	#define VERIFY DEFAULT_VERIFY

	#undef ASSERTD
	#define ASSERTD DEFAULT_ASSERT

	#undef VERIFYD
	#define VERIFYD DEFAULT_VERIFY

#endif // ENABLE_DEBUG_LOGGING

#endif // __AFX_H__


#if ENABLE_DEBUG_LOGGING

#if _MFC_VER < 0x0700
	// Declare logging enabled ATLTRACE macros
	#undef ATLTRACE
	#define ATLTRACE  THE_DEBUG_LOG.Trace
	#undef ATLTRACE2
	#define ATLTRACE2           AtlTrace2
#endif

	#undef WARN
	#define WARN THE_DEBUG_LOG.Warn

	#undef FIXME
	#define FIXME THE_DEBUG_LOG.Fixme
	
	// Declare logging enabled ATLASSERT macros
	#undef ATLASSERT
	#define ATLASSERT(f) \
	do \
	{ \
		if ( THE_DEBUG_LOG.IsReleaseBuildEmulation() ) \
		{ \
		} \
		else if ( THE_DEBUG_LOG.IsAssertActive() ) \
		{ \
			if ( !(f) ) \
			{ \
				if ( THE_DEBUG_LOG.IsAssertionPopups() ) \
					DEFAULT_ATLASSERT(f); \
				THE_DEBUG_LOG.OutputAssert( #f, __FILE__, __LINE__ ); \
			} \
		} \
		else \
		{ \
			DEFAULT_ATLASSERT(f); \
		} \
	} while (0)

#else // No debug logging 

	#ifndef __AFX_H__		// define macros only for non MFC clients
		#undef WARN
		#define WARN	1 ? (void)0 : ::AtlTrace

		#undef FIXME
		#define FIXME	1 ? (void)0 : ::AtlTrace
	#endif 

#endif // ENABLE_DEBUG_LOGGING



// Helpers to simplify usage of TTSLogger 

ITTSLog* STDMETHODCALLTYPE _LOG_CREATE( LPCTSTR lpszPath, int opt = 0 );
STDMETHODIMP _LOG_ADD_CHANNEL( ITTSLog* pl, LPCTSTR lpszChanName, int *phChan );
STDMETHODIMP _LOG_OUTPUT_CHANNELV( ITTSLog* pl, int hChan, int nLevel, LPCSTR fmt, va_list argList );
STDMETHODIMP _LOG_OUTPUT_CHANNELV( ITTSLog* pl, int hChan, int nLevel, LPCWSTR fmt, va_list argList );
STDMETHODIMP _LOG_OUTPUT( ITTSLog* pl, int nLevel, const char* fmt, ... );
STDMETHODIMP _LOG_OUTPUT_CHANNEL( ITTSLog* pl, int hChan, int nLevel, const char* fmt, ... );
STDMETHODIMP _LOG_OUTPUT_CHANNEL_TRACE( int hChan, const char* fmt, ... );

inline HRESULT _LOG_DESTROY( ITTSLog* pl )	
	{ if ( pl ) pl->Release(); return S_OK; }

inline HRESULT _LOG_START( ITTSLog* pl )
	{ return ( pl ) ? pl->Start() : S_OK; }

inline HRESULT _LOG_STOP( ITTSLog* pl )
	{ return ( pl ) ? pl->Stop() : S_OK; }

inline HRESULT _LOG_REMOVE_CHANNEL( ITTSLog* pl, int hChan )
	{ return ( pl ) ? pl->RemoveChannel( hChan ) : S_OK; }

inline HRESULT _LOG_REMOVE_CHANNELS( ITTSLog* pl )
	{ return ( pl ) ? pl->RemoveAllChannels() : S_OK; }

inline HRESULT _LOG_SET_LOGGING_LEVEL( ITTSLog* pl, int hChan, int nLevel )
	{ return ( pl ) ? pl->SetLoggingLevel(hChan, nLevel) : S_OK; }

inline HRESULT _LOG_PAUSE( ITTSLog* pl )
	{ return ( pl ) ? pl->Pause() : S_OK; }


// Debugging macro that outputs the DirectShow error description for an HRESULT 
// It evaluates to nothing in the release build
#ifdef _DEBUG
	#define TRACE_ERRORTEXT(_hr) \
	{ \
			TCHAR szErrMsg[128]; \
			if ( AMGetErrorText( (_hr), szErrMsg, _countof(szErrMsg) ) ) \
				WARN( "%hs(%ld) %s\r\n", THIS_FILE, __LINE__, szErrMsg ); \
	}
#else  // !_DEBUG
	#define TRACE_ERRORTEXT(_hr)	((void)0)
#endif // !_DEBUG


// moved to end so we have trace defined
//////////////////////////////////////////////////////////////////
// Simple sandwich class for Timing Measurements

// create one of these statically and pass it to CPerformanceMetric
class CPerformanceMetricAvg
{
public:
	CPerformanceMetricAvg(int nDumpEvery=100);
	~CPerformanceMetricAvg();
	void Update(TCHAR* pszCaption, double dTime, bool bUseOutputDbg);

private:
	int m_nDumpEvery;
	int m_nCurrent;
	double m_dAccumulatedTime;
};

class CPerformanceMetric
{
public:
	CPerformanceMetric( TCHAR* pszCaption = NULL, double dThreshSec = 0, bool bUseOutputDebugString = false, CPerformanceMetricAvg * pAvg = NULL );
	inline void Start()
	{
		m_bRunning = true;
	
		// Grab the System Count Frequency
		::QueryPerformanceFrequency( &m_liFreq );

		// Get the Starting Count
		::QueryPerformanceCounter( &m_liPre );
	}
	inline void Elapsed()
	{
		m_lParts++;

		if ( !m_bRunning ) // can happen if you call Elapsed 2x in a row.  Doesn't require calling Start/Elapsed/Start/Elapsed.
		{		
			m_liPre.QuadPart = m_liPost.QuadPart;
		}
		m_bRunning = false;
		// Get the Ending Count
		::QueryPerformanceCounter( &m_liPost );

		// Convert to Seconds
		double dSec = (double)((m_liPost.QuadPart - m_liPre.QuadPart)) / m_liFreq.QuadPart;

		// Only trace if it is over our Threshold
		if ( dSec >= m_dThreshold )
		{
			if ( m_pAvg )
				m_pAvg->Update(m_pszCaption, dSec, m_bUseOutputDebugString);
			else
			{
				if ( m_bUseOutputDebugString ) // we may want to see output in full release builds.
				{
					TCHAR chBuf[MAX_PATH] = {NULL};
					if ( m_lParts > 1 )
						_sntprintf(chBuf, MAX_PATH, _T("%s(%d) Took %.8f Sec\n"), m_pszCaption, m_lParts, dSec);
					else
						_sntprintf(chBuf, MAX_PATH, _T("%s Took %.8f Sec\n"), m_pszCaption, dSec);
					OutputDebugString(chBuf);
				}
				#if defined (TRACE)
				else
				{
					
					if ( m_lParts > 1 )
						TRACE( _T("%s(%d) Took %.5f Sec\n"), m_pszCaption, m_lParts, dSec );
					else
						TRACE( _T("%s Took %.5f Sec\n"), m_pszCaption, dSec );					
				}
				#endif
			}
		}
	}

	~CPerformanceMetric();

private:
	TCHAR*				m_pszCaption;
	CPerformanceMetricAvg * m_pAvg;
	double				m_dThreshold;
	bool					m_bUseOutputDebugString;
	bool				m_bRunning;
	long				m_lParts;
	LARGE_INTEGER		m_liPre;
	LARGE_INTEGER		m_liPost;
	LARGE_INTEGER		m_liFreq;
};

#ifdef _DEBUG
#include "float.h" // only need include if we need to dump the fcw
#endif
static void DumpFloatControlWord()
{
	#ifdef _DEBUG
	unsigned int nc87 = _control87(0,0); // read only
			
	// dump what's on now
	OutputDebugString(_T("float control word on bit definitions:\n"));

	// exception masks
	if (nc87 & _EM_INEXACT   )
		OutputDebugString(_T("_EM_INEXACT   \n"));
	if (nc87 & _EM_UNDERFLOW )
		OutputDebugString(_T("_EM_UNDERFLOW \n"));
	if (nc87 & _EM_OVERFLOW  )
		OutputDebugString(_T("_EM_OVERFLOW  \n"));
	if (nc87 & _EM_ZERODIVIDE)
		OutputDebugString(_T("_EM_ZERODIVIDE\n"));
	if (nc87 & _EM_INVALID   )
		OutputDebugString(_T("_EM_INVALID   \n"));
	if (nc87 & _EM_DENORMAL  )
		OutputDebugString(_T("_EM_DENORMAL  \n"));

	// rounding control
	if (nc87 & _RC_CHOP      )
		OutputDebugString(_T("_RC_CHOP      \n"));
	else if (nc87 & _RC_DOWN      )
		OutputDebugString(_T("_RC_DOWN      \n"));
	else if (nc87 & _RC_UP        )
		OutputDebugString(_T("_RC_UP        \n"));
	else 
		OutputDebugString(_T("_RC_NEAR      \n"));
	
	// precision control
	if (nc87 & _PC_24 )
		OutputDebugString(_T("_PC_24 \n"));
	else if (nc87 & _PC_53 )
		OutputDebugString(_T("_PC_53 \n"));
	else
		OutputDebugString(_T("_PC_64 \n"));


	// infinity control
	if (nc87 & _IC_AFFINE    )
		OutputDebugString(_T("_IC_AFFINE    \n"));
	else
		OutputDebugString(_T("_IC_PROJECTIVE\n"));

	// denormal control
	if (nc87 & _DN_SAVE_OPERANDS_FLUSH_RESULTS)
		OutputDebugString(_T("_DN_SAVE_OPERANDS_FLUSH_RESULTS\n"));
	else if (nc87 & _DN_FLUSH_OPERANDS_SAVE_RESULTS)
		OutputDebugString(_T("_DN_FLUSH_OPERANDS_SAVE_RESULTS\n"));
	else if (nc87 & _DN_FLUSH)
		OutputDebugString(_T("_DN_FLUSH\n"));
	else
		OutputDebugString(_T("_DN_SAVE\n"));
	#endif
}


#endif	// #ifndef __TTSDBG_H__
