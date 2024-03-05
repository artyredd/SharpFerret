#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "core/memory.h"
#include "core/csharp.h"
#include "core/hashing.h"

static char GetByteGrouping(ulong value);
static void PrintGroupedNumber(FILE* stream, ulong value);

static void* SafeAlloc(ulong size, ulong typeID);
static void* SafeCalloc(ulong nitems, ulong size, ulong typeID);
static void* SafeAllocAligned(ulong alignment, ulong size, ulong typeID);
static void SafeFree(void* address, ulong typeID);
static bool TryRealloc(void* address, const ulong previousSize, const ulong newSize, void** out_address);
static void ZeroArray(void* address, const ulong size);
static void* DuplicateAddress(const void* address, const  ulong length, const  ulong newLength, ulong typeID);
static bool ReallocOrCopy(void** address, const ulong previousLength, const ulong newLength, ulong typeID);
static void RegisterTypeName(const char* name, ulong* out_typeId);
private void PrintAlloc(FILE* stream);
private void PrintFree(FILE* stream);
private int CompareMemoryAndHash(const char* left, ulong leftSize, const char* right, ulong rightSize, ulong* out_leftHash, ulong* out_rightHash);


struct _memoryMethods Memory = {
	.AllocSize = 0,
	.AllocSize = 0,
	.GenericMemoryBlock = 0x0,
	.String = 0x01,
	.Alloc = &SafeAlloc,
	.Free = &SafeFree,
	.Calloc = &SafeCalloc,
	.TryRealloc = &TryRealloc,
	.ZeroArray = &ZeroArray,
	.DuplicateAddress = &DuplicateAddress,
	.ReallocOrCopy = &ReallocOrCopy,
	.RegisterTypeName = &RegisterTypeName,
	.PrintAlloc = PrintAlloc,
	.PrintFree = PrintFree,
	.CompareMemoryAndHash = CompareMemoryAndHash
};

#define MAX_TYPENAME_LENGTH 1024
#define MAX_REGISTERED_TYPENAMES 1024

struct _typeName {
	ulong Id;
	const char Name[MAX_TYPENAME_LENGTH];
	// the amount of active instances 
	ulong Active;
	// the amount of freed instances
	ulong Freed;
	// Whether or not this block of the hash table is used
	bool Used;
};

struct _typeName RegisteredTypeNames[MAX_REGISTERED_TYPENAMES] =
{
	{
		.Id = 0,
		.Name = "GenericMemoryBlock",
		.Active = 0,
		.Freed = 0,
		.Used = true
	},
	{
		.Id = 1,
		.Name = "String",
		.Active = 0,
		.Freed = 0,
		.Used = true
	}
};

private int CompareMemoryAndHash(const char* left, ulong leftSize, const char* right, ulong rightSize, ulong* out_leftHash, ulong* out_rightHash)
{
	ulong leftHash = 0;
	ulong rightHash = 0;

	const int length = min(leftSize, rightSize);

	for (ulong i = 0; i < length; i++)
	{
		const char leftByte = left[i];
		const char rightByte = right[i];

		leftHash = Hashing.ChainHashSingle(leftByte, leftHash);
		rightHash = Hashing.ChainHashSingle(rightByte, rightHash);

		if (leftByte != rightByte)
		{
			*out_leftHash = leftHash;
			*out_rightHash = rightHash;

			return leftByte > rightByte ? 1 : -1;
		}
	}

	*out_leftHash = leftHash;
	*out_rightHash = rightHash;

	return 0;
}

/// <summary>
/// Prints the current allocation statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
private void PrintAlloc(FILE* stream)
{
	// determine how to shorten the number of bytes
	fprintf(stream, "Allocated ");
	PrintGroupedNumber(stream, Memory.AllocSize);
	fprintf(stream, " (%lli)\n", Memory.AllocCount);

	// manually register typename for generic memory
	for (ulong i = 0; i < MAX_REGISTERED_TYPENAMES; i++)
	{
		const struct _typeName* typeName = &RegisteredTypeNames[i];

		if (typeName->Used)
		{
			fprintf(stream, "Type: %-32s	Active: %-16lli	Freed: %-16lli\n", typeName->Name, typeName->Active, typeName->Freed);
		}
	}
}

/// <summary>
/// Prints the current free statistics to the provided stream
/// </summary>
/// <param name="stream"></param>
private void PrintFree(FILE* stream)
{
	fprintf(stream, "Free Count (%lli) ", Memory.FreeCount);
}

/// <summary>
/// This function returns a pointer to the allocated memory, or NULL if the request fails.
/// </summary>
/// <param name="nitems">This is the number of elements to be allocated.</param>
/// <param name="size">This is the size of elements.</param>
/// <returns>This function returns a pointer to the allocated memory, or NULL if the request fails.</returns>
static void* SafeCalloc(ulong nitems, ulong size, ulong typeID)
{
	void* ptr = calloc(nitems, size);

	if (ptr is null)
	{
		throw(OutOfMemoryException);
	}

	Memory.AllocSize += nitems * size;

	++Memory.AllocCount;

	const ulong index = typeID % MAX_REGISTERED_TYPENAMES;

	++(RegisteredTypeNames[index].Active);

	return ptr;
}

