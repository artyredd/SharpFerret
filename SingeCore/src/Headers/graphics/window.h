#pragma once

#include "GLFW/glfw3.h"
#include "csharp.h"
#include "imaging.h"
#include "singine/enumerable.h"
#include "cglm/vec2.h"
#include "math/shapes.h"

#ifndef _window_h_
#define _window_h_
#endif // !_window_h_

// The default window starting position
#define DEFAULT_WINDOW_X 100
// The defaault window starting position
#define MINIMUM_WINDOW_Y 100

// The way a window is displayed, 0 = windowed, 1 = BorderlessFullScreen, 2 = FullScreen
typedef int WindowMode;

// The way a window is displayed, 0 = windowed, 1 = BorderlessFullScreen, 2 = FullScreen
static const struct _WindowModes {
	const WindowMode Windowed;
	const WindowMode BorderlessFullScreen;
	const WindowMode FullScreen;
	/// <summary>
	/// The smallest value a WindowMode can be
	/// </summary>
	const WindowMode Min;
	/// <summary>
	/// The largest value a WindowMode can be
	/// </summary>
	const WindowMode Max;
} WindowModes = {
	0,
	1,
	2,
	0,
	2
};

typedef int Hint;

static const struct _Hints {
	const Hint MSAASamples;
	const Hint Resizable;
	const Hint Decorated;
	const Hint Visible;
	const Hint Focused;
	const Hint AlwaysOnTop;
	const Hint Maximized;
	const Hint CenterCursor;
	const Hint FocusOnShow;
	const Hint ScaleToMonitor;
	const Hint RefreshRate;
	const Hint RedBits;
	const Hint GreenBits;
	const Hint BlueBits;
	const Hint AlphaBits;
} WindowHints = {
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

static const struct _ContectHints {
	const Hint API;
	const Hint VersionMajor;
	const Hint VersionMinor;
	const Hint Robustness;
	const Hint ReleaseBehavior;
} ContextHints = {
	GLFW_CONTEXT_CREATION_API,
	GLFW_CONTEXT_VERSION_MAJOR,
	GLFW_CONTEXT_VERSION_MINOR,
	GLFW_CONTEXT_ROBUSTNESS,
	GLFW_CONTEXT_RELEASE_BEHAVIOR
};

static const struct _OpenGLHints {
	const Hint ForwardCompatibility;
	const Hint DebugContext;
	const Hint Profile;
} OpenGLHints = {
	GLFW_OPENGL_FORWARD_COMPAT,
	GLFW_OPENGL_DEBUG_CONTEXT,
	GLFW_OPENGL_PROFILE
};

typedef struct _windowObject* Window;

// stores the state of a window
struct _windowObject {
	/// <summary>
	/// The title of window
	/// </summary>
	char* Title;
	/// <summary>
	/// The tranform of the window
	/// </summary>
	int_rect Transform;
	/// <summary>
	/// The underlying handle for this window
	/// </summary>
	GLFWwindow* Handle;
	/// <summary>
	/// The monitor this window is displayed on
	/// </summary>
	GLFWmonitor* Monitor;
	/// <summary>
	/// The video mode used by this window
	/// </summary>
	const GLFWvidmode* VideoMode;
	/// <summary>
	/// The display mode the window was last set to
	/// </summary>
	WindowMode Mode;
	/// <summary>
	/// The refreshrate of the window
	/// </summary>
	int RefreshRate;
	/// <summary>
	/// Disposes and frees the window
	/// </summary>
	void(*Dispose)(Window);
};

#ifndef _window_methods_
#define _window_methods_
struct _windowMethods {
	/// <summary>
	/// Sets the current graphics context to the provided window
	/// </summary>
	/// <param name=""></param>
	void (*SetCurrent)(const Window);
	/// <summary>
	/// Retrieves the attribute's value of the provided window
	/// </summary>
	int(*GetAttribute)(const Window window, const int attribute);
	/// <summary>
	/// Sets the provided attribute on the provided window
	/// </summary>
	void (*SetAttribute)(Window, const int attribute, const int value);
	/// <summary>
	/// Sets the window icon of the provided window
	/// </summary>
	void (*SetIcon)(Window, const Image);
	/// <summary>
	/// Sets the display mode of the provided window
	/// </summary>
	void (*SetMode)(Window, const WindowMode);
	/// <summary>
	/// Sets the size of the provided window(if it's in windowed) otherwise sets the resultion of the window(if it's fullscreen)
	/// </summary>
	void (*SetSize)(Window, const int width, const int height);
} sWindow;
#endif

/// <summary>
/// Creates a new window with the provided dimensions and title
/// </summary>
/// <param name="width">The width(resolution) the window should be in, use 0 for whatever the width of the screen is</param>
/// <param name="height">The height(resolution) the window should be in, use 0 for whatever the height of the screen is</param>
/// <param name="title">The title of the window</param>
/// <returns></returns>
Window CreateWindow(int width, int height, char* title);

// returns: true if the underlying window system(glfw) is initiliazed
bool RuntimeStarted();
// Starts the GLFW runtime, use RuntimeStarted() to determine if the runtime is currently started
void StartRuntime();
// Stops the GLFW runtime
void StopRuntime();

/// Sets the hint value for the next created window globally
void SetHint(int attribute, int value);

bool ShouldClose(Window window);