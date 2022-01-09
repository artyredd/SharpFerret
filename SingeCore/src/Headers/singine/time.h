#pragma once

/// when this is defined the frame time is calculated, otherwise it's not
#define CALCULATE_FRAME_TIME
// when this is defined the limits (lowest, highest) are calculated
#define CALCULATE_FRAME_TIME_LIMITS 

// The smallest value DeltaTime can be, generally no purpose to this
#define DELTA_TIME_MIN 0.0
// The largest value DeltaTime can be, used to prevent time issues on low-end hardware or when the engine's thread is frozen etc..
#define DELTA_TIME_MAX 1.0/15.0 // 15FPS

/// <summary>
/// Returns the total number of frames since the start of the program
/// </summary>
/// <returns></returns>
size_t FrameCount();

/// <summary>
/// Returns the time between the current frame and the last frame, in seconds
/// </summary>
/// <returns></returns>
double DeltaTime();

// Should be called once per frame to update DeltaTime()
void UpdateTime();

/// <summary>
/// Returns the total time since the start of the program in seconds
/// </summary>
/// <returns></returns>
double Time();

// The average time between frames in the last 5 seconds, length can be set with SetFrameTimePollingLength()
double FrameTime();

// Sets the length of time, in seconds, that frametime should be benchmarked
void SetFrameTimePollingLength(double length);

double LowestFrameTime();

double HighestFrameTime();

double AverageFrameTime();