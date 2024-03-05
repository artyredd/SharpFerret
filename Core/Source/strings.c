#include "core/strings.h"
#include "core/guards.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "core/memory.h"


static StringArray CreateStringArray(void);
static void DisposeStringArray(StringArray);
static void DisposeMembers(StringArray);
static void TrimArray(StringArray);

const struct _stringArrayMethods StringArrays = {
	.Create = &CreateStringArray,
	.Dispose = &DisposeStringArray,
	.DisposeMembers = &DisposeMembers,
	.Trim = &TrimArray
};

static void ToLower(char* buffer, ulong bufferLength, ulong offset);
static void ToUpper(char* buffer, ulong bufferLength, ulong offset);
static ulong Trim(char* buffer, const ulong bufferLength);
static char* Duplicate(const char* source, ulong length);
static char* DuplicateTerminated(const char* source);
static bool Contains(const char* source, ulong length, const char* target, const ulong targetLength);
static bool Equals(const char* left, ulong leftLength, const char* right, ulong rightLength);
static bool TrySplit(const char* source, ulong length, int delimiter, StringArray resultStringArray);
static bool TryParse(const char* buffer, const ulong bufferLength, char** out_string);
static int IndexOf(const char* buffer, const ulong bufferLength, int character);
private ulong Length(const char* str);
private bool TryLoopSplitByFunction(string str, string* out_string, bool(*selector)(char));
private partial_string TrimAll(string str);

const struct _stringMethods Strings = {
	.Length = Length,
	.ToUpper = &ToUpper,
	.ToLower = &ToLower,
	.Trim = &Trim,
	.Duplicate = &Duplicate,
	.DuplicateTerminated = &DuplicateTerminated,
	.Contains = &Contains,
	.Equals = &Equals,
	.TrySplit = &TrySplit,
	.TryParse = &TryParse,
	.IndexOf = IndexOf,
	.TryLoopSplitByFunction = TryLoopSplitByFunction,
	.TrimAll = TrimAll
};

partial_string TrimAll(string str)
{
	int start = 0;
	int end = str->Count - 1;

	for (int i = 0; i < str->Count; i++)
	{
		const int c = str->Values[i];

		if (isspace(c))
		{
			continue;
		}

		start = i;
		break;
	}

	int i = str->Count;
	while (i-- > 0)
	{
		const int c = str->Values[i];

		if (isspace(c))
		{
			continue;
		}

		end = i;
		break;
	}

	return partial_substring(str, start, end - start + 1);
}

private bool TryLoopSplitByFunction(string str, string* out_string, bool(*selector)(char))
{
	*out_string = (array(char)){ 0 };

	for (ulong i = 0; i < str->Count; i++)
	{
		if (selector(str->Values[i]))
		{
			*out_string = stack_subarray_front(char, str, i);
			return true;
		}
	}

	return false;
}

private ulong Length(const char* str)
{
	return strlen(str ? str : "");
}

static void ToLower(char* buffer, ulong bufferLength, ulong offset)
{
	GuardNotNull(buffer);
	GuardNotZero(bufferLength);
	GuardLessThan(offset, bufferLength);

	for (ulong i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)tolower(buffer[i]);
	}
}

static void ToUpper(char* buffer, ulong bufferLength, ulong offset)
{
	GuardNotNull(buffer);

	for (ulong i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)toupper(buffer[i]);
	}
}

static ulong Trim(char* buffer, const ulong bufferLength)
{
	ulong startIndex = 0;
	ulong endIndex = bufferLength - 1;

	ulong index = 0;
	// traverse the string the first non-whitespace char we hit is the start of the string
	while (index < bufferLength and buffer[index] isnt '\0' and isspace(buffer[index])) index++;

	startIndex = index;

	// if the start index is the end index that mean the whole thing is whitespace return 0
	if (startIndex is endIndex)
	{
		buffer[0] = '\0';
		return 0;
	}

	for (ulong i = startIndex; i < bufferLength; i++)
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
	ulong length = max(endIndex - startIndex, 0) + 1;
	memmove(buffer, buffer + startIndex, length);

	// null terminate
	buffer[length] = '\0';

	return length;
}

static char* Duplicate(const char* source, ulong length)
{
	return Memory.DuplicateAddress(source, length, length, Memory.String);
}

static char* DuplicateTerminated(const char* source)
{
	// it's debatable whether or not duplicating null should be an error or null itself should be the return value
	// i would vote the latter
	if (source is null) { return null; }

	ulong length = strlen(source) + 1;

	char* result = Memory.DuplicateAddress(source, length, length, Memory.String);

	// duplicate address uses zero initilization, this shouldn;'t be necessary since '\0' is 0
	//// nul terminate
	//result[length] = '\0';

	return result;
}

