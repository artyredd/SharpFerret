#pragma once

#include <vcruntime_c11_stdatomic.h>

typedef atomic_bool Locker;

// waits and locks the given Locker object
// this will 
#define lock(atomic_locker) while (atomic_flag_test_and_set(&lock)){/**/}