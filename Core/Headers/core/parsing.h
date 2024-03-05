#pragma once

#include "core/csharp.h"

struct _parsing {
	bool (*TryGetBool)(const char* buffer, const ulong bufferLength, bool* out_bool);
	bool (*TryGetLine)(const char* buffer, const ulong bufferLength, const ulong maxStringLength, char** out_string);
	bool (*TryGetString)(const char* buffer, const ulong bufferLength, const ulong maxStringLength, char** out_string);
	bool (*TryGetStrings)(const char* buffer, const ulong bufferLength, char*** out_array, ulong** out_lengths, ulong* out_count);
	void(*RunParsingUnitTests)();
};

extern const struct _parsing Parsing;