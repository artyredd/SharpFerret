#pragma once

#include "core/array.h"
#include "sourceParser.h"
#include "core/file.h"
#include "core/strings.h"
#include "core/cunit.h"
#include <string.h>
#include <ctype.h>
#include "core/guards.h"

/*
#pragma section("mysec",discard)
MACRO T Multiply<T>(T left, T right)
{
	return left * right;
}

#define DefineMultiplyT(T)\
T Multiply_##T(T left, T right)\
{\
	return left * right;\
}\
*/

bool IsValidNameCharacter(const int c);

typedef struct
{
	// Location of the first alligator
	int AlligatorStartIndex;
	// Index of last alligator
	int AlligatorEndIndex;
	// index of the begining of the scope of the generics
	int StartScopeIndex;
	// index of the end of the scope of the generics
	int EndScopeIndex;
	// struct name<T>{ T thing; };
	bool Struct;
	// T name<T>(T left, T right);
	bool Declaration;
	// T name<T>(T left, T right) { return left + right; }
	bool Definition;
	// T number = add<T>(var1, var2);
	bool Call;
	// True when a pragma warning should be inserted 
	bool InsertFailedToIndentifyWarning;
} location;

DEFINE_CONTAINERS(location);

// Returns -1 if the left is found first
// Returns 0 if neither are found
// returns 1 if right is found first
private int LookAheadForCharacters(string data, int left, int right)
{
	int leftIndex = Strings.IndexOf(data->Values, data->Count, left);
	int rightIndex = Strings.IndexOf(data->Values, data->Count, right);

	if (leftIndex < rightIndex)
	{
		return -1;
	}

	if (rightIndex < leftIndex)
	{
		return 1;
	}

	return 0;
}

private bool StringContains(array(char) characters, int c)
{
	for (size_t i = 0; i < characters->Count; i++)
	{
		if (c is characters->Values[i])
		{
			return true;
		}
	}

	return false;
}

private bool LookAheadIsGenericCall(string data)
{
	// no matching alligator possible and therefor not a generic call
	if (data->Count is 0)
	{
		return false;
	}

	// <>
	int alligatorDepth = 0;
	// {}
	int braceDepth = 0;
	bool foundFinalAlligatorBrace = false;

	for (size_t i = 0; i < data->Count; i++)
	{
		int c = data->Values[i];

		// ignore all things within braces
		// we are allowed to define things in
		// alligators like structs
		// array < struct vector2{ int x; int y; } >
		if (braceDepth)
		{
			if (c is '{')
			{
				++braceDepth;
			}

			if (c is '}')
			{
				--braceDepth;
			}

			continue;
		}


		if (c is '>')
		{
			if (alligatorDepth is 0)
			{
				if (i is 0)
				{
					// <> is invalid because i say it is
					return false;
				}

				foundFinalAlligatorBrace = true;

				// found matching alligator
				break;
			}
			else
			{
				--alligatorDepth;
				continue;
			}
		}
		// increased aligator depth
		if (c is '<')
		{
			++alligatorDepth;
			continue;
		}

		if (c is '{')
		{
			++braceDepth;

			continue;
		}

		// check for blacklist
		// generics can only have letters, alligators, and commas, whitespace
		if (StringContains(stack_string(".;()/\\'\"}|-+=&^%$#@!`~"), c))
		{
			return false;
		}
	}

	return foundFinalAlligatorBrace;
}

private int IndexOfLastBlockExpressionOrMacro(string data, int start, int lastMacroIndex)
{
	Guard(start <= data->Count);

	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int indexOfLastValidChar = -1;
	int depth = 0;
	int parenDepth = 0;
	int indexOfFirstBrace = -1;

	for (int i = start; i-- > 0;)
	{
		int previousC = data->Values[max(0, i - 1)];
		int c = data->Values[i];
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		if (isspace(c))
		{
			continue;
		}

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '}' && parenDepth is 0)
		{
			++depth;

			if (indexOfFirstBrace is - 1)
			{
				indexOfFirstBrace = i;
			}
		}
		if (c is '{' && parenDepth is 0)
		{
			--depth;

			if (depth is 0)
			{
				if (indexOfFirstBrace > 0 and PreviousCharacterIgnoringWhiteSpace(stack_substring_front(data, i - 1)) is ')')
				{
					return indexOfFirstBrace;
				}

				return IndexOfLastBlockExpressionOrMacro(data, i, lastMacroIndex);
			}
		}

		// if we're in a block inside the params, ignore the contents. We 
		// wont find the matching paren inside of a block
		if (depth)
		{
			continue;
		}

		if (c is '(')
		{
			++parenDepth;
		}
		if (c is ')')
		{
			--parenDepth;
		}

		// ignore contents of parens
		if (parenDepth)
		{
			continue;
		}

		// skip words like private, static, int, float etc
		if (IsValidNameCharacter(c))
		{
			indexOfLastValidChar = i;
			continue;
		}

		// always denotes the previous block unless its within a type
		if (c is ';' and depth is 0)
		{
			return indexOfLastValidChar > 0 ? indexOfLastValidChar : i;
		}
	}

	return lastMacroIndex;
}

