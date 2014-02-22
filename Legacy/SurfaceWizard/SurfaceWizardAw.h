#if !defined(AFX_SURFACEWIZARDAW_H__4BB3F991_2B8B_11D2_9A70_00AA00A70355__INCLUDED_)
#define AFX_SURFACEWIZARDAW_H__4BB3F991_2B8B_11D2_9A70_00AA00A70355__INCLUDED_

// SurfaceWizardaw.h : header file
//

class CDialogChooser;

// All function calls made by mfcapwz.dll to this custom AppWizard (except for
//  GetCustomAppWizClass-- see SurfaceWizard.cpp) are through this class.  You may
//  choose to override more of the CCustomAppWiz virtual functions here to
//  further specialize the behavior of this custom AppWizard.
class CSurfaceWizardAppWiz : public CCustomAppWiz
{
public:
	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
	virtual CAppWizStepDlg* Back(CAppWizStepDlg* pDlg);
		
	virtual void InitCustomAppWiz();
	virtual void ExitCustomAppWiz();
	virtual void CustomizeProject(IBuildProject* pProject);

protected:
	CDialogChooser* m_pChooser;
};

// This declares the one instance of the CSurfaceWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global SurfaceWizardaw.  (Its definition is in SurfaceWizardaw.cpp.)
extern CSurfaceWizardAppWiz SurfaceWizardaw;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURFACEWIZARDAW_H__4BB3F991_2B8B_11D2_9A70_00AA00A70355__INCLUDED_)
