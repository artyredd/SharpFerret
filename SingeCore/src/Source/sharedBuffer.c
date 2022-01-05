#include "graphics/sharedBuffer.h"
#include "singine/memory.h"
#include "csharp.h"

static void Dispose(SharedBuffer buffer, void(*OnDispose)(unsigned int handle));

const struct _sharedBufferMethods SharedBuffers = {
	.Dispose = &Dispose
};

static void Dispose(SharedBuffer buffer, void(*OnDispose)(unsigned int handle))
{
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

SharedBuffer CreateSharedBuffer()
{
	SharedBuffer buffer = SafeAlloc(sizeof(struct _sharedBuffer));

	buffer->Handle = 0;
	buffer->ActiveInstances = 1;

	return buffer;
}