// returns whether or not the character is a valid c name character that could be used
// to name a method or variable
private bool IsValidNameCharacter(const int c)
{
	if (isalnum(c))
	{
		return true;
	}
	if (c is '_')
	{
		return true;
	}

	return false;
}

private bool HasAllValidNameCharactersOrWhiteSpace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = data->Values[i];

		if (IsValidNameCharacter(c) is false)
		{
			if (isspace(c))
			{
				continue;
			}

			return false;
		}
	}

	return true;
}

private int IndexOfClosingAlligator(string data, int startDepth, int openAlligatorIndex)
{
	// <>
	int alligatorDepth = 0;
	// {}
	int braceDepth = startDepth;
	bool foundFinalAlligatorBrace = false;

	for (size_t i = openAlligatorIndex; i < data->Count; i++)
	{
		int c = data->Values[i];

		// ignore all things within braces
		// we are allowed to define things in
		// alligators like structs
		// array < struct vector2{ int x; int y; } >
		if (braceDepth)
		{
			if (c is '{')
			{
				++braceDepth;
			}

			if (c is '}')
			{
				--braceDepth;
			}

			continue;
		}


		if (c is '>')
		{
			--alligatorDepth;

			if (alligatorDepth is 0)
			{
				return i;
			}

			continue;
		}

		// increased aligator depth
		if (c is '<')
		{
			++alligatorDepth;
			continue;
		}

		if (c is '{')
		{
			++braceDepth;

			continue;
		}
	}

	return -1;
}

// looks for the mactching }
// handles, comments, macros, and strings in between
private int IndexOfClosingBrace(string data)
{
	Guard(data->Count > 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;

	for (int i = 0; i < data->Count; i++)
	{
		int previousC = data->Values[max(0, i - 1)];
		int c = data->Values[i];
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		// check for macros
		if (c is '#' && !inMacro and inString is false and inSingleComment is false and inMultiComment is false)
		{
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		// check to see if we're at a newline
		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and inString is false)
		{
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro)
		{
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '{')
		{
			++depth;
		}
		if (c is '}')
		{
			--depth;

			if (depth is 0)
			{
				return i;
			}
		}
	}

	return -1;
}

// looks for the mactching )
// handles, comments, macros, and strings in between
private int IndexOfClosingParen(string data)
{
	Guard(data->Count > 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;
	int parenDepth = 0;

	for (int i = 0; i < data->Count; i++)
	{
		int previousC = data->Values[max(0, i - 1)];
		int c = data->Values[i];
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		// check for macros
		if (c is '#' && !inMacro and inString is false and inSingleComment is false and inMultiComment is false)
		{
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		// check to see if we're at a newline
		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and inString is false)
		{
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro)
		{
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '{')
		{
			++depth;
		}
		if (c is '}')
		{
			--depth;
		}

		// if we're in a block inside the params, ignore the contents. We 
		// wont find the matching paren inside of a block
		if (depth)
		{
			continue;
		}

		if (c is '(')
		{
			++parenDepth;
		}
		if (c is ')')
		{
			--parenDepth;
			if (parenDepth is 0)
			{
				return i;
			}
		}
	}

	return -1;
}

private int NextCharacterIgnoringWhiteSpace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = data->Values[i];

		if (isspace(c)) {
			continue;
		}

		return c;
	}

	return 0;
}

private int IndexOfNextCharacterIgnoringWhitespace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = data->Values[i];

		if (isspace(c)) {
			continue;
		}

		return i;
	}

	return -1;
}

