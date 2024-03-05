#pragma once

#ifndef _stdio_h_
#include <stdio.h>
#endif // !_stdio_h_

#include "core/csharp.h"

struct _memoryMethods {
	ulong FreeCount;
	ulong AllocCount;
	ulong AllocSize;
	void (*PrintAlloc)(FILE* stream);
	void (*PrintFree)(FILE* stream);
	// MemoryID for a generic block of memory with no associated type
	const ulong GenericMemoryBlock;
	// MemoryId for a string
	const ulong String;
	/// <summary>
	/// Safely allocates the provided size of memory, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use Memory.AllocCount for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
	/// typeID is the id of the registed type name
	/// </summary>
	void* (*Alloc)(ulong size, ulong typeID);
	// Safely frees the provided block of memory, use Memory.FreeCount to get number of times this method was invoked
	void (*Free)(void* address, ulong typeID);
	// This function returns a pointer to the allocated memory, or NULL if the request fails.
	void* (*Calloc)(ulong nitems, ulong size, ulong typeID);
	/// <summary>
	/// Safely allocates the provided size of memory with the provided alignment, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use Memory.AllocCount for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
	/// </summary>
	void* (*AllocAligned)(ulong alignment, ulong size, ulong typeID);

	// Attempts to realloc the address to the new size, returns true if successfull, otherwise false
	bool (*TryRealloc)(void* address, const ulong previousSize, const ulong newSize, void** out_address);

	// Fills the array with zeros
	void (*ZeroArray)(void* address, const ulong size);

	// Duplicates the provided address with length, allocs a new address with newLength bytes and copies length bytes to the new address,
	// returns the new address
	void* (*DuplicateAddress)(const void* address, const ulong length, const ulong newLength, ulong typeID);

	// Attempts to realloc, or copy previousLength bytes to a new address, sets the pointer provided to the new address
	// returns TRUE(1) when realloced, FALSE(0) when the bytes were copied
	bool (*ReallocOrCopy)(void** address, ulong previousLength, ulong newLength, ulong typeID);

	// registers the provided typename and returns the id that should be passed into the Alloc() method
	void(*RegisterTypeName)(const char* name, ulong* out_typeId);
	// performs memcmp while also performing a hash at the same time
	int (*CompareMemoryAndHash)(const char* left, ulong leftSize, const char* right, ulong rightSize, ulong* out_leftHash, ulong* out_rightHash);
};

extern struct _memoryMethods Memory;

// generates a static type id storage location for the given type name
#define DEFINE_TYPE_ID(name) static ulong name##TypeId = 0
#define _EXPAND_typeid(type) type##TypeId 
#define typeid(type) _EXPAND_typeid(type)

// registers the provided name with the memory handler
#define REGISTER_TYPE(name) Memory.RegisterTypeName(#name, &name##TypeId)

DEFINE_TYPE_ID(char);
DEFINE_TYPE_ID(int);
DEFINE_TYPE_ID(long);
DEFINE_TYPE_ID(ulong);
DEFINE_TYPE_ID(double);
DEFINE_TYPE_ID(float);
DEFINE_TYPE_ID(bool);