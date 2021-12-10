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

typedef struct _windowObject* Window;

// stores the state of a window
struct _windowObject {
	char* Title;
	int_rect Transform;
	GLFWwindow* Handle;
	GLFWmonitor* Monitor;
	const GLFWvidmode* VideoMode;
	WindowMode Mode;
	int RefreshRate;
	void(*Dispose)(Window);
};

#ifndef _window_methods_
#define _window_methods_
struct _windowMethods {
	void (*SetIcon)(Window, const Image);
	void (*SetMode)(Window, const WindowMode);
	/// <summary>
	/// Sets the size of the provided window(if it's in windowed) otherwise sets the resultion of the window(if it's fullscreen)
	/// </summary>
	void (*SetSize)(Window, const size_t width, const size_t height);
} WindowMethods;
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