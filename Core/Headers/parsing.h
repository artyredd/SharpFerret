#pragma once

#include "csharp.h"

struct _parsing {
	bool (*TryGetBool)(const char* buffer, const size_t bufferLength, bool* out_bool);
	bool (*TryGetLine)(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);
	bool (*TryGetString)(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);
	bool (*TryGetStrings)(const char* buffer, const size_t bufferLength, char*** out_array, size_t** out_lengths, size_t* out_count);
	bool (*RunParsingUnitTests)();
};

extern const struct _parsing Parsing;