//----------------------------------------------------------
// Surface Transport Time Helpers

#pragma once

#include <math.h>

namespace SurfaceTransportTimeUtils
{

enum JogResolution
{
	JOG_TICKS,
	JOG_BEATS,
	JOG_MEASURES,
	JOG_HOURS,
	JOG_MINUTES,
	JOG_SECONDS,
	JOG_FRAMES
};

////////////////////////////////////////////////////////////////////////////
static void ZeroTime( MFX_TIME& mfxTime )
{
	switch( mfxTime.timeFormat )
	{
	case TF_MBT:
		mfxTime.mbt.nMeas = mfxTime.mbt.nBeat = 1;
		mfxTime.mbt.nTick = 0;
		break;
	case TF_SECONDS:
		mfxTime.dSeconds = 0;
		break;
	case TF_SAMPLES:
		mfxTime.llSamples = 0;
		break;
	case TF_TICKS:
		mfxTime.lTicks = 0;
		break;
	case TF_UTICKS:
		mfxTime.llUTicks = 0;
		break;
	case TF_FRAMES:
	case TF_FRAMES_REL:
		mfxTime.frames.lFrame = 0;
		break;
	case TF_SMPTE:
	case TF_SMPTE_REL:
		mfxTime.smpte.nFrame = mfxTime.smpte.nHour = mfxTime.smpte.nMin = mfxTime.smpte.nSec = 0;
		mfxTime.smpte.nSub400 = 0;
		break;
	}
}

//---------------------------------------------------------------------
// Given a time, a res, and an amount, Alter the time
static void NudgeTimeCursor( MFX_TIME& mfxTime,					// [in][out] Current Time in, Modified time out
							JogResolution eJogResolution,		// Time Res
							int iDir )								// Motion increment
{
	if ( iDir == 0 )	// no change?
		return;

	// Fixup for when the displayed time is (incorrectly?) rounded up
	switch (eJogResolution)
	{
		case JOG_HOURS:
		case JOG_MINUTES:
		case JOG_SECONDS:
			if (iDir > 0)
			{
				double c = ceil(mfxTime.dSeconds);
				double d = c - mfxTime.dSeconds;

				if (d > 0.0 && d < 1e-8)
				{
					mfxTime.dSeconds = c;
				}
			}
			else
			{
				double f = floor(mfxTime.dSeconds);
				double d = mfxTime.dSeconds - f;

				if (d > 0.0 && d < 1e-8)
				{
					mfxTime.dSeconds = f;
				}
			}
			break;
	}


	switch (eJogResolution)
	{
		case JOG_MEASURES:
			mfxTime.mbt.nMeas += iDir;
			mfxTime.mbt.nBeat = 1;
			mfxTime.mbt.nTick = 0;
			break;

		case JOG_BEATS:
			mfxTime.mbt.nBeat += short(iDir);	// oy I don;'t think the host handles rolling over the measure for us!
			mfxTime.mbt.nTick = 0;
			break;

		case JOG_TICKS:
			mfxTime.mbt.nTick += short(iDir);
			break;

		case JOG_HOURS:
			if (iDir > 0)
				mfxTime.dSeconds = 3600 * (floor(mfxTime.dSeconds / 3600) + iDir);
			else
				mfxTime.dSeconds = 3600 * (ceil(mfxTime.dSeconds / 3600) + iDir);
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_MINUTES:
			if (iDir > 0)
				mfxTime.dSeconds = 60 * (floor(mfxTime.dSeconds / 60) + iDir);
			else
				mfxTime.dSeconds = 60 * (ceil(mfxTime.dSeconds / 60) + iDir);
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_SECONDS:
			if (iDir > 0)
				mfxTime.dSeconds = floor(mfxTime.dSeconds) + iDir;
			else
				mfxTime.dSeconds = ceil(mfxTime.dSeconds) + iDir;
			mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			break;

		case JOG_FRAMES:
			{
				// argh this is a bug.  we're modifying the dSeconds field which
				// means we can't possibly be using the .smpte.fps field since they
				// are  unionized together. What we need to do is accept a separate arg
				// for FPS.
				double dFramePeriod = dFramePeriod = 1.0 / 30.0;

				switch (mfxTime.smpte.fps)
				{
					case FPS_24:			dFramePeriod = 1.0 / 24.0;		break;
					case FPS_25:			dFramePeriod = 1.0 / 25.0;		break;
					case FPS_2997:			dFramePeriod = 1.0 / 29.97;		break;
					case FPS_2997_DROP:		dFramePeriod = 1.0 / 29.97;		break;
					case FPS_30:			dFramePeriod = 1.0 / 30.0;		break;
					case FPS_30_DROP:		dFramePeriod = 1.0 / 30.0;		break;
				}

				mfxTime.dSeconds += (double)iDir * dFramePeriod;
				mfxTime.dSeconds = max(0, mfxTime.dSeconds);
			}
			break;

		default:
			return;
	}
}







};