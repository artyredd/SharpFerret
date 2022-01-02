#pragma once

#include "graphics/camera.h"

#define DEFAULT_MOUSE_SENSITIVITY 1.0
#define DEFAULT_MOUSEX_SENSITIVITY 1.0
#define DEFAULT_MOUSEY_SENSITIVITY 1.0
#define DEFAULT_INVERTY true
#define DEFAULT_INVERT_AXES false

struct _fpsCameraState {
	double HorizontalAngle;
	double VerticalAngle;
	Quaternion HorizontalRotation;
	Quaternion VerticalRotation;
	Quaternion State;
};

typedef struct _fpsCameraScript* FpsCameraScript;

struct _fpsCameraScript {
	double MouseSensitivity;
	double MouseXSensitivity;
	double MouseYSensitivity;
	bool InvertY;
	bool InvertAxes;
	struct _fpsCameraState State;
	void(*Update)(Camera);
};

extern struct _fpsCameraScript FPSCamera;

