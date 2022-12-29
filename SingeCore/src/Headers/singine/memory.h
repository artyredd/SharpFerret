#pragma once

#ifndef _stdio_h_
#include <stdio.h>
#endif // !_stdio_h_

#include "csharp.h"

// These perform only helper methods for benchmarking the application

/// <summary>
/// The number of calls made to free
/// </summary>
size_t FreeCount(void);

/// <summary>
/// The number of calls to alloc made
/// </summary>
size_t AllocCount(void);

/// <summary>
/// The total number in bytes that has been allocated using SafeAlloc()
/// </summary>
size_t AllocSize(void);

/// <summary>
/// Resets the state of SafeAlloc allocation statistics; AllocSize(), AllocCount()
/// </summary>
void ResetAlloc(void);

/// <summary>
/// Resets the state of SafeFree statistics; FreeCount()
/// </summary>
void ResetFree(void);

/// <summary>
/// Prints the current allocation statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
void PrintAlloc(FILE* stream);

/// <summary>
/// Prints the current free statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
void PrintFree(FILE* stream);

struct _memoryMethods {
	// MemoryID for a generic block of memory with no associated type
	const size_t GenericMemoryBlock;
	// MemoryId for a string
	const size_t String;
	/// <summary>
	/// Safely allocates the provided size of memory, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use AllocCount() for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
	/// typeID is the id of the registed type name
	/// </summary>
	void* (*Alloc)(size_t size, size_t typeID);
	// Safely frees the provided block of memory, use FreeCount() to get number of times this method was invoked
	void (*Free)(void* address, size_t typeID);
	// This function returns a pointer to the allocated memory, or NULL if the request fails.
	void* (*Calloc)(size_t nitems, size_t size, size_t typeID);
	/// <summary>
	/// Safely allocates the provided size of memory with the provided alignment, otherwise throws OutOfMemoryException and exits program forcibly, 
	/// use AllocCount() for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
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
};

extern const struct _memoryMethods Memory;

// generates a static type id storage location for the given type name
#define TYPE_ID(name) static size_t name##TypeId = 0;

// registers the provided name with the memory handler
#define REGISTER_TYPE(name) Memory.RegisterTypeName(#name, &name##TypeId)