#pragma once

#include "core/csharp.h"
#include "memory.h"
#include "pointer.h"

// TEMPLATE
#define _ARRAY_DEFINE_STRUCT(type) struct _array_##type\
{\
	/* The pointer to the backing array */\
	type* Values;\
	/* The size, in bytes, of the backing array*/\
	size_t Size;\
	/* The size in bytes between elements of the array\
	// This would typically be the size of the element stored */\
	size_t ElementSize;\
	/* the size in elements that this array can store\
	this is the same as Size/ElementSize */\
	size_t Capacity;\
	/* The number of elements stored in the array */\
	size_t Count;\
	/* The Hash of the elements of this array */\
	size_t Hashcode; \
	/* The typeid of the element stored in the array */\
	size_t TypeId; \
}; \
typedef struct _array_##type* type##_array;

#define MAKE_CONST_ARRAY(type,count, ...) &(struct _array_##type)\
{\
	/* The pointer to the backing array */\
	.Values = (type*) __VA_ARGS__,\
	/* The size, in bytes, of the backing array*/\
	.Size = (sizeof(type) * (count)),\
	/* The size in bytes between elements of the array\
	// This would typically be the size of the element stored */\
	.ElementSize = sizeof(type),\
	/* the size in elements that this array can store\
	this is the same as Size/ElementSize */\
	.Capacity = count,\
	/* The number of elements stored in the array */\
	.Count = count,\
	.Hashcode = 0,\
	/* The typeid of the element stored in the array */\
	.TypeId = 0\
}

#define MAKE_CONST_CHAR_ARRAY(string) MAKE_CONST_ARRAY(char, (sizeof(string) / sizeof(char)) - 1, string)

#define _ARRAY(type) type##_array
// Creates an array of the given type, remember to define the type if this fails to compile
#define ARRAY(type) _ARRAY(type)
#define _ARRAYS(type) type##_array##Arrays
#define ARRAYS(type) _ARRAYS(type)

_ARRAY_DEFINE_STRUCT(void);
typedef ARRAY(void) Array;

