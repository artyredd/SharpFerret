#include "scripts/fpsCamera.h"
#include "graphics/renderMesh.h"
#include "graphics/shaders.h"
#include "input.h"
#include "singine/time.h"
#include "cglm/quat.h"



void Update(Camera camera);

struct _fpsCameraScript FPSCamera = {
	.MouseSensitivity = DEFAULT_MOUSE_SENSITIVITY,
	.MouseXSensitivity = DEFAULT_MOUSEX_SENSITIVITY,
	.MouseYSensitivity = DEFAULT_MOUSEY_SENSITIVITY,
	.InvertX = DEFAULT_INVERTX,
	.InvertY = DEFAULT_INVERTY,
	.InvertAxes = DEFAULT_INVERT_AXES,
	.State = {
		.HorizontalAngle = DEFAULT_HORIZONTAL_ANGLE * GLM_PI,
		.VerticalAngle = DEFAULT_VERTICAL_ANGLE * GLM_PI
	},
	.Update = &Update
};

void Update(Camera camera)
{
	// get the axis
	double xAxis = GetAxis(FPSCamera.InvertAxes ? DEFAULT_VERTICAL_AXIS : DEFAULT_HORIZONTAL_AXIS);
	double yAxis = GetAxis(FPSCamera.InvertAxes ? DEFAULT_HORIZONTAL_AXIS : DEFAULT_VERTICAL_AXIS);

	// invert the y if requested
	yAxis = FPSCamera.InvertY ? -yAxis : yAxis;
	xAxis = FPSCamera.InvertX ? -xAxis : xAxis;

	FPSCamera.State.HorizontalAngle += xAxis * DeltaTime() * FPSCamera.MouseSensitivity * FPSCamera.MouseXSensitivity;
	FPSCamera.State.VerticalAngle += yAxis * DeltaTime() * FPSCamera.MouseSensitivity * FPSCamera.MouseYSensitivity;

	// create a quaternion that represents spinning around the Y axis(horizontally spinning)
	glm_quat(FPSCamera.State.HorizontalRotation, (float)FPSCamera.State.HorizontalAngle, 0, 1, 0);

	// create a quaternion that represents spinning around Z axis (looking up and down)
	glm_quat(FPSCamera.State.VerticalRotation, (float)FPSCamera.State.VerticalAngle, 1, 0, 0);

	// combine the two
	// order here matters since multiplying two quaternions is one rotation THEN a second rotation after
	// they are not combined, we should rotate horizontally THEN up and down otherwise weird things happen
	glm_quat_mul(FPSCamera.State.HorizontalRotation, FPSCamera.State.VerticalRotation, FPSCamera.State.State);

	// set the camera's rotation to the new rotation
	Transforms.SetRotation(camera->Transform, FPSCamera.State.State);

	SetMousePosition(0, 0);
}