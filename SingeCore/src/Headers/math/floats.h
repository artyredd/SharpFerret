#pragma once
#include "csharp.h"

struct _floatMethods {
	bool (*TryDeserialize)(const char* buffer, size_t bufferLength, float* out_float);
};

extern const struct _floatMethods Floats;