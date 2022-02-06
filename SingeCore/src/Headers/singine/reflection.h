#pragma once
#include "csharp.h"

#define SIGNIFICANT_VARIABLE_NAME_SIZE 31

typedef struct _parsableValue* ParsableValue;

typedef struct _parsableValue parsableValue;

struct _parsableValue {
	/// <summary>
	/// The parsable name for this value, this would traditionally be the variable name this struct is stored in
	/// </summary>
	char Name[SIGNIFICANT_VARIABLE_NAME_SIZE];
	/// <summary>
	/// The value of this value
	/// </summary>
	union {
		/// <summary>
		/// The value of this struct as a signed int
		/// </summary>
		int AsInt;
		unsigned int AsUInt;
	} Value;
};

struct _parsableValueMethods {
	/// <summary>
	/// Attempts to find the name of the member of a _parsableValue* using the provided value
	/// </summary>
	bool (*TryGetName)(const void* values, const size_t count, const int value, const char** out_name);
	/// <summary>
	/// Attempts to find the value of the provided name within the provided _parsableValue* array
	/// </summary>
	bool (*TryGetInt)(const void* values, const size_t count, const char* buffer, const size_t bufferLength, int* out_int);
	/// <summary>
	/// Attempts to find the name of the member of a _parsableValue* using the provided value
	/// </summary>
	bool (*TryGetNameUInt)(const void* values, const size_t count, const unsigned int value, const char** out_name);
	/// <summary>
	/// Attempts to find the value of the provided name within the provided _parsableValue* array
	/// </summary>
	bool (*TryGetUInt)(const void* values, const size_t count, const char* buffer, const size_t bufferLength, unsigned int* out_int);
	bool (*TryGetMemberByName)(const void* values, const size_t count, const char* buffer, const size_t bufferLength, parsableValue** out_value);
	bool (*TryGetMemberByValue)(const void* values, const size_t count, const void* value, const size_t typeSize, parsableValue** out_value);


};

extern const struct _parsableValueMethods ParsableValues;