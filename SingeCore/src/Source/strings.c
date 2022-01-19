#include "singine/strings.h"
#include "singine/guards.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "singine/memory.h"

static void ToLower(char* buffer, size_t bufferLength, size_t offset);
static void ToUpper(char* buffer, size_t bufferLength, size_t offset);
static size_t Trim(char* buffer, const size_t bufferLength);
static char* Duplicate(const char* source, size_t length);
static char* DuplicateTerminated(const char* source);
static bool Contains(const char* source, size_t length, const char* target, const size_t targetLength);

const struct _stringMethods Strings = {
	.ToUpper = &ToUpper,
	.ToLower = &ToLower,
	.Trim = &Trim,
	.Duplicate = &Duplicate,
	.DuplicateTerminated = &DuplicateTerminated,
	.Contains = &Contains
};

static void ToLower(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);
	GuardNotZero(bufferLength);
	GuardLessThan(offset, bufferLength);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)tolower(buffer[i]);
	}
}

static void ToUpper(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)toupper(buffer[i]);
	}
}

static size_t Trim(char* buffer, const size_t bufferLength)
{
	size_t startIndex = 0;
	size_t endIndex = bufferLength - 1;

	size_t index = 0;
	// traverse the string the first non-whitespace char we hit is the start of the string
	while (index < bufferLength and buffer[index] isnt '\0' and isspace(buffer[index])) index++;

	startIndex = index;

	// if the start index is the end index that mean the whole thing is whitespace return 0
	if (startIndex is endIndex)
	{
		buffer[0] = '\0';
		return 0;
	}

	for (size_t i = startIndex; i < bufferLength; i++)
	{
		int c = buffer[i];
		if (isspace(c) is false)
		{
			endIndex = i;
		}
	}

	// if there was nothing to trim return
	if (startIndex is 0 and endIndex is(bufferLength - 1))
	{
		return bufferLength;
	}

	// now we have the start and end indexes we should move the entire string to the beginning of the buffer
	size_t length = max(endIndex - startIndex, 0) + 1;
	memmove(buffer, buffer + startIndex, length);

	// null terminate
	buffer[length] = '\0';

	return length;
}

static char* Duplicate(const char* source, size_t length)
{
	return DuplicateAddress(source, length, length);
}

static char* DuplicateTerminated(const char* source)
{
	// it's debatable whether or not duplicating null should be an error or null itself should be the return value
	// i would vote the latter
	if (source is null) { return null; }

	size_t length = strlen(source) + 1;

	char* result = DuplicateAddress(source, length, length);

	// duplicate address uses zero initilization, this shouldn;'t be necessary since '\0' is 0
	//// nul terminate
	//result[length] = '\0';

	return result;
}

static bool Contains(const char* source, size_t length, const char* target, const size_t targetLength)
{
	if (source is null || target is null)
	{
		return false;
	}

	for (size_t start = 0; start < min(length - targetLength, max(length - targetLength, 0)); start++)
	{
		const char* buffer = source + start;

		if (memcmp(buffer, target, targetLength) is 0)
		{
			return true;
		}
	}

	return false;
}