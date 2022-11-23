#pragma once

#include "csharp.h"

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
	// Creates a new pointer object that manages NO memory
	Pointer(*Create)(void);
	// sets the value of the block of memory to the given value
	void (*SetValue)(Pointer, void* value, size_t blockSize);
	bool (*GetValue)(Pointer, void* value);
	void (*Dispose)(Pointer);
};

extern const struct _pointerMethods Pointers;

#define _DECLARE_STRUCT_NAME(ID) struct _##ID##pointer

#define _CLASS_NAME(NAME) NAME##Pointer

#define _DECLARE_TYPEDEF(NAME,ID) typedef _DECLARE_STRUCT_NAME(ID)* _CLASS_NAME(NAME);

#define _DECLARE_STRUCT(ID) _DECLARE_STRUCT_NAME(ID) { struct _pointer; };

#define _METHOD(ID,METHODNAME) _##ID##pointer##METHODNAME

#define _CREATE_NAME(ID) _METHOD(ID,Create)

#define _DECLARE_CREATE(NAME,ID) static _CLASS_NAME(NAME) _CREATE_NAME(ID)(void) { return (_CLASS_NAME(NAME))Pointers.Create(); }

#define _SET_VALUE_NAME(ID) _METHOD(ID,SetValue)

#define _DECLARE_SET_VALUE(REFERENCE_NAME,ID,STRUCT_NAME) static void _SET_VALUE_NAME(ID)(_CLASS_NAME(REFERENCE_NAME) pointer, REFERENCE_NAME value){ Pointers.SetValue((Pointer)pointer, value, sizeof(struct STRUCT_NAME)); }

#define _GET_VALUE_NAME(ID) _METHOD(ID,GetValue)

#define _DECLARE_GET_VALUE(REFERENCE_NAME,ID) static bool _GET_VALUE_NAME(ID)(_CLASS_NAME(REFERENCE_NAME) pointer, REFERENCE_NAME out_value){ return Pointers.GetValue((Pointer)pointer, out_value); }

#define _DISPOSE_NAME(ID) _METHOD(ID,Dispose)

#define _DECLARE_DISPOSE(REFERENCE_NAME,ID) static void _DISPOSE_NAME(ID)(_CLASS_NAME(REFERENCE_NAME) pointer){ Pointers.Dispose((Pointer)pointer ); }

#define _DECLARE_MEMBERS(NAME) _CLASS_NAME(NAME)(*Create)(void); void (*SetValue)(_CLASS_NAME(NAME), NAME value); bool (*GetValue)(_CLASS_NAME(NAME), NAME value); void (*Dispose)(_CLASS_NAME(NAME));

#define _DECLARE_METHODS_STRUCT(REFERENCE_NAME, ID) static struct _##ID##pointerMethods{ _DECLARE_MEMBERS(REFERENCE_NAME) } REFERENCE_NAME##Pointers = { .Create = _CREATE_NAME(ID), .Dispose = _DISPOSE_NAME(ID),	.SetValue = _SET_VALUE_NAME(ID), .GetValue = _GET_VALUE_NAME(ID) };

#define _DECLARE_METHODS(REFERENCE_NAME, STRUCT_NAME, ID)  _DECLARE_CREATE(REFERENCE_NAME,ID); _DECLARE_SET_VALUE(REFERENCE_NAME, ID, STRUCT_NAME); _DECLARE_GET_VALUE(REFERENCE_NAME,ID); _DECLARE_DISPOSE(REFERENCE_NAME,ID);

#define _DEFINE_POINTER(REFERENCE_NAME, STRUCT_NAME, ID) _DECLARE_TYPEDEF(REFERENCE_NAME, ID); _DECLARE_STRUCT(ID); _DECLARE_METHODS(REFERENCE_NAME, STRUCT_NAME, ID); _DECLARE_METHODS_STRUCT(REFERENCE_NAME, ID);

#define DEFINE_POINTER(REFERENCE_NAME, VALUE_NAME) _DEFINE_POINTER(REFERENCE_NAME, VALUE_NAME, __COUNTER__)