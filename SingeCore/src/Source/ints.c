#include "math/ints.h"

static bool TryDeserialize(const char* buffer, size_t bufferLength, size_t* out_value);

const struct _intMethods Ints = {
	.TryDeserialize = &TryDeserialize
};

#pragma warning(disable: 4100) // disable warning for unused bufferLength
static bool TryDeserialize(const char* buffer, size_t bufferLength, size_t* out_value)
{
	int count = sscanf_s(buffer, "%lli", out_value);

	return count == 1;
}
#pragma warning(default: 4100)