#include "csharp.h"
#include "graphics/window.h"

// Ensures that GLFW is Initilized, otherwise throws an exception
#define EnsureGlfwInitialized() if(IsRuntimeStarted is false) {throw(NotIntializedExceptionGLFW);}

// private methods declarations
static void InitializeStaticMethods(void);

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

	Window window = null;

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

	InitializeStaticMethods();

	IsRuntimeStarted = true;
}

// terminates GLFW and sets Initialized() to false
void StopRuntime()
{
	glfwTerminate();
	IsRuntimeStarted = false;
}
#pragma endregion

#pragma region WindowMethods
static void SetSize(Window window, const size_t width, const size_t height)
{
	throw(NotImplementedException);
}

static void SetMode(Window window, WindowMode mode)
{
	throw(NotImplementedException);
}

static void InitializeStaticMethods()
{
	WindowMethods.SetSize = &SetSize;
	WindowMethods.SetMode = &SetMode;
}
#pragma endregion
