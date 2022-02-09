#include "singine/reflection.h"
#include "csharp.h"
#include "singine/strings.h"
#include <string.h>

static bool TryGetName(parsableValue* values, const size_t count, const int value, const char** out_name);
static bool TryGetInt(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, int* out_int);
static bool TryGetNameUInt(parsableValue* values, const size_t count, const unsigned int value, const char** out_name);
static bool TryGetUInt(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, unsigned int* out_int);
static bool TryGetMemberByName(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, parsableValue* out_value);
static bool TryGetMemberByValue(parsableValue* values, const size_t count, const void* value, const size_t typeSize, parsableValue* out_value);

const struct _parsableValueMethods ParsableValues = {
	.TryGetName = &TryGetName,
	.TryGetInt = &TryGetInt,
	.TryGetMemberByName = &TryGetMemberByName,
	.TryGetMemberByValue = &TryGetMemberByValue
};

static bool TryGetMemberByName(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, parsableValue* out_value)
{
	for (size_t i = 0; i < count; i++)
	{
		if (Strings.Equals(buffer, bufferLength, values[i].Name, min(strlen(values[i].Name), SIGNIFICANT_VARIABLE_NAME_SIZE)))
		{
			*out_value = values[i];
			return true;
		}
	}

	return false;
}

static bool TryGetMemberByValue(parsableValue* values, const size_t count, const void* value, const size_t typeSize, parsableValue* out_value)
{
	// dry violation here to reduce stack frame performance issues
	for (size_t i = 0; i < count; i++)
	{
		// this is some hacking generic type comparison
		// can't wait for a typo to blow this up
		const void* unionAddress = &values[i].Value;

		if (memcmp(unionAddress, value, typeSize))
		{
			*out_value = values[i];
			return true;
		}
	}

	return false;
}

static bool TryGetNameAliased(parsableValue* values, const size_t count, const void* value, const size_t typeSize, const char** out_name)
{
	for (size_t i = 0; i < count; i++)
	{
		// this is some hacking generic type comparison
		// can't wait for a typo to blow this up
		const void* unionAddress = &values[i].Value;

		if (memcmp(unionAddress, value, typeSize))
		{
			*out_name = values[i].Name;
			return true;
		}
	}

	*out_name = null;
	return false;
}

static bool TryGetName(parsableValue* values, const size_t count, const int value, const char** out_name)
{
	return TryGetNameAliased(values, count, &value, sizeof(int), out_name);
}

static bool TryGetInt(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, int* out_int)
{
	parsableValue value;
	if (TryGetMemberByName(values, count, buffer, bufferLength, &value))
	{
		*out_int = value.Value.AsInt;
		return true;
	}
	return false;
}

static bool TryGetNameUInt(parsableValue* values, const size_t count, const unsigned int value, const char** out_name)
{
	return TryGetNameAliased(values, count, &value, sizeof(unsigned int), out_name);
}

static bool TryGetUInt(parsableValue* values, const size_t count, const char* buffer, const size_t bufferLength, unsigned int* out_int)
{
	parsableValue value;
	if (TryGetMemberByName(values, count, buffer, bufferLength, &value))
	{
		*out_int = value.Value.AsUInt;
		return true;
	}
	return false;
}