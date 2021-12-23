#pragma once

#include "math/vectors.h"

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
	/// <summary>
	/// The position in 3d space the camera is located
	/// </summary>
	vec3 Position;
	/// <summary>
	/// Where the camera should be looking in 3d space
	/// </summary>
	vec3 TargetPosition;
	/// <summary>
	/// The direction the camera should consider is up
	/// </summary>
	vec3 UpDirection;
	struct cameraState State;
	void(*DrawMesh)(Camera, RenderMesh, Shader);
	/// <summary>
	/// Forces the camera to recalculate it's fields and members on the next draw call, use the provided methods to modify fields instead of manually
	/// </summary>
	void(*ForceRefresh)(Camera);
	void(*SetFoV)(Camera, float);
	void(*SetAspectRatio)(Camera, float);
	void(*SetNearClippingDistance)(Camera, float);
	void(*SetFarClippingDistance)(Camera, float);
	void(*SetPositionVector)(Camera, vec3);
	void(*SetPositionX)(Camera, float);
	void(*SetPositionY)(Camera, float);
	void(*SetPositionZ)(Camera, float);
	void(*SetPosition)(Camera, float, float, float);
	void(*AddPositionVector)(Camera, vec3);
	void(*AddPositionX)(Camera, float);
	void(*AddPositionY)(Camera, float);
	void(*AddPositionZ)(Camera, float);
	void(*AddPosition)(Camera, float, float, float);
	void(*SetTargetVector)(Camera, vec3);
	void(*SetUpDirectionVector)(Camera, vec3);
	/// <summary>
	/// Diposes the managed resources and frees this object
	/// </summary>
	void(*Dispose)(Camera);
};

Camera CreateCamera();