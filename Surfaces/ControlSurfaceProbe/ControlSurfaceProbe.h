// ControlSurfaceProbe.h : main header file for the ControlSurfaceProbe DLL
//

#if !defined(AFX_ControlSurfaceProbe_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
#define AFX_ControlSurfaceProbe_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <vector>

extern const GUID CLSID_ControlSurfaceProbe;
extern const GUID CLSID_ControlSurfaceProbePropPage;
extern const GUID LIBID_ControlSurfaceProbe;

/////////////////////////////////////////////////////////////////////////////
// Friendly name of component

#ifdef _DEBUG
static const char s_szFriendlyName[] = "Control Surface Probe (DEBUG)";
static const char s_szFriendlyNamePropPage[] = "Control Surface Probe Property Page (DEBUG)";
#else
static const char s_szFriendlyName[] = "Control Surface Probe";
static const char s_szFriendlyNamePropPage[] = "Control Surface Probe Property Page";
#endif

/////////////////////////////////////////////////////////////////////////////

#define snprintf	_snprintf


#define SAFE_RELEASE( _p ) {if (_p) _p->Release(); _p=NULL;}

/////////////////////////////////////////////////////////////////////////////

extern long g_lComponents;	// Count of active components
extern long g_lServerLocks;	// Count of locks

/////////////////////////////////////////////////////////////////////////////
// CControlSurfaceProbe

// Forward ref
class CControlSurfaceProbePropPage;