static bool Contains(const char* source, ulong length, const char* target, const ulong targetLength)
{
	if (source is null || target is null)
	{
		return false;
	}

	for (ulong start = 0; start < length; start++)
	{
		const char* buffer = source + start;

		// check if this character can be the start of the target string, if it can't move on
		int firstChar = buffer[0];

		if (firstChar isnt target[0])
		{
			continue;
		}

		if (length - start >= targetLength)
		{
			if (memcmp(buffer, target, min(length - start, targetLength)) is 0)
			{
				return true;
			}
		}
	}

	return false;
}

static int IndexOf(const char* buffer, const ulong bufferLength, int character)
{
	int position = 0;

	while (position < bufferLength)
	{
		if (buffer[position] is character)
		{
			return position;
		}

		++position;
	}

	return -1;
}

static bool Equals(const char* left, ulong leftLength, const char* right, ulong rightLength)
{
	// if both strings have different lengths they can not be the same
	if (leftLength isnt rightLength)
	{
		return false;
	}

	// if both the lengths are the same AND both strings point to the same address they have to be equal
	// including null and 0
	if (left is right)
	{
		return true;
	}

	// if both the lengths are the same and the pointers aren't equal to eachother, ensure neither are null
	// a non-null pointer and a null pointer can never be equal
	if (left is null || right is null)
	{
		return false;
	}

	// at this point we know both pointers are not null, aren't eachother, and are the same length
	// compare each byte to verify they are the same
	return memcmp(left, right, leftLength) is 0;
}

static bool TryParse(const char* buffer, const ulong bufferLength, char** out_string)
{
	*out_string = null;

	// skip until we encounter non-whitespace or end of buffer
	ulong index = 0;
	while (index < bufferLength && isspace(buffer[index]) && buffer[index++] != '\0');

	// if the entire string was whitespace return false
	if (index == bufferLength)
	{
		return false;
	}

	// get the next string after any whitespace
	// determine how much we should alloc for the string
	// this is O(2n)
	ulong size = index;
	while (size < bufferLength && isspace(buffer[size]) is false && buffer[size++] != '\0');

	// convert size from index into length
	size = size - index;

	// make sure there is a string to return
	if (size is 0)
	{
		return false;
	}

	// make sure not to exceed the maxStringLength
	size = min(size, MAX_PARSABLE_STRING_LENGTH);

	char* result = Memory.Alloc(size + 1, Memory.String);

	// copy the char over
	memcpy(result, buffer + index, size);

	// make sure to NUL terminate the string even though we may not use it
	result[size] = '\0';

	*out_string = result;

	return true;
}

static StringArray CreateStringArray(void)
{
	return Memory.Alloc(sizeof(struct _stringArray), Memory.String);
}

static void DisposeMembers(StringArray array)
{
	if (array is null)
	{
		return;
	}

	for (ulong i = 0; i < array->Count; i++)
	{
		Memory.Free(array->Strings[i], Memory.String);
	}

	Memory.Free(array->StringLengths, Memory.String);
	Memory.Free(array->Strings, Memory.String);
}

static void DisposeStringArray(StringArray array)
{
	DisposeMembers(array);

	Memory.Free(array, Memory.String);
}

static bool TrySplit(const char* buffer, ulong bufferLength, int delimiter, StringArray result)
{
	result->Count = 0;
	result->StringLengths = null;
	result->Strings = null;

	// every i'th index is the position within the buffer of the start of a string
	// every i'th+1 index is the length of that string
	ulong stringPositions[1024];

	memset(stringPositions, 0, sizeof(ulong) * 1024);

	// traverse the buffer and save the indexes of of the start of every string
	// the first index is always 0
	ulong count = 0;

	int c = 0;
	ulong index = 0;
	ulong length = 0;

	// loop until we either found the end of the buffer, exceed the buffer length or we move to the next line
	do
	{
		c = buffer[index++];

		// if we found the end to a string we should record the position and previouslength
		if (c is delimiter or c is '\0' or c is '\n' or c is '\r')
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

	// alloc string array and ulong array to store the destination strings and lengths
	ulong* lengths = Memory.Alloc(sizeof(ulong) * count, Memory.String);
	char** strings = Memory.Alloc(sizeof(char*) * count, Memory.String);

	for (ulong i = 0; i < count; i++)
	{
		// the index of the start of the buffer is i * 2
		// the length of the buffer is (i * 2) +1
		const char* subBuffer = buffer + stringPositions[(i << 1)];
		ulong subLength = stringPositions[(i << 1) + 1];

		// alloc one more byte for nul terminator
		char* string = Memory.Alloc((sizeof(char) * subLength) + 1, Memory.String);

		string[subLength] = '\0';

		strings[i] = string;
		lengths[i] = subLength;

		// copy over the string
		memcpy(string, subBuffer, subLength);
	}

	// set the out variables
	result->Count = count;
	result->StringLengths = lengths;
	result->Strings = strings;

	return true;
}

static void TrimArray(StringArray array)
{
	GuardNotNull(array);

	if (array->Strings is null || array->StringLengths is null)
	{
		return;
	}

	for (ulong i = 0; i < array->Count; i++)
	{
		array->StringLengths[i] = Trim(array->Strings[i], array->StringLengths[i]);
	}
}