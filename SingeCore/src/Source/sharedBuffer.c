#include "graphics/sharedBuffer.h"
#include "singine/memory.h"
#include "csharp.h"
#include <stdlib.h>

static void Dispose(SharedHandle buffer, void* state, void(*OnDispose)(void* state));
static SharedHandle CreateSharedHandle(void);

const struct _sharedHandleMethods SharedHandles = {
	.Create = &CreateSharedHandle,
	.Dispose = &Dispose
};

static void Dispose(SharedHandle buffer, void* state, void(*OnDispose)(void* state))
{
	// if passed null for some reason ignore it?
	if (buffer is null) { return; }

	// only actually free the buffer when there is a single instance left
	if (buffer->ActiveInstances <= 1)
	{
		// if the caller provided a callback to perform before this object actually disposes invoke it
		if (OnDispose isnt null)
		{
			// no guard on the state here, if the caller wants to handle null then so be it
			OnDispose(state);
		}

		// actually free the buffer since this is the last instance
		SafeFree(buffer);
		return;
	}

	// since there remains more instances of this shared object decrement the total count and do nothing
	buffer->ActiveInstances = min(buffer->ActiveInstances - 1, buffer->ActiveInstances);
}

static SharedHandle CreateSharedHandle()
{
	SharedHandle buffer = SafeAlloc(sizeof(struct _sharedHandle));

	buffer->Handle = 0;
	buffer->ActiveInstances = 1;

	return buffer;
}
