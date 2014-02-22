# Microsoft Developer Studio Project File - Name="MotorMix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MotorMix - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MotorMix.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MotorMix.mak" CFG="MotorMix - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MotorMix - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MotorMix - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Src/ControlSurfaces/MotorMix", FFLDAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MotorMix - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Framework" /I "$(TOOL)\HTML Help Workshop\Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /I "..\Framework" /Oicf
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib "$(TOOL)\HTML Help Workshop\Lib\HtmlHelp.lib" /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\MotorMix.def" /out:"../../bin/Debug/MotorMix.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\Src\bin\Debug\MotorMix.dll
InputPath=\Src\bin\Debug\MotorMix.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MotorMix - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MotorMix___Win32_Release"
# PROP BASE Intermediate_Dir "MotorMix___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\Framework" /I "$(TOOL)\HTML Help Workshop\Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# ADD MTL /I "..\Framework" /Oicf
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib "$(TOOL)\HTML Help Workshop\Lib\HtmlHelp.lib" /nologo /base:"@..\..\include\dllbase.txt,MotorMix" /subsystem:windows /dll /map /machine:I386
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=.\Release\MotorMix.dll
InputPath=.\Release\MotorMix.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "MotorMix - Win32 Debug"
# Name "MotorMix - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Framework\ControlSurface.idl
# ADD MTL /I "."
# End Source File
# Begin Source File

SOURCE=.\ControlSurfacePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\MotorMix.cpp
# End Source File
# Begin Source File

SOURCE=.\MotorMix.def

!IF  "$(CFG)" == "MotorMix - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "MotorMix - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MotorMix.idl
# ADD MTL /I "." /tlb ".\MotorMix.tlb" /h "MotorMix.h" /iid "MotorMix_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\MotorMix.rc
# End Source File
# Begin Source File

SOURCE=.\MotorMixQueries.cpp
# End Source File
# Begin Source File

SOURCE=.\MotorMixStrip.cpp
# End Source File
# Begin Source File

SOURCE=.\MotorMixSubclasses.cpp
# End Source File
# Begin Source File

SOURCE=.\MotorMixSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPage.cpp
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkMidi.cpp
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkParams.cpp
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkStates.cpp
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\Surface.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Framework\CommandIDs.h
# End Source File
# Begin Source File

SOURCE=.\ControlSurfacePropPage.h
# End Source File
# Begin Source File

SOURCE=.\MotorMixQueries.h
# End Source File
# Begin Source File

SOURCE=.\MotorMixStrip.h
# End Source File
# Begin Source File

SOURCE=.\MotorMixSubclasses.h
# End Source File
# Begin Source File

SOURCE=.\MotorMixSurface.h
# End Source File
# Begin Source File

SOURCE=.\PropPage.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkMidi.h
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkParams.h
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkStateDefs.h
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkStates.h
# End Source File
# Begin Source File

SOURCE=..\Framework\SfkUtils.h
# End Source File
# Begin Source File

SOURCE=.\StateDefs.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ControlSurface.rgs
# End Source File
# Begin Source File

SOURCE=.\ControlSurfacePropPage.rgs
# End Source File
# End Group
# End Target
# End Project
