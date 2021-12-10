// SingeCore.c
#include <stdio.h>
#include <stdlib.h>

#include "csharp.h"
#include "singine/memory.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "graphics/window.h"
#include "singine/enumerable.h"

#include "graphics/window.h"

Window window;

int main()
{
	StartRuntime();

	glfwWindowHint(GLFW_SAMPLES, 4);

	// use opengl 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able

	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_DECORATED, true);

	glClearColor(1.0f, 1.0f, 0.4f, 0.0f);

	window = CreateWindow(1920, 1080, "Singine");

	// initiliaze GLEW
	glewExperimental = true;
	if (glewInit() != true)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	// bind graphics card with GDI
	glfwMakeContextCurrent(window->Handle);

	bool swap = false;

	WindowMethods.SetMode(window, WindowModes.FullScreen);

	do {
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window->Handle, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			WindowMethods.SetMode(window, WindowModes.Windowed);
		}

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		// poll for input
		glfwPollEvents();
	} while (glfwGetKey(window->Handle, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window->Handle) != true);

	window->Dispose(window);

	StopRuntime();

	// ensure leak free
	PrintAlloc(stdout);
	PrintFree(stdout);

	if (AllocCount() > FreeCount())
	{
		throw(MemoryLeakException);
	}
}