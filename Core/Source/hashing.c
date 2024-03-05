#include "core/hashing.h"
#include "core/csharp.h"

private ulong Hash(const char* bytes);
private ulong ChainHash(const char* bytes, const ulong previousHash);
private ulong HashSafe(const char* bytes, ulong size);
private ulong ChainHashSafe(const char* bytes, const ulong size, const ulong previousHash);
private ulong ChainHashSingle(const char byte, const ulong previousHash);

const struct _hashingMethods Hashing = {
	.Hash = &Hash,
	.ChainHash = &ChainHash,
	.HashSafe = HashSafe,
	.ChainHashSafe = ChainHashSafe,
	.ChainHashSingle = ChainHashSingle
};

private ulong ChainHashSingle(const char byte, const ulong previousHash)
{
	ulong hash = previousHash is 0 ? 5381 : previousHash;

	hash = ((hash << 5) + hash) + byte; /* hash * 33 + c */

	return hash;
}

private ulong ChainHashSafe(const char* bytes, const ulong size, const ulong previousHash)
{
	// modified djb2 hash
	/*http://www.cse.yorku.ca/~oz/hash.html*/

	ulong hash = previousHash is 0 ? 5381 : previousHash;

	for (ulong i = 0; i < size; i++)
	{
		const int c = bytes[i];

		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}


private ulong HashSafe(const char* bytes, ulong size)
{
	return ChainHashSafe(bytes, size, 0);
}

private ulong ChainHash(const char* bytes, const ulong previousHash)
{
	// modified djb2 hash
	/*http://www.cse.yorku.ca/~oz/hash.html*/

	ulong hash = previousHash is 0 ? 5381 : previousHash;

	int c;

#pragma warning (disable : 4706)
	while (c = *bytes++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
#pragma warning (default : 4706)

	return hash;
}

private ulong Hash(const char* bytes)
{
	return ChainHash(bytes, 0);
}