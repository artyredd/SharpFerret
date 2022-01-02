#include "input.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <math.h>
#include "math/macros.h"

#define EnsureWindowSet() if (ActiveWindow is null)\
	{\
	fprintf(stderr, "There is no active window that was set with SetInputWindow(Window)"NEWLINE);\
	throw(NoActiveWindowException);\
	}\

void CalculateAxes(void);

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

const struct _cursorModes CursorModes = {
	.Normal = GLFW_CURSOR_NORMAL,
	.Hidden = GLFW_CURSOR_HIDDEN,
	.Disabled = GLFW_CURSOR_DISABLED
};

static Window ActiveWindow = null;
static CursorMode CurrentCursorMode = GLFW_CURSOR_NORMAL;
static bool RawMouseEnabled = false;

static double AxisStates[MAX_AXES];

void PollInput()
{
	glfwPollEvents();
	glfwGetCursorPos(ActiveWindow->Handle, &MousePosition[0], &MousePosition[1]);
	CalculateAxes();
}

bool GetKey(KeyCode key)
{
	EnsureWindowSet();
	return glfwGetKey(ActiveWindow->Handle, key);
}

void SetInputWindow(Window window)
{
	ActiveWindow = window;
}

Window GetInputWindow()
{
	return ActiveWindow;
}

void SetMousePosition(double x, double y)
{
	EnsureWindowSet();
	glfwSetCursorPos(ActiveWindow->Handle, x, y);
	MousePosition[0] = x;
	MousePosition[1] = y;
}

void SetCursorMode(CursorMode mode)
{
	EnsureWindowSet();
	glfwSetInputMode(ActiveWindow->Handle, GLFW_CURSOR, mode);
	CurrentCursorMode = mode;
}

CursorMode GetCursorMode(void)
{
	return CurrentCursorMode;
}

void SetRawMouseEnabled(bool value)
{
	if (glfwRawMouseMotionSupported())
	{
		if (value)
		{
			glfwSetInputMode(ActiveWindow->Handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			glfwSetInputMode(ActiveWindow->Handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}
		RawMouseEnabled = value;
	}
}

bool GetRawMouseEnabled()
{
	return RawMouseEnabled;
}

static double CalculateAxis(AxisDefinition definition, double newValue)
{
	double deadZoneLower = (definition->PreviousValue - definition->MinimumValue);
	double deadZoneUpper = (definition->PreviousValue + definition->MinimumValue);

	// check if the new value is within the deadzone, if it is return as unchanged
	if (newValue > deadZoneLower && newValue < deadZoneUpper)
	{
		return 0.0;
	}

	double difference = newValue - definition->PreviousValue;

	definition->PreviousValue = newValue;

	return difference > 0.0 ? 1.0 : -1.0;
}

void CalculateAxes()
{
	AxisStates[Axes.None] = 0.0;
	AxisStates[Axes.MouseX] = CalculateAxis(&AxisDefinitions[Axes.MouseX], MousePosition[0]);
	AxisStates[Axes.MouseY] = CalculateAxis(&AxisDefinitions[Axes.MouseY], MousePosition[1]);
	AxisStates[Axes.Horizontal] = -(double)GetKey(KeyCodes.Left) + (double)GetKey(KeyCodes.Right);
	AxisStates[Axes.Vertical] = -(double)GetKey(KeyCodes.Down) + (double)GetKey(KeyCodes.Up);
}

double GetAxis(Axis axis)
{
	// make sure we get a valid axis
	if (axis >= MAX_AXES)
	{
		return 0.0;
	}

	return AxisStates[axis];
}