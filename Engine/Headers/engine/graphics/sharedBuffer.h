#pragma once
#include "core/csharp.h"

typedef struct _sharedHandle* SharedHandle;

struct _sharedHandle {
	// The opengl handle for this handle
	uint Handle;
	// The number of objects that reference this struct with pointers, when this reaches 0 this handle can be disposed and freed
	ulong ActiveInstances;
};

struct _sharedHandleMethods
{
	SharedHandle(*Create)(void);
	/// <summary>
	/// Disposes the handle if the active instances of this object is less than or equal to 0, if it diposes OnDipose is invoked with the provided state allowing last minute cleanup before the 
	/// handle is cleared and the object is disposed
	/// </summary>
	void (*Dispose)(SharedHandle, void* state, void(*OnDispose)(void* state));
	/// <summary>
	/// Auto instances the provided handle
	/// </summary>
	SharedHandle(*Instance)(SharedHandle);
};

extern const struct _sharedHandleMethods SharedHandles;