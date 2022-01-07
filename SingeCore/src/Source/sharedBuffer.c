#include "graphics/sharedBuffer.h"
#include "singine/memory.h"
#include "csharp.h"

static void Dispose(SharedHandle buffer, void(*OnDispose)(unsigned int handle));
static SharedHandle CreateSharedHandle(void);

const struct _sharedHandleMethods SharedHandles = {
	.Create = &CreateSharedHandle,
	.Dispose = &Dispose
};

static void Dispose(SharedHandle buffer, void(*OnDispose)(unsigned int handle))
{
	if (buffer is null)
	{
		return;
	}

	if (buffer->ActiveInstances <= 1)
	{
		if (OnDispose isnt null)
		{
			OnDispose(buffer->Handle);
		}

		SafeFree(buffer);
		return;
	}

	--(buffer->ActiveInstances);
}

static SharedHandle CreateSharedHandle()
{
	SharedHandle buffer = SafeAlloc(sizeof(struct _sharedHandle));

	buffer->Handle = 0;
	buffer->ActiveInstances = 1;

	return buffer;
}
