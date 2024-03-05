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
	ulong* StringLengths;
	/// <summary>
	/// The total number of strings within the string array and length array
	/// </summary>
	ulong Count;
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
	ulong(*Length)(const char* buffer);
	// Converts all characters in the buffer to lowercase
	void (*ToLower)(char* buffer, ulong bufferLength, ulong offset);
	// Converts all characters in the buffer to uppercase
	void (*ToUpper)(char* buffer, ulong bufferLength, ulong offset);
	// Trims the leading and trailing whitespace from a string and ending the string with a nul terminator, returning the new size of the string within the buffer
	ulong(*Trim)(char* buffer, const ulong bufferLength);
	/// <summary>
	/// Allocs new string and copies 'length' bytes of the provided source string
	/// </summary>
	char* (*Duplicate)(const char* source, ulong length);
	/// <summary>
	/// POTENTIALLY INSECURE; Allocs a new string and copies all bytes INCLUDING nul terminator to new string
	/// </summary>
	char* (*DuplicateTerminated)(const char* source);
	/// <summary>
	/// Returns true(1) if the target string was found within the source string 
	/// </summary>
	bool(*Contains)(const char* source, ulong length, const char* target, const ulong targetLength);
	/// <summary>
	/// Returns true if both strings contain the same bytes, supports null and same reference strings
	/// </summary>
	bool (*Equals)(const char* left, ulong leftLength, const char* right, ulong rightLength);
	/// <summary>
	/// Attempts to split the provided string using the provided delimiter, sets the members of the provided string array
	/// </summary>
	bool (*TrySplit)(const char* source, ulong length, int delimiter, StringArray resultStringArray);
	bool (*TryParse)(const char* buffer, const ulong bufferLength, char** out_string);
	// returns the index of the found character,
	// otherwise returns -1
	int(*IndexOf)(const char* buffer, const ulong bufferLength, int character);

	// Splits str into a stack string using the given selector
	// can be used in a loop to loop through all the segments of a split string
	// without using dynamic memory
	bool (*TryLoopSplitByFunction)(string str, string* out_string, bool(*selector)(char));
	// Trims leading and trailing whitespace
	// by returning a partial string
	partial_string(*TrimAll)(string str);
};

extern const struct _stringMethods Strings;