static void RegisterTypeName(const char* name, ulong* out_typeId)
{
	if (*out_typeId != 0)
	{
		return;
	}

	ulong hash = Hashing.Hash(name);

	ulong index = hash % MAX_REGISTERED_TYPENAMES;

	ulong previousFreed = 0;
	ulong previousActive = 0;

	// make sure we have an unused piece of the hash table
	while (RegisteredTypeNames[index].Used)
	{
		if (RegisteredTypeNames[index].Id == hash)
		{
			previousFreed = RegisteredTypeNames[index].Freed;
			previousActive = RegisteredTypeNames[index].Active;

			break;
		}

		++hash;
		index = hash % MAX_REGISTERED_TYPENAMES;
	}

	// check to see if we already set it
	if (RegisteredTypeNames[index].Id == hash)
	{
		// set out var
		*out_typeId = hash;
	}

	RegisteredTypeNames[index].Id = hash;
	RegisteredTypeNames[index].Freed = previousFreed;
	RegisteredTypeNames[index].Active = previousActive;
	RegisteredTypeNames[index].Used = true;

	ulong length = min(strlen(name), MAX_TYPENAME_LENGTH);

#pragma warning (disable : 4090)
	char* ptr = RegisteredTypeNames[index].Name;
#pragma warning (default : 4090)

	memcpy(ptr, name, length);

	// set out var
	*out_typeId = hash;
}

static void* SafeAlloc(ulong size, ulong typeID)
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

	Memory.AllocSize += size;

	++Memory.AllocCount;

	const ulong index = typeID % MAX_REGISTERED_TYPENAMES;

	++(RegisteredTypeNames[index].Active);

	return ptr;
}

static void* SafeAllocAligned(ulong alignment, ulong size, ulong typeID)
{
	void* ptr = _aligned_malloc(alignment, size);

	if (ptr is null)
	{
		throw(OutOfMemoryException);
	}

	Memory.AllocSize += size;

	++Memory.AllocCount;

	const ulong index = typeID % MAX_REGISTERED_TYPENAMES;

	++(RegisteredTypeNames[index].Active);

	return ptr;
}

static void SafeFree(void* address, ulong typeID)
{
	if (address is null)
	{
		return;
	}

	free(address);

	++Memory.FreeCount;

	const ulong index = typeID % MAX_REGISTERED_TYPENAMES;

	++(RegisteredTypeNames[index].Freed);
}

static bool TryRealloc(void* address, const ulong previousSize, const ulong newSize, void** out_address)
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

static void ZeroArray(void* address, const ulong size)
{
	memset(address, 0, size);
}

static void* DuplicateAddress(const void* address, const  ulong length, const  ulong newLength, ulong typeID)
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

	void* newAddress = SafeAlloc(newLength, typeID);

	memcpy(newAddress, address, min(length, newLength));

	return newAddress;
}

static bool ReallocOrCopy(void** address, const ulong previousLength, const ulong newLength, const ulong typeID)
{
	if (*address is null)
	{
		*address = SafeAlloc(newLength, typeID);

		return true;
	}

	if (TryRealloc(*address, previousLength, newLength, address))
	{
		return true;
	}

	// since we couldn't realloc to the right size alloc new space and copy the bytes
	void* newAddress = DuplicateAddress(*address, previousLength, newLength, typeID);

	// free the old address
	SafeFree(*address, typeID);

	*address = newAddress;

	return false;
}

static void PrintGroupedNumber(FILE* stream, ulong value)
{


	static ulong ByteGroupingDivisors[5] =
	{
	#if _WIN32
		1,
		1024,
		1024 * 1024,
		1024 * 1024 * 1024,
		(ulong)1024 * 1024 * 1024 * 1024,
	#else
		1,
		1000,
		1000 * 1000,
		1000 * 1000 * 1000,
		(ulong)1000 * 1000 * 1000 * 1000,
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

	ulong groupedValue = value / ByteGroupingDivisors[2];

#else

	ulong divisor = ByteGroupingDivisors[grouping];

	ulong groupedValue = value / divisor;

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
static char GetByteGrouping(ulong value)
{
#pragma warning(disable: 4127)
	if (sizeof(ulong) != 8)
	{
		fprintf(stderr, "Failed to get the grouping of value, ulong assumed to be 64 bit unsigned long long int");
		throw(InvalidArgumentException);
	}
#pragma warning(default: 4127)
#if _WIN32
	// < 1Kb return b
	if (value < ((ulong)1 * (ulong)1024)) return 0;
	// < 1Mb return Kb
	if (value < ((ulong)1 * (ulong)1024 * (ulong)1024)) return 1;
	// < 1Gb return Mb
	if (value < ((ulong)1 * (ulong)1024 * (ulong)1024 * (ulong)1024)) return 2;
	// < 1Tb return Gb
	if (value < ((ulong)1 * (ulong)1024 * (ulong)1024 * (ulong)1024 * (ulong)1024)) return 3;

	// if >= TB return TB
	return 4;
#else
	// < 1KB return B
	if (value < (ulong)1 * (ulong)1000) return 0;
	// < 1MB return KB
	if (value < (ulong)1 * (ulong)1000 * (ulong)1000) return 1;
	// < 1GB return MB
	if (value < (ulong)1 * (ulong)1000 * (ulong)1000 * 1000) return 2;
	// < 1TB return GB
	if (value < (ulong)1 * (ulong)1000 * (ulong)1000 * (ulong)1000 * (ulong)1000) return 3;

	// if >= TB return TB
	return 4;
#endif
}