/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// StateDefs.h:	Definitions of ID's for all the state shifters in the
// support DLL, as well as the possible values those state shifters
// can assume.
/////////////////////////////////////////////////////////////////////////

#pragma once

#include "SfkStateDefs.h"

enum EMotorMixStateIDs
{
	// MotorMix specific states
	stBankShiftMode = SURFACE_STATE_ID_BASE,
	stBurnButtonsChoice,
	stMultiButtonsChoice,
	stShiftKey,
	stTrackRotaryMapping,
	stBusRotaryMapping,
	stSendPanOrLevel,
	stQueryState,
	stInsertEditMode,
	stFineTweakMode,
	stMeterMode,
};

// possible states for StateShifter ID stBankShiftMode
enum EBankShiftMode
{
	smByBanks = 0,
	smOneByOne
};

// possible states for StateShifter ID stBurnButtonsChoice
enum EBurnButtonsChoice
{
	bbRecRdy = 0,
	bbWrite,
	bbOther,
};

// possible states for StateShifter ID stMultiButtonsChoice
enum EMultiButtonsChoice
{
	mbFxBypassE1 = 0,
	mbSMuteE2,
	mbPrePostE3,
	mbSelectE4,
};

// possible states for StateShifter ID stShiftKey
enum EShiftKey
{
	skShiftIn = 0,
	skShiftOut,
};

// possible states for StateShifter ID stTrackRotaryMapping
enum ETrackRotaryMapping
{
	trPan = 0,
	trSendBase
};

// possible states for StateShifter ID stBusRotaryMapping
enum EBusRotaryMapping
{
	buPan = 0,
	buInputPan,
	buSendBase
};

// possible states for StateShifter ID stSendPanOrLevel
enum ESendPanOrLevel
{
	sendPan = 0,
	sendLevel,
};


// possible states for StateShifter ID stQueryState
enum EQueryState
{
	qsIdle = 0,
	qsTransport,
	qsLocator,
	qsChooseInsert,
	qsContainerMode,
	qsMeterMode,
};

// possible states for StateShifter ID stInsertEditMode
enum EInsertEditMode
{
	ieDisengaged = 0,
	ieEngaged,
};

// possible states for StateShifter ID stFineTweakMode
enum EFineTweakMode
{
	ftDisengaged = 0,
	ftEngagedTimed,
	ftEngaged,
};


enum EMeterEnable
{
	meMeterOff = 0,
	meMeterOn
};
/////////////////////////////////////////////////////////////////////////