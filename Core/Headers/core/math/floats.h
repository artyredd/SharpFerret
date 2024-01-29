#pragma once
#include "core/csharp.h"
#include "core/file.h"

struct _floatMethods {
	bool (*TryDeserialize)(const char* buffer, size_t bufferLength, float* out_float);
	void(*SerializeStream)(File stream, float value);
};

extern const struct _floatMethods Floats;