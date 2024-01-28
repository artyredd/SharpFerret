#include "math/floats.h"

bool TryDeserialize(const char* buffer, size_t bufferLength, float* out_float);
static void SerializeStream(File stream, float value);

const struct _floatMethods Floats = {
	.TryDeserialize = &TryDeserialize,
	.SerializeStream = SerializeStream
};

bool TryDeserialize(const char* buffer, size_t bufferLength, float* out_float)
{
	ignore_unused(bufferLength);
	return sscanf_s(buffer, "%f", out_float) == 1;
}

static void SerializeStream(File stream, float value)
{
	fprintf(stream, "%f", value);
}