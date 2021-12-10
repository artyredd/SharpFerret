#include "csharp.h"
#include "graphics/window.h"
#include "singine/memory.h"

// Ensures that GLFW is Initilized, otherwise throws an exception
#define EnsureGlfwInitialized() if(IsRuntimeStarted is false) {throw(NotIntializedExceptionGLFW);}

// private methods declarations
static void InitializeStaticMethods(void);
static void Dispose(Window window);

// private variables

/// <summary>
///  tracks whether or not glfw has been initialized, is set to true by StartRuntime and false by StopRuntime
/// </summary>
static bool IsRuntimeStarted = false;


Window CreateWindow(int width, int height, char* title)
{
	if (width < 0)
	{
		fprintf(stderr, "Width provided %i must be greater than or equal to 0"NEWLINE, width);
		throw(InvalidArgumentException);
	}

	if (height < 0)
	{
		fprintf(stderr, "Height provided %i must be greater than or equal to 0"NEWLINE, height);
		throw(InvalidArgumentException);
	}

	if (title is null)
	{
		fprintf(stderr, "Title can not be null"NEWLINE);
	}

	// Start the runtime if we haven't already
	if (IsRuntimeStarted is false)
	{
		StartRuntime();
	}

	Window window = SafeAlloc(sizeof(struct _windowObject));

	window->Title = title;

	window->Dispose = &Dispose;

	window->Transform.x = 0;
	window->Transform.y = 0;

	window->Transform.Width = width;
	window->Transform.Height = height;

	window->Handle = glfwCreateWindow(width, height, title, null, null);

	if (window->Handle is null)
	{
		StopRuntime();
		window->Dispose(window);
		throw(FailedToCreateWindowExceptionGLFW);
	}

	window->Monitor = glfwGetWindowMonitor(window->Handle);

	if (window->Monitor is null)
	{
		window->Monitor = glfwGetPrimaryMonitor();
	}

	if (window->Monitor is null)
	{
		window->Dispose(window);
		StopRuntime();
		throw(FailedToGetWindowMonitorException);
	}

	window->VideoMode = glfwGetVideoMode(window->Monitor);

	if (window->VideoMode is null)
	{
		window->Dispose(window);
		StopRuntime();
		throw(FailedToGetVideoModeException);
	}

	if (width is 0)
	{
		window->Transform.Width = window->VideoMode->width;
	}
	if (height is 0)
	{
		window->Transform.Height = window->VideoMode->height;
	}

	return window;
}

static void Dispose(Window window)
{
	if (window is null)
	{
		return;
	}

	if (window->Handle != null)
	{
		glfwDestroyWindow(window->Handle);
	}

	SafeFree(window);
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

static void SetMode(Window window, const WindowMode mode)
{
	if (window is null)
	{
		fprintf(stderr, "window was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	// check to see if the window is already in the requested mode
	if (window->Mode is mode)
	{
		return;
	}

	int_rect transform = window->Transform;

	if (mode is WindowModes.Windowed)
	{
		// for some reason glfwSetWindowMonitor wont make it windowed if you set the width and height to be the same thing?
		// so i set it to width and height - 1 then to the desired resolution to work around this
		glfwSetWindowMonitor(window->Handle, null, transform.x, transform.y, transform.Width - 1, transform.Height - 1, 0);
		glfwSetWindowMonitor(window->Handle, null, transform.x, transform.y, transform.Width, transform.Height, 0);
	}
	else if (mode is WindowModes.FullScreen)
	{
		glfwSetWindowMonitor(window->Handle, window->Monitor, 0, 0, transform.Width, transform.Height, window->VideoMode->refreshRate);
	}
	else if (mode is WindowModes.BorderlessFullScreen)
	{
		glfwSetWindowMonitor(window->Handle, window->Monitor, 0, 0, window->VideoMode->width, window->VideoMode->height, window->VideoMode->refreshRate);
	}
	else
	{
		fprintf(stderr, "The provided WindowMode %i is out of range [%i - %i]", mode, WindowModes.Min, WindowModes.Max);
		throw(InvalidArgumentException);
	}

	// set the mode to match the new mode
	window->Mode = mode;
}

static void InitializeStaticMethods()
{
	WindowMethods.SetSize = &SetSize;
	WindowMethods.SetMode = &SetMode;
}
#pragma endregion