private int PreviousCharacterIgnoringWhiteSpace(string data)
{
	for (int i = data->Count; i-- > 0;)
	{
		const int c = data->Values[i];

		if (isspace(c)) {
			continue;
		}

		return c;
	}

	return 0;
}

private int IndexOfPreviousCharacterIgnoringWhitespace(string data)
{
	for (int i = data->Count; i-- > 0;)
	{
		const int c = data->Values[i];

		if (isspace(c)) {
			continue;
		}

		return i;
	}

	return -1;
}

// returns true if it has a valid name, and sets the out_name (int nameStartIndex,int length) so you
// can construct a stack_subarray from the indices
private bool TryGetNameBeforeAlligator(string data, tuple(int, int)* out_name)
{
	*out_name = (tuple(int, int)){ 0,0 };

	// when we got past here
	//           \/
	// void method <>()
	bool exitedWhitespace = false;
	int nameLength = 0;

	int nameStart = -1;
	int nameEnd = -1;

	// traverse backwards
	for (int i = data->Count; i-- > 0;)
	{
		const int c = data->Values[i];

		if (isspace(c))
		{
			if (exitedWhitespace)
			{
				nameStart = i + 1;
				break;
			}

			continue;
		}

		exitedWhitespace = true;

		if (IsValidNameCharacter(c))
		{
			if (nameEnd is - 1)
			{
				nameEnd = i;
			}

			++nameLength;

			continue;
		}

		// contains invalid characters or is improperly formatted
		return false;
	}

	if (nameStart is - 1 or nameEnd is - 1)
	{
		return false;
	}

	*out_name = (tuple(int, int)){ nameStart, nameEnd - nameStart + 1 };

	return nameLength > 0;
}

private bool TryGetTypeReturnType(string data, tuple(int, int)* out_returnType)
{
	*out_returnType = (tuple(int, int)){ 0,0 };

	/*
		struct { int x; int y; } method();
		int method();
		int* method();
	*/
	int depth = 0;
	bool encounteredNameCharacter = false;
	int typeEnd = -1;
	int typeStart = -1;

	// iterate backwards
	// this DOESNT account for comments or strings
	for (int i = data->Count; i-- > 0;)
	{
		const int previousC = data->Values[i + 1];
		const int c = data->Values[i];
		const int nextC = data->Values[max(0, i - 1)];

		// ignore whitespace
		if (isspace(c))
		{
			if (encounteredNameCharacter)
			{
				typeStart = i + 1;

				// we want the struct portion of the type
				if (PreviousCharacterIgnoringWhiteSpace(stack_substring_front(data, i)) isnt 't')
				{
					break;
				}

				i = IndexOfPreviousCharacterIgnoringWhitespace(stack_substring_front(data, i));
			}

			continue;
		}

		// first non-whitespace character
		// denotes end of type
		if (typeEnd is - 1)
		{
			typeEnd = i;
		}

		// the start (end) of a type can only begin with *,}, or a normal c typename

		if (c is '}')
		{
			depth++;
			continue;
		}

		if (c is '{')
		{
			depth--;
			continue;
		}

		// ignore things in braces
		if (depth)
		{
			continue;
		}

		if (c is '*')
		{
			// can't have a type like int*float
			// only int***********
			if (encounteredNameCharacter)
			{
				return false;
			}

			continue;
		}

		if (IsValidNameCharacter(c))
		{
			encounteredNameCharacter = true;

			typeStart = i;

			continue;
		}

		return false;
	}

	// there should be a matching braces
	// otherwise its an invalid type
	// this DOESNT account for comments or strings
	if (depth isnt 0)
	{
		return false;
	}

	// we failed to find something
	if (typeStart is - 1 or typeEnd is - 1)
	{
		return false;
	}

	const int nameLength = typeEnd - typeStart + 1;

	*out_returnType = (tuple(int, int)){ typeStart, nameLength };

	// make sure the name isnt just struct
	if (nameLength is sizeof("struct") - 1)
	{
		return strings.Equals(stack_substring(data, typeStart, nameLength), stack_string("struct")) is false;
	}

	return true;
}

// proccesses the data before the alligator to see if it adheres to valid
// method signature requirements
// [void Method<]T>(T value);
private bool IsValidMethodSignature(string data)
{
	// make sure the end is the alligator
	Guard(data->Values[data->Count - 1] is '<');

	tuple(int, int) nameInfo;
	if (TryGetNameBeforeAlligator(stack_substring_front(data, data->Count - 2), &nameInfo) is false)
	{
		return false;
	}

	tuple(int, int) returnTypeInfo;
	if (TryGetTypeReturnType(stack_substring_front(data, nameInfo.First - 1), &returnTypeInfo) is false)
	{
		return false;
	}

	return true;
}

