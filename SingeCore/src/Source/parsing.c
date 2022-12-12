#include "singine/parsing.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "singine/memory.h"
#include "singine/strings.h"
#include "cunit.h"
#include "singine/file.h"
#include "csharp.h"

char* ValidTrueBooleans[] = {
	"TRUE",
	"1",
	"YES",
	"T",
	"Y",
	"ON",
	"ENABLED"
};

char* ValidFalseBooleans[] = {
	"FALSE",
	"0",
	"NO",
	"F",
	"N",
	"OFF",
	"DISABLED"
};

bool TryParseBoolean(const char* buffer, const size_t bufferLength, bool* out_bool)
{
	*out_bool = false;

	char copiedString[11];

	copiedString[10] = '\0';

	sscanf_s(buffer, "%10s", &copiedString, 10);

	size_t length = strlen(copiedString) - 1;

	bool parsed = false;

	Strings.ToUpper(copiedString, strlen(copiedString), 0);

	// this is probably platform dependent, 
	// this assumes the size of a pointer address is the same size as size_t
	// on this architexture (amdx64) ptrs are x64 and so is the size of the DWORD
	for (size_t i = 0; i < (sizeof(ValidTrueBooleans) / sizeof(size_t)); i++)
	{
		if (memcmp(copiedString, ValidTrueBooleans[i], min(length, bufferLength)) is 0)
		{
			parsed = true;
			*out_bool = true;
			break;
		}
	}

	if (parsed is false)
	{
		for (size_t i = 0; i < (sizeof(ValidFalseBooleans) / sizeof(size_t)); i++)
		{
			if (memcmp(copiedString, ValidFalseBooleans[i], min(length, bufferLength)) is 0)
			{
				parsed = true;
				*out_bool = false;
				break;
			}
		}
	}

	return parsed;
}

bool TryParseLine(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string)
{
	*out_string = null;

	// skip until we encounter non-whitespace or end of buffer
	size_t index = 0;
	while (index < bufferLength && isspace(buffer[index]) && buffer[index++] != '\0');

	// if the entire string was whitespace return false
	if (index == bufferLength)
	{
		return false;
	}

	// get the next string after any whitespace
	// determine how much we should alloc for the string
	// this is O(2n)
	size_t size = index;
	while (size < bufferLength && buffer[size++] != '\0');

	// convert size from index into length
	size = size - index;

	// make sure there is a string to return
	if (size is 0)
	{
		return false;
	}

	// make sure not to exceed the maxStringLength
	size = min(size, maxStringLength);

	char* result = Memory.Alloc(size + 1, Memory.String);

	// copy the char over
	memcpy(result, buffer + index, size);

	// make sure to NUL terminate the string even though we may not use it
	result[size] = '\0';

	*out_string = result;

	return true;
}

bool TryParseString(const char* buffer, const size_t bufferLength, const size_t maxStringLength, char** out_string)
{
	*out_string = null;

	// skip until we encounter non-whitespace or end of buffer
	size_t index = 0;
	while (index < bufferLength && isspace(buffer[index]) && buffer[index++] != '\0');

	// if the entire string was whitespace return false
	if (index == bufferLength)
	{
		return false;
	}

	// get the next string after any whitespace
	// determine how much we should alloc for the string
	// this is O(2n)
	size_t size = index;
	while (size < bufferLength && isspace(buffer[size]) is false && buffer[size++] != '\0');

	// convert size from index into length
	size = size - index;

	// make sure there is a string to return
	if (size is 0)
	{
		return false;
	}

	// make sure not to exceed the maxStringLength
	size = min(size, maxStringLength);

	char* result = Memory.Alloc(size + 1, Memory.String);

	// copy the char over
	memcpy(result, buffer + index, size);

	// make sure to NUL terminate the string even though we may not use it
	result[size] = '\0';

	*out_string = result;

	return true;
}

bool TryParseStringArray(const char* buffer, const size_t bufferLength, char*** out_array, size_t** out_lengths, size_t* out_count)
{
	*out_lengths = null;
	*out_count = 0;
	*out_array = null;

	// every i'th index is the position within the buffer of the start of a string
	// every i'th+1 index is the length of that string
	size_t stringPositions[1024];

	memset(stringPositions, 0, sizeof(size_t) * 1024);

	// traverse the buffer and save the indexes of of the start of every string
	// the first index is always 0
	size_t count = 0;

	int c = 0;
	size_t index = 0;
	size_t length = 0;

	// loop until we either found the end of the buffer, exceed the buffer length or we move to the next line
	do
	{
		c = buffer[index++];

		// if we found the end to a string we should record the position and previouslength
		if (c is ';' or c is ',' or c is '\0' or c is '\n' or c is '\r')
		{
			// set the length of the previous string
			stringPositions[(count << 1) + 1] = length;

			// increment the number of strings we found
			count++;

			// record the starting offset of the next string
			stringPositions[count << 1] = index;
			// reset the length for the next string
			length = 0;

			continue;
		}

		// if the character wasn't a delimiter increment the length of the string
		++(length);
	} while (index <= bufferLength);

	// count will be 0 if no delimiter is found, but length with be non-zero if a string does exist
	// abscence of both is no string found
	if (count is 0 and length is 0)
	{
		return false;
	}

	// now we have all the indexes of the start of each string and the lengths of each of them
	// (with a max of bufferSize/2 strings(512 magic number for laziness)

	// alloc string array and size_t array to store the destination strings and lengths
	size_t* lengths = Memory.Alloc(sizeof(size_t) * count, Memory.GenericMemoryBlock);
	char** strings = Memory.Alloc(sizeof(char*) * count, Memory.String);

	for (size_t i = 0; i < count; i++)
	{
		// the index of the start of the buffer is i * 2
		// the length of the buffer is (i * 2) +1
		const char* subBuffer = buffer + stringPositions[(i << 1)];
		size_t subLength = stringPositions[(i << 1) + 1];

		// alloc one more byte for nul terminator
		char* string = Memory.Alloc((sizeof(char) * subLength) + 1, Memory.String);

		string[subLength] = '\0';

		strings[i] = string;
		lengths[i] = subLength;

		// copy over the string
		memcpy(string, subBuffer, subLength);
	}

	// set the out variables
	*out_array = strings;
	*out_lengths = lengths;
	*out_count = count;

	return true;
}


