#pragma once
#include "core/csharp.h"
#include "core/file.h"
#include <float.h>

#define DEFAULT_EPSILON 0.00001
#define approximate(left,right) (fabs(left-right) < DEFAULT_EPSILON)

struct _floatMethods {
	bool (*TryDeserialize)(const char* buffer, ulong bufferLength, float* out_float);
	void(*SerializeStream)(File stream, float value);
};

extern const struct _floatMethods Floats;