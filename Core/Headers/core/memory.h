#pragma once

#ifndef _stdio_h_
#include <stdio.h>
#endif // !_stdio_h_

#include "core/csharp.h"

struct _memoryMethods {
	size_t FreeCount;
	size_t AllocCount;
	size_t AllocSize;
	void (*PrintAlloc)(FILE* stream);
	void (*PrintFree)(FILE* stream);
	// MemoryID for a generic block of memory with no associated type
	const size_t GenericMemoryBlock;
	// MemoryId for a string
	const size_t String;
	/// <summary>
	/// Safely allocates the provided size of memory, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use Memory.AllocCount for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
	/// typeID is the id of the registed type name
	/// </summary>
	void* (*Alloc)(size_t size, size_t typeID);
	// Safely frees the provided block of memory, use Memory.FreeCount to get number of times this method was invoked
	void (*Free)(void* address, size_t typeID);
	// This function returns a pointer to the allocated memory, or NULL if the request fails.
	void* (*Calloc)(size_t nitems, size_t size, size_t typeID);
	/// <summary>
	/// Safely allocates the provided size of memory with the provided alignment, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use Memory.AllocCount for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
	/// </summary>
	void* (*AllocAligned)(size_t alignment, size_t size, size_t typeID);

	// Attempts to realloc the address to the new size, returns true if successfull, otherwise false
	bool (*TryRealloc)(void* address, const size_t previousSize, const size_t newSize, void** out_address);

	// Fills the array with zeros
	void (*ZeroArray)(void* address, const size_t size);

	// Duplicates the provided address with length, allocs a new address with newLength bytes and copies length bytes to the new address,
	// returns the new address
	void* (*DuplicateAddress)(const void* address, const size_t length, const size_t newLength, size_t typeID);

	// Attempts to realloc, or copy previousLength bytes to a new address, sets the pointer provided to the new address
	// returns TRUE(1) when realloced, FALSE(0) when the bytes were copied
	bool (*ReallocOrCopy)(void** address, size_t previousLength, size_t newLength, size_t typeID);

	// registers the provided typename and returns the id that should be passed into the Alloc() method
	void(*RegisterTypeName)(const char* name, size_t* out_typeId);
	// performs memcmp while also performing a hash at the same time
	int (*CompareMemoryAndHash)(const char* left, size_t leftSize, const char* right, size_t rightSize, size_t* out_leftHash, size_t* out_rightHash);
};

extern struct _memoryMethods Memory;

// generates a static type id storage location for the given type name
#define DEFINE_TYPE_ID(name) static size_t name##TypeId = 0
#define _EXPAND_typeid(type) type##TypeId 
#define typeid(type) _EXPAND_typeid(type)

// registers the provided name with the memory handler
#define REGISTER_TYPE(name) Memory.RegisterTypeName(#name, &name##TypeId)

DEFINE_TYPE_ID(char);
DEFINE_TYPE_ID(int);
DEFINE_TYPE_ID(long);
DEFINE_TYPE_ID(size_t);
DEFINE_TYPE_ID(double);
DEFINE_TYPE_ID(float);
DEFINE_TYPE_ID(bool);