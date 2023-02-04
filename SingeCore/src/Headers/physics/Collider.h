#pragma once

#include "csharp.h"

#include "graphics/transform.h"
#include "modeling/model.h"
#include "helpers/quickmask.h"

typedef struct _collision collision;

struct _collision
{
	// index of the triangle within the left model where the first collision
	// took place
	size_t LeftHitIndex;
	// index of the triangle within the right model where the first collision
	// took place
	size_t RightHitIndex;
};

typedef struct _collider* Collider;

struct _collider
{
	// pointer to the controlling transform
	Transform Transform;
	// flag denoting the collider is not a physical collider
	// and no physics should be applied
	bool IsTrigger;
	// the path to the model that should be used to represent the collider
	char* ModelPath;
	// Triangle information for the collider
	Model Model;
	// the bit mask that determines whether or not a collider can interact
	// with another collider
	intMask Mask;
	// two objects interact when the layer of a collider
	// contains any of the bits in interaction mask
	intMask Layer;
};

struct _colliderMethods
{
	// the interaction mask that every collider starts with
	intMask DefaultMask;
	// the default collision layer of this object, two objects interact when 
	// the layer of a collider contains any of the bits in interaction mask
	intMask DefaultLayer;
	Collider(*Create)();
	// determins whether the provided two colliders intersect withh
	// one another
	bool (*Intersects)(Collider left,Collider right);
	Collider(*Load)(const char* path);
	void (*Dispose)(Collider);
};

extern struct _colliderMethods Colliders;