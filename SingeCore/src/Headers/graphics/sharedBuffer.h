#pragma once

typedef struct _sharedHandle* SharedHandle;

struct _sharedHandle {
	// The opengl handle for this handle
	unsigned int Handle;
	// The number of objects that reference this struct with pointers, when this reaches 0 this handle can be disposed and freed
	size_t ActiveInstances;
};

struct _sharedHandleMethods
{
	SharedHandle(*Create)(void);
	/// <summary>
	/// Disposes the handle if the active instances of this object is less than or equal to 0, if it diposes OnDipose is invoked with the handle of the buffer
	/// </summary>
	void (*Dispose)(SharedHandle, void(*OnDispose)(unsigned int handle));
};

extern const struct _sharedHandleMethods SharedHandles;