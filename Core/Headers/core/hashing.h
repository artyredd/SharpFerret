#pragma once
#include <stdlib.h>

struct _hashingMethods {
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Hashes the provided bytes
	/// </summary>
	size_t(*Hash)(const char* bytes);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Hashes the provided bytes
	/// </summary>
	size_t(*HashSafe)(const char* bytes, size_t size);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Used to chain multiple hashes in a row to produce deterministic hashing using multiple sets
	/// of char*
	/// </summary>
	size_t(*ChainHash)(const char* bytes, const size_t previousHash);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Used to chain multiple hashes in a row to produce deterministic hashing using multiple sets
	/// of char*
	/// </summary>
	size_t(*ChainHashSafe)(const char* bytes, const size_t size, const size_t previousHash);
	// Chain hashes a single byte
	size_t(*ChainHashSingle)(const char byte, const size_t previousHash);
};

extern const struct _hashingMethods Hashing;