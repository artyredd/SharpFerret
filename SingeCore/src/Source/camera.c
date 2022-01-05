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
	camera->Transform->Dispose(camera->Transform);
	SafeFree(camera);
}

static void DrawMesh(Camera camera, RenderMesh mesh, Material material)
{
	// make sure the meshes transform is up to date
	vec4* modelMatrix = RefreshTransform(mesh->Transform);

	vec4* cameraVP = RefreshCamera(camera);

	// create the MVP
	mat4 MVP;
	glm_mat4_mul(cameraVP, modelMatrix, MVP);

	if (material isnt null)
	{
		if (material->Shader isnt null)
		{
			Shader shader = material->Shader;

			if (shader->BeforeDraw isnt null)
			{
				shader->BeforeDraw(shader, MVP);
			}

			Materials.Draw(material);

			mesh->Draw(mesh, material);

			if (shader->AfterDraw isnt null)
			{
				shader->AfterDraw(shader);
			}
		}
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

	// becuase all objects move around the camera and not the other way around
	// to move the camera right we must move all objects left
	// since the matrix for the camera is in normal units we have to flip the direction
	// so if the camera is at 0,1,0 we have to render all objects with 0,-1,0 so the camera apears above them
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

	// since the camera's transform is special we DONT want it to rotate around it's own position since that will
	// affect all objects in scene
	// have the camera rotate around origin since the camera is always at origin
	camera->Transform->RotateAroundCenter = true;

	// since the camera's transform is used to draw all of the other objects in the scene
	// we should invert the camera's transform
	// if we want to "turn" the camera 90deg to the right we should rotate the entire scene 90deg to the left
	camera->Transform->InvertTransform = true;

	// mark the state as needing a full refresh
	camera->State.Modified = AllModifiedFlag;

	return camera;
}