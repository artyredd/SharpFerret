#pragma once
#include <stdlib.h>
#include "core/file.h"
#include "core/csharp.h"

struct _intMethods {
	bool (*TryDeserialize)(const char* buffer, size_t bufferLength, size_t* out_value);
	void (*Serialize)(File, size_t value);
};

extern const struct _intMethods Ints;