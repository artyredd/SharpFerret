
#include "physics/physics.h"

static void Update(double deltaTime);
static void RegisterCollider(Collider collider);
static void UnRegisterCollider(Collider collider);

struct _physics Physics = {
	.Update = &Update,
	.RegisterCollider = RegisterCollider,
	.UnRegisterCollider = UnRegisterCollider
};

// TODO make this an expanding array instead of fixed
size_t Global_ColliderCapacity = 1024;
Collider Global_Colliders[1024];
size_t Global_AvailableIndex = 0;

static size_t GetAvailableIndex()
{
	if (Global_AvailableIndex < Global_ColliderCapacity)
	{
		return Global_AvailableIndex;
	}

	for (size_t index = 0; index < Global_ColliderCapacity; index++)
	{
		if (Global_Colliders[index] == null)
		{
			Global_AvailableIndex = index;
			return Global_AvailableIndex;
		}
	}
}

static void RegisterCollider(Collider collider)
{
	size_t index = GetAvailableIndex();
	Global_Colliders[index] = collider;
}

static void UnRegisterCollider(Collider collider)
{

}

static void Update(double deltaTime)
{
	// do nothing for now
}