// checks for 
// struct name<T>{ T thing; };
// typedef struct <T>{ T thing; } name;
private bool IsGenericStruct(string data, int openAlligatorIndex, location* out_location)
{
	Guard(data->Count isnt 0);
	Guard(data->Values[openAlligatorIndex] is '<');

	// struct name<T>{ T thing; };
	// typedef struct <T>{ T thing; } name;

	string startThroughAlligator = stack_subarray(char, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex);

	const bool hasStructKeyword = Strings.Contains(startThroughAlligator->Values, startThroughAlligator->Count, "struct", sizeof("struct") - 1);

	if (hasStructKeyword is false)
	{
		return false;
	}

	const bool hasValidNameOrNoName = HasAllValidNameCharactersOrWhiteSpace(startThroughAlligator);

	if (hasValidNameOrNoName is false)
	{
		return false;
	}

	const int closingIndex = out_location->AlligatorEndIndex;

	if (closingIndex is - 1)
	{
		// no matching alligator
		return false;
	}

	// open brace must follow generic declaration
	const int nextCharacter = NextCharacterIgnoringWhiteSpace(stack_subarray_back(char, data, closingIndex + 1));

	if (nextCharacter isnt '{')
	{
		return false;
	}

	// at this point we know its a struct def
	string block = stack_subarray_back(char, data, closingIndex + 1);

	const int indexOfBrace = IndexOfClosingBrace(block) + closingIndex + 1;

	out_location->EndScopeIndex = indexOfBrace;

	out_location->Struct = true;

	return true;
}

// checks for
// void Method<T>(T value);
private bool IsGenericMethodDeclaration(string data, int openAlligatorIndex, location* out_location)
{
	Guard(data->Count isnt 0);
	Guard(data->Values[openAlligatorIndex] is '<');

	// method declarations have a return type, name, params, and a semicolon at the end
	string startThroughAlligator = stack_subarray(char, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex + 1);

	const bool hasValidMethodSignature = IsValidMethodSignature(startThroughAlligator);

	if (hasValidMethodSignature is false)
	{
		return false;
	}

	const int closingIndex = out_location->AlligatorEndIndex;

	if (closingIndex is - 1)
	{
		// no matching alligator
		return false;
	}

	// open brace must follow generic declaration
	const int nextCharacter = NextCharacterIgnoringWhiteSpace(stack_substring_back(data, closingIndex + 1));

	if (nextCharacter isnt '(')
	{
		return false;
	}

	// at this point we know its a struct def
	string block = stack_substring_back(data, closingIndex + 1);

	const int indexOfBrace = IndexOfClosingParen(block) + closingIndex + 1;

	// make sure the declaration is ended with a semicolon
	if (NextCharacterIgnoringWhiteSpace(stack_substring_back(data, indexOfBrace + 1)) isnt ';')
	{
		return false;
	}

	out_location->EndScopeIndex = indexOfBrace + IndexOfNextCharacterIgnoringWhitespace(stack_substring_back(data, indexOfBrace));

	out_location->Declaration = true;

	return true;
}

// checks for 
// void Method<T>(){}
private bool IsGenericMethodDefinition(string data, int openAlligatorIndex, location* out_location)
{
	Guard(data->Count isnt 0);
	Guard(data->Values[openAlligatorIndex] is '<');

	// method declarations have a return type, name, params, and a semicolon at the end
	string startThroughAlligator = stack_subarray(char, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex + 1);

	const bool hasValidMethodSignature = IsValidMethodSignature(startThroughAlligator);

	if (hasValidMethodSignature is false)
	{
		return false;
	}

	if (out_location->AlligatorEndIndex is - 1)
	{
		// no matching alligator
		return false;
	}

	// make sure we have parens
	string block = stack_substring_back(data, out_location->AlligatorEndIndex + 1);

	const int indexOfStartBrace = IndexOfClosingParen(block) + out_location->AlligatorEndIndex + 2;

	// make sure the signature ends with a brace to define the method body
	if (NextCharacterIgnoringWhiteSpace(stack_substring_back(data, indexOfStartBrace)) isnt '{')
	{
		return false;
	}

	const int indexOfEndBrace = IndexOfClosingBrace(stack_substring_back(data, indexOfStartBrace));

	if (indexOfEndBrace is - 1)
	{
		return false;
	}

	out_location->EndScopeIndex = indexOfStartBrace + indexOfEndBrace;
	out_location->Definition = true;

	return true;
}

