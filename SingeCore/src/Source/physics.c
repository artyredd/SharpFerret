
#include "physics/physics.h"

static void Update(double deltaTime);

struct _physics Physics = {
	.Update = &Update
};

static void Update(double deltaTime)
{
	// do nothing for now
}