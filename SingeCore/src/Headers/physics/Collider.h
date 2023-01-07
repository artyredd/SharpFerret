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
	Collider(*Load)(const char* path);
	void (*Dispose)(Collider);
};

extern struct _colliderMethods Colliders;