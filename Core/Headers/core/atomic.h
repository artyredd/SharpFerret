#pragma once

struct locker
{
	_Atomic(_Bool)_Val;
};

typedef struct locker locker;

extern struct _atomicMethods
{
	int (*TryLock)(locker*);
	void (*Release)(locker*);
} Atomics;

// waits and locks the given Locker object
// this will 
#define lock(atomicLocker, body) while (Atomics.TryLock(&atomicLocker)){/**/} { body } Atomics.Release(&atomicLocker);