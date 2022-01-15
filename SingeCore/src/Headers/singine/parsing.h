#pragma once

#include "csharp.h"

bool TryParseBoolean(const char* buffer, const size_t bufferLength, bool* out_bool);

bool TryParseLine(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);

bool TryParseString(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string);

/// <summary>
/// Tries to extract an array of strings delimited by a semicolon, or comma, all strings are nul terminated
/// </summary>
bool TryParseStringArray(const char* buffer, const size_t bufferLength, char*** out_array, size_t** out_lengths, size_t* out_count);

bool RunParsingUnitTests();