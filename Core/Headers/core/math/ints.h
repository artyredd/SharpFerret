#pragma once
#include <stdlib.h>
#include "core/file.h"
#include "core/csharp.h"

struct _intMethods {
	bool (*TryDeserialize)(const char* buffer, ulong bufferLength, ulong* out_value);
	void (*Serialize)(File, ulong value);
};

extern const struct _intMethods Ints;