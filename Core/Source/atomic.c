#include "core/atomic.h"
#include "stdatomic.h"
#include <vcruntime_c11_stdatomic.h>

int TryLock(locker*);
void Release(locker*);

struct _atomicMethods Atomics = {
	.TryLock = TryLock,
	.Release = Release
};

static inline int TryLock(locker* lock)
{
	return atomic_flag_test_and_set(lock);
}

static inline void Release(locker* lock)
{
	atomic_flag_clear(lock);
}