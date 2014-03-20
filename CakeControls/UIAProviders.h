#pragma once

#include <UIAutomationCore.h>
#include <UIAutomationCoreAPI.h>
#include "NWControl.h"

class CNWControlUIAProvider;

// Various identifiers that have to be looked up.
typedef struct UiaIdentifiers
{
    PROPERTYID    LocalizedControlTypeProperty;
    PROPERTYID    AutomationIdProperty;
    PROPERTYID    NameProperty;
    PROPERTYID    ControlTypeProperty;
    PROPERTYID    HasKeyboardFocusProperty;
    PROPERTYID    IsControlElementProperty;
    PROPERTYID    IsContentElementProperty;
    PROPERTYID    IsKeyboardFocusableProperty;
	 PROPERTYID		HelpTextProperty;
	 PROPERTYID		IsEnabledProperty;
	 PROPERTYID		ValueProperty;
	 PATTERNID		TogglePattern;
	 PATTERNID		RangeValuePattern;
	 PATTERNID		ValuePattern;
	 PATTERNID		InvokePattern;
	 CONTROLTYPEID	ButtonControlType;
	 CONTROLTYPEID SliderControlType;
	 EVENTID			AutomationFocusChangedEvent;
	 EVENTID			PropertyChangedEvent;
} UiaIds;


class CNWControlSiteUIAProvider : public IRawElementProviderSimple, 
    public IRawElementProviderFragment, 
    public IRawElementProviderFragmentRoot,
	 public IRawElementProviderAdviseEvents
{
public:

    // Constructor/destructor.
    CNWControlSiteUIAProvider(CNWControlSite* pCtrlSite);

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

    // IRawElementProviderSimple methods
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions * pRetVal);
    IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
    IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );
    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple ** pRetVal );

    // IRawElementProviderFragment methods
    IFACEMETHODIMP Navigate(NavigateDirection direction, IRawElementProviderFragment ** pRetVal );
    IFACEMETHODIMP GetRuntimeId(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP get_BoundingRectangle(UiaRect * pRetVal );
    IFACEMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY ** pRetVal );
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP get_FragmentRoot( IRawElementProviderFragmentRoot * * pRetVal );

    // IRawElementProviderFragmenRoot methods
    IFACEMETHODIMP ElementProviderFromPoint(double x, double y, IRawElementProviderFragment ** pRetVal );
    IFACEMETHODIMP GetFocus(IRawElementProviderFragment ** pRetVal );

	 // IRawElementProviderAdviseEvents methods
	 IFACEMETHODIMP AdviseEventAdded( EVENTID eventId, SAFEARRAY *propertyIDs );
	 IFACEMETHODIMP AdviseEventRemoved( EVENTID eventId, SAFEARRAY *propertyIDs );
	 
    // Various methods.
    void InitIds();
    BOOL AnyFocusEventListeners();
	 BOOL AnyPropChangedEventListeners( PROPERTYID idProp );
	 void OnCtrlSiteDestroyed() { m_pCtrlSite = NULL; }

protected:
	virtual ~CNWControlSiteUIAProvider();

private:
	// Ref counter for this COM object.
	long m_refCount;

	struct UiaEventProps
	{
		UiaEventProps() : idEvent(0), idProp(0)
		{}

		EVENTID		idEvent;
		PROPERTYID	idProp;

		bool operator < ( const UiaEventProps &props) const
		{
			if (idEvent == props.idEvent)
				return ( idProp < props.idProp);
			else
				return (idEvent < props.idEvent);
		}
	};

	std::multiset<UiaEventProps> m_setEventListeners;

	// Parent control.
	HWND m_controlHwnd;
	CNWControlSite* m_pCtrlSite;
};

