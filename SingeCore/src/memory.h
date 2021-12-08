#pragma once

#ifndef _stdio_h_
#include <stdio.h>
#endif // !_stdio_h_


#ifdef DISABLE_OVERRIDE_ALLOC
#else
#define malloc(size) SafeAlloc(size)
#define free(address) SafeFree(address)
#define calloc(nitems,size) SafeCalloc(nitems,size)
#endif

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