#include "graphics/transform.h"
#include "singine/memory.h"

#include "cglm/affine.h"

Transform CreateTransform()
{
	Transform transform = SafeAlloc(sizeof(struct _transform));

	InitializeVector3(transform->Position);
	SetVector3(transform->Scale, 1, 1, 1);
	SetVector4(transform->Rotation, 0, 0, 0, 1);

	InitializeMat4(transform->PreviousState);

	transform->Modified = true;

	return transform;
}

void RecalculateTransform(Transform transform)
{
	if (transform->Modified is false)
	{
		return;
	}

	ForceRefreshTransform(transform);
}

void ForceRefreshTransform(Transform transform)
{
	// scale, rotation then translation
	glm_mat4_identity(transform->PreviousState);

	glm_scale(transform->PreviousState, transform->Scale);

	glm_quat_rotate(transform->PreviousState, transform->Rotation, transform->PreviousState);

	glm_translate(transform->PreviousState, transform->Position);
}