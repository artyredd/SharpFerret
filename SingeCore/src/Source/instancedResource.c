#include "singine/InstancedResource.h"
#include "csharp.h"
#include "singine/memory.h"
#include <stdlib.h>

static InstancedResource Create(void);
static void Dispose(InstancedResource, void* state, void(*OnDispose)(InstancedResource, void* state));
static InstancedResource Instance(InstancedResource resource);

const struct _instancedResourceMethods InstancedResources = {
	.Create = &Create,
	.Dispose = &Dispose,
	.Instance = &Instance
};

TYPE_ID(InstancedResource);

static InstancedResource Create(void)
{
	Memory.RegisterTypeName("InstancedResource", &InstancedResourceTypeId);

	InstancedResource resource = Memory.Alloc(sizeof(struct _instancedResource), InstancedResourceTypeId);

	resource->Instances = 1;

	return resource;
}

static void Dispose(InstancedResource resource, void* state, void(*OnDispose)(InstancedResource, void* state))
{
	// if passed null for some reason ignore it?
	if (resource is null) { return; }

	// only actually free the buffer when there is a single instance left
	if (resource->Instances is 1)
	{
		// if the caller provided a callback to perform before this object actually disposes invoke it
		if (OnDispose isnt null)
		{
			// no guard on the state here, if the caller wants to handle null then so be it
			OnDispose(resource, state);
		}

		// actually free the buffer since this is the last instance
		resource->Resource = null;
		Memory.Free(resource, InstancedResourceTypeId);
		return;
	}

	// since there remains more instances of this shared object decrement the total count and do nothing
	resource->Instances = min(resource->Instances - 1, resource->Instances);
}

static InstancedResource Instance(InstancedResource resource)
{
	if (resource is null)
	{
		throw(NullReferenceException);
	}

	size_t newCount = max(resource->Instances + 1, resource->Instances);

	if (newCount == resource->Instances)
	{
		fprintf(stderr, "Too many instances of resource at address: %llx (integer overflow)", (size_t)resource->Resource);
	}

	resource->Instances = newCount;

	return resource;
}