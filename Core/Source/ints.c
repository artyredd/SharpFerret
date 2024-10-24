#include "core/math/ints.h"

static bool TryDeserialize(const char* buffer, ulong bufferLength, ulong* out_value);
static void Serialize(File stream, ulong value);

const struct _intMethods Ints = {
	.TryDeserialize = &TryDeserialize,
	.Serialize = &Serialize
};

#pragma warning(disable: 4100) // disable warning for unused bufferLength
static bool TryDeserialize(const char* buffer, ulong bufferLength, ulong* out_value)
{
	int count = sscanf_s(buffer, "%lli", out_value);

	return count == 1;
}
#pragma warning(default: 4100)

static void Serialize(File stream, ulong value)
{
	fprintf(stream, "%lli", value);
}