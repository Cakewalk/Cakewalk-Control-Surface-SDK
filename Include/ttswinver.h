
// This file should be included in any Stdafx.h (for the PCH file), BEFORE any Windows headers

#if _MSC_VER >= 1000
  #pragma once
#endif // _MSC_VER >= 1000

#ifndef WINVER
  #if _MSC_VER >= 1700		// VS2012 and higher
    #define WINVER 0x0600	// _WIN32_WINNT_VISTA is the base version
  #elif _MSC_VER >= 1400
    #define WINVER 0x0501  // _WIN32_WINNT_WINXP
  #else
    #define WINVER 0x0500	// _WIN32_WINNT_WIN2K
  #endif
#endif

#ifndef _WIN32_WINNT
  #if _MSC_VER >= 1700				// VS2012 and higher
    #define _WIN32_WINNT 0x0600	// _WIN32_WINNT_VISTA
  #elif _MSC_VER >= 1400
    #define _WIN32_WINNT 0x0501  // _WIN32_WINNT_WINXP
  #else
    #define _WIN32_WINNT 0x0500	// _WIN32_WINNT_WIN2K
  #endif
#endif

#ifndef NTDDI_VERSION
  #if _MSC_VER >= 1700				// VS2012 and higher
	 #define NTDDI_VERSION  NTDDI_VISTA	// Windows Vista
  #else
	 #define NTDDI_VERSION  0x05010300 // XP SP3 = NTDDI_WINXPSP3 = 0x05010300  
  #endif
#endif

// Work around bug in .NET 2005 compiling with Vista SDK
#ifndef POINTER_64
#define POINTER_64 __ptr64
#endif