struct _arrayMethods
{
	Array(*Create)(size_t elementSize, size_t count, size_t typeId);
	Array(*CreateFromCArray)(const void* cArray, size_t elementSize, size_t count, size_t typeId);
	void (*AutoResize)(Array);
	void (*Resize)(Array, size_t newCount);
	// Appends the given item to the end of the array
	void (*Append)(Array, void*);
	// Removes the given index, moving all contents to the left
	// in its place
	void (*RemoveIndex)(Array, size_t index);
	// Swaps the positions of two elements
	void (*Swap)(Array, size_t firstIndex, size_t secondIndex);
	// Insertion sorts given the provided comparator Func
	void (*InsertionSort)(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
	// Gets a pointer to the value contained at index
	void* (*At)(Array, size_t index);
	// Appends the given value array to the end of the given array
	void (*AppendArray)(Array array, Array appendedValue);
	void (*Clear)(Array array);
	void (*Foreach)(Array, void(*method)(void*));
	void (*ForeachWithContext)(Array, void* context, void(*method)(void* context, void* item));
	// Clones the provided array with no reference to the old array
	Array(*Clone)(Array);
	// Checks if the two arrays are the same
	bool (*Equals)(Array, Array);
	// Hashes the array bytewise
	size_t(*Hash)(Array);
	void (*Dispose)(Array);
};

extern const struct _arrayMethods Arrays;

// TEMPLATE FOR METHODS
#define DEFINE_ARRAY(type) _ARRAY_DEFINE_STRUCT(type)\
DEFINE_TYPE_ID(type##_array); \
private ARRAY(type) _array_##type##_Create(size_t count)\
{\
REGISTER_TYPE(type##_array); \
return (ARRAY(type))Arrays.Create(sizeof(type), count, type##_arrayTypeId); \
}\
private ARRAY(type) _array_##type##_CreateFromCArray(const type* carray, size_t count)\
{\
REGISTER_TYPE(type##_array); \
return (ARRAY(type))Arrays.CreateFromCArray(carray, sizeof(type), count, type##_arrayTypeId); \
}\
private void _array_##type##_AutoResize(ARRAY(type) array)\
{\
Arrays.AutoResize((Array)array); \
}\
private void _array_##type##_Resize(ARRAY(type) array, size_t newCount)\
{\
Arrays.Resize((Array)array, newCount); \
}\
private void _array_##type##_Append(ARRAY(type)array, type value)\
{\
Arrays.Append((Array)array, &value); \
}\
private void _array_##type##_RemoveIndex(ARRAY(type) array, size_t index)\
{\
Arrays.RemoveIndex((Array)array, index); \
}\
private void _array_##type##_Swap(ARRAY(type) array, size_t firstIndex, size_t secondIndex)\
{\
Arrays.Swap((Array)array, firstIndex, secondIndex); \
}\
private void _array_##type##_InsertionSort(ARRAY(type) array, bool(comparator)(type* leftMemoryBlock, type* rightMemoryBlock))\
{\
Arrays.InsertionSort((Array)array, comparator); \
}\
private type* _array_##type##_At(ARRAY(type) array, size_t index)\
{\
return (type*)Arrays.At((Array)array, index); \
}\
private void _array_##type##_AppendArray(ARRAY(type) array, ARRAY(type) appendedValue)\
{\
Arrays.AppendArray((Array)array, (Array)appendedValue); \
}\
private void _array_##type##_Clear(ARRAY(type)array)\
{\
Arrays.Clear((Array)array); \
}\
private bool _array_##type##_Contains(ARRAY(type) array, type value)\
{\
for (size_t i = 0; i < array->Count; i++)\
{\
	if (memcmp(&value, _array_##type##_At(array, i), array->ElementSize) == 0)\
	{\
		return true;\
	}\
}\
return false;\
}\
private ARRAY(type) _array_##type##_Clone(ARRAY(type) array)\
{\
	return (ARRAY(type))Arrays.Clone((Array)array);\
}\
private void _array_##type##_Foreach(ARRAY(type)array, void(*method)(type*))\
{\
Arrays.Foreach((Array)array, method); \
}\
private void _array_##type##_ForeachWithContext(ARRAY(type)array, void* context, void(*method)(void* context, type* item))\
{\
Arrays.ForeachWithContext((Array)array, context, method); \
}\
private bool _array_##type##_Equals(ARRAY(type)left, ARRAY(type)right)\
{\
return Arrays.Equals((Array)left,(Array)right);\
}\
private size_t _array_##type##_Hash(ARRAY(type)array)\
{\
return Arrays.Hash((Array)array); \
}\
private void _array_##type##_Dispose(ARRAY(type)array)\
{\
Arrays.Dispose((Array)array); \
}\
const static struct _array_##type##_methods\
{\
ARRAY(type) (*Create)(size_t count); \
ARRAY(type) (*CreateFromCArray)(const type* cArray, size_t count); \
void (*AutoResize)(ARRAY(type)); \
void (*Resize)(ARRAY(type), size_t newCount); \
void (*Append)(ARRAY(type), type); \
void (*RemoveIndex)(ARRAY(type), size_t index); \
void (*Swap)(ARRAY(type), size_t firstIndex, size_t secondIndex); \
void (*InsertionSort)(ARRAY(type), bool(comparator)(type* left, type* right)); \
type* (*At)(ARRAY(type), size_t index); \
void (*AppendArray)(ARRAY(type), ARRAY(type) appendedValue); \
void (*Clear)(ARRAY(type)); \
bool (*Contains)(ARRAY(type), type value); \
void (*Foreach)(ARRAY(type), void(*method)(type*)); \
void (*ForeachWithContext)(ARRAY(type), void* context, void(*method)(void*, type*)); \
ARRAY(type) (*Clone)(ARRAY(type)); \
bool (*Equals)(ARRAY(type), ARRAY(type)); \
size_t(*Hash)(ARRAY(type)); \
void (*Dispose)(ARRAY(type)); \
} type##_array##Arrays = \
{\
.Create = _array_##type##_Create, \
.CreateFromCArray = _array_##type##_CreateFromCArray, \
.AutoResize = _array_##type##_AutoResize, \
.Resize = _array_##type##_Resize, \
.Append = _array_##type##_Append, \
.RemoveIndex = _array_##type##_RemoveIndex, \
.Swap = _array_##type##_Swap, \
.InsertionSort = _array_##type##_InsertionSort, \
.At = _array_##type##_At, \
.AppendArray = _array_##type##_AppendArray, \
.Clear = _array_##type##_Clear, \
.Contains = _array_##type##_Contains, \
.Foreach = _array_##type##_Foreach, \
.ForeachWithContext = _array_##type##_ForeachWithContext, \
.Clone = _array_##type##_Clone, \
.Equals = _array_##type##_Equals, \
.Hash = _array_##type##_Hash, \
.Dispose = _array_##type##_Dispose\
};

// define common types
DEFINE_ARRAY(bool);
DEFINE_ARRAY(char);
DEFINE_ARRAY(int);
DEFINE_ARRAY(float);
DEFINE_ARRAY(double);
DEFINE_ARRAY(size_t);

DEFINE_ARRAY(char_array);

#define DEFINE_TUPLE_FULL(left,leftname,right,rightname) struct tuple_##left##_##right{ left leftname;right rightname;}
#define DEFINE_TUPLE(left,right) DEFINE_TUPLE_FULL(left,First,right,Second)
#define tuple(left,right) struct tuple_##left##_##right

#define DEFINE_TUPLE_BOTH_WAYS(T1,T2) DEFINE_TUPLE(T1,T2); DEFINE_TUPLE(T2,T1);
#define DEFINE_TUPLE_ALL(major,T1,T2,T3,T4,T5,T6) DEFINE_TUPLE(major,major);DEFINE_TUPLE(major, T1);DEFINE_TUPLE(major, T2);DEFINE_TUPLE(major, T3);DEFINE_TUPLE(major, T4);DEFINE_TUPLE(major, T5);DEFINE_TUPLE(major, T6);

DEFINE_TUPLE_ALL(bool, char, int, long, size_t, float, double)
DEFINE_TUPLE_ALL(char, bool, int, long, size_t, float, double)
DEFINE_TUPLE_ALL(int, bool, char, long, size_t, float, double)
DEFINE_TUPLE_ALL(long, bool, char, int, size_t, float, double)
DEFINE_TUPLE_ALL(size_t, bool, char, int, long, float, double)
DEFINE_TUPLE_ALL(float, bool, char, int, long, size_t, double)
DEFINE_TUPLE_ALL(double, bool, char, int, long, size_t, float)

#define DEFINE_TUPLE_ALL_INTRINSIC(type)\
DEFINE_TUPLE_BOTH_WAYS(type,bool);\
DEFINE_TUPLE_BOTH_WAYS(type,char);\
DEFINE_TUPLE_BOTH_WAYS(type,int);\
DEFINE_TUPLE_BOTH_WAYS(type,long);\
DEFINE_TUPLE_BOTH_WAYS(type,size_t);\
DEFINE_TUPLE_BOTH_WAYS(type,float);\
DEFINE_TUPLE_BOTH_WAYS(type,double);\

#pragma warning(disable:4113)
DEFINE_POINTER(_Bool);
DEFINE_POINTER(char);
DEFINE_POINTER(int);
DEFINE_POINTER(short);
DEFINE_POINTER(long);
DEFINE_POINTER(size_t);
DEFINE_POINTER(float);
DEFINE_POINTER(double);
#pragma warning(default: 4113)

#define DEFINE_CONTAINERS(type) __pragma(warning(disable:4113)); DEFINE_ARRAY(type); DEFINE_TUPLE(type,type); DEFINE_TUPLE_ALL_INTRINSIC(type); DEFINE_POINTER(type); DEFINE_ARRAY(type##Pointer); __pragma(warning(default:4113))