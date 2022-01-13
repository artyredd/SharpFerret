#pragma once

#include "csharp.h"

bool TryParseBoolean(const char* buffer, const size_t bufferLength, bool* out_bool);

bool TryParseLine(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);

bool TryParseString(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);

bool RunParsingUnitTests();