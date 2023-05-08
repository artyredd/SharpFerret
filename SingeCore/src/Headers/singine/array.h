#pragma once

#include "csharp.h"
#include "memory.h"

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
	/* The typeid of the element stored in the array */\
	size_t TypeId;\
};\
typedef struct _array_##type* type##_array; 

#define ARRAY(type) type##_array
#define ARRAYS(type) type##_array##Arrays

_ARRAY_DEFINE_STRUCT(void);
typedef ARRAY(void) Array;

struct _arrayMethods
{
	Array(*Create)(size_t elementSize, size_t count, size_t typeId);
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
	void (*Dispose)(Array);
};

extern const struct _arrayMethods Arrays;

// TEMPLATE FOR METHODS
#define DEFINE_ARRAY(type) _ARRAY_DEFINE_STRUCT(type)\
TYPE_ID(type##_array); \
private ARRAY(type) _array_##type##_Create(size_t count)\
{\
REGISTER_TYPE(type##_array); \
return (ARRAY(type))Arrays.Create(sizeof(type), count, type##_arrayTypeId); \
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
private void _array_##type##_Dispose(ARRAY(type)array)\
{\
Arrays.Dispose((Array)array); \
}\
const static struct _array_##type##_methods\
{\
ARRAY(type) (*Create)(size_t count); \
void (*AutoResize)(ARRAY(type)); \
void (*Resize)(ARRAY(type), size_t newCount); \
void (*Append)(ARRAY(type), type); \
void (*RemoveIndex)(ARRAY(type), size_t index); \
void (*Swap)(ARRAY(type), size_t firstIndex, size_t secondIndex); \
void (*InsertionSort)(ARRAY(type), bool(comparator)(type* leftMemoryBlock, type* rightMemoryBlock)); \
type* (*At)(ARRAY(type), size_t index); \
void (*AppendArray)(ARRAY(type), ARRAY(type) appendedValue); \
void (*Dispose)(ARRAY(type)); \
} type##_array##Arrays = \
{\
.Create = _array_##type##_Create, \
.AutoResize = _array_##type##_AutoResize, \
.Resize = _array_##type##_Resize, \
.Append = _array_##type##_Append, \
.RemoveIndex = _array_##type##_RemoveIndex, \
.Swap = _array_##type##_Swap, \
.InsertionSort = _array_##type##_InsertionSort, \
.At = _array_##type##_At, \
.AppendArray = _array_##type##_AppendArray, \
.Dispose = _array_##type##_Dispose\
};


DEFINE_ARRAY(int);