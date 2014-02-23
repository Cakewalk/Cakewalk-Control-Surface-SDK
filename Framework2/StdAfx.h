/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently
/////////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDAFX_H__AADBCB44_7626_4BE6_9A7B_53D5F2026C52__INCLUDED_)
#define AFX_STDAFX_H__AADBCB44_7626_4BE6_9A7B_53D5F2026C52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT

#include "ttswinver.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED
#define _CRT_SECURE_NO_WARNINGS

#include <afxwin.h>
#include <afxdisp.h>
#include <atlbase.h>
#include <mmsystem.h>
#include <crtdbg.h>

#include <afxctl.h>			 // for COlePropertyPage

#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <vector>

#pragma warning( push )
#pragma warning(disable:4786)
#pragma warning(disable:4100)

#include "CommandIDs.h"
#include "ControlSurface.h"
#include "SfkUtils.h"
#include "SfkMidi.h"
//#include "SfkParams.h"
//#include "SfkStates.h"
#pragma warning( pop )

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AADBCB44_7626_4BE6_9A7B_53D5F2026C52__INCLUDED)
