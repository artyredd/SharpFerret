#include "singine/time.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>

size_t totalFrames;
double totalTime;

double previousTime;
double deltaTime;

double frameTime;
double frameTimeTimer;
double frameTimeLength;
size_t frameTimeFrames;

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

	// process frame time 
	++frameTimeFrames;

	frameTimeTimer += deltaTime;

	if (frameTimeTimer >= frameTimeLength)
	{
		frameTime = frameTimeTimer / (double)frameTimeFrames;

		frameTimeFrames = 0;

		frameTimeTimer = 0.0;
	}
}