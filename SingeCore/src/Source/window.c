#include "csharp.h"
#include "graphics/window.h"
#include "singine/memory.h"
#include "GLFW/glfw3.h"
#include "singine/guards.h"

const struct _Hints WindowHints = {
	GLFW_SAMPLES,
	GLFW_RESIZABLE,
	GLFW_DECORATED,
	GLFW_VISIBLE,
	GLFW_FOCUSED,
	GLFW_FLOATING,
	GLFW_MAXIMIZED,
	GLFW_CENTER_CURSOR,
	GLFW_FOCUS_ON_SHOW,
	GLFW_SCALE_TO_MONITOR,
	GLFW_REFRESH_RATE,
	GLFW_RED_BITS,
	GLFW_GREEN_BITS,
	GLFW_BLUE_BITS,
	GLFW_ALPHA_BITS
};

const struct _contextHints ContextHints = {
	GLFW_CONTEXT_CREATION_API,
	GLFW_CONTEXT_VERSION_MAJOR,
	GLFW_CONTEXT_VERSION_MINOR,
	GLFW_CONTEXT_ROBUSTNESS,
	GLFW_CONTEXT_RELEASE_BEHAVIOR
};

const struct _OpenGLHints OpenGLHints = {
	GLFW_OPENGL_FORWARD_COMPAT,
	GLFW_OPENGL_DEBUG_CONTEXT,
	GLFW_OPENGL_PROFILE
};


static void SetCurrent(const Window);
static int GetWindowAttribute(const Window window, const int attribute);
static void SetWindowAttribute(Window, const int attribute, const int value);
static void  SetIcon(Window, const Image);
static void  SetMode(Window, const WindowMode);
static void  SetSize(Window, const int width, const int height);
static void  Focus(Window);
static bool RuntimeStarted();
static void StartRuntime(void);
static void StopRuntime();
static bool ShouldClose(Window window);
static Window CreateWindow(int width, int height, char* title);
static void Dispose(Window window);
static void SetClearColor(float r, float g, float b, float a);
static void SetHint(int attribute, int value);

const struct _windowMethods Windows = {
	.SetClearColor = &SetClearColor,
	.SetHint = &SetHint,
	.RuntimeStarted = &RuntimeStarted,
	.Create = &CreateWindow,
	.ShouldClose = &ShouldClose,
	.StopRuntime = &StopRuntime,
	.StartRuntime = &StartRuntime,
	.SetCurrent = &SetCurrent,
	.GetAttribute = &GetWindowAttribute,
	.SetAttribute = &SetWindowAttribute,
	.SetIcon = &SetIcon,
	.SetMode = &SetMode,
	.SetSize = &SetSize,
	.Focus = &Focus,
	.Dispose = &Dispose
};

// Ensures that GLFW is Initilized, otherwise throws an exception
#define EnsureGlfwInitialized() if(IsRuntimeStarted is false) {throw(NotIntializedExceptionGLFW);}

// private methods declarations
static void Dispose(Window window);

// private variables

/// <summary>
///  tracks whether or not glfw has been initialized, is set to true by StartRuntime and false by StopRuntime
/// </summary>
static bool IsRuntimeStarted = false;

