#pragma once

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