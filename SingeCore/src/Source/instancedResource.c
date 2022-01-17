#include "singine/InstancedResource.h"
#include "csharp.h"
#include "singine/memory.h"

static InstancedResource Create(void);
static void Dispose(InstancedResource, void* state, void(*OnDispose)(InstancedResource, void* state));

const struct _instancedResourceMethods InstancedResources = {
	.Create = &Create,
	.Dispose = &Dispose
};

static InstancedResource Create(void)
{
	InstancedResource resource = SafeAlloc(sizeof(struct _instancedResource));

	resource->Instances = 1;

	return resource;
}

static void Dispose(InstancedResource resource, void* state, void(*OnDispose)(InstancedResource, void* state))
{
	// if passed null for some reason ignore it?
	if (resource is null) { return; }

	// only actually free the buffer when there is a single instance left
	if (resource->Instances <= 1)
	{
		// if the caller provided a callback to perform before this object actually disposes invoke it
		if (OnDispose isnt null)
		{
			// no guard on the state here, if the caller wants to handle null then so be it
			OnDispose(resource, state);
		}

		// actually free the buffer since this is the last instance
		SafeFree(resource);
		resource->Resource = null;
		return;
	}

	// since there remains more instances of this shared object decrement the total count and do nothing
	--(resource->Instances);
}