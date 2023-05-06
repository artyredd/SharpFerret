#pragma once

#include "csharp.h"

// TEMPLATE
#define DEFINE_ARRAY(type) struct _array_##type\
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

DEFINE_ARRAY(void)

typedef ARRAY(void) Array;

extern const struct _arrayMethods
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
	void (*InsertionSort)(Array, int(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
	void (*Dispose)(Array);
} Arrays;