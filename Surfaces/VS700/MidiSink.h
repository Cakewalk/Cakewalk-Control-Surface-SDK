#pragma once

//////////////////////////////////////////////////////////////////////
// IMidiSink: defines the interface for a MIDI input listener.
// Should override OnMidiShortMsg with code to take action on
// MIDI input.
// NOTE: Midi input does not occur on the UI thread and thread considerations
// apply. The implementation must not take too long to return.

interface IMidiSink
{
	virtual BOOL OnMidiShortMsg( DWORD dwMsg, DWORD dwTime, UINT nPort ) = 0;
	virtual BOOL OnMidiLongMsg( DWORD cbLongMsg, const BYTE *pbLongMsg, UINT nPort ) = 0;
	virtual void OnPortsChanged() = 0;
};