class CNWControlUIAProvider : public IRawElementProviderSimple, 
    public IRawElementProviderFragment,
	 public IValueProvider
{
public:

	// Constructor / destructor
	CNWControlUIAProvider(CNWControl* pCtrl, CWnd* pMainFrm); 

	// IUnknown methods
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

	// IRawElementProviderSimple methods
	IFACEMETHODIMP get_ProviderOptions(ProviderOptions * pRetVal);
	virtual IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
	virtual IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );
	IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple ** pRetVal );

	// IRawElementProviderFragment methods
	IFACEMETHODIMP Navigate(NavigateDirection direction, IRawElementProviderFragment ** pRetVal );
	IFACEMETHODIMP GetRuntimeId(SAFEARRAY ** pRetVal );
	IFACEMETHODIMP get_BoundingRectangle(UiaRect * pRetVal );
	IFACEMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY ** pRetVal );
	IFACEMETHODIMP SetFocus();
	IFACEMETHODIMP get_FragmentRoot( IRawElementProviderFragmentRoot * * pRetVal );

	// IValueProvider methods
	IFACEMETHODIMP SetValue( LPCWSTR val );
	IFACEMETHODIMP get_IsReadOnly( BOOL *pRetVal );
	IFACEMETHODIMP get_Value( BSTR *pRetVal );

    // Various methods
	void NotifyFocusChanged();
	void NotifyValueChanged( LPCTSTR strOld, LPCTSTR strNew );
	void OnCtrlDestroyed() { m_pCtrl = NULL; m_pCtrlSite = NULL; }

protected:
	virtual ~CNWControlUIAProvider();

	// Ref Counter for this COM object
	long m_refCount;

	// Pointers to the owning NWControl and control site.
	CNWControl*			m_pCtrl;
	CNWControlSite*	m_pCtrlSite;
	CWnd*					m_pMainFrm;
};

class CNWToggleControlUIAProvider : public CNWControlUIAProvider,	
	public IToggleProvider
{
public:
	CNWToggleControlUIAProvider(CNWControl* pCtrl, CWnd* pMainFrm);

	// IUnknown methods
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

	// IRawElementProviderSimple overrides
	IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
	IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );

	// IToggleProvider methods
	IFACEMETHODIMP Toggle();
	IFACEMETHODIMP get_ToggleState(ToggleState* pRetVal);

private:
	virtual ~CNWToggleControlUIAProvider();
};

class CNWSliderControlUIAProvider : public CNWControlUIAProvider
{
public:
	CNWSliderControlUIAProvider (CNWControl* pCtrl, CWnd* pMainFrm);

	// IUnknown methods
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

	// IRawElementProviderSimple overrides
	IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
	IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );

	// Various methods
	void SetRangeValue( double dVal );

private:

	// IRangeValueProvider is implemented in an inner class to avoid conlicts with IValueProvider's
	// get_IsReadOnly() method
	class CRangeValueProviderImp : public IRangeValueProvider
	{
	public:
		CRangeValueProviderImp( CNWSliderControlUIAProvider* pProvider ) : 
			m_pProvider(pProvider)
		{};

		// IUnknown methods
		IFACEMETHODIMP_(ULONG) AddRef();
		IFACEMETHODIMP_(ULONG) Release();
		IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

		// IRangeValueProvider methods
		IFACEMETHODIMP SetValue( double val );
		IFACEMETHODIMP get_IsReadOnly( BOOL *pRetVal );
		IFACEMETHODIMP get_LargeChange( double *pRetVal );
		IFACEMETHODIMP get_SmallChange( double *pRetVal );
		IFACEMETHODIMP get_Maximum( double *pRetVal );
		IFACEMETHODIMP get_Minimum( double *pRetVal );
		IFACEMETHODIMP get_Value( double *pRetVal );

	protected:
		CNWSliderControlUIAProvider* m_pProvider;
	};
	
	virtual ~CNWSliderControlUIAProvider();

	CRangeValueProviderImp* m_pRgValImp;
};

struct UIAProviderCallbackInfo
{
	CNWSliderControlUIAProvider*	pProvider;
	double								dVal;
};

class CNWDropdownControlUIAProvider : public CNWControlUIAProvider,	
	public IInvokeProvider
{
public:
	CNWDropdownControlUIAProvider(CNWControl* pCtrl, CWnd* pMainFrm);

	// IUnknown methods
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
	IFACEMETHODIMP QueryInterface(REFIID riid, void**ppInterface);

	// IRawElementProviderSimple overrides
	IFACEMETHODIMP GetPatternProvider(PATTERNID iid,IUnknown * * pRetVal );
	IFACEMETHODIMP GetPropertyValue(PROPERTYID idProp,VARIANT * pRetVal );

	// IInvokeProvider methods
	IFACEMETHODIMP Invoke();

private:
	virtual ~CNWDropdownControlUIAProvider();
};