#pragma once
#include <stdlib.h>

#define DEFINE_POINTER_STRUCT(type)\
struct _pointer_##type {\
type* Resource; \
size_t Instances; \
};\
typedef struct _pointer_##type* type##Pointer;

// A pointer to the given type, that keeps track of references to that pointer
#define Pointer(type) type##Pointer
#define Pointers(type) type##PointerMethods

DEFINE_POINTER_STRUCT(void);

#define DEFINE_INSTANCE_METHOD_POINTER(type) Pointer(type)(*Instance)(Pointer(type))
#define DEFINE_CREATE_METHOD_POINTER(type) Pointer(type)(*Create)(type* resourceToInstance)
#define DEFINE_DISPOSE_METHOD_POINTER(type) void (*Dispose)(Pointer (type), void* state, void(*OnDispose)(Pointer(type), void* state))

#define DEFINE_POINTER_METHODS(type)\
const struct _pointerMethods_##type{\
DEFINE_INSTANCE_METHOD_POINTER(type); \
DEFINE_CREATE_METHOD_POINTER(type); \
DEFINE_DISPOSE_METHOD_POINTER(type); \
} type##PointerMethods

extern DEFINE_POINTER_METHODS(void);

#define DEFINE_INSTANCE_METHOD_WRAPPER(type) \
static inline Pointer(type) _##type##pointer_Instance(Pointer(type) instance)\
{\
	return (Pointer(type))Pointers(void).Instance((Pointer(void))instance);\
}

#define DEFINE_CREATE_METHOD_WRAPPER(type)\
static inline Pointer(type) _##type##pointer_Create(type* resourceToInstance)\
{\
	return (Pointer(type))Pointers(void).Create(resourceToInstance);\
}

#define DEFINE_DISPOSE_METHOD_WRAPPER(type)\
static inline void _##type##pointer_Dispose(Pointer(type) ptr, void* state, void(*OnDispose)(Pointer(type), void* state))\
{\
	Pointers(void).Dispose((Pointer(void))ptr, state, OnDispose);\
}

#define DEFINE_METHOD_WRAPPERS(type) DEFINE_DISPOSE_METHOD_WRAPPER(type); DEFINE_CREATE_METHOD_WRAPPER(type); DEFINE_INSTANCE_METHOD_WRAPPER(type);

#define DEFINE_POINTER(type) \
DEFINE_POINTER_STRUCT(type);\
DEFINE_METHOD_WRAPPERS(type);\
static DEFINE_POINTER_METHODS(type) = { \
	.Create = _##type##pointer_Create,\
	.Dispose = _##type##pointer_Dispose,\
	.Instance = _##type##pointer_Instance\
};
