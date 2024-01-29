#pragma once

#include "core/csharp.h"

typedef struct _objectPool* ObjectPool;

struct _objectPool
{
	struct state
	{
		void** Objects;
		size_t Capacity;
		size_t FirstAvailableIndex;
	} State;

	size_t Count;
	
	// function that should be invoked with a to-be removed object
	// this would typically be a dispose method
	void (*ObjectRemover)(void* object);
	
	// the function that should be called when the object pool needs to be extended
	// and new objects need to be generated
	void* (*ObjectProvider)(void* ProviderState);

	// the state that should be passed to the allocator
	// this would typically be information that should be passed
	// into the ObjectProvider that is needed to generate new objects for the pool
	void* ProviderState;

	// whether or not we should automatically resize the pool dynamically
	// if more gets are called than releases
	bool AutoResize;
};

struct _objectPoolMethods
{
	ObjectPool(*Create)();

	void* (*Get)(ObjectPool);
	void (*Release)(ObjectPool, void* object);
	void (*ReleaseAll)(ObjectPool);
	void (*Resize)(ObjectPool, size_t newCapacity);

	void (*Dispose)(ObjectPool);
};

extern const struct _objectPoolMethods ObjectPools;