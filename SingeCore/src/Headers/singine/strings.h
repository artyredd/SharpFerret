#pragma once
#include <stdlib.h>
#include "csharp.h"

struct _stringMethods {
	// Converts all characters in the buffer to lowercase
	void (*ToLower)(char* buffer, size_t bufferLength, size_t offset);
	// Converts all characters in the buffer to uppercase
	void (*ToUpper)(char* buffer, size_t bufferLength, size_t offset);
	// Trims the leading and trailing whitespace from a string and ending the string with a nul terminator, returning the new size of the string within the buffer
	size_t(*Trim)(char* buffer, const size_t bufferLength);
	/// <summary>
	/// Allocs new string and copies 'length' bytes of the provided source string
	/// </summary>
	char* (*Duplicate)(const char* source, size_t length);
	/// <summary>
	/// POTENTIALLY INSECURE; Allocs a new string and copies all bytes INCLUDING nul terminator to new string
	/// </summary>
	char* (*DuplicateTerminated)(const char* source);
	/// <summary>
	/// Returns true(1) if the target string was found within the source string 
	/// </summary>
	bool(*Contains)(const char* source, size_t length, const char* target, const size_t targetLength);
	/// <summary>
	/// Returns true if both strings contain the same bytes, supports null and same reference strings
	/// </summary>
	bool (*Equals)(const char* left, size_t leftLength, const char* right, size_t rightLength);
};

extern const struct _stringMethods Strings;