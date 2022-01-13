#include "singine/parsing.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "singine/memory.h"
#include "singine/conversions.h"
#include "cunit.h"
#include "singine/file.h"

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

	ToUpper(copiedString, strlen(copiedString), 0);

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

	char* result = SafeAlloc(size + 1);

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

	char* result = SafeAlloc(size + 1);

	// copy the char over
	memcpy(result, buffer + index, size);

	// make sure to NUL terminate the string even though we may not use it
	result[size] = '\0';

	*out_string = result;

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

bool RunParsingUnitTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "TryParseBoolean", &Test_TryParseBoolean);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}