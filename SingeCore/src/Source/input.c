#include "input.h"
#include "GLFW/glfw3.h"

const struct _keyCodes KeyCodes = {
	.none = 0,
	.Unknown = GLFW_KEY_UNKNOWN,

	.Alpha0 = GLFW_KEY_0,
	.Alpha1 = GLFW_KEY_1,
	.Alpha2 = GLFW_KEY_2,
	.Alpha3 = GLFW_KEY_3,
	.Alpha4 = GLFW_KEY_4,
	.Alpha5 = GLFW_KEY_5,
	.Alpha6 = GLFW_KEY_6,
	.Alpha7 = GLFW_KEY_7,
	.Alpha8 = GLFW_KEY_8,
	.Alpha9 = GLFW_KEY_9,

	.Minus = GLFW_KEY_MINUS,
	.Equals = GLFW_KEY_EQUAL,

	.Num0 = GLFW_KEY_KP_0,
	.Num1 = GLFW_KEY_KP_1,
	.Num2 = GLFW_KEY_KP_2,
	.Num3 = GLFW_KEY_KP_3,
	.Num4 = GLFW_KEY_KP_4,
	.Num5 = GLFW_KEY_KP_5,
	.Num6 = GLFW_KEY_KP_6,
	.Num7 = GLFW_KEY_KP_7,
	.Num8 = GLFW_KEY_KP_8,
	.Num9 = GLFW_KEY_KP_9,
	.NumMinus = GLFW_KEY_KP_SUBTRACT,
	.NumPlus = GLFW_KEY_KP_ADD,

	.A = GLFW_KEY_A,
	.B = GLFW_KEY_B,
	.C = GLFW_KEY_C,
	.D = GLFW_KEY_D,
	.E = GLFW_KEY_E,
	.F = GLFW_KEY_F,
	.G = GLFW_KEY_G,
	.H = GLFW_KEY_H,
	.I = GLFW_KEY_I,
	.J = GLFW_KEY_J,
	.K = GLFW_KEY_K,
	.L = GLFW_KEY_L,
	.M = GLFW_KEY_M,
	.N = GLFW_KEY_N,
	.O = GLFW_KEY_O,
	.P = GLFW_KEY_P,
	.Q = GLFW_KEY_Q,
	.R = GLFW_KEY_R,
	.S = GLFW_KEY_S,
	.T = GLFW_KEY_T,
	.U = GLFW_KEY_U,
	.V = GLFW_KEY_V,
	.W = GLFW_KEY_W,
	.X = GLFW_KEY_X,
	.Y = GLFW_KEY_Y,
	.Z = GLFW_KEY_Z,

	.Escape = GLFW_KEY_ESCAPE,
	.Space = GLFW_KEY_SPACE,

	.LeftShift = GLFW_KEY_LEFT_SHIFT,
	.RightShift = GLFW_KEY_RIGHT_SHIFT,

	.F1 = GLFW_KEY_F1,
	.F2 = GLFW_KEY_F2,
	.F3 = GLFW_KEY_F3,
	.F4 = GLFW_KEY_F4,
	.F5 = GLFW_KEY_F5,
	.F6 = GLFW_KEY_F6,
	.F7 = GLFW_KEY_F7,
	.F8 = GLFW_KEY_F8,
	.F9 = GLFW_KEY_F9,
	.F10 = GLFW_KEY_F10,
	.F11 = GLFW_KEY_F11,
	.F12 = GLFW_KEY_F12,

	.Up = GLFW_KEY_UP,
	.Down = GLFW_KEY_DOWN,
	.Left = GLFW_KEY_LEFT,
	.Right = GLFW_KEY_RIGHT,

	.Backspace = GLFW_KEY_BACKSPACE,
	.Enter = GLFW_KEY_ENTER,

	.Tilde = GLFW_KEY_GRAVE_ACCENT,

	.Comma = GLFW_KEY_COMMA,
	.Period = GLFW_KEY_PERIOD,
	.ForwardSlash = GLFW_KEY_SLASH,
	.Semicolon = GLFW_KEY_SEMICOLON,
	.Apostrophe = GLFW_KEY_APOSTROPHE,

	.LeftBracket = GLFW_KEY_LEFT_BRACKET,
	.RightBracket = GLFW_KEY_RIGHT_BRACKET,

	.LeftAlt = GLFW_KEY_LEFT_ALT,
	.RightAlt = GLFW_KEY_RIGHT_ALT,

	.LeftControl = GLFW_KEY_LEFT_CONTROL,
	.RightControl = GLFW_KEY_RIGHT_CONTROL,

	.CapsLock = GLFW_KEY_CAPS_LOCK,
	.Tab = GLFW_KEY_TAB,

	.Insert = GLFW_KEY_INSERT,
	.Delete = GLFW_KEY_DELETE,
	.Home = GLFW_KEY_HOME,
	.End = GLFW_KEY_END,
	.PageUp = GLFW_KEY_PAGE_UP,
	.PageDown = GLFW_KEY_PAGE_DOWN
};

void PollInput()
{
	glfwPollEvents();
}

bool GetKey(Window window, KeyCode key)
{
	return glfwGetKey(window->Handle, key);
}