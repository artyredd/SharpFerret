#include "graphics/camera.h"
#include "singine/memory.h"
#include "csharp.h"
#include "cglm/mat4.h"
#include "cglm/cam.h"
#include "graphics/shaders.h"
#include "graphics/transform.h"
#include "helpers/quickmask.h"
#include "cglm/quat.h"
#include "cglm/affine.h"

#include "math/vectors.h"

#define ProjectionModifiedFlag FLAG_0
#define ViewModifiedFlag FLAG_1

#define AllModifiedFlag ( ProjectionModifiedFlag | ViewModifiedFlag)

#define PreventUneccesaryAssignment(left,right,comparison) if((left) comparison (right)) {return;}

static matrix4 RefreshCamera(Camera camera);
static void Dispose(Camera camera);
static void ForceRefresh(Camera camera);

// a macro produces these methods, it will always show message of not found here
static void SetFoV(Camera camera, float value);
static void SetAspectRatio(Camera camera, float value);
static void SetNearClippingDistance(Camera camera, float value);
static void SetFarClippingDistance(Camera camera, float value);
static void SetLeftDistance(Camera camera, float value);
static void SetRightDistance(Camera camera, float value);
static void SetTopDistance(Camera camera, float value);
static void SetBottomDistance(Camera camera, float value);

static Camera CreateCamera();

const struct _cameraMethods Cameras = {
	.Refresh = &RefreshCamera,
	.Create = &CreateCamera,
	.ForceRefresh = &ForceRefresh,
	.SetAspectRatio = &SetAspectRatio,
	.SetFarClippingDistance = &SetFarClippingDistance,
	.SetNearClippingDistance = &SetNearClippingDistance,
	.SetFoV = &SetFoV,
	.Dispose = &Dispose,
	.SetLeftDistance = &SetLeftDistance,
	.SetRightDistance = SetRightDistance,
	.SetTopDistance = SetTopDistance,
	.SetBottomDistance = SetBottomDistance
};

size_t CameraTypeId = 0;

static void Dispose(Camera camera)
{
	Transforms.Dispose(camera->Transform);
	Memory.Free(camera, CameraTypeId);
}

static void RecalculateProjection(Camera camera)
{
	if (camera->Orthographic)
	{
		glm_ortho(camera->LeftDistance,
			camera->RightDistance,
			camera->BottomDistance,
			camera->TopDistance,
			camera->NearClippingDistance,
			camera->FarClippingDistance,
			(vec4*)&camera->State.Projection);
	}
	else
	{
		glm_perspective(glm_rad(camera->FieldOfView),
			camera->AspectRatio,
			camera->NearClippingDistance,
			camera->FarClippingDistance,
			(vec4*)&camera->State.Projection);
	}
}

static void RecalculateView(Camera camera)
{
	Transforms.Refresh(camera->Transform);

	// becuase all objects move around the camera and not the other way around
	// to move the camera right we must move all objects left
	// since the matrix for the camera is in normal units we have to flip the direction
	// so if the camera is at 0,1,0 we have to render all objects with 0,-1,0 so the camera apears above them
}

static void RecalculateViewProjection(Camera camera)
{
	camera->State.View = Matrix4s.Inverse(camera->Transform->State.State);

	camera->State.State = Matrix4s.Multiply(camera->State.Projection,
		camera->State.View);

	ResetFlags(camera->State.Modified);
}

static matrix4 Recalculate(Camera camera)
{
	RecalculateProjection(camera);
	RecalculateView(camera);
	RecalculateViewProjection(camera);

	ResetFlags(camera->State.Modified);

	return camera->State.State;
}

static matrix4 RefreshCamera(Camera camera)
{
	unsigned int mask = camera->State.Modified;

	// check to see if our transform has changed
	if (camera->Transform->State.Modified isnt 0)
	{
		SetFlag(mask, ViewModifiedFlag);
	}

	// check to see if it's state was modified at all
	if (mask is 0)
	{
		return camera->State.State;
	}

	if (HasFlag(mask, AllModifiedFlag))
	{
		return Recalculate(camera);
	}

	// check to see if the projection was modified, if it was we need to re-calculate it and re-multiply it with the view
	if (HasFlag(mask, ProjectionModifiedFlag))
	{
		RecalculateProjection(camera);
	}

	if (HasFlag(mask, ViewModifiedFlag))
	{
		RecalculateView(camera);
	}

	// re-multiply the two and store the state in the state object to be returned
	RecalculateViewProjection(camera);

	return camera->State.State;
}

static void ForceRefresh(Camera camera)
{
	Recalculate(camera);
}

// ignore C5103, states invalid tokens generated, but tokens do compile and work as expected
#pragma warning(disable:5103)
// Produce duplicate methods during pre-processor compile
#define SetFloatBase(name,fieldName) static void Set ## name(Camera camera, float value)\
{\
	PreventUneccesaryAssignment(camera-> ## fieldName, value, == );\
	camera-> ## fieldName = value;\
	SetFlag(camera->State.Modified, ProjectionModifiedFlag);\
}

SetFloatBase(FoV, FieldOfView);
SetFloatBase(AspectRatio, AspectRatio);
SetFloatBase(NearClippingDistance, NearClippingDistance);
SetFloatBase(FarClippingDistance, FarClippingDistance);
SetFloatBase(LeftDistance, LeftDistance);
SetFloatBase(RightDistance, RightDistance);
SetFloatBase(TopDistance, TopDistance);
SetFloatBase(BottomDistance, BottomDistance);
#pragma warning(default:5103)


static Camera CreateCamera()
{
	Memory.RegisterTypeName("Camera", &CameraTypeId);

	Camera camera = Memory.Alloc(sizeof(struct _camera), CameraTypeId);

	camera->Orthographic = false;
	camera->FieldOfView = DefaultFieldOfView;
	camera->AspectRatio = DefaultAspectRatio;
	camera->NearClippingDistance = DefaultNearClippingPlane;
	camera->FarClippingDistance = DefaultFarClippingPlane;

	camera->Transform = Transforms.Create();

	// since the camera's transform is special we DONT want it to rotate around it's own position since that will
	// affect all objects in scene
	// have the camera rotate around origin since the camera is always at origin
	camera->Transform->RotateAroundCenter = true;

	// since the camera's transform is used to draw all of the other objects in the scene
	// we should invert the camera's transform
	// if we want to "turn" the camera 90deg to the right we should rotate the entire scene 90deg to the left
	camera->Transform->InvertTransform = false;

	// mark the state as needing a full refresh
	camera->State.Modified = AllModifiedFlag;

	camera->LeftDistance = -DefaultOrthographicDistance;
	camera->RightDistance = DefaultOrthographicDistance;
	camera->TopDistance = DefaultOrthographicDistance;
	camera->BottomDistance = -DefaultOrthographicDistance;

	return camera;
}