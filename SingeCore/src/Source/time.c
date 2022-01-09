#include "singine/time.h"
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

double Time()
{
	return totalTime;
}

size_t FrameCount()
{
	return totalFrames;
}

double DeltaTime()
{
	return deltaTime;
}

double FrameTime()
{
	return frameTime;
}

double LowestFrameTime()
{
	return lowestFrameTime;
}

double HighestFrameTime()
{
	return highestFrameTime;
}

double AverageFrameTime()
{
	return averageFrameTime;
}

void SetFrameTimePollingLength(double length)
{
	frameTimeLength = length;
}

void UpdateTime()
{
	// process delta time
	double current = glfwGetTime();

	deltaTime = current - previousTime;

	deltaTime = min(deltaTime, DELTA_TIME_MAX);
	deltaTime = max(deltaTime, DELTA_TIME_MIN);

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