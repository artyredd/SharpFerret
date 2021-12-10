#pragma once

#include "GLFW/glfw3.h"
#include "csharp.h"
#include "imaging.h"
#include "singine/enumerable.h"

// The way a window is displayed, 0 = windowed, 1 = BorderlessFullScreen, 2 = FullScreen
typedef int WindowMode;

// The way a window is displayed, 0 = windowed, 1 = BorderlessFullScreen, 2 = FullScreen
static const struct _WindowModes {
	const WindowMode Windowed;
	const WindowMode BorderlessFullScreen;
	const WindowMode FullScreen;
} WindowModes = {
	0,
	1,
	2
};

typedef struct _windowObject* Window;

// stores the state of a window
struct _windowObject {
	void* Handle;
	void* Monitor;
	void* VideoMode;
	WindowMode Mode;
};

static struct _windowMethods {
	void(*SetMode)(Window, const WindowMode);
	void(*SetSize)(Window, const size_t width, const size_t height);
} WindowMethods;

Window CreateWindow(size_t width, size_t height, char* name, WindowMode windowMode);

// returns: true if the underlying window system(glfw) is initiliazed
bool RuntimeStarted();
// Starts the GLFW runtime, use RuntimeStarted() to determine if the runtime is currently started
void StartRuntime();
// Stops the GLFW runtime
void StopRuntime();