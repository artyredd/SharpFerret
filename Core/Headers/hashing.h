#pragma once
#include <stdlib.h>

struct _hashingMethods {
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Hashes the provided bytes
	/// </summary>
	size_t (*Hash)(const char* bytes);
	/// <summary>
	/// NON-CRYPTOGRAPHIC; Used to chain multiple hashes in a row to produce deterministic hashing using multiple sets
	/// of char*
	/// </summary>
	size_t(*ChainHash)(const char* bytes, const size_t previousHash);
};

extern const struct _hashingMethods Hashing;