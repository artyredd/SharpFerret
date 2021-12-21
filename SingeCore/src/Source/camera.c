#include "graphics/camera.h"
#include "singine/memory.h"
#include "csharp.h"
#include "cglm/mat4.h"
#include "cglm/cam.h"
#include "graphics/shaders.h"

static void Dispose(Camera camera)
{

}

static void DrawMesh(Camera camera, Mesh mesh, Shader shader)
{
	// create the MVP
	mat4 MVP;
	glm_mat4_mul(camera->ViewProjectionMatrix,mesh->Transform,MVP);

	throw(NotImplementedException);
}

static void RecalculateProjection(Camera camera)
{
	glm_perspective(glm_rad(camera->FieldOfView), 
		camera->AspectRatio, 
		camera->NearClippingDistance, 
		camera->FarClippingDistance, 
		camera->ProjectionMatrix);
}

static void RecalculateView(Camera camera)
{
	glm_lookat(camera->Position,
		camera->CameraTargetPositon,
		camera->UpDirection,
		camera->ViewMatrix);
}

static void RecalculateViewProjection(Camera camera)
{
	glm_mat4_mul(camera->ProjectionMatrix,
		camera->ViewMatrix,
		camera->ViewProjectionMatrix);
}

Camera CreateCamera()
{
	Camera camera = SafeAlloc(sizeof(struct _camera));

	camera->FieldOfView = DefaultFieldOfView;
	camera->AspectRatio = DefaultAspectRatio;
	camera->NearClippingDistance = DefaultNearClippingPlane;
	camera->FarClippingDistance = DefaultFarClippingPlane;

	camera->Dispose = &Dispose;
	camera->DrawMesh = &DrawMesh;

	camera->RecalculateView = &RecalculateView;
	camera->RecalculateProjection = &RecalculateProjection;
	camera->RecalculateViewProjection = &RecalculateViewProjection;

	return camera;
}