class CControlSurfaceProbe :
	public IControlSurface,
	public ISurfaceParamMapping,
	public IPersistStream,
	public ISpecifyPropertyPages
{
public:
	// Ctors
	CControlSurfaceProbe();
	virtual ~CControlSurfaceProbe();

	// *** IUnknown methods ***
	// Handled in ControlSurfaceProbeGen.cpp
	STDMETHODIMP_(HRESULT)	QueryInterface( REFIID riid, void** ppv );
  	STDMETHODIMP_(ULONG)		AddRef();
	STDMETHODIMP_(ULONG)		Release();

	// *** IControlSurface methods ***
	// You need to implement these methods when creating "ControlSurfaceProbe"
	// Note: Skeleton methods are provided in ControlSurfaceProbe.cpp.
	// Basic dev caps
	STDMETHODIMP GetStatusText( LPSTR pszStatus, DWORD* pdwLen );
	STDMETHODIMP MidiInShortMsg( DWORD dwShortMsg );
	STDMETHODIMP MidiInLongMsg( DWORD cbLongMsg, const BYTE* pbLongMsg );
	STDMETHODIMP RefreshSurface( DWORD fdwRefresh, DWORD dwCookie );
	STDMETHODIMP GetNoEchoMask( WORD* pwMask, BOOL* pbNoEchoSysx );
	// (ControlSurfaceProbeGen.cpp)
	STDMETHODIMP Connect( IUnknown* pUnknown, HWND hwndApp );
	STDMETHODIMP Disconnect();


	// *** ISurfaceParamMapping ***
	STDMETHODIMP GetStripRangeCount( DWORD* pdwCount );
	STDMETHODIMP GetStripRange( DWORD dwIndex, DWORD* pdwLowStrip, DWORD* pdwHighStrip, SONAR_MIXER_STRIP* pmixerStrip );
	STDMETHODIMP SetStripRange( DWORD dwLowStrip, SONAR_MIXER_STRIP mixerStrip );
	// access to mapped controls
	STDMETHODIMP GetDynamicControlCount( DWORD* pdwCount );
	STDMETHODIMP GetDynamicControlInfo( DWORD dwIndex, DWORD* pdwKey, SURFACE_CONTROL_TYPE* pcontrolType );
	STDMETHODIMP SetLearnState(BOOL);


	// *** IPersistStream methods ***
	STDMETHODIMP Load( IStream* pStm );
	STDMETHODIMP Save( IStream* pStm, BOOL bClearDirty );
	STDMETHODIMP GetSizeMax( ULARGE_INTEGER* pcbSize );
	// (ControlSurfaceProbeGen.cpp)
	STDMETHODIMP GetClassID( CLSID* pClsid );
	STDMETHODIMP IsDirty();

	// *** ISpecifyPropertyPages methods (ControlSurfaceProbeGen.cpp) ***
	STDMETHODIMP GetPages( CAUUID* pPages );

	// *** IDispatch methods (ControlSurfaceProbeGen.cpp) ***
	STDMETHODIMP GetTypeInfoCount( UINT* );
	STDMETHODIMP GetTypeInfo( UINT, LCID, ITypeInfo** );
	STDMETHODIMP GetIDsOfNames( REFIID, OLECHAR**, UINT, LCID, DISPID* );
	STDMETHODIMP Invoke( DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT* );
	
// Attributes
	// TODO:  Declare methods which deal with setting or retreiving values here.

	SONAR_MIXER_STRIP GetMixerStrip();
	void SetMixerStrip(SONAR_MIXER_STRIP eMixerStrip);
	DWORD GetStripNum();
	void SetStripNum(DWORD dwStripNum);
	SONAR_MIXER_PARAM GetMixerParam();
	void SetMixerParam(SONAR_MIXER_PARAM eMixerParam);
	DWORD GetParamNum();
	void SetParamNum(DWORD dwParamNum);
	bool HasMapping();
	void GetMappingName( CString* pstr );
	void GetUIContext( CString* pstr );

	bool	GetContextLocked();
	void	SetContextLocked( bool b );

	bool	GetLearnEnabled();
	void	SetLearnEnabled( bool b );

	DWORD GetSurfaceID() const { return m_dwSurfaceID; }

	void GetUpdateCount(CString *str);
	void GetStripCount(SONAR_MIXER_STRIP eMixerStrip, CString *str);
	void GetStripName(CString *str);
	
	void GetValueLabel(CString *str);
	void GetValueLabel(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, CString *str);

	void GetValue(CString *str);
	void GetValue(SONAR_MIXER_STRIP eMixerStrip, DWORD dwStripNum, SONAR_MIXER_PARAM eMixerParam, DWORD dwParamNum, float* fValue, CString *str);
	
	void GetValueText(CString *str);
	void GetValueText(SONAR_MIXER_STRIP, DWORD, SONAR_MIXER_PARAM, DWORD, float,CString *str);

	void GetArm(CString *str);
	void GetNumMeters(CString *str);
	void GetMetersValues(CString *str);
	void GetTransportState(SONAR_TRANSPORT_STATE eTransportState, CString *str);
	void SetTransportState(SONAR_TRANSPORT_STATE eTransportState, bool bEnable);
	DWORD GetCommandCount();
	DWORD GetCommandInfo(DWORD n, CString *str);
	void DoCommand(DWORD dwCmdId);
	void GetUniqueID(CString *str);
	void GetHostVersion(CString *str);
	void GetHostName(CString *str);
	void PopupCapabilities();
	void GetMarkerCount(CString *str);
	void GetMarkerList(CString *str);
	void GetMarkerIndexForTime(MFX_TIME *pTime, CString *str);
	void GetMarkerByIndex(DWORD dwIndex, CString *str);

	void SetValue(float fValue);
	void SetValue( SONAR_MIXER_STRIP, DWORD, SONAR_MIXER_PARAM, DWORD, float );
	void WriteEnable( bool b );
	void ReadEnable( bool b );
	bool GetWriteEnable();
	bool GetReadEnable();

   void GetWindowState(WindowType wt, WindowState *pws);
   void SetWindowState(WindowType wt, WindowState ws);
   void DoWindowAction(WindowType wt, WindowAction wa, EMoveOperator wo);

protected:
	ISonarMidiOut*				m_pMidiOut;
	ISonarKeyboard*			m_pKeyboard;
	ISonarCommands*			m_pCommands;
	ISonarProject*				m_pProject;
	ISonarProject2*			m_pProject2;
	ISonarMixer*				m_pMixer;
	ISonarMixer2*				m_pMixer2;
	ISonarTransport*			m_pTransport;
	ISonarIdentity*			m_pIdentity;
	ISonarVUMeters*			m_pVUMeters;
	ISonarParamMapping*		m_pSonarParamMapping;
   IHostWindow*            m_pHostWindow;

	HWND							m_hwndApp;
	DWORD							m_dwSurfaceID;
	
	void releaseSonarInterfaces();

// Implementation
private:
	LONG				m_cRef;
	BOOL				m_bDirty;
	CRITICAL_SECTION	m_cs;		// critical section to guard properties

	SONAR_MIXER_STRIP m_eMixerStrip;
	DWORD m_dwStripNum;
	SONAR_MIXER_PARAM m_eMixerParam;
	DWORD m_dwParamNum;

	float m_fLastValue;

	DWORD m_dwUpdateCount;

	struct DYNCONTROL
	{
		DYNCONTROL( DWORD dw, SURFACE_CONTROL_TYPE sct ) : dwKey(dw), sctType(sct){}
		DWORD dwKey;
		SURFACE_CONTROL_TYPE sctType;
	};

	std::vector<DYNCONTROL>	m_vDynControls;
};


/////////////////////////////////////////////////////////////////////////////
// CCriticalSectionAuto

class CCriticalSectionAuto
{
public:
	CCriticalSectionAuto( CRITICAL_SECTION* pcs ) : m_pcs(pcs)
	{
		::EnterCriticalSection( m_pcs );
	}
	virtual ~CCriticalSectionAuto()
	{
		::LeaveCriticalSection( m_pcs );
	}
private:
	CRITICAL_SECTION* m_pcs;
};

/////////////////////////////////////////////////////////////////////////////
// CControlSurfaceProbeApp
// See ControlSurfaceProbeGen.cpp for the implementation of this class
// This class should need to be modified when creating your plugin
//

class CControlSurfaceProbeApp : public CWinApp
{
public:
	CControlSurfaceProbeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlSurfaceProbeApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CControlSurfaceProbeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CControlSurfaceProbeApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ControlSurfaceProbe_H__00EDAF29_0203_11D2_AFFF_00A0C90DA071__INCLUDED_)