static bool Test_Helper_TryParseBoolean(File stream, char* data, bool shouldParse, bool expected)
{
	bool actual;

	Equals(TryParseBoolean(data, strlen(data), &actual), shouldParse, "%i");

	Equals(expected, actual, "%i");

	return true;
}

static bool Test_TryParseBoolean(File stream)
{
	Test_Helper_TryParseBoolean(stream, "TRUE", true, true);
	Test_Helper_TryParseBoolean(stream, "true", true, true);
	Test_Helper_TryParseBoolean(stream, "True", true, true);
	Test_Helper_TryParseBoolean(stream, "eurt", false, false);
	Test_Helper_TryParseBoolean(stream, "notTrue", false, false);

	Test_Helper_TryParseBoolean(stream, "FALSE", true, false);
	Test_Helper_TryParseBoolean(stream, "False", true, false);
	Test_Helper_TryParseBoolean(stream, "false", true, false);
	Test_Helper_TryParseBoolean(stream, "eslaf", false, false);
	Test_Helper_TryParseBoolean(stream, "alsds", false, false);

	Test_Helper_TryParseBoolean(stream, "T", true, true);
	Test_Helper_TryParseBoolean(stream, "F", true, false);
	Test_Helper_TryParseBoolean(stream, "A", false, false);
	Test_Helper_TryParseBoolean(stream, "G", false, false);

	Test_Helper_TryParseBoolean(stream, "0", true, false);
	Test_Helper_TryParseBoolean(stream, "1", true, true);
	Test_Helper_TryParseBoolean(stream, "9", false, false);
	Test_Helper_TryParseBoolean(stream, "23928398239283", false, false);

	return true;
}

bool Test_TryParseStringArray(File stream)
{
	ResetAlloc();
	ResetFree();

	char* data = "this, should, be a, string, array";
	size_t dataLength = strlen(data);

	char** strings;
	size_t* lengths;
	size_t count;

	Assert(TryParseStringArray(data, dataLength, &strings, &lengths, &count));

	Equals((size_t)5, count, "%lli");

	const char* word = "this";
	size_t index = 0;
	// "this"
	Equals(strlen(word), strlen(strings[index]), "%lli");
	Equals(strlen(word), lengths[index], "%lli");
	Equals(0, memcmp(word, strings[index], strlen(word)), "%i");

	// " should"
	word = " should";
	index = 1;
	Equals(strlen(word), strlen(strings[index]), "%lli");
	Equals(strlen(word), lengths[index], "%lli");
	Equals(0, memcmp(word, strings[index], strlen(word)), "%i");

	// " be a"
	word = " be a";
	index = 2;
	Equals(strlen(word), strlen(strings[index]), "%lli");
	Equals(strlen(word), lengths[index], "%lli");
	Equals(0, memcmp(word, strings[index], strlen(word)), "%i");

	// " string"
	word = " string";
	index = 3;
	Equals(strlen(word), strlen(strings[index]), "%lli");
	Equals(strlen(word), lengths[index], "%lli");
	Equals(0, memcmp(word, strings[index], strlen(word)), "%i");

	// " array"
	word = " array";
	index = 4;
	Equals(strlen(word), strlen(strings[index]), "%lli");
	Equals(strlen(word), lengths[index], "%lli");
	Equals(0, memcmp(word, strings[index], strlen(word)), "%i");

	// parse string allocs the strings for us free them
	Memory.Free(lengths, Memory.GenericMemoryBlock);
	for (size_t i = 0; i < count; i++)
	{
		Memory.Free(strings[i], Memory.String);
	}
	Memory.Free(strings, Memory.String);

	// ensure no memory leak
	Assert(AllocCount() <= FreeCount());

	return true;
}

bool RunParsingUnitTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "TryParseBoolean", &Test_TryParseBoolean);
	suite->Append(suite, "TryParseStringArray", &Test_TryParseStringArray);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}