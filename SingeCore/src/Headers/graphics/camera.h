#pragma once

#include "math/vectors.h"

#include "graphics/renderModel.h"

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
	vec3 CameraTargetPositon;
	/// <summary>
	/// The direction the camera should consider is up
	/// </summary>
	vec3 UpDirection;
	/// <summary>
	/// The matrix representing how objects should be rendered based on FoV, clipping and aspect ration
	/// </summary>
	mat4 ProjectionMatrix;
	/// <summary>
	/// The matrix representing where the camera is looking
	/// </summary>
	mat4 ViewMatrix;
	void(*DrawMesh)(Camera,Mesh);
	/// <summary>
	/// Diposes the managed resources and frees this object
	/// </summary>
	void(*Dispose)(Camera);
};