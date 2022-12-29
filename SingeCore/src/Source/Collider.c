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

#define GENERATE_INTERSECTS_PLANE(dimension,index)\
static bool Intersects##dimension##Plane(const Collider left, const Collider right)\
{\
	const float upperLeft = max(left->FirstMarker[index], left->SecondMarker[index]);\
	const float lowerLeft = min(left->FirstMarker[index], left->SecondMarker[index]);\
	const float upperRight = max(right->FirstMarker[index], right->SecondMarker[index]);\
	const float lowerRight = min(right->FirstMarker[index], right->SecondMarker[index]);\
	const bool lowerIntersects = lowerRight < upperLeft;\
	const bool upperIntersects = lowerIntersects || upperRight > lowerLeft;\
	return lowerIntersects || upperIntersects;\
}

GENERATE_INTERSECTS_PLANE(X, 0);
GENERATE_INTERSECTS_PLANE(Y, 1);
GENERATE_INTERSECTS_PLANE(Z, 2);

#undef GENERATE_INTERSECTS_PLANE

static bool Intersects(const Collider left, const Collider right)
{
	const bool intersextsX = IntersectsXPlane(left, right);
	const bool intersectsY = intersextsX && IntersectsYPlane(left, right);
	const bool intersects = intersectsY && IntersectsZPlane(left, right );

	return intersects;
}

static void Dispose(Collider collider)
{
	Memory.Free(collider->State, ColliderStateTypeId);
	Memory.Free(collider, ColliderTypeId);
}