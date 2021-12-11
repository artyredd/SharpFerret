#pragma once

#include "graphics/window.h"

typedef int KeyCode;

static const struct _keyCodes {
	KeyCode none;
	KeyCode Unknown;

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

	KeyCode Minus;
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
	KeyCode K;
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
	KeyCode F11;
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
	KeyCode Apostrophe;

	KeyCode LeftBracket;
	KeyCode RightBracket;

	KeyCode LeftAlt;
	KeyCode RightAlt;

	KeyCode LeftControl;
	KeyCode RightControl;

	KeyCode CapsLock;
	KeyCode Tab;

	KeyCode Insert;
	KeyCode Delete;
	KeyCode Home;
	KeyCode End;
	KeyCode PageUp;
	KeyCode PageDown;
} KeyCodes = {
	0,
	GLFW_KEY_UNKNOWN,

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

	GLFW_KEY_F1,
	GLFW_KEY_F2,
	GLFW_KEY_F3,
	GLFW_KEY_F4,
	GLFW_KEY_F5,
	GLFW_KEY_F6,
	GLFW_KEY_F7,
	GLFW_KEY_F8,
	GLFW_KEY_F9,
	GLFW_KEY_F10,
	GLFW_KEY_F11,
	GLFW_KEY_F12,

	GLFW_KEY_UP,
	GLFW_KEY_DOWN,
	GLFW_KEY_LEFT,
	GLFW_KEY_RIGHT,

	GLFW_KEY_BACKSPACE,
	GLFW_KEY_ENTER,

	GLFW_KEY_GRAVE_ACCENT,

	GLFW_KEY_COMMA,
	GLFW_KEY_PERIOD,
	GLFW_KEY_SLASH,
	GLFW_KEY_SEMICOLON,
	GLFW_KEY_APOSTROPHE,

	GLFW_KEY_LEFT_BRACKET,
	GLFW_KEY_RIGHT_BRACKET,

	GLFW_KEY_LEFT_ALT,
	GLFW_KEY_RIGHT_ALT,

	GLFW_KEY_LEFT_CONTROL,
	GLFW_KEY_RIGHT_CONTROL,

	GLFW_KEY_CAPS_LOCK,
	GLFW_KEY_TAB,

	GLFW_KEY_INSERT,
	GLFW_KEY_DELETE,
	GLFW_KEY_HOME,
	GLFW_KEY_END,
	GLFW_KEY_PAGE_UP,
	GLFW_KEY_PAGE_DOWN
};


void PollInput(void);

bool GetKey(Window window, KeyCode key);