static Window CreateWindow(int width, int height, char* title)
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

	GuardNotNull(title);

	// Start the runtime if we haven't already
	if (IsRuntimeStarted is false)
	{
		StartRuntime();
	}

	Window window = SafeAlloc(sizeof(struct _windowObject));

	window->Title = title;

	window->Transform.x = DEFAULT_WINDOW_X;
	window->Transform.y = MINIMUM_WINDOW_Y;

	window->Transform.Width = width;
	window->Transform.Height = height;

	window->Handle = glfwCreateWindow(width, height, title, null, null);

	if (window->Handle is null)
	{
		StopRuntime();
		Windows.Dispose(window);
		throw(FailedToCreateWindowExceptionGLFW);
	}

	window->Monitor = glfwGetWindowMonitor(window->Handle);

	if (window->Monitor is null)
	{
		window->Monitor = glfwGetPrimaryMonitor();
	}

	if (window->Monitor is null)
	{
		Windows.Dispose(window);
		StopRuntime();
		throw(FailedToGetWindowMonitorException);
	}

	window->VideoMode = glfwGetVideoMode(window->Monitor);

	if (window->VideoMode is null)
	{
		Windows.Dispose(window);
		StopRuntime();
		throw(FailedToGetVideoModeException);
	}

	if (width is 0)
	{
		window->Transform.Width = ((GLFWvidmode*)window->VideoMode)->width;
	}
	if (height is 0)
	{
		window->Transform.Height = ((GLFWvidmode*)window->VideoMode)->height;
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

static void SetHint(int attribute, int value)
{
	glfwWindowHint(attribute, value);
}

static void SetClearColor(float r, float g, float b, float a)
{
	EnsureGlfwInitialized();
	glClearColor(r, g, b, a);
}

static bool ShouldClose(Window window)
{
	GuardNotNull(window);
	GuardNotNull(window->Handle);
	return glfwWindowShouldClose(window->Handle);
}

#pragma region Runtime
static bool RuntimeStarted()
{
	return IsRuntimeStarted;
}

// Initializes GLFW and returns true, otherwise throws an exception
static void StartRuntime(void)
{
	if (glfwInit() != true)
	{
		throw(FailedToInitializeExceptionGLFW);
	}

	IsRuntimeStarted = true;
}

// terminates GLFW and sets Initialized() to false
static void StopRuntime()
{
	glfwTerminate();
	IsRuntimeStarted = false;
}
#pragma endregion

#pragma region WindowMethods
static void SetSize(Window window, const int width, const int height)
{
	GuardNotNull(window);

	window->Transform.Width = width;
	window->Transform.Height = height;

	glfwSetWindowSize(window->Handle, (int)width, (int)height);
}

static void SetMode(Window window, const WindowMode mode)
{
	GuardNotNull(window);

	// check to see if the window is already in the requested mode
	if (window->Mode is mode)
	{
		return;
	}

	GLFWvidmode* videoMode = ((GLFWvidmode*)window->VideoMode);

	int_rect transform = window->Transform;

	if (mode is WindowModes.Windowed)
	{
		// remove always on top if it's enabled
		Windows.SetAttribute(window, WindowHints.AlwaysOnTop, false);

		// for some reason glfwSetWindowMonitor wont make it windowed if you set the width and height to be the same thing?
		// so i set it to width and height - 1 then to the desired resolution to work around this
		glfwSetWindowMonitor(window->Handle, null, transform.x, transform.y, transform.Width - 1, transform.Height - 1, 0);
		glfwSetWindowMonitor(window->Handle, null, transform.x, transform.y, transform.Width, transform.Height, 0);
	}
	else if (mode is WindowModes.FullScreen)
	{
		glfwSetWindowMonitor(window->Handle, window->Monitor, 0, 0, transform.Width, transform.Height, videoMode->refreshRate);
	}
	else if (mode is WindowModes.BorderlessFullScreen)
	{
		glfwSetWindowMonitor(window->Handle, window->Monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);

		// set it to always on top
		Windows.SetAttribute(window, WindowHints.AlwaysOnTop, true);
	}
	else
	{
		fprintf(stderr, "The provided WindowMode %i is out of range [%i - %i]", mode, WindowModes.Min, WindowModes.Max);
		throw(InvalidArgumentException);
	}

	// set the mode to match the new mode
	window->Mode = mode;
}

static void SetIcon(Window window, Image image)
{
	GuardNotNull(window);

	GuardNotNull(image);

	// create copy for GLFW
	GLFWimage* copy = SafeAlloc(sizeof(GLFWimage));

	copy->pixels = image->Pixels;

	copy->height = (int)image->Height;
	copy->width = (int)image->Width;

	glfwSetWindowIcon(window->Handle, 1, copy);

	SafeFree(copy);
}

static int GetWindowAttribute(Window window, const int attribute)
{
	GuardNotNull(window);

	GuardNotNull(window->Handle);

	return glfwGetWindowAttrib(window->Handle, attribute);
}

static void SetWindowAttribute(Window window, const int attribute, const int value)
{
	GuardNotNull(window);

	GuardNotNull(window->Handle);

	glfwSetWindowAttrib(window->Handle, attribute, value);
}

static void SetCurrent(Window window)
{
	GuardNotNull(window);
	GuardNotNull(window->Handle);

	glfwMakeContextCurrent(window->Handle);
}

static void Focus(Window window)
{
	glfwFocusWindow(window->Handle);
}
#pragma endregion

#undef EnsureGlfwInitialized