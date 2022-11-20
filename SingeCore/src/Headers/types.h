#pragma once

#include "csharp.h"

#define _DECLARE_POINTER(RAW_POINTER,NAME,ID) typedef struct _##ID##pointer* NAME##Pointer;\

#define DECLARE_POINTER(NAME,STRUCT_NAME,REFERENCE_NAME) _DECLARE_POINTER(RAW_POINTER,NAME,__COUNTER__)

extern const struct _pointerMethods Pointers;

typedef struct _pointer* Pointer;

struct _pointer
{
	// the id of the type that this pointer represents
	size_t TypeId;
	// the pointer to the block of memory this pointer manages
	size_t Pointer;
	// the size of the memory this block of memory manages
	size_t BlockSize;
};

struct _pointerMethods
{
	// Creates a new pointer object that managers NO memory
	Pointer(*Create)(void);
	// sets the value of the block of memory to the given value
	void (*SetValue)(Pointer, void* value, size_t blockSize);
	bool (*GetValue)(Pointer, void* value);
	void (*Dispose)(Pointer);
};

extern const struct _pointerMethods Pointers;


typedef struct _Material* MyClass;

struct _MyClass
{
	int materialInstance;
};


typedef struct _1pointer* MyClassPointer;

struct _1pointer
{
	struct _pointer;
};

static MyClassPointer _1pointerCreate(void)
{
	return (MyClassPointer)Pointers.Create();
}

static void _1pointerSetValue(MyClassPointer pointer, MyClass value)
{
	Pointers.SetValue( (Pointer)pointer, value, sizeof(struct _MyClass));
}

static bool _1pointerGetValue(MyClassPointer pointer, MyClass out_value)
{
	return Pointers.GetValue( (Pointer)pointer, out_value );
}

static void _1pointerDispose(MyClassPointer pointer)
{
	Pointers.Dispose((Pointer)pointer );
}

static struct _1pointerMethods
{
	// Creates a new pointer object that managers NO memory
	MyClassPointer(*Create)(void);
	// sets the value of the block of memory to the given value
	void (*SetValue)(MyClassPointer, MyClass value);
	bool (*GetValue)(MyClassPointer, MyClass value);
	void (*Dispose)(MyClassPointer);
} MyClassPointers = {
	.Create = _1pointerCreate,
	.Dispose = _1pointerDispose,
	.SetValue = _1pointerSetValue,
	.GetValue = _1pointerGetValue
};