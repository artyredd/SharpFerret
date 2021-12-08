// SingeCore.c
#include <stdio.h>
#include <stdlib.h>

#include "csharp.h"
#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "stb_image.h"

const char* WindowName = "Singe";

size_t WindowWidth = 1920;
size_t WindowHeight = 1080;

GLFWwindow* window;
GLFWmonitor* monitor;
const GLFWvidmode* videoMode;

void LoadAndSetWindowIcon(GLFWwindow* window, char* iconPath);

int main()
{
	if (glfwInit() != true)
	{
		throw(IndexOutOfRangeException);
	}

	monitor = glfwGetPrimaryMonitor();
	videoMode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// use opengl 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able

	glfwWindowHint(GLFW_RESIZABLE, true);
	glfwWindowHint(GLFW_DECORATED, true);

	if (WindowWidth == 0)
	{
		WindowWidth = videoMode->width;
	}

	if (WindowHeight == 0)
	{
		WindowHeight = videoMode->height;
	}

	window = glfwCreateWindow((int)WindowWidth, (int)WindowHeight, WindowName, NULL, NULL);

	if (window is null)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	LoadAndSetWindowIcon(window, "icon.png");

	// initiliaze GLEW
	glewExperimental = true;
	if (glewInit() != true)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	do {
		// swap the back buffer with the front one
		glfwSwapBuffers(window);

		// poll for input
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) != true);

	// bind graphics card with GDI
	glfwMakeContextCurrent(window);

	// clean up glfwResources
	glfwTerminate();
}

GLFWimage icon = {
	0,0,NULL
};

void LoadAndSetWindowIcon(GLFWwindow* window, char* iconPath)
{
	int n;

	icon.pixels = stbi_load(iconPath, &icon.width, &icon.height, &n, 0);

	glfwSetWindowIcon(window, 1, &icon);

	stbi_image_free(icon.pixels);
}