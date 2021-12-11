#include "input.h"


void PollInput()
{
	glfwPollEvents();
}

bool GetKey(Window window, KeyCode key)
{
	return glfwGetKey(window->Handle, key);
}