#pragma once
#include "core/csharp.h"

/// when this is defined the frame time is calculated, otherwise it's not
#define CALCULATE_FRAME_TIME
// when this is defined the limits (lowest, highest) are calculated
#define CALCULATE_FRAME_TIME_LIMITS 


struct _Time {
	// The largest value DeltaTime can be, used to prevent time issues on low-end hardware or when the engine's thread is frozen etc..
	double MaxDeltaTime;
	// The smallest value DeltaTime can be, generally no purpose to this
	double MinDeltaTime;
	ulong FrameCount;

	// Returns the total time since the start of the program in seconds
	double (*Time)();

	// Should be called once per frame to update DeltaTime()
	void (*Update)();

	// Returns the time between the current frame and the last frame, in seconds
	double (*DeltaTime)();

	// Various values and methods regarding the calculations and values of aspects of the time system
	struct _statistics {
		double FrameTimePollingLength;

		// Returns the total number of frames since the start of the program
		double (*FrameTime)();

		double (*LowestFrameTime)();

		double (*HighestFrameTime)();

		double (*AverageFrameTime)();
	} Statistics;

};

// Methods and values of the time of the engine
extern struct _Time Time;