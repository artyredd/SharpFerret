#pragma once

#include "engine/graphics/window.h"

typedef int KeyCode;

const struct _keyCodes {
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
};

extern const struct _keyCodes KeyCodes;

typedef int CursorMode;

struct _cursorModes {
	const CursorMode Normal;
	const CursorMode Hidden;
	const CursorMode Disabled;
};

extern const struct _cursorModes CursorModes;

// The number of axes that are supported
#define MAX_AXES 5

/// <summary>
/// Integer representation of an axis
/// </summary>
typedef unsigned int Axis;

const static struct _availableAxes {
	Axis None;
	Axis MouseX;
	Axis MouseY;
	Axis Horizontal;
	Axis Vertical;
} Axes = {
	.None = 0,
	.MouseX = 1,
	.MouseY = 2,
	.Horizontal = 3,
	.Vertical = 4
};

typedef struct _axisDefinition* AxisDefinition;

struct _axisDefinition {
	double MinimumValue;
	double MaximumValue;
	double PreviousValue;
};

static struct _axisDefinition AxisDefinitions[MAX_AXES] = {
	// none
	{
		.MinimumValue = 0.0,
		.MaximumValue = 0.0,
		.PreviousValue = 0.0
	},
	// MouseX
	{
		.MinimumValue = 1.0,
		.MaximumValue = 4000.0,
		.PreviousValue = 0.0
	},
	// MouseY
	{
		.MinimumValue = 1.0,
		.MaximumValue = 4000.0,
		.PreviousValue = 0.0
	},
	// Horizontal
	{
		.MinimumValue = 1.0,
		.MaximumValue = 1.0,
		.PreviousValue = 0.0
	},
	// Vertical
	{
		.MinimumValue = 1.0,
		.MaximumValue = 1.0,
		.PreviousValue = 0.0
	}
};



extern struct _inputMethods {
	void (*PollInput)(void);

	/// <summary>
	/// Sets the provided window as the active window that should receive input
	/// </summary>
	/// <param name="window"></param>
	void (*SetInputWindow)(Window window);

	/// <summary>
	/// Gets the last set active window
	/// </summary>
	/// <returns></returns>
	Window(*GetInputWindow)();

	/// <summary>
	/// Gets whether the given key is held down for the active window
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	bool (*GetKey)(KeyCode key);

	// Sets the mouse position for the current window to the position provided
	void (*SetMousePosition)(double x, double y);
	void (*SetCursorMode)(CursorMode mode);
	CursorMode(*GetCursorMode)(void);
	void (*SetRawMouseEnabled)(bool value);
	bool (*GetRawMouseEnabled)();
	double (*GetAxis)(Axis axis);
	struct State
	{
		/// <summary>
		/// The current mouse position for the active window
		/// </summary>
		double MousePosition[2];
	} State;
} Inputs;