/////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001- Twelve Tone Systems, Inc.  All rights reserved.
//
// StateDefs.h:	Definitions of ID's for all the state shifters in the
// support DLL, as well as the possible values those state shifters
// can assume.
/////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////
//	State IDs
//
// The enum EStates is a list of enum IDs for all the states in use.
// Make sure each state is assigned a UNIQUE ID and that once the code
// has reached the general public (that is, files have been saved with it)
// the state IDs do not change, as they are also the state's
// persistance chunk ID. Rather, new state ID's should be added at the end.
// Likewise, the numerical meaning of each state should not change.
// If new states are needed, they should be added at the end.
/////////////////////////////////////////////////////////////////////////

enum EStateIDs
{
	stNoCondition = SHIFTER_NO_CONDITION,	// reserved as an unmapped ID

	// State ID for the state shifter that governs which kind of container
	// is being looked at.
	stContainerClass,

	// State IDs for use with instances of CSonarContainerBankShifter
	stBaseTrack,
	stCurrentTrack,
	stBaseBus,
	stCurrentBus,
	stBaseMain,
	stCurrentMain,
	stBaseEffect,
	stCurrentEffect,
	stBaseEffectParam,
	stCurrentEffectParam,

	// NOTE: You may need to have more than one bank in the module.
	// An example of this would be a surface that is somehow split in
	// two, and each half could move around a certain kind of container
	// (i.e. one half is tracks, the other is mains)
	//
	// The bank related components allow identifying them by a number:
	// WORD wBankID.
	//
	// Notice that by default, any method that takes a bank ID assumes
	// a value of zero. So you only need to be concerned about this if 
	// your surface has multiple banks.
	//
	// If your surface has multiple banks, declare an enum containing 
	// identifiers for the various banks. Example:
	//
	//		enum EBankIDs
	//		{
	//			bidLeftHandBank,
	//			bidRightHandBank
	//		};
	//
	// Then, do the following:
	// When creating the CStateShifter for container class, instead
	// of using as an ID stContainerClass use MAKELONG( stContainerClass, bidXXX )
	// Example:
	//
	// m_pssContainerClass = new CStateShifter(
	//			m_pSurface,
	//			MAKELONG( stContainerClass, bidOneOfTheBanks ),
	//			ccTracks
	//		);
	//
	// When creating any CSonarContainerBankShifters pass in the
	// state ID as usual, and also pass in
	// the bank ID as the last parameter:
	// Example:
	//
	// m_pssBaseTrack = new CSonarContainerBankShifter(
	//			m_pSurface,
	//			stBaseTrack,
	//			BANK_WIDTH,
	//			bidOneOfTheBanks
	//		);
	//
	// When creating the CMixStripBank pass in the bank ID as the
	// last parameter.
	// Example:
	//
	// m_pTheBank = new CMixStripBank<CFooStrip>(
	//			BANK_WIDTH,
	//			m_pSurface,
	//			bidOneOfTheBanks
	//		);
	//	
	// Repeat this for each set of components associated with each bank ID.
	//
	// NOTE: keep in mind that when using multiple banks,
	// you may not define state ID's exeeding a number of
	// 0x0000FFFF. (65535) since the bank ID will be packed
	// at the most significant 16 bits of the state ID's.
	// This should not be a limitation.
	//
	SURFACE_STATE_ID_BASE
};

// possible states for StateShifter ID stContainerClass
enum EContainerClass
{
	ccTracks = 0,
	ccBus,
	ccMains,
};

/////////////////////////////////////////////////////////////////////////