#pragma once

#include "core/array.h"
#include "sourceParser.h"
#include "core/file.h"
#include "core/strings.h"
#include "core/cunit.h"
#include <string.h>
#include <ctype.h>
#include "core/guards.h"
#include "helpers.c"
#include "core/runtime.h"

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

private bool TryGetTypeReturnType(string data, tuple(int, int)* out_returnType)
{
	*out_returnType = (tuple(int, int)){ 0,0 };

	/*
		struct { int x; int y; } method();
		int method();
		int* method();
	*/
	int depth = 0;
	bool encounteredNamebyteacter = false;
	int typeEnd = -1;
	int typeStart = -1;

	// iterate backwards
	// this DOESNT account for comments or strings
	for (int i = data->Count; i-- > 0;)
	{
		const int previousC = data->Values[i + 1];
		const int c = at(data, i);
		const int nextC = at(data, max(0, i - 1));

		// ignore whitespace
		if (isspace(c))
		{
			if (encounteredNamebyteacter)
			{
				typeStart = i + 1;

				// we want the struct portion of the type
				if (PreviousbyteacterIgnoringWhiteSpace(stack_substring_front(data, i)) isnt 't')
				{
					break;
				}

				i = IndexOfPreviousbyteacterIgnoringWhitespace(stack_substring_front(data, i));
			}

			continue;
		}

		// first non-whitespace byteacter
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
			if (encounteredNamebyteacter)
			{
				return false;
			}

			continue;
		}

		if (IsValidNameCharacter(c))
		{
			encounteredNamebyteacter = true;

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

	string startThroughAlligator = stack_subarray(byte, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex);

	const bool hasStructKeyword = Strings.Contains(startThroughAlligator->Values, startThroughAlligator->Count, "struct", sizeof("struct") - 1);

	if (hasStructKeyword is false)
	{
		return false;
	}

	const bool hasValidNameOrNoName = HasAllValidNamebyteactersOrWhiteSpace(startThroughAlligator);

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
	const int nextbyteacter = NextbyteacterIgnoringWhiteSpace(stack_subarray_back(byte, data, closingIndex + 1));

	if (nextbyteacter isnt '{')
	{
		return false;
	}

	// at this point we know its a struct def
	string block = stack_subarray_back(byte, data, closingIndex + 1);

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
	string startThroughAlligator = stack_subarray(byte, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex + 1);

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
	const int nextbyteacter = NextbyteacterIgnoringWhiteSpace(stack_substring_back(data, closingIndex + 1));

	if (nextbyteacter isnt '(')
	{
		return false;
	}

	// at this point we know its a struct def
	string block = stack_substring_back(data, closingIndex + 1);

	const int indexOfBrace = IndexOfClosingParen(block) + closingIndex + 1;

	// make sure the declaration is ended with a semicolon
	if (NextbyteacterIgnoringWhiteSpace(stack_substring_back(data, indexOfBrace + 1)) isnt ';')
	{
		return false;
	}

	out_location->EndScopeIndex = 1 + indexOfBrace + IndexOfNextbyteacterIgnoringWhitespace(stack_substring_back(data, indexOfBrace));

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
	string startThroughAlligator = stack_subarray(byte, data, out_location->StartScopeIndex, openAlligatorIndex - out_location->StartScopeIndex + 1);

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
	if (NextbyteacterIgnoringWhiteSpace(stack_substring_back(data, indexOfStartBrace)) isnt '{')
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
		// end scope not needed, since calls are simple compression and replacement
		// of the alligator args and nothing more
		result.EndScopeIndex = -1;
	}

	return result;
}

