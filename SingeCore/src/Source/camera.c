#include "graphics/camera.h"
#include "singine/memory.h"
#include "csharp.h"
#include "cglm/mat4.h"
#include "cglm/cam.h"
#include "graphics/shaders.h"

static void Dispose(Camera camera)
{
	SafeFree(camera);
}

static void DrawMesh(Camera camera, RenderMesh mesh, Shader shader)
{
	// create the MVP
	mat4 MVP;
	glm_mat4_mul(camera->ViewProjectionMatrix, mesh->Transform, MVP);

	if (shader->BeforeDraw isnt null)
	{
		shader->BeforeDraw(shader, MVP);
	}

	if (shader->DrawMesh isnt null)
	{
		shader->DrawMesh(shader, mesh);
	}

	if (shader->AfterDraw isnt null)
	{
		shader->AfterDraw(shader);
	}
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
		camera->TargetPosition,
		camera->UpDirection,
		camera->ViewMatrix);
}

static void RecalculateViewProjection(Camera camera)
{
	glm_mat4_mul(camera->ProjectionMatrix,
		camera->ViewMatrix,
		camera->ViewProjectionMatrix);
}

static void Recalculate(Camera camera)
{
	camera->RecalculateProjection(camera);
	camera->RecalculateView(camera);
	camera->RecalculateViewProjection(camera);
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

	camera->Recalculate = &Recalculate;

	glm_vec3_zero(camera->TargetPosition);

	camera->UpDirection[0] = 0;
	camera->UpDirection[1] = 1.0f;
	camera->UpDirection[2] = 0;

	glm_mat4_identity(camera->ProjectionMatrix);
	glm_mat4_identity(camera->ViewMatrix);
	glm_mat4_identity(camera->ViewProjectionMatrix);


	return camera;
}