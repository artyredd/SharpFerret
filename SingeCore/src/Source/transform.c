#include "graphics/transform.h"
#include "singine/memory.h"

#include "cglm/affine.h"
#include "helpers/quickmask.h"


#define PositionModifiedFlag FLAG_0
#define RotationModifiedFlag FLAG_1
#define ScaleModifiedFlag FLAG_2

#define AllModifiedFlag (PositionModifiedFlag | RotationModifiedFlag | ScaleModifiedFlag)

Transform CreateTransform()
{
	Transform transform = SafeAlloc(sizeof(struct _transform));

	InitializeVector3(transform->Position);
	SetVector3(transform->Scale, 1, 1, 1);
	SetVector4(transform->Rotation, 0, 0, 0, 1);

	transform->PreviousState.Modified = 0;

	return transform;
}

void RecalculateTransform(Transform transform)
{
	unsigned int mask = transform->PreviousState.Modified;

	// if nothing changed return
	if (mask is 0)
	{
		return;
	}

	// check to see if we should perform a whole refresh
	// since the transform starts with scale a change to it requires a whole refresh by default
	if (HasFlag(mask, AllModifiedFlag) || HasFlag(mask, ScaleModifiedFlag))
	{
		ForceRefreshTransform(transform);

		return;
	}

	
	// since we dont need to do a full refresh, do only the calcs needed to save cpu time
	// this engine was orignally designed to be single threaded
	if (HasFlag(mask, RotationModifiedFlag | PositionModifiedFlag))
	{
		vec4* state = transform->PreviousState.PreviousScaleMatrix;

		glm_quat_rotate(state, transform->Rotation, state);

		glm_translate(state, transform->Position);

		return;
	}

	vec4* state = transform->PreviousState.PreviousRotationMatrix;

	glm_translate(state, transform->Position);
}

void ForceRefreshTransform(Transform transform)
{
	// mat4 is just a typedef'ed vec4[4]
	vec4* state = transform->PreviousState.State;

	// scale, rotation then translation
	glm_mat4_identity(state);

	glm_scale(state, transform->Scale);

	glm_quat_rotate(state, transform->Rotation, state);

	glm_translate(state, transform->Position);
}