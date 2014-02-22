========================================================================
                    Control Surface Plugin Wizard: "$$friendly_name$$"
========================================================================

This file contains information about your new Control Surface project.

1) This project contains the code for your "$$friendly_name$$" surface.  This surface
   is a COM object implemented in a Win 32 DLL, and is already minimally functional; 
   this means that all required interfaces have been implemented to a basic level of 
   functionality.  
   
   If you build the project, and then register the server you will be able to 
   test "$$friendly_name$$."  Upon doing so you will quickly see that 
   although "$$friendly_name$$" will indeed run, it has no effect 
   on the host and the user interface is quite minimal -- it is up to you to add 
   functionality to the surface and its GUI.  To learn the basics of creating a Control
   Surface, please see additional documentation


2) The main files for the "$$friendly_name$$" project are:

  a) $$root$$.h, $$root$$.cpp:
	These files represent the main source files for your C$$Safe_root$$ surface class.
  
  b) $$root$$PropPage.h, $$root$$PropPage.cpp:
	These files are the main source files for your C$$root$$PropPage class -- an 
	MFC CDialog class that implements the surface's GUI.

  c) $$root$$Gen.cpp
	This file contains the implementation for each of the methods that are standard 
	for all Control Surfaces.  You should not need to edit this file.

  d) $$root$$.rc
	The resource file for "$$friendly_name$$".  Includes a list of all resources in 
	the project.  You can use the DevStudio Resource editor to edit this file.
	
  e) Readme.txt
	The file you are currently viewing.  Provides a general description of each of 
	the components of your new project.

  f) StdAfx.h, StdAfx.cpp:
    These files are used to build a precompiled header (PCH) file named $$root$$.pch 
	and a precompiled types file named StdAfx.obj.

  g) Resource.h
    This is the standard header file, which defines new resource IDs.  Visual C++ reads 
	and updates this file.

  h) $$root$$.clw
    This file contains information used by ClassWizard to edit existing classes or add 
	new classes.  ClassWizard also uses this file to store information needed to create 
	and edit message maps and dialog data maps and to create prototype member functions.
