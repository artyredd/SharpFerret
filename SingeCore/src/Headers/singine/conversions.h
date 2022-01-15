#pragma once

#include "csharp.h"

// Converts all characters in the buffer to lowercase
void ToLower(char* buffer, size_t bufferLength, size_t offset);

// Converts all characters in the buffer to uppercase
void ToUpper(char* buffer, size_t bufferLength, size_t offset);

// Trims the leading and trailing whitespace from a string and ending the string with a nul terminator, returning the new size of the string within the buffer
size_t Trim(char* buffer, const size_t bufferLength);