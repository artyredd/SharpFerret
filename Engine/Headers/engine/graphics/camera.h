#pragma once

#include "core/math/vectors.h"
#include "engine/graphics/transform.h"
#include "engine/graphics/renderMesh.h"

static float DefaultFieldOfView = 70.0f;
static float DefaultAspectRatio = 16.0f / 9.0f;
static float DefaultNearClippingPlane = 0.1f;
static float DefaultFarClippingPlane = 100.0f;
static float DefaultOrthographicDistance = 10.0f;

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
	matrix4 Projection;
	/// <summary>
	/// The previous view matrix that was calculated, this needs to be refreshed any time the Position, TargetPosition, or up direction are modified
	/// </summary>
	matrix4 View;
	/// <summary>
	/// The previous combined state of this camera, this is the combined matrices of the view and projection
	/// </summary>
	matrix4 State;
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
	/// <summary>
	/// Orthographic; The left distance from the center of the camera
	/// </summary>
	float LeftDistance;
	/// <summary>
	/// Orthographic; The right distance from the center of the camera
	/// </summary>
	float RightDistance;
	/// <summary>
	/// Orthographic; The top distance from the center of the camera
	/// </summary>
	float TopDistance;
	/// <summary>
	/// Orthographic; The bottom distance from the center of the camera
	/// </summary>
	float BottomDistance;
	/// <summary>
	/// Whether or not the camera should use ortho or perspective for the projection
	/// </summary>
	bool Orthographic;
	Transform Transform;
	struct cameraState State;
};

struct _cameraMethods {
	Camera(*Create)();
	/// <summary>
	/// Refreshes the camera's transform
	/// </summary>
	matrix4 (*Refresh)(Camera camera);
	/// <summary>
	/// Forces the camera to recalculate it's fields and members on the next draw call, use the provided methods to modify fields instead of manually
	/// </summary>
	void (*ForceRefresh)(Camera);
	void (*SetFoV)(Camera, float);
	void (*SetAspectRatio)(Camera, float);
	void (*SetNearClippingDistance)(Camera, float);
	void (*SetFarClippingDistance)(Camera, float);
	void (*SetLeftDistance)(Camera camera, float value);
	void (*SetRightDistance)(Camera camera, float value);
	void (*SetTopDistance)(Camera camera, float value);
	void (*SetBottomDistance)(Camera camera, float value);
	/// <summary>
	/// Diposes the managed resources and frees this object
	/// </summary>
	void(*Dispose)(Camera);
};

extern const struct _cameraMethods Cameras;

