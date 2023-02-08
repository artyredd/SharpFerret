#pragma once

#include "graphics/camera.h"
#include "math/range.h"

#define DEFAULT_MOUSE_SENSITIVITY 1.0
#define DEFAULT_MOUSEX_SENSITIVITY 1.0
#define DEFAULT_MOUSEY_SENSITIVITY 1.0
#define DEFAULT_INVERTY true
#define DEFAULT_INVERTX true
#define DEFAULT_INVERT_AXES false
#define DEFAULT_HORIZONTAL_ANGLE 0.0 // in radians/pi so 1.0 would be pi or 180deg
#define DEFAULT_VERTICAL_ANGLE 0.0 // In radians/pi so 1.0 would be pi or 180deg
#define DEFAULT_HORIZONTAL_AXIS Axes.MouseX
#define DEFAULT_VERTICAL_AXIS Axes.MouseY
#define DEFAULT_VERTICAL_MIN -0.5 // radians/pi so 1.0 would be pi or 180deg
#define DEFAULT_VERTICAL_MAX  0.5 // radians/pi so 1.0 would be pi or 180deg

struct _fpsCameraState {
	// the angle in radians that the camera is facing horiontally
	double HorizontalAngle;
	// the angle in radians that the camera is facing vertically, 1pi rads is right side up
	double VerticalAngle;
	quaternion HorizontalRotation;
	quaternion VerticalRotation;
	quaternion State;
};

typedef struct _fpsCameraScript* FpsCameraScript;

struct _fpsCameraScript {
	double MouseSensitivity;
	double MouseXSensitivity;
	double MouseYSensitivity;
	bool InvertY;
	bool InvertX;
	bool InvertAxes;
	range VerticalLimits;
	struct _fpsCameraState State;
	void(*Update)(Camera);
};

extern struct _fpsCameraScript FPSCamera;