array(location) GetGenericLocations(string data)
{
	// the start and end index of macro locations we can ignore when
	// walking backwards
	array(tuple(int, int)) macroLocations = arrays(tuple(int, int)).Create(0);
	array(location) locations = arrays(location).Create(0);

	bool inMacro = false;
	bool inString = false;
	bool inComment = false;
	bool inMultiLineComment = false;
	int depth = 0;

	int macroStart = 0;
	int macroEnd = 0;
	for (int i = 0; i < data->Count; i++)
	{
		int previousC = at(data, max(0, i - 1));
		int c = at(data, i);
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
			// skip past next byte since we know what it is
			i++;
			continue;
		}

		if (c is '/' and nextC is '*')
		{
			inMultiLineComment = true;
			// skip past next byte since we know what it is
			i++;
			continue;
		}

		if (c is '*' and nextC is '/')
		{
			inMultiLineComment = false;
			// skip past next byte since we know what it is
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

				arrays(tuple(int, int)).Append(macroLocations, (tuple(int, int)) { macroStart, macroEnd });

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
			if (LookAheadIsGenericCall(stack_subarray_back(byte, data, i + 1)))
			{
				location callLocation = IdentifyGenericCall(data, depth, i, macroEnd);

				arrays(location).Append(locations, callLocation);
			}
		}
	}

	return locations;
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
	IsEqual(14, IndexOfLastBlockExpressionOrMacro(data, data->Count - 6, 0));

	data = stack_string("void main(){  int x = 13; T myType<T>(); }");
	IsEqual(26, IndexOfLastBlockExpressionOrMacro(data, data->Count - 8, 0));

	data = stack_string("void main(){} tuple(T,T) Type<T>();");
	IsEqual(14, IndexOfLastBlockExpressionOrMacro(data, data->Count - 6, 0));


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
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("void")));

	data = stack_string("struct vector2{ float x; float y; } ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("struct vector2{ float x; float y; }")));

	data = stack_string("struct vector4{ struct vector3{ struct vector2{float x; float y;}; float z;}; float w; } ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("struct vector4{ struct vector3{ struct vector2{float x; float y;}; float z;}; float w; }")));

	data = stack_string("int* ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("int*")));

	data = stack_string("static inline int**************** ");
	IsTrue(TryGetTypeReturnType(data, &result));
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("int****************")));

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
	IsTrue(strings.Equals(stack_subarray(byte, data, result.First, result.Second), stack_string("MyMethod")));

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

	data = stack_string("int Method<T>(int left, int right){ struct result{int x; const byte* s = \"thi)}g\"}; if(left && right){ return left; } return right + Method(); }");
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
	// struct definition
	string data = stack_string("#include <stdio.h> // comment int Create<int>(); here\nint i = 0; struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;}; int main(){return 0;} int GreaterThan(int left, int right){ return left < right; }");

	array(location) locations = GetGenericLocations(data);

	location result = at(locations, 0);

	IsEqual((size_t)1, locations->Count);
	IsEqual(74, result.AlligatorStartIndex);
	IsEqual(76, result.AlligatorEndIndex);
	IsEqual(65, result.StartScopeIndex);
	IsEqual(108, result.EndScopeIndex);
	IsTrue(result.Struct);
	IsFalse(result.Definition);
	IsFalse(result.Call);
	IsFalse(result.Declaration);

	// Method declaration
	data = stack_string("#include <stdio.h>\n int Create<int>(); here int i = 0; /*struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;};*/nt main(){return 0;} int GreaterThan(int left, int right){ return left < right; }");

	locations = GetGenericLocations(data);

	result = at(locations, 0);

	IsEqual((size_t)1, locations->Count);
	IsEqual(30, result.AlligatorStartIndex);
	IsEqual(34, result.AlligatorEndIndex);
	IsEqual(20, result.StartScopeIndex);
	IsEqual('i', at(data, 20));
	IsEqual(37, result.EndScopeIndex);
	IsFalse(result.Struct);
	IsFalse(result.Definition);
	IsFalse(result.Call);
	IsTrue(result.Declaration);

	// Method definiton
	data = stack_string("#include <stdio.h>\n/*int Create<int>();*/ here int i = 0; /*struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;};*/nt main(){return 0;} int GreaterThan<int>(int left, int right) { return left < right; }");

	locations = GetGenericLocations(data);

	result = at(locations, 0);

	IsEqual((size_t)1, locations->Count);
	IsEqual(143, result.AlligatorStartIndex);
	IsEqual(147, result.AlligatorEndIndex);
	IsEqual(128, result.StartScopeIndex);
	IsEqual('}', at(data, 126));
	IsEqual(193, result.EndScopeIndex);
	IsFalse(result.Struct);
	IsTrue(result.Definition);
	IsFalse(result.Call);
	IsFalse(result.Declaration);

	// call
	data = stack_string("#include <stdio.h>\n/*int Create<int>();*/ int i = GetNum<int>(\"12\"); /*struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;};*/nt main(){return 0;} int GreaterThan<(int left, int right) { return left < right; }");

	locations = GetGenericLocations(data);

	result = at(locations, 0);

	IsEqual((size_t)1, locations->Count);
	IsEqual(56, result.AlligatorStartIndex);
	IsEqual(60, result.AlligatorEndIndex);
	IsEqual(42, result.StartScopeIndex);
	IsEqual(-1, result.EndScopeIndex);
	IsFalse(result.Struct);
	IsFalse(result.Definition);
	IsTrue(result.Call);
	IsFalse(result.Declaration);

	// make sure generics can be used as a>return type
	data = stack_string("#define Thing(x)\\x\n\tarray<int> GetArray(struct myThing{14,15});/*int Create<int>();*/ int i = GetNum<int>(\"12\"); /*struct my<T>{/* <C,O,M,M,E,N,T> */ T Value;};*/nt main(){return 0;} int GreaterThan<(int left, int right) { return left < right; }");

	locations = GetGenericLocations(data);

	result = at(locations, 0);

	IsEqual((size_t)2, locations->Count);
	IsEqual(25, result.AlligatorStartIndex);
	IsEqual(29, result.AlligatorEndIndex);
	IsEqual(20, result.StartScopeIndex);
	IsEqual('a', at(data, 20));
	IsEqual(-1, result.EndScopeIndex);
	IsFalse(result.Struct);
	IsFalse(result.Definition);
	IsTrue(result.Call);
	IsFalse(result.Declaration);

	// make sure generics can be used in method calls without being considered a method call
	data = stack_string("#include <stdio.h>\n\r\tT GetSum<T>(array<int> arr){return (T)arr->Data[0];}");

	locations = GetGenericLocations(data);

	// first result should be the method
	// but second should be the call within the method
	IsEqual((size_t)2, locations->Count);

	result = at(locations, 1);

	IsEqual(38, result.AlligatorStartIndex);
	IsEqual((byte)'y', at(data, 37));
	IsEqual(42, result.AlligatorEndIndex);
	IsEqual(33, result.StartScopeIndex);
	IsEqual(-1, result.EndScopeIndex);
	IsFalse(result.Struct);
	IsFalse(result.Definition);
	IsTrue(result.Call);
	IsFalse(result.Declaration);

	return true;
}

TEST(IndexOfClosingParen)
{
	string data = stack_string("(.{}");

	int expected = -1;
	int actual = IndexOfClosingParen(data);

	IsEqual(expected, actual);

	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(IndexOfLastBlockExpressionOrMacro)
	APPEND_TEST(LookAheadIsGenericCall)
	APPEND_TEST(IsGenericStruct)
	APPEND_TEST(IndexOfClosingBrace)
	APPEND_TEST(IndexOfClosingAlligator)
	APPEND_TEST(IndexOfClosingParen)
	APPEND_TEST(TryGetNameBeforeAlligator)
	APPEND_TEST(TryGetTypeReturnType)
	APPEND_TEST(IsGenericMethodDeclaration)
	APPEND_TEST(IsGenericMethodDefinition)
	APPEND_TEST(GetGenericLocations)
);

OnStart(1) {
	RunUnitTests();
}
