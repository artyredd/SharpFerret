#pragma once

typedef struct _sharedBuffer* SharedHandle;

struct _sharedBuffer {
	// The opengl handle for this buffer
	unsigned int Handle;
	// The number of objects that reference this struct with pointers, when this reaches 0 this buffer can be reclaimed by OpenGL(freed)
	size_t ActiveInstances;
};

struct _sharedBufferMethods
{
	/// <summary>
	/// Disposes the buffer if the active instances of this object is less than or equal to 0, if it diposes OnDipose is invoked with the handle of the buffer
	/// </summary>
	void (*Dispose)(SharedHandle, void(*OnDispose)(unsigned int handle));
};

extern const struct _sharedBufferMethods SharedHandles;

SharedHandle CreateSharedHandle();