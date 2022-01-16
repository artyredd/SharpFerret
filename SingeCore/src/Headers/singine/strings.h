#pragma once
#include <stdlib.h>

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
};

extern const struct _stringMethods Strings;