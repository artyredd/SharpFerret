#include "csharp.h"
#include "graphics/window.h"

// Ensures that GLFW is Initilized, otherwise throws an exception
#define EnsureGlfwInitialized() if(IsRuntimeStarted is false) {throw(NotIntializedExceptionGLFW);}

// private methods declarations

void StartRuntime(void);

// private variables

/// <summary>
///  tracks whether or not glfw has been initialized, is set to true by StartRuntime and false by StopRuntime
/// </summary>
static bool IsRuntimeStarted = false;


Window CreateWindow(size_t width, size_t height, char* name, WindowMode windowMode)
{
	// Start the runtime if we haven't already
	if (IsRuntimeStarted is false)
	{
		StartRuntime();
	}



	return null;
}

#pragma region Runtime Stop, Start, Started
bool RuntimeStarted()
{
	return IsRuntimeStarted;
}

// Initializes GLFW and returns true, otherwise throws an exception
void StartRuntime(void)
{
	if (glfwInit() != true)
	{
		throw(FailedToInitializeExceptionGLFW);
	}

	IsRuntimeStarted = true;
}

// terminates GLFW and sets Initialized() to false
void StopRuntime()
{
	glfwTerminate();
	IsRuntimeStarted = false;
}
#pragma endregion