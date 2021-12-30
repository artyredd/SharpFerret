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

#define ProjectionModifiedFlag FLAG_0
#define ViewModifiedFlag FLAG_1

#define AllModifiedFlag ( ProjectionModifiedFlag | ViewModifiedFlag)

#define PreventUneccesaryAssignment(left,right,comparison) if((left) comparison (right)) {return;}

static vec4* RefreshCamera(CameracameraVP);

static void Dispose(Camera camera)
{
	SafeFree(camera);
}

static void DrawMesh(Camera camera, RenderMesh mesh, Shader shader)
{
	// make sure the meshes transform is up to date
	vec4* modelMatrix = RefreshTransform(mesh->Transform);

	vec4* cameraVP = RefreshCamera(camera);

	// create the MVP
	mat4 MVP;
	glm_mat4_mul(cameraVP, modelMatrix, MVP);

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
		camera->State.Projection);
}

static void RecalculateView(Camera camera)
{
	/*glm_lookat(camera->Position,
		camera->TargetPosition,
		camera->UpDirection,
		camera->State.View);*/
	RefreshTransform(camera->Transform);
}

static void RecalculateViewProjection(Camera camera)
{
	glm_mat4_mul(camera->State.Projection,
		camera->Transform->State.State,
		camera->State.State);

	ResetFlags(camera->State.Modified);
}

static vec4* Recalculate(Camera camera)
{
	RecalculateProjection(camera);
	RecalculateView(camera);
	RecalculateViewProjection(camera);

	ResetFlags(camera->State.Modified);

	return camera->State.State;
}

static vec4* RefreshCamera(Camera camera)
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

static void SetFoV(Camera camera, float value)
{
	PreventUneccesaryAssignment(camera->FieldOfView, value, == );
	camera->FieldOfView = value;
	SetFlag(camera->State.Modified, ProjectionModifiedFlag);
}

static void SetAspectRatio(Camera camera, float value)
{
	PreventUneccesaryAssignment(camera->AspectRatio, value, == );
	camera->AspectRatio = value;
	SetFlag(camera->State.Modified, ProjectionModifiedFlag);
}

static void SetNearClippingDistance(Camera camera, float value)
{
	PreventUneccesaryAssignment(camera->NearClippingDistance, value, == );
	camera->NearClippingDistance = value;
	SetFlag(camera->State.Modified, ProjectionModifiedFlag);
}

static void SetFarClippingDistance(Camera camera, float value)
{
	PreventUneccesaryAssignment(camera->FarClippingDistance, value, == );
	camera->FarClippingDistance = value;
	SetFlag(camera->State.Modified, ProjectionModifiedFlag);
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

	camera->ForceRefresh = &ForceRefresh;
	camera->SetAspectRatio = &SetAspectRatio;
	camera->SetFarClippingDistance = &SetFarClippingDistance;
	camera->SetNearClippingDistance = &SetNearClippingDistance;
	camera->SetFoV = &SetFoV;

	camera->Transform = CreateTransform();

	// mark the state as needing a full refresh
	camera->State.Modified = AllModifiedFlag;

	return camera;
}