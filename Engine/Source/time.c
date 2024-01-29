#include "engine/time.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <float.h>

size_t totalFrames;
double totalTime;

double previousTime;
double deltaTime;

double frameTime;
double frameTimeTimer;
double frameTimeLength = 1.0;
size_t frameTimeFrames;

double lowestFrameTime = DBL_MAX;
double highestFrameTime = DBL_MIN;
double averageFrameTime;

static double TotalTime();;
static double LowestFrameTime();
static double HighestFrameTime();
static double AverageFrameTime();
static void UpdateTime();
static void SetFrameTimePollingLength(double length);
static size_t FrameCount();
static double DeltaTime();
static double FrameTime();

struct _Time Time = {
	.MaxDeltaTime = 1.0 / 0.05,
	.MinDeltaTime = 0,
	.FrameCount = 0,
	.Time = &TotalTime,
	.Update = &UpdateTime,
	.DeltaTime = &DeltaTime,
	.Statistics = {
		.FrameTimePollingLength = 0,
		.FrameTime = &FrameTime,
		.LowestFrameTime = &LowestFrameTime,
		.HighestFrameTime = &HighestFrameTime,
		.AverageFrameTime = &AverageFrameTime
	}
};

static double TotalTime()
{
	return totalTime;
}

static size_t FrameCount()
{
	return totalFrames;
}

static double DeltaTime()
{
	return deltaTime;
}

static double FrameTime()
{
	return frameTime;
}

static double LowestFrameTime()
{
	return lowestFrameTime;
}

static double HighestFrameTime()
{
	return highestFrameTime;
}

static double AverageFrameTime()
{
	return averageFrameTime;
}

static void SetFrameTimePollingLength(double length)
{
	frameTimeLength = length;
}

static void UpdateTime()
{
	// process delta time
	double current = glfwGetTime();

	deltaTime = current - previousTime;

	deltaTime = min(deltaTime, Time.MaxDeltaTime);
	deltaTime = max(deltaTime, Time.MinDeltaTime);

	previousTime = current;

	totalTime += deltaTime;

	++totalFrames;

#ifdef CALCULATE_FRAME_TIME
	// process frame time 
	++frameTimeFrames;

	frameTimeTimer += deltaTime;

	if (frameTimeTimer >= frameTimeLength)
	{
		frameTime = frameTimeTimer / (double)frameTimeFrames;

#ifdef CALCULATE_FRAME_TIME_LIMITS
		if (frameTime > highestFrameTime)
		{
			highestFrameTime = frameTime;
		}
		else if (frameTime < lowestFrameTime)
		{
			lowestFrameTime = frameTime;
		}
		averageFrameTime = totalTime / (double)totalFrames;
#endif
		frameTimeFrames = 0;

		frameTimeTimer = 0.0;
	}
#endif
}