#include "core/hashing.h"
#include "core/csharp.h"

private size_t Hash(const char* bytes);
private size_t ChainHash(const char* bytes, const size_t previousHash);
private size_t HashSafe(const char* bytes, size_t size);
private size_t ChainHashSafe(const char* bytes, const size_t size, const size_t previousHash);
private size_t ChainHashSingle(const char byte, const size_t previousHash);

const struct _hashingMethods Hashing = {
	.Hash = &Hash,
	.ChainHash = &ChainHash,
	.HashSafe = HashSafe,
	.ChainHashSafe = ChainHashSafe,
	.ChainHashSingle = ChainHashSingle
};

private size_t ChainHashSingle(const char byte, const size_t previousHash)
{
	size_t hash = previousHash is 0 ? 5381 : previousHash;

	hash = ((hash << 5) + hash) + byte; /* hash * 33 + c */

	return hash;
}

private size_t ChainHashSafe(const char* bytes, const size_t size, const size_t previousHash)
{
	// modified djb2 hash
	/*http://www.cse.yorku.ca/~oz/hash.html*/

	size_t hash = previousHash is 0 ? 5381 : previousHash;

	for (size_t i = 0; i < size; i++)
	{
		const int c = bytes[i];

		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}


private size_t HashSafe(const char* bytes, size_t size)
{
	return ChainHashSafe(bytes, size, 0);
}

private size_t ChainHash(const char* bytes, const size_t previousHash)
{
	// modified djb2 hash
	/*http://www.cse.yorku.ca/~oz/hash.html*/

	size_t hash = previousHash is 0 ? 5381 : previousHash;

	int c;

#pragma warning (disable : 4706)
	while (c = *bytes++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
#pragma warning (default : 4706)

	return hash;
}

private size_t Hash(const char* bytes)
{
	return ChainHash(bytes, 0);
}