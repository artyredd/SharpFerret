#pragma once
#include <stdlib.h>

typedef struct _instancedResource* InstancedResource;

struct _instancedResource {
	void* Resource;
	size_t Instances;
};

struct _instancedResourceMethods {
	/// <summary>
	/// Creates an instance of the provided resource
	/// </summary>
	InstancedResource(*Instance)(InstancedResource);
	InstancedResource(*Create)(void);
	void (*Dispose)(InstancedResource, void* state, void(*OnDispose)(InstancedResource, void* state));
};

extern const struct _instancedResourceMethods InstancedResources;