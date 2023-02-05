#pragma once

#include "Collider.h"

struct _physics
{
	void (*Update)(double deltaTime);
	void (*RegisterCollider)(Collider collider);
	void (*UnRegisterCollider)(Collider collider);
};

extern struct _physics Physics;