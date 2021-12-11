#include "input.h"

typedef int KeyCode;

static struct _window {
	KeyCode none;
	KeyCode Alpha0;
	KeyCode Alpha1;
	KeyCode Alpha2;
	KeyCode Alpha3;
	KeyCode Alpha4;
	KeyCode Alpha5;
	KeyCode Alpha6;
	KeyCode Alpha7;
	KeyCode Alpha8;
	KeyCode Alpha9;
	KeyCode AlphaMinus;
	KeyCode Equals;
	KeyCode Num0;
	KeyCode Num1;
	KeyCode Num2;
	KeyCode Num3;
	KeyCode Num4;
	KeyCode Num5;
	KeyCode Num6;
	KeyCode Num7;
	KeyCode Num8;
	KeyCode Num9;
	KeyCode NumMinus;
	KeyCode NumPlus;
	KeyCode A;
	KeyCode B;
	KeyCode C;
	KeyCode D;
	KeyCode E;
	KeyCode F;
	KeyCode G;
	KeyCode H;
	KeyCode I;
	KeyCode J;
	KeyCode L;
	KeyCode M;
	KeyCode N;
	KeyCode O;
	KeyCode P;
	KeyCode Q;
	KeyCode R;
	KeyCode S;
	KeyCode T;
	KeyCode U;
	KeyCode V;
	KeyCode W;
	KeyCode X;
	KeyCode Y;
	KeyCode Z;
	KeyCode Escape;
	KeyCode Space;
	KeyCode LeftShift;
	KeyCode RightShift;
	KeyCode F1;
	KeyCode F2;
	KeyCode F3;
	KeyCode F4;
	KeyCode F5;
	KeyCode F6;
	KeyCode F7;
	KeyCode F8;
	KeyCode F9;
	KeyCode F10;
	KeyCode F12;
	KeyCode Up;
	KeyCode Down;
	KeyCode Left;
	KeyCode Right;
	KeyCode Backspace;
	KeyCode Enter;
	KeyCode Tilde;
	KeyCode Comma;
	KeyCode Period;
	KeyCode ForwardSlash;
	KeyCode Semicolon;
	KeyCode Apostraphe;
	KeyCode LeftBracket;
	KeyCode RightBracket;
} KeyCodes = {
	GLFW_KEY_0,
	GLFW_KEY_1,
	GLFW_KEY_2,
	GLFW_KEY_3,
	GLFW_KEY_4,
	GLFW_KEY_5,
	GLFW_KEY_6,
	GLFW_KEY_7,
	GLFW_KEY_8,
	GLFW_KEY_9,
	GLFW_KEY_MINUS,
	GLFW_KEY_EQUAL,
	GLFW_KEY_KP_0,
	GLFW_KEY_KP_1,
	GLFW_KEY_KP_2,
	GLFW_KEY_KP_3,
	GLFW_KEY_KP_4,
	GLFW_KEY_KP_5,
	GLFW_KEY_KP_6,
	GLFW_KEY_KP_7,
	GLFW_KEY_KP_8,
	GLFW_KEY_KP_9,
	GLFW_KEY_KP_SUBTRACT,
	GLFW_KEY_KP_ADD,
	GLFW_KEY_A,
	GLFW_KEY_B,
	GLFW_KEY_C,
	GLFW_KEY_D,
	GLFW_KEY_E,
	GLFW_KEY_F,
	GLFW_KEY_G,
	GLFW_KEY_H,
	GLFW_KEY_I,
	GLFW_KEY_J,
	GLFW_KEY_K,
	GLFW_KEY_L,
	GLFW_KEY_M,
	GLFW_KEY_N,
	GLFW_KEY_O,
	GLFW_KEY_P,
	GLFW_KEY_Q,
	GLFW_KEY_R,
	GLFW_KEY_S,
	GLFW_KEY_T,
	GLFW_KEY_U,
	GLFW_KEY_V,
	GLFW_KEY_W,
	GLFW_KEY_X,
	GLFW_KEY_Y,
	GLFW_KEY_Z,
	GLFW_KEY_ESCAPE,
	GLFW_KEY_SPACE,
	GLFW_KEY_LEFT_SHIFT,
	GLFW_KEY_RIGHT_SHIFT,
};

bool GetKey(Window window, KeyCode keycode)
{
	return glfwGetKey(window->Handle, keycode);
}