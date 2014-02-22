========================================================================
                    MFX Plugin Wizard: "MackieControl Control Surface"
========================================================================

This file contains information about your new MFX plugin project.  For additional 
information refer to the "MIDI Effects.rtf" included in the SDK.

1) This project contains the code for your "MackieControl Control Surface" plugin.  This plugin
   is a COM object implemented in a Win 32 DLL, and is already minimally functional; 
   this means that all required interfaces have been implemented to a basic level of 
   functionality.  
   
   If you build the project, and then register the server (as discussed in 
   "MIDI Effects.rtf") you will be able to test "MackieControl Control Surface."  Upon doing so you 
   will quickly see that although "MackieControl Control Surface" will indeed run, it has no effect 
   on the MIDI data stream and the user interface is quite minimal -- it is up to you to add 
   functionality to the plugin and its GUI.  To learn the basics of creating an MFX plugin 
   please read "MIDI Effects.rtf."


2) The main files for the "MackieControl Control Surface" project are:

  a) MackieControl.h, MackieControl.cpp:
	These files represent the main source files for your CMackieControl plugin class.
  
  b) MackieControlPropPage.h, MackieControlPropPage.cpp:
	These files are the main source files for your CMackieControlPropPage class -- an 
	MFC CDialog class that implements the plugin's GUI.

  c) MackieControlGen.cpp
	This file contains the implementation for each of the methods that are standard 
	for all MFX plugins.  You should not need to edit this file.

  d) MackieControl.rc
	The resource file for "MackieControl Control Surface".  Includes a list of all resources in 
	the project.  You can use the DevStudio Resource editor to edit this file.
	
  e) Readme.txt
	The file you are currently viewing.  Provides a general description of each of 
	the components of your new project.

  f) StdAfx.h, StdAfx.cpp:
    These files are used to build a precompiled header (PCH) file named MackieControl.pch 
	and a precompiled types file named StdAfx.obj.

  g) Resource.h
    This is the standard header file, which defines new resource IDs.  Visual C++ reads 
	and updates this file.

  h) MackieControl.clw
    This file contains information used by ClassWizard to edit existing classes or add 
	new classes.  ClassWizard also uses this file to store information needed to create 
	and edit message maps and dialog data maps and to create prototype member functions.
