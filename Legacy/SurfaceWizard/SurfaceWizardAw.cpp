// SurfaceWizardaw.cpp : implementation file
//

#include "stdafx.h"
#include "SurfaceWizard.h"
#include "SurfaceWizardaw.h"
#include "chooser.h"

#include <atlbase.h>
#include <InitGuid.h>
#include <ObjModel/AppGuid.h>

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This is called immediately after the custom AppWizard is loaded.  Initialize
//  the state of the custom AppWizard here.
void CSurfaceWizardAppWiz::InitCustomAppWiz()
{
	// Create a new dialog chooser; CDialogChooser's constructor initializes
	//  its internal array with pointers to the steps.
	m_pChooser = new CDialogChooser;

	// Set the maximum number of steps.
	SetNumberOfSteps(LAST_DLG);

	// Inform AppWizard that we're making a DLL.
	m_Dictionary[_T("PROJTYPE_DLL")] = _T("1");
	m_Dictionary[_T("MFCDLL")] = _T("1");

	// TODO: Add any other custom AppWizard-wide initialization here.
}

// This is called just before the custom AppWizard is unloaded.
void CSurfaceWizardAppWiz::ExitCustomAppWiz()
{
	// Deallocate memory used for the dialog chooser
	ASSERT(m_pChooser != NULL);
	delete m_pChooser;
	m_pChooser = NULL;

	// TODO: Add code here to deallocate resources used by the custom AppWizard
}

// This is called when the user clicks "Create..." on the New Project dialog
//  or "Next" on one of the custom AppWizard's steps.
CAppWizStepDlg* CSurfaceWizardAppWiz::Next(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Next(pDlg);
}

// This is called when the user clicks "Back" on one of the custom
//  AppWizard's steps.
CAppWizStepDlg* CSurfaceWizardAppWiz::Back(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Back(pDlg);
}

void CSurfaceWizardAppWiz::CustomizeProject(IBuildProject* pProject)
{
	// TODO: Add code here to customize the project.  If you don't wish
	//  to customize project, you may remove this virtual override.
	
	// This is called immediately after the default Debug and Release
	//  configurations have been created for each platform.  You may customize
	//  existing configurations on this project by using the methods
	//  of IBuildProject and IConfiguration such as AddToolSettings,
	//  RemoveToolSettings, and AddCustomBuildStep. These are documented in
	//  the Developer Studio object model documentation.

	// WARNING!!  IBuildProject and all interfaces you can get from it are OLE
	//  COM interfaces.  You must be careful to release all new interfaces
	//  you acquire.  In accordance with the standard rules of COM, you must
	//  NOT release pProject, unless you explicitly AddRef it, since pProject
	//  is passed as an "in" parameter to this function.  See the documentation
	//  on CCustomAppWiz::CustomizeProject for more information.
	IConfigurations* pCfgs;
	if (S_OK == pProject->get_Configurations( &pCfgs ))
	{
		long nNumCfgs;
		if (S_OK == pCfgs->get_Count( &nNumCfgs ))
		{
			VARIANT vReserved;
			vReserved.vt = VT_ERROR;
			vReserved.scode = DISP_E_PARAMNOTFOUND;

			for (long i = 0; i < nNumCfgs; ++i)
			{
				VARIANT				vIndex;
				IConfiguration*	pCfg;
				CComBSTR				bstrFile;
				CComBSTR				bstrParams;

				vIndex.vt = VT_I4;
				vIndex.lVal = i + 1; // one-based!
				if (S_OK == pCfgs->Item( vIndex, &pCfg ))
				{
					CString strSafeRoot;
					m_Dictionary.Lookup(_T("Safe_root"), strSafeRoot);

					// Generate ControlSurface.H from ControlSurface.IDL
					bstrFile = "ControlSurface.idl";
					bstrParams = "/h ControlSurface.h /Oicf /I \".\"";
					pCfg->AddFileSettings( bstrFile, bstrParams, vReserved );

					bstrFile = "midl.exe";
					bstrParams = "/mktyplib203";
					pCfg->RemoveToolSettings( bstrFile, bstrParams, vReserved );

					// Add RTTI, precompiled headers and path to C++ command line
					bstrFile = "cl.exe";
					bstrParams = "/GR /I. /YX\"stdafx.h\"";
					pCfg->AddToolSettings( bstrFile, bstrParams, vReserved );

					// Do some custom things, like register the plug-in
					CComBSTR bstrSteps(
						"regsvr32.exe /s \"$(OutDir)\\$(TargetName).DLL\"\n"
						"echo >custom.bld" );
					CComBSTR bstrOut( "custom.bld" );
					CComBSTR bstrName( "Custom Build Steps" );

					pCfg->AddCustomBuildStep( bstrSteps, bstrOut, bstrName, vReserved );
				}
			}
		}
	}
}


// Here we define one instance of the CSurfaceWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global SurfaceWizardaw.
CSurfaceWizardAppWiz SurfaceWizardaw;

