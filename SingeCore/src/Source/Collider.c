#include "physics/Collider.h"

#include "types.h"
#include "singine/memory.h"
#include <stdlib.h>


static Collider Create();
static void Dispose(Collider);
static bool Intersects(Collider, Collider);

struct _colliderMethods Colliders = {
	.Create = &Create,
	.Dispose = &Dispose,
	.Intersects = &Intersects
};


struct _colliderState {
	// this is the marker in world position
	vec3 FirstMarker;
	vec3 SecondMarker;
};

TYPE_ID(Collider);
TYPE_ID(ColliderState);

static Collider Create()
{
	REGISTER_TYPE(Collider);
	REGISTER_TYPE(ColliderState);

	Collider result = Memory.Alloc(sizeof(struct _collider), ColliderTypeId);

	result->State = Memory.Alloc(sizeof(struct _colliderState), ColliderStateTypeId);
	result->Transform = null;

	return result;
}

static bool Intersects(const Collider left, const Collider right)
{
	(void)left;
	(void)right;
	return true;
}

static void Dispose(Collider collider)
{
	Memory.Free(collider->State, ColliderStateTypeId);
	Memory.Free(collider, ColliderTypeId);
}