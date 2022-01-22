#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "csharp.h"

static char GetByteGrouping(size_t value);
static void PrintGroupedNumber(FILE* stream, size_t value);

// Total amount of memory (bytes) allocated
size_t ALLOC_SIZE;
// Total amount of calls made to malloc()
size_t ALLOC_COUNT;
// Total amount of calls made to free()
size_t FREE_COUNT;

/// <summary>
/// The number of calls made to free
/// </summary>
size_t FreeCount(void) { return FREE_COUNT; }

/// <summary>
/// The number of calls to alloc made
/// </summary>
size_t AllocCount(void) { return ALLOC_COUNT; }

/// <summary>
/// The total number in bytes that has been allocated using SafeAlloc()
/// </summary>
size_t AllocSize(void) { return ALLOC_SIZE; }

/// <summary>
/// Resets the state of SafeAlloc allocation statistics; AllocSize(), AllocCount()
/// </summary>
void ResetAlloc(void) { ALLOC_COUNT = ALLOC_SIZE = 0; }

/// <summary>
/// Resets the state of SafeFree statistics; FreeCount()
/// </summary>
void ResetFree(void) { FREE_COUNT = 0; }

/// <summary>
/// Prints the current allocation statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
void PrintAlloc(FILE* stream)
{
	// determine how to shorten the number of bytes
	fprintf(stream, "Allocated ");
	PrintGroupedNumber(stream, ALLOC_SIZE);
	fprintf(stream, " (%lli) ", ALLOC_COUNT);
}

/// <summary>
/// Prints the current free statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
void PrintFree(FILE* stream)
{
	fprintf(stream, "Free Count (%lli) ", FREE_COUNT);
}

/// <summary>
/// This function returns a pointer to the allocated memory, or NULL if the request fails.
/// </summary>
/// <param name="nitems">This is the number of elements to be allocated.</param>
/// <param name="size">This is the size of elements.</param>
/// <returns>This function returns a pointer to the allocated memory, or NULL if the request fails.</returns>
void* SafeCalloc(size_t nitems, size_t size)
{
	void* ptr = calloc(nitems, size);

	if (ptr is null)
	{
		throw(OutOfMemoryException);
	}

	ALLOC_SIZE += nitems * size;

	++ALLOC_COUNT;

	return ptr;
}

void* SafeAlloc(size_t size)
{
	if (size is 0)
	{
		return null;
	}

	void* ptr = calloc(1, size);

	if (ptr is null)
	{
		throw(OutOfMemoryException);
	}

	ALLOC_SIZE += size;

	++ALLOC_COUNT;

	return ptr;
}

void* SafeAllocAligned(size_t alignment, size_t size)
{
	void* ptr = _aligned_malloc(alignment, size);

	if (ptr is null)
	{
		throw(OutOfMemoryException);
	}

	ALLOC_SIZE += size;

	++ALLOC_COUNT;

	return ptr;
}

void SafeFree(void* address)
{
	if (address is null)
	{
		return;
	}

	free(address);

	++FREE_COUNT;
}

bool TryRealloc(void* address, const size_t previousSize, const size_t newSize, void** out_address)
{
	void* newAddress = realloc(address, newSize);

	*out_address = newAddress;


	if (newAddress isnt null)
	{
		// set new bytes to zero
		if (previousSize < newSize)
		{
			char* offset = (char*)newAddress + previousSize;
			memset(offset, 0, newSize - previousSize);
		}

		return true;
	}

	return false;
}

void ZeroArray(void* address, const size_t size)
{
	memset(address, 0, size);
}

void* DuplicateAddress(const void* address, const  size_t length, const  size_t newLength)
{
	if (address is null)
	{
		throw(NullReferenceException);
	}

	// gaurd sensless duplications
	if (length is 0 or newLength is 0)
	{
		throw(InvalidLogicException);
	}

	void* newAddress = SafeAlloc(newLength);

	memcpy(newAddress, address, min(length, newLength));

	return newAddress;
}

bool ReallocOrCopy(void** address, const size_t previousLength, const size_t newLength)
{
	if (*address is null)
	{
		*address = SafeAlloc(newLength);

		return true;
	}

	if (TryRealloc(address, previousLength, newLength, address))
	{
		return true;
	}

	// since we couldn't realloc to the right size alloc new space and copy the bytes
	void* newAddress = DuplicateAddress(address, previousLength, newLength);

	// free the old address
	SafeFree(*address);

	*address = newAddress;

	return false;
}

static void PrintGroupedNumber(FILE* stream, size_t value)
{


	static size_t ByteGroupingDivisors[5] =
	{
	#if _WIN32
		1,
		1024,
		1024 * 1024,
		1024 * 1024 * 1024,
		(size_t)1024 * 1024 * 1024 * 1024,
	#else
		1,
		1000,
		1000 * 1000,
		1000 * 1000 * 1000,
		(size_t)1000 * 1000 * 1000 * 1000,
	#endif
	};



	static char* ByteGroupingNames[5] = {
		"bytes",
		"kb",
		"mb",
		"gb",
		"tb"
	};

	int grouping = GetByteGrouping(value);

	char* name = ByteGroupingNames[grouping];

#if _WIN32

	size_t groupedValue = value / ByteGroupingDivisors[2];

#else

	size_t divisor = ByteGroupingDivisors[grouping];

	size_t groupedValue = value / divisor;

#endif

	fprintf(stream, "%lli %s", groupedValue, name);
}

/// <summary>
/// Returns the grouping bracket the provided value is in
/// <para>&lt; 1KB -&gt; 0</para>
/// <para>&lt; 1MB -&gt; 1</para>
/// <para>&lt; 1GB -&gt; 2</para>
/// <para>&lt; 1TB -&gt; 3</para>
/// <para>&gt;= 1TB -&gt; 4</para>
/// </summary>
static char GetByteGrouping(size_t value)
{
#pragma warning(disable: 4127)
	if (sizeof(size_t) != 8)
	{
		fprintf(stderr, "Failed to get the grouping of value, size_t assumed to be 64 bit unsigned long long int");
		throw(InvalidArgumentException);
	}
#pragma warning(default: 4127)
#if _WIN32
	// < 1Kb return b
	if (value < (1 * 1024)) return 0;
	// < 1Mb return Kb
	if (value < (1 * 1024 * 1024)) return 1;
	// < 1Gb return Mb
	if (value < (1 * 1024 * 1024 * 1024)) return 2;
	// < 1Tb return Gb
	if (value < ((size_t)1 * 1024 * 1024 * 1024 * 1024)) return 3;

	// if >= TB return TB
	return 4;
#else
	// < 1KB return B
	if (value < 1 * 1000) return 0;
	// < 1MB return KB
	if (value < 1 * 1000 * 1000) return 1;
	// < 1GB return MB
	if (value < 1 * 1000 * 1000 * 1000) return 2;
	// < 1TB return GB
	if (value < (size_t)1 * 1000 * 1000 * 1000 * 1000) return 3;

	// if >= TB return TB
	return 4;
#endif
}