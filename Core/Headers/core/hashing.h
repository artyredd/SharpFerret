#pragma once
#include <stdlib.h>
#include "core/csharp.h"

struct _hashingMethods {
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Hashes the provided bytes
	/// </summary>
	ulong(*Hash)(const char* bytes);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Hashes the provided bytes
	/// </summary>
	ulong(*HashSafe)(const char* bytes, ulong size);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Used to chain multiple hashes in a row to produce deterministic hashing using multiple sets
	/// of char*
	/// </summary>
	ulong(*ChainHash)(const char* bytes, const ulong previousHash);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Used to chain multiple hashes in a row to produce deterministic hashing using multiple sets
	/// of char*
	/// </summary>
	ulong(*ChainHashSafe)(const char* bytes, const ulong size, const ulong previousHash);
	// Chain hashes a single byte
	ulong(*ChainHashSingle)(const char byte, const ulong previousHash);
};

extern const struct _hashingMethods Hashing;