private location IdentifyGenericCall(string data, int depth, int openAlligatorIndex, int lastMacroEndIndex)
{
	location result = { 0 };

	result.AlligatorStartIndex = openAlligatorIndex;

	const int start = IndexOfLastBlockExpressionOrMacro(data, openAlligatorIndex, lastMacroEndIndex);

	result.StartScopeIndex = start;

	const int closingIndex = IndexOfClosingAlligator(data, depth, openAlligatorIndex);

	result.AlligatorEndIndex = closingIndex;

	const bool isGenericStruct = IsGenericStruct(data, openAlligatorIndex, &result);

	const bool isGenericMethodDeclaration = IsGenericMethodDeclaration(data, openAlligatorIndex, &result);

	const bool isGenericMethodDefinition = IsGenericMethodDefinition(data, openAlligatorIndex, &result);

	// cover things like
	// myStruct<int> myStruct = { 0 };
	// callMyMethod<int>();
	// int result = (int)(MACRO()OtherCall().MyGeneric<float>() * 10);
	if ((isGenericStruct or isGenericMethodDeclaration or isGenericMethodDefinition) is false)
	{
		result.Call = true;
	}

	return result;
}

private array(location) GetGenericLocations(string data)
{
	// the start and end index of macro locations we can ignore when
	// walking backwards
	array(tuple(int, int)) macroLocations = Arrays(tuple(int, int)).Create(0);
	array(location) locations = Arrays(location).Create(0);

	bool inMacro = false;
	bool inString = false;
	bool inComment = false;
	bool inMultiLineComment = false;
	int depth = 0;

	int macroStart = 0;
	int macroEnd = 0;
	for (int i = 0; i < data->Count; i++)
	{
		int previousC = data->Values[max(0, i - 1)];
		int c = data->Values[i];
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		// check to see if we're in a string
		if (c is '"' or c is '\'')
		{
			// look back and check to see if it's escaped
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		// ignore everything inside of strings, we dont care
		if (inString)
		{
			continue;
		}

		if (c is '/' and nextC is '/')
		{
			inComment = true;
			// skip past next char since we know what it is
			i++;
			continue;
		}

		if (c is '/' and nextC is '*')
		{
			inMultiLineComment = true;
			// skip past next char since we know what it is
			i++;
			continue;
		}

		if (c is '*' and nextC is '/')
		{
			inMultiLineComment = false;
			// skip past next char since we know what it is
			i++;
			continue;
		}

		// skip everything if we're in a big comment
		if (inMultiLineComment)
		{
			continue;
		}

		// check for macros
		if (c is '#' && !inMacro)
		{
			inMacro = true;
			macroStart = i;
			continue;
		}

		// check to see if we're at a newline
		if ((c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\')))
		{
			if (inMacro)
			{
				inMacro = false;
				macroEnd = i;

				Arrays(tuple(int, int)).Append(macroLocations, (tuple(int, int)) { macroStart, macroEnd });

				macroStart = 0;
				// keep last macro position
			}

			if (inComment)
			{
				inComment = false;
			}

			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro)
		{
			continue;
		}

		// check to see if this is the start of a generic definition
		if (c is '<')
		{
			// check to see if it's actually a generic definition or 
			// something else
			if (LookAheadIsGenericCall(stack_subarray_back(char, data, i + 1)))
			{
				location callLocation = IdentifyGenericCall(data, depth, i, macroEnd);

				Arrays(location).Append(locations, callLocation);
			}
		}
	}

	return locations;
}

private array(string) ReadTokensFromString(string data)
{
	array(string) result = Arrays(string).Create(0);

	return result;
}

// Reads the tokens from the provided file
array(string) ReadTokens(string path)
{
	string data = Files.ReadAll(path);

	array(string) result = ReadTokensFromString(data);

	return result;
}

TEST(IndexOfLastBlockExpressionOrMacro)
{
	string data = stack_string("struct thing{int x; int y}; start");
	IsEqual(28, IndexOfLastBlockExpressionOrMacro(data, data->Count, 0));

	data = stack_string("struct <T>{};");
	IsEqual(data->Count - 1, (size_t)IndexOfLastBlockExpressionOrMacro(data, data->Count, 0));

	data = stack_string("static inline void start");
	IsEqual(0, IndexOfLastBlockExpressionOrMacro(data, data->Count, 0));

	data = stack_string("void main(){} T myType<T>();");
	IsEqual(12, IndexOfLastBlockExpressionOrMacro(data, data->Count - 6, 0));

	data = stack_string("void main(){  int x = 13; T myType<T>(); }");
	IsEqual(26, IndexOfLastBlockExpressionOrMacro(data, data->Count - 8, 0));

	return true;
}

TEST(IsGenericMethodDeclaration)
{
	string data = stack_string("struct <T>{};");

	location dataLocation = {
		.AlligatorStartIndex = 7,
		.AlligatorEndIndex = 9,
		.StartScopeIndex = 0
	};

	IsFalse(IsGenericMethodDeclaration(data, 7, &dataLocation));

	data = stack_string("void MyMethod<T>();");
	dataLocation = (location){
		.AlligatorStartIndex = 13,
		.AlligatorEndIndex = 15,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDeclaration(data, 13, &dataLocation));

	data = stack_string("struct myStruct{int x; int y;} MyMethod<T>();");
	dataLocation = (location){
		.AlligatorStartIndex = 39,
		.AlligatorEndIndex = 41,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDeclaration(data, 39, &dataLocation));

	data = stack_string("struct myStruct{int x; int y;} MyMethod<T,U,V,W>();");
	dataLocation = (location){
		.AlligatorStartIndex = 39,
		.AlligatorEndIndex = 47,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDeclaration(data, 39, &dataLocation));

	data = stack_string("struct myStruct{int x; int y;} MyMethod<T,U,V,W>(int x, int y, int z);");
	IsTrue(IsGenericMethodDeclaration(data, 39, &dataLocation));

	data = stack_string("struct myStruct{int x; int y;} MyMethod<T,U,V,W>(int x, int y, int z, struct vector2{int x; int y;});");
	IsTrue(IsGenericMethodDeclaration(data, 39, &dataLocation));

	data = stack_string("struct myStruct{int x; int y;} MyMethod<T,U,V,W>(int x, int y, int z, struct vector2{int x; int y;} /* co}mment he)re */);");
	IsTrue(IsGenericMethodDeclaration(data, 39, &dataLocation));

	data = stack_string("int x = MyMethod<int>(14);");
	dataLocation = (location){
		.AlligatorStartIndex = 16,
		.AlligatorEndIndex = 20,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericMethodDeclaration(data, 16, &dataLocation));

	data = stack_string("*MyMethod<T>(12) = 14;");
	dataLocation = (location){
		.AlligatorStartIndex = 9,
		.AlligatorEndIndex = 11,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericMethodDeclaration(data, 9, &dataLocation));

	data = stack_string("float x = MyMethod<float>() + MyMethod<int>(33);");
	dataLocation = (location){
		.AlligatorStartIndex = 18,
		.AlligatorEndIndex = 24,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericMethodDeclaration(data, 18, &dataLocation));

	return true;
}

TEST(TryGetTypeReturnType)
{
	string data = stack_string("void MyMethod<T>();");
	tuple(int, int) result;

	data = stack_string("void ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("void")));

	data = stack_string("struct vector2{ float x; float y; } ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("struct vector2{ float x; float y; }")));

	data = stack_string("struct vector4{ struct vector3{ struct vector2{float x; float y;}; float z;}; float w; } ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("struct vector4{ struct vector3{ struct vector2{float x; float y;}; float z;}; float w; }")));

	data = stack_string("int* ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("int*")));

	data = stack_string("static inline int**************** ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("int****************")));

	data = stack_string("int*float* ");
	IsFalse(TryGetTypeReturnType(data, &result));

	data = stack_string("int x = y;int* x = ");
	IsFalse(TryGetTypeReturnType(data, &result));

	return true;
}

TEST(TryGetNameBeforeAlligator)
{
	string data = stack_string("void MyMethod<T>();");
	tuple(int, int) result;

	IsFalse(TryGetNameBeforeAlligator(data, &result));
	IsZero(result.First);
	IsZero(result.Second);

	data = stack_string("void MyMethod");
	IsTrue(TryGetNameBeforeAlligator(data, &result));
	IsTrue(strings.Equals(stack_subarray(char, data, result.First, result.Second), stack_string("MyMethod")));

	data = stack_string("void MyMethod;");
	IsFalse(TryGetNameBeforeAlligator(data, &result));
	IsZero(result.First);
	IsZero(result.Second);

	data = stack_string("void MyMe*thod");
	IsFalse(TryGetNameBeforeAlligator(data, &result));
	IsZero(result.First);
	IsZero(result.Second);

	return true;
}

TEST(IndexOfClosingBrace)
{
	string data = stack_string("{};");

	IsEqual(1, IndexOfClosingBrace(data));

	data = stack_string("{ };");
	IsEqual(2, IndexOfClosingBrace(data));

	data = stack_string("{ int x; };");
	IsEqual(9, IndexOfClosingBrace(data));

	data = stack_string("{ struct sub_struct{ struct subsubstruct { int x; int y; }} }");
	IsEqual(60, IndexOfClosingBrace(data));

	data = stack_string("{ struct sub_struct{ struct subsubstruct { int x; int y; }} myVar; /* }comme}{}}{{{nt{}} */ int Index; // other}}}}}}{{{[[[{{Comment\n\tfloat Value; }");
	IsEqual(147, IndexOfClosingBrace(data));

	return true;
}

TEST(IndexOfClosingAlligator)
{
	string data = stack_string("#include <stdio.h> // comment int Create<int>(); here\nint i = 0; struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;}; int main(){return 0;} int GreaterThan(int left, int right){ return left < right; }");
	const int endAlligatorIndex = IndexOfClosingAlligator(data, 0, 74);
	IsEqual(76, endAlligatorIndex);

	return true;
}

TEST(IsGenericStruct)
{
	string data = stack_string("struct <T>{};");

	location dataLocation = {
		.AlligatorStartIndex = 7,
		.AlligatorEndIndex = 9,
		.StartScopeIndex = 0
	};

	IsTrue(IsGenericStruct(data, 7, &dataLocation));

	data = stack_string("struct<T>{};");
	dataLocation = (location){
		.AlligatorStartIndex = 6,
		.AlligatorEndIndex = 8,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericStruct(data, 6, &dataLocation));

	data = stack_string("struct	<T> { } ;");
	dataLocation = (location){
		.AlligatorStartIndex = 7,
		.AlligatorEndIndex = 9,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericStruct(data, 7, &dataLocation));

	data = stack_string("struct mystruct<T>{};");
	dataLocation = (location){
		.AlligatorStartIndex = 15,
		.AlligatorEndIndex = 17,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericStruct(data, 15, &dataLocation));

	data = stack_string("typedef struct <T>{int x; int y;} vector2;");
	IsTrue(IsGenericStruct(data, 15, &dataLocation));

	data = stack_string("array<T> myVar = { 0 };");
	dataLocation = (location){
		.AlligatorStartIndex = 5,
		.AlligatorEndIndex = 7,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericStruct(data, 5, &dataLocation));

	data = stack_string("int x = Arrays<int>.Sum(myArray)");
	dataLocation = (location){
		.AlligatorStartIndex = 14,
		.AlligatorEndIndex = 16,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericStruct(data, 14, &dataLocation));

	data = stack_string("struct vector2 MyMethod<T>();");
	dataLocation = (location){
		.AlligatorStartIndex = 23,
		.AlligatorEndIndex = 25,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericStruct(data, 23, &dataLocation));

	data = stack_string("struct vector2 MyMethod<T>(){ return (struct vector2){ 0,0 };}");
	IsFalse(IsGenericStruct(data, 23, &dataLocation));

	return true;
}

TEST(IsGenericMethodDefinition)
{
	string data = stack_string("struct <T>{};");

	location dataLocation = {
		.AlligatorStartIndex = 7,
		.AlligatorEndIndex = 9,
		.StartScopeIndex = 0
	};

	IsFalse(IsGenericMethodDefinition(data, 7, &dataLocation));

	data = stack_string("void Method<T>();");
	dataLocation = (location){
		.AlligatorStartIndex = 11,
		.AlligatorEndIndex = 13,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericMethodDefinition(data, 11, &dataLocation));

	data = stack_string("void Method<T>(int x, int y, /*comment)*/ float y);");
	IsFalse(IsGenericMethodDefinition(data, 11, &dataLocation));

	data = stack_string("void Method<T>(){}");
	IsTrue(IsGenericMethodDefinition(data, 11, &dataLocation));

	data = stack_string("int Method<T>(int left, int right){ struct result{int x; const char* s = \"thi)}g\"}; if(left && right){ return left; } return right + Method(); }");
	dataLocation = (location){
		.AlligatorStartIndex = 10,
		.AlligatorEndIndex = 12,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDefinition(data, 10, &dataLocation));

	data = stack_string("struct Method<T>{}");
	dataLocation = (location){
		.AlligatorStartIndex = 13,
		.AlligatorEndIndex = 15,
		.StartScopeIndex = 0
	};
	IsFalse(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>(){}");
	dataLocation = (location){
		.AlligatorStartIndex = 13,
		.AlligatorEndIndex = 15,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>(){}");
	dataLocation = (location){
		.AlligatorStartIndex = 13,
		.AlligatorEndIndex = 15,
		.StartScopeIndex = 0
	};
	IsTrue(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>..{}");
	IsFalse(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>(.{}");
	IsFalse(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>(({}");
	IsFalse(IsGenericMethodDefinition(data, 13, &dataLocation));

	data = stack_string("stract Method<T>(({});");
	IsFalse(IsGenericMethodDefinition(data, 13, &dataLocation));


	return true;
}

TEST(LookAheadIsGenericCall)
{
	IsTrue(LookAheadIsGenericCall(stack_string("T>")));
	IsTrue(LookAheadIsGenericCall(stack_string("T,T>")));
	IsTrue(LookAheadIsGenericCall(stack_string("T,T1,T2,T3,T4,TResult,Tasiojsioqwjd>")));
	IsTrue(LookAheadIsGenericCall(stack_string("struct vector2{ int x; int y; }, T2, T3>")));
	IsTrue(LookAheadIsGenericCall(stack_string("struct vector3{ struct vector2{ int x; int y;}; int y; }, T2, T3>")));
	IsTrue(LookAheadIsGenericCall(stack_string("T>();")));
	IsTrue(LookAheadIsGenericCall(stack_string("T>(){ return 5 < 12; }")));
	IsTrue(LookAheadIsGenericCall(stack_string("T,int*,int**,float*,double>(){ return 5 < 12; }")));
	IsTrue(LookAheadIsGenericCall(stack_string("T,array<array<array<int>>>>();")));

	IsTrue(LookAheadIsGenericCall(stack_string("T>T1")));
	IsTrue(LookAheadIsGenericCall(stack_string("T>T1 = array<T>.Create(0)")));

	IsFalse(LookAheadIsGenericCall(stack_string(">")));
	IsFalse(LookAheadIsGenericCall(stack_string("")));
	IsFalse(LookAheadIsGenericCall(stack_string("y")));
	IsFalse(LookAheadIsGenericCall(stack_string("y;")));


	return true;
}

TEST(GetGenericLocations)
{
	string data = stack_string("#include <stdio.h> // comment int Create<int>(); here\nint i = 0; struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;}; int main(){return 0;} int GreaterThan(int left, int right){ return left < right; }");

	array(location) locations = GetGenericLocations(data);

	location result = locations->Values[0];

	IsEqual((size_t)1, locations->Count);
	IsEqual(74, result.AlligatorStartIndex);
	IsEqual(76, result.AlligatorEndIndex);
	IsEqual(65, result.StartScopeIndex);
	IsEqual(108, result.EndScopeIndex);
	IsTrue(result.Struct);
	IsFalse(result.Definition);
	IsFalse(result.Call);
	IsFalse(result.Declaration);

	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(IndexOfLastBlockExpressionOrMacro)
	APPEND_TEST(LookAheadIsGenericCall)
	APPEND_TEST(IsGenericStruct)
	APPEND_TEST(IndexOfClosingBrace)
	APPEND_TEST(IndexOfClosingAlligator)
	APPEND_TEST(TryGetNameBeforeAlligator)
	APPEND_TEST(TryGetTypeReturnType)
	APPEND_TEST(IsGenericMethodDeclaration)
	APPEND_TEST(IsGenericMethodDefinition)
	APPEND_TEST(GetGenericLocations)
);

void RunTests()
{
	RunUnitTests();
}