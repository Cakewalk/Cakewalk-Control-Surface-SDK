//////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// MotorMixQueries.h:	Definitions of objects that ask information
// from the user as if by a dialog.
//////////////////////////////////////////////////////////////////////
#pragma once


enum ELedState {
	LedOn,
	LedOff,
	LedFlash
};

/////////////////////////////////////////////////////////////////////////
// CMMQuery:
//
// Base class for any motor mix state in which we flash the select buttons
// and wait for 
/////////////////////////////////////////////////////////////////////////

class CMMQuery:		public CMultiStateListener,
							public CMidiMsgListener
{
public:
	CMMQuery( int nQueryState, CControlSurface *pSurface );
	~CMMQuery();

	// CMultiStateListener override
	HRESULT OnStateChange( DWORD dwStateID, int nNewState );

protected:

	// return here text to display at index position
	virtual void	getOptionText( int ix, char* pText ) PURE;

	// return here whether to use this button at all
	virtual enum ELedState	getOptionLedState( int ix ) PURE;

	// return count of options
	virtual int		getOptionCount() PURE;

	// a select button was pressed:
	virtual HRESULT onOption( int ix ) PURE;

	// query mode was activated:
	virtual HRESULT onQueryMode( BOOL bActive ) { return S_FALSE; }

	// query state we go to upon escape
	virtual int getEscapeQueryState();

	// query state we go to upon completion of query
	virtual int getFinishQueryState( DWORD dwButtonIx );

	// CMidiMsgListener override
	HRESULT setValue( CMidiMsg *pMsg, DWORD dwVal );
	EValueType getValueType() { return TypeInt; }

	// management of options more than 8
	DWORD		getBaseOption();
	void		setBaseOption( DWORD dwBaseOpt );
	void		incBaseOption();
	void		decBaseOption();
	BOOL		hasMoreLeftOptions();
	BOOL		hasMoreRightOptions();
	char*		getLeftArrowText();
	char*		getRightArrowText();
	void		drawOptions();
	void		flashEscape( bool bFlash );

	// members
	CMidiMsg				m_msgSelectBtns;
	CMidiMsg				m_msgSelectLED;
	CMidiMsg				m_msgRightBtnLED;
	CMidiMsg				m_msgLeftBtnLED;
	CMidiMsg				m_msgRightBtns;
	CMidiMsg				m_msgScribble;
	int					m_nQueryState;
	DWORD					m_dwBaseOption;
};


//----------------------------------------------------------
// A query to turn enable or disable VU metering on the LCD
class CMMMeterMode : public CMMQuery
{
public:
	CMMMeterMode( CControlSurface *pSurface );
	~CMMMeterMode() {}

protected:
	// CMMQuery overrides
	HRESULT	onOption( int ix );
	ELedState getOptionLedState( int ix );
	void		getOptionText( int ix, char* pText );
	int		getOptionCount() { return 2; }
	HRESULT	onQueryMode( BOOL bActive );

	HRESULT	enableMeters( bool bEnable );
};



/////////////////////////////////////////////////////////////////////////
// CMMTransport:
//
// Implements offering the user extended transport operations.
/////////////////////////////////////////////////////////////////////////

class CMMTransport:	public CMMQuery									
{
public:
	CMMTransport( CControlSurface *pSurface );
	~CMMTransport() {}

protected:

	// CMMQuery overrides
	HRESULT	onOption( int ix );
	ELedState getOptionLedState( int ix );
	void		getOptionText( int ix, char* pText );
	int		getOptionCount() { return 8; }
	HRESULT	onQueryMode( BOOL bActive );

	void record();
	void write();
	void setPunchIn();
	void setPunchOut();
	void setLoopIn();
	void setLoopOut();
	void setEnableLoop( BOOL bEnable );
	BOOL isLoopEnabled();
};



/////////////////////////////////////////////////////////////////////////
#define MAX_LOCATE_POINTS		4
/////////////////////////////////////////////////////////////////////////
// CMMLocator:
//
// Implements asking the user the locate point choices.
/////////////////////////////////////////////////////////////////////////

class CMMLocator:		public CMMQuery
{
public:
	CMMLocator( CControlSurface *pSurface );
	~CMMLocator() {}

protected:

	// CMMQuery overrides
	HRESULT	onOption( int ix );
	ELedState	getOptionLedState( int ix );
	void		getOptionText( int ix, char* pText );
	int		getOptionCount() { return 8; }
	HRESULT	onQueryMode( BOOL bActive );

	void setLoc( int nLoc );
	void gotoLoc( int nLoc );

	MFX_TIME				m_LocatePoints[ MAX_LOCATE_POINTS ];
	BOOL					m_bIsLocDefined[ MAX_LOCATE_POINTS ];
};


/////////////////////////////////////////////////////////////////////////
// CMMContainerMode:
//
// Implements asking the user which plugin to edit
/////////////////////////////////////////////////////////////////////////

class CMMContainerMode:		public CMMQuery
{
public:
	CMMContainerMode( CControlSurface *pSurface );
	~CMMContainerMode() {}

protected:

	// CMMQuery overrides
	HRESULT	onOption( int ix );
	ELedState getOptionLedState( int ix );
	void		getOptionText( int ix, char *pText );
	int		getOptionCount();
	HRESULT	onQueryMode( BOOL bActive );
};


/////////////////////////////////////////////////////////////////////////
// CMMChooseInsert:
//
// Implements asking the user which plugin to edit
/////////////////////////////////////////////////////////////////////////

class CMMChooseInsert:		public CMMQuery
{
public:
	CMMChooseInsert( CControlSurface *pSurface );
	~CMMChooseInsert() {}

protected:

	// CMMQuery overrides
	HRESULT	onOption( int ix );
	ELedState getOptionLedState( int ix );
	void		getOptionText( int ix, char *pText );
	int		getOptionCount();
	HRESULT	onQueryMode( BOOL bActive );
};

/////////////////////////////////////////////////////////////////////////