#pragma once
#include <stdlib.h>
#include "core/csharp.h"
#include "core/array.h"

// max number of bytes to alloc for parsing a string from a file buffer
#define MAX_PARSABLE_STRING_LENGTH 1024

/// <summary>
/// Pointer to _stringArray struct containing an array of strings, array of string lengths, and the number of strings within the array
/// </summary>
typedef struct _stringArray* StringArray;

struct _stringArray {
	/// <summary>
	/// The array of pointers to each string
	/// </summary>
	char** Strings;
	/// <summary>
	/// The lengths for each string
	/// </summary>
	size_t* StringLengths;
	/// <summary>
	/// The total number of strings within the string array and length array
	/// </summary>
	size_t Count;
};

struct _stringArrayMethods {
	/// <summary>
	/// Allocs a string array on the heap
	/// </summary>
	StringArray(*Create)(void);
	/// <summary>
	/// Trims all strings within the array
	/// </summary>
	void (*Trim)(StringArray);
	/// <summary>
	/// Disposes of the provided string array
	/// </summary>
	void(*Dispose)(StringArray);
	/// <summary>
	/// Disposes of the string array's members but not the string array itself
	/// </summary>
	void (*DisposeMembers)(StringArray);
};

extern const struct _stringArrayMethods StringArrays;

struct _stringMethods {
	// Length of terminated string, handles null
	size_t(*Length)(const char* buffer);
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
	/// <summary>
	/// Attempts to split the provided string using the provided delimiter, sets the members of the provided string array
	/// </summary>
	bool (*TrySplit)(const char* source, size_t length, int delimiter, StringArray resultStringArray);
	bool (*TryParse)(const char* buffer, const size_t bufferLength, char** out_string);
	// returns the index of the found character,
	// otherwise returns -1
	int(*IndexOf)(const char* buffer, const size_t bufferLength, int character);
};

extern const struct _stringMethods Strings;