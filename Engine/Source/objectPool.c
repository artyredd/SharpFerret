#include "engine/data/objectPool.h"
#include "core/memory.h"

static ObjectPool Create(size_t capacity);
static void Dispose(ObjectPool);
static void* Get(ObjectPool);
static void Release(ObjectPool, void* object);
static void Resize(ObjectPool, size_t newCapacity);
static void ReleaseAll(ObjectPool);

const struct _objectPoolMethods ObjectPools = {
	.Create = &Create,
	.Dispose = &Dispose,
	.Get = Get,
	.Release = Release,
	.ReleaseAll = ReleaseAll,
	.Resize = Resize
};

DEFINE_TYPE_ID(ObjectPool);

static ObjectPool Create(size_t capacity)
{
	REGISTER_TYPE(ObjectPool);

	ObjectPool pool = Memory.Alloc(sizeof(struct _objectPool), ObjectPoolTypeId);

	pool->ProviderState = null;
	pool->AutoResize = true;

	Resize(pool, capacity);

	return pool;
}

static void Dispose(ObjectPool pool)
{
	ReleaseAll(pool);
	Memory.Free(pool->State.Objects, Memory.GenericMemoryBlock);
	Memory.Free(pool, ObjectPoolTypeId);
}

static void AutoResizeOrThrow(ObjectPool pool)
{
	// resize the pool if it's dynamic
	if (pool->AutoResize)
	{
		Resize(pool, (pool->State.Capacity + 1) << 1);
	}
	else
	{
		// enable dynamic, or keep track of what's in the pool lmao
		throw(NoAvailableObjectInPoolException);
	}
}

static void* Get(ObjectPool pool)
{
	const size_t index = pool->State.FirstAvailableIndex;

	// if the first available index is pas the end of the capacity of the array
	// we probably need to resize
	if (index >= pool->State.Capacity)
	{
		AutoResizeOrThrow(pool);

		// recurse
		return Get(pool);
	}

	void* object = pool->ObjectProvider(pool->ProviderState);

	pool->State.Objects[index] = object;

	safe_increment(pool->State.FirstAvailableIndex);

	safe_increment(pool->Count);

	return object;
}

static void ReleaseObject(ObjectPool pool, void* object, size_t index)
{
	pool->ObjectRemover(object);
	pool->State.Objects[index] = null;
	pool->State.FirstAvailableIndex = index;

	safe_decrement(pool->Count);
}

static void Release(ObjectPool pool, void* object)
{
	for (size_t i = 0; i < pool->State.Capacity; i++)
	{
		if (object == pool->State.Objects[i])
		{
			ReleaseObject(pool, object, i);

			return;
		}
	}

	throw(ItemNotFoundInCollectionException);
}

static void ReleaseAll(ObjectPool pool)
{
	for (size_t i = 0; i < pool->Count; i++)
	{
		ReleaseObject(pool, pool->State.Objects[i], i);
	}
}

static void Resize(ObjectPool pool, size_t newCapacity)
{
	// dont resize if the block is already the same 
	// size
	if (newCapacity == pool->State.Capacity)
	{
		return;
	}

	const size_t previousSize = pool->State.Capacity * sizeof(void*);
	const size_t newSize = newCapacity * sizeof(void*);

	if (pool->State.Objects is null)
	{
		pool->State.Objects = Memory.Alloc(newSize, Memory.GenericMemoryBlock);
	}
	else
	{
		Memory.ReallocOrCopy((void**)&pool->State.Objects, previousSize, newSize, Memory.GenericMemoryBlock);
	}

	pool->State.Capacity = newCapacity;
}