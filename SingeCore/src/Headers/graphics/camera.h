#pragma once

#include "math/vectors.h"
#include "graphics/transform.h"
#include "graphics/renderMesh.h"

static float DefaultFieldOfView = 70.0f;
static float DefaultAspectRatio = 16.0f / 9.0f;
static float DefaultNearClippingPlane = 0.1f;
static float DefaultFarClippingPlane = 100.0f;

/// <summary>
/// holds the internal state of the camera, NOT meant to be manually modified, use _camera methods
/// </summary>
struct cameraState {
	/// <summary>
	/// This is a bit mask that controls the dirty state of this object
	/// 0 : Not Modified; != 0 : Modified
	/// </summary>
	unsigned int Modified;
	/// <summary>
	/// The previous projection matrix that was calculated, this needs to be  refreshed any time the FOV, Aspect Ratio, or near and far clipping distances
	/// are changed
	/// </summary>
	mat4 Projection;
	/// <summary>
	/// The previous view matrix that was calculated, this needs to be refreshed any time the Position, TargetPosition, or up direction are modified
	/// </summary>
	mat4 View;
	/// <summary>
	/// The previous combined state of this camera, this is the combined matrices of the view and projection
	/// </summary>
	mat4 State;
};

typedef struct _camera* Camera;

struct _camera
{
	/// <summary>
	/// The field of view of this camera in degrees
	/// </summary>
	float FieldOfView;
	/// <summary>
	/// The aspect ratio of this camera, for example: 4:3 this would be 4.0f/3.0f
	/// </summary>
	float AspectRatio;
	/// <summary>
	/// The distance from the camera where close objects should be clipped
	/// </summary>
	float NearClippingDistance;
	/// <summary>
	/// The distance from the camera where objects should not be rendered
	/// </summary>
	float FarClippingDistance;
	Transform Transform;
	struct cameraState State;
};

struct _cameraMethods {
	Camera(*CreateCamera)();
	/// <summary>
	/// Refreshes the camera's transform
	/// </summary>
	vec4* (*Refresh)(Camera camera);
	/// <summary>
	/// Forces the camera to recalculate it's fields and members on the next draw call, use the provided methods to modify fields instead of manually
	/// </summary>
	void(*ForceRefresh)(Camera);
	void(*SetFoV)(Camera, float);
	void(*SetAspectRatio)(Camera, float);
	void(*SetNearClippingDistance)(Camera, float);
	void(*SetFarClippingDistance)(Camera, float);
	/// <summary>
	/// Diposes the managed resources and frees this object
	/// </summary>
	void(*Dispose)(Camera);
};

extern const struct _cameraMethods Cameras;

