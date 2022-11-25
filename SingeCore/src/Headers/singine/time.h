#pragma once

/// when this is defined the frame time is calculated, otherwise it's not
#define CALCULATE_FRAME_TIME
// when this is defined the limits (lowest, highest) are calculated
#define CALCULATE_FRAME_TIME_LIMITS 

// The smallest value DeltaTime can be, generally no purpose to this
#define DELTA_TIME_MIN 0.0
// The largest value DeltaTime can be, used to prevent time issues on low-end hardware or when the engine's thread is frozen etc..
#define DELTA_TIME_MAX 1.0/15.0 // 15FPS

struct _Time {
	double MaxDeltaTime;
	double MinDeltaTime;
	size_t FrameCount;

	double (*Time)();
	void (*Update)();
	double (*DeltaTime)();

	struct _statistics {
		double FrameTimePollingLength;
		double (*FrameTime)();

		double (*LowestFrameTime)();

		double (*HighestFrameTime)();

		double (*AverageFrameTime)();
	} Statistics;
	
};

extern struct _Time Time;