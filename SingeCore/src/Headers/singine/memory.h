#pragma once

#ifndef _stdio_h_
#include <stdio.h>
#endif // !_stdio_h_

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

/// <summary>
/// Safely allocates the provided size of memory, otherwise throws OutOfMemoryException and exits program forcibly, 
/// use AllocCount() for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
/// </summary>
void* SafeAlloc(size_t size);

// Safely frees the provided block of memory, use FreeCount() to get number of times this method was invoked
void SafeFree(void* address);

// This function returns a pointer to the allocated memory, or NULL if the request fails.
void* SafeCalloc(size_t nitems, size_t size);

/// <summary>
/// Safely allocates the provided size of memory with the provided alignment, otherwise throws OutOfMemoryException and exits program forcibly, 
/// use AllocCount() for the number of calls to this method, use AllocSize() for the amount of memory allocated using this method
/// </summary>
void* SafeAllocAligned(size_t alignment, size_t size);

// Attempts to realloc the address to the new size, returns true if successfull, otherwise false
int TryRealloc(void* address, const size_t previousSize, const size_t newSize, void** out_address);

// Fills the array with zeros
void ZeroArray(void* address, const size_t size);

// Duplicates the provided address with length, allocs a new address with newLength bytes and copies length bytes to the new address,
// returns the new address
void* DuplicateAddress(const void* address, const size_t length, const size_t newLength);

// Attempts to realloc, or copy previousLength bytes to a new address, sets the pointer provided to the new address
// returns TRUE(1) when realloced, FALSE(0) when the bytes were copied
int ReallocOrCopy(void** address, size_t previousLength, size_t newLength);