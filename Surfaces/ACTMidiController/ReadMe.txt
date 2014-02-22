========================================================================
                    Control Surface Plugin Wizard: "Generic Control Surface"
========================================================================

This file contains information about your new Control Surface project.

1) This project contains the code for your "Generic Control Surface" surface.  This surface
   is a COM object implemented in a Win 32 DLL, and is already minimally functional; 
   this means that all required interfaces have been implemented to a basic level of 
   functionality.  
   
   If you build the project, and then register the server you will be able to 
   test "Generic Control Surface."  Upon doing so you will quickly see that 
   although "Generic Control Surface" will indeed run, it has no effect 
   on the host and the user interface is quite minimal -- it is up to you to add 
   functionality to the surface and its GUI.  To learn the basics of creating a Control
   Surface, please see additional documentation


2) The main files for the "Generic Control Surface" project are:

  a) ACTController.h, ACTController.cpp:
	These files represent the main source files for your CACTController surface class.
  
  b) ACTControllerPropPage.h, ACTControllerPropPage.cpp:
	These files are the main source files for your CACTControllerPropPage class -- an 
	MFC CDialog class that implements the surface's GUI.

  c) ACTControllerGen.cpp
	This file contains the implementation for each of the methods that are standard 
	for all Control Surfaces.  You should not need to edit this file.

  d) ACTController.rc
	The resource file for "Generic Control Surface".  Includes a list of all resources in 
	the project.  You can use the DevStudio Resource editor to edit this file.
	
  e) Readme.txt
	The file you are currently viewing.  Provides a general description of each of 
	the components of your new project.

  f) StdAfx.h, StdAfx.cpp:
    These files are used to build a precompiled header (PCH) file named ACTController.pch 
	and a precompiled types file named StdAfx.obj.

  g) Resource.h
    This is the standard header file, which defines new resource IDs.  Visual C++ reads 
	and updates this file.

  h) ACTController.clw
    This file contains information used by ClassWizard to edit existing classes or add 
	new classes.  ClassWizard also uses this file to store information needed to create 
	and edit message maps and dialog data maps and to create prototype member functions.
