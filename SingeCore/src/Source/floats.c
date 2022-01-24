#include "math/floats.h"

bool TryDeserialize(const char* buffer, size_t bufferLength, float* out_float);

const struct _floatMethods Floats = {
	.TryDeserialize = &TryDeserialize
};

#pragma warning(disable: 4100)// disable unused bufferLength warning
bool TryDeserialize(const char* buffer, size_t bufferLength, float* out_float)
{
	return sscanf_s(buffer, "%f", out_float) == 1;
}
#pragma warning(default: 4100)