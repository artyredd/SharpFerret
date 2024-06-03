#pragma once

#include <vcruntime_c11_stdatomic.h>

typedef struct atomic_flag atomic_flag;

typedef atomic_flag Locker;

// waits and locks the given Locker object
// this will 
#define lock(atomic_locker, body) while (atomic_flag_test_and_set(&atomic_locker)){/**/} { body } atomic_flag_clear(&atomic_locker);