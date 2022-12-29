#pragma once

#include "csharp.h"

#include "graphics/transform.h"

typedef struct _collider* Collider;

struct _colliderState;

struct _collider
{
	struct _colliderState* State;
	// pointer to the controlling transform
	Transform Transform;
	// offset from the transform where the collider should start
	vec3 FirstMarker;
	// offset from the transform where the collider should end
	vec3 SecondMarker;
	// flag denoting the collider is not a physical collider
	// and no physics should be applied
	bool IsTrigger;
};

struct _colliderMethods
{
	Collider(*Create)();
	// determins whether the provided two colliders intersect withh
	// one another
	bool (*Intersects)(Collider left,Collider right);
	void (*Dispose)(Collider);
};

extern struct _colliderMethods Colliders;