#pragma once

#include "csharp.h"

bool TryParseBoolean(char* buffer, size_t bufferLength, bool* out_bool);

bool TryParseLine(char* buffer, size_t bufferLength, size_t maxStringLength, char** out_string);

bool TryParseString(char* buffer, size_t bufferLength, size_t maxStringLength, char** out_string);

bool RunParsingUnitTests();