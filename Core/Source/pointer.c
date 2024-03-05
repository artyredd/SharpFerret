#include "core/pointer.h"
#include "core/csharp.h"
#include "core/memory.h"
#include <stdlib.h>

private Pointer(void) Create(void* resourceToInstance);
private void Dispose(Pointer(void), void* state, void(*OnDispose)(Pointer(void), void* state));
private Pointer(void) Instance(Pointer(void) resource);

const struct _pointerMethods_void Pointers(void) = {
	.Create = &Create,
	.Dispose = &Dispose,
	.Instance = &Instance
};

DEFINE_TYPE_ID(voidPointer);

private Pointer(void) Create(void* resourceToInstance)
{
	REGISTER_TYPE(voidPointer);

	Pointer(void) resource = Memory.Alloc(sizeof(struct _pointer_void), typeid(voidPointer));

	resource->Instances = 1;
	resource->Resource = resourceToInstance;

	return resource;
}

private void Dispose(Pointer(void) resource, void* state, void(*OnDispose)(Pointer(void), void* state))
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
		Memory.Free(resource, typeid(voidPointer));
		return;
	}

	// since there remains more instances of this shared object decrement the total count and do nothing
	resource->Instances = min(resource->Instances - 1, resource->Instances);
}

private Pointer(void) Instance(Pointer(void) resource)
{
	if (resource is null)
	{
		throw(NullReferenceException);
	}

	ulong newCount = max(resource->Instances + 1, resource->Instances);

	if (newCount == resource->Instances)
	{
		fprintf(stderr, "Too many instances of resource at address: %llx (integer overflow)", (ulong)resource->Resource);
	}

	resource->Instances = newCount;

	return resource;
}