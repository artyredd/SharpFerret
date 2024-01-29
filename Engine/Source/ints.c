#include "math/ints.h"

static bool TryDeserialize(const char* buffer, size_t bufferLength, size_t* out_value);
static void Serialize(File stream, size_t value);

const struct _intMethods Ints = {
	.TryDeserialize = &TryDeserialize,
	.Serialize = &Serialize
};

#pragma warning(disable: 4100) // disable warning for unused bufferLength
static bool TryDeserialize(const char* buffer, size_t bufferLength, size_t* out_value)
{
	int count = sscanf_s(buffer, "%lli", out_value);

	return count == 1;
}
#pragma warning(default: 4100)

static void Serialize(File stream, size_t value)
{
	fprintf(stream, "%lli", value);
}