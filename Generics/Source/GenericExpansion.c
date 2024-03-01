#include "GenericExpansion.h"
#include "core/cunit.h"

#include "core/file.h"
#include "core/strings.h"
#include <string.h>
#include <ctype.h>
#include "core/guards.h"
#include "helpers.c"

#define MAX_ARG_LENGTH 4096


typedef struct {
	// list of type names within the 
	array(string) TypeNames;
	// First is the typeName, Second is the index its located at
	array(tuple(int, int)) TypeLocations;
} genericTokens;


// Reduces any arument to a CName
// buffer should be 4 times len of arg
// ex.                        int* | int_ptr
//		struct vec2{int x; int y;} | struct_vec2_int_x_int_y_
//		              MACRO(thing) | MACRO_thing_
// ARG(struct MAKE_NAME(2){ int**** First; float second; }) ARG_struct_MAKE_NAME_2_int_ptr_ptr_ptr_ptr_First_float_second_
string FlattenArgumentToCName(string arg, string stack_buffer)
{
	// due to * expanding to _ptr
	// empty buffer should be larger than the provided string
	// incase of <int************************************>
	Guard(stack_buffer->Capacity >= (arg->Count * 4));

	string result = stack_buffer;

	// prevent int_______float___________name____macro
	for (int i = 0; i < arg->Count; i++)
	{
		const int c = arg->Values[i];

		if (IsValidNameCharacter(c))
		{
			strings.Append(result, c);
			continue;
		}

		if (c is '*')
		{
			// do stuff
			strings.AppendCArray(result, "_ptr", sizeof("_ptr") - 1);
			continue;
		}

		if (result->Values[result->Count - 1] != '_')
		{
			strings.Append(result, '_');
			continue;
		}
	}

	return stack_buffer;
}

array(string) GetGenericArguments(string data, location location)
{
	array(string) result = dynamic_array(string, 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;
	int parenDepth = 0;

	int nameStartIndex = 1;

	string subdata = stack_substring(data, location.AlligatorStartIndex, location.AlligatorEndIndex - location.AlligatorStartIndex + 1);

	for (int i = 1; i < subdata->Count; i++)
	{
		int previousC = subdata->Values[max(0, i - 1)];
		int c = subdata->Values[i];
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = subdata->Values[i + 1];

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

		if (inSingleComment or inMultiComment)
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
			continue;
		}
		if (c is '}')
		{
			--depth;
			continue;
		}
		if (c is '(')
		{
			parenDepth++;
			continue;
		}
		if (c is ')')
		{
			parenDepth--;
			continue;
		}

		// if we're in the middle of defining a struct
		// or calling a macro skip
		//          \/
		// <struct { }>
		// <MACRO(13 )>
		if (depth or parenDepth)
		{
			continue;
		}

		// end of 
		if (c is ',')
		{
			string stackName = stack_substring(subdata, nameStartIndex, i - nameStartIndex);
			string name = empty_dynamic_string(stackName->Count);
			strings.AppendArray(name, stackName);
			Arrays(string).Append(result, name);
			// the next index is the start of the next type name
			nameStartIndex = i + 1;
		}

		if (i is subdata->Count - 1 and nameStartIndex > -1)
		{
			string stackName = stack_substring(subdata, nameStartIndex, i - nameStartIndex);
			string name = empty_dynamic_string(stackName->Count);
			strings.AppendArray(name, stackName);
			Arrays(string).Append(result, name);
		}
	}

	// check for single argument
	// or no argument
	if (result->Count is 0)
	{
		if (subdata->Count is 2)
		{
			fprintf_red(stderr, "A minium of one type argument is required when using generics.", "");
			throw(MissingGenericArgumentException);
		}

		// assume a single argument
		string stackName = stack_substring(subdata, 1, subdata->Count - 2);
		string name = empty_dynamic_string(stackName->Count);
		strings.AppendArray(name, stackName);
		Arrays(string).Append(result, name);
	}

	return result;
}

genericTokens GetGenericTokens(string data, location location)
{
	return (genericTokens) { 0 };
}

void ExpandCall(string data, location location)
{
	// calls just get replaced with a valid C name
	// and the preprocessor and compiler will do lookup
	const size_t start = location.AlligatorStartIndex;
	const size_t count = location.AlligatorEndIndex - start + 1;

	string buffer = empty_stack_array(char, MAX_ARG_LENGTH);

	string arguments = stack_substring(data, start, count);

	string flattened = FlattenArgumentToCName(arguments, buffer);

	// we dont know for sure the flattend name is the same length
	// it may be shorter or longer, shift old text left over it
	strings.RemoveRange(data, start, count);
	strings.InsertArray(data, flattened, start);
}

void ExpandMethodDeclaration(string data, location location)
{
	// T MyMethod<T>(T, T);
}

void ExpandGeneric(string data, location location)
{
	Guard(data->Count > 0);
	Guard(location.Call or location.Declaration or location.Definition or location.Struct);

	if (location.Call)
	{
		ExpandCall(data, location);
	}
	else if (location.Declaration)
	{
		//ExpandDeclaration(data, location);
	}
	else if (location.Definition)
	{
		//ExpandDefinition(data, location);
	}
	else if (location.Struct)
	{
		//ExpandStruct(data, location);
	}
}

void ExpandGenerics(string data, array(location) locations)
{
	// expand backwards so the indexes dont change
	int i = locations->Count;
	while (i-- > 0)
	{
		location location = locations->Values[i];

		ExpandGeneric(data, location);
	}
}

TEST(FlattenArgumentToCName)
{
	string data = stack_string("");
	string result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string(""), FlattenArgumentToCName(data, result)));

	data = stack_string("int");
	result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string("int"), FlattenArgumentToCName(data, result)));

	data = stack_string("int*");
	result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string("int_ptr"), FlattenArgumentToCName(data, result)));

	data = stack_string("int**");
	result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string("int_ptr_ptr"), FlattenArgumentToCName(data, result)));

	data = stack_string("struct vec2{ int x; int y; }");
	result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string("struct_vec2_int_x_int_y_"), FlattenArgumentToCName(data, result)));

	data = stack_string("<T,U,V,X,Y,Z>");
	result = empty_stack_array(char, 1024);

	IsTrue(strings.Equals(stack_string("_T_U_V_X_Y_Z_"), FlattenArgumentToCName(data, result)));

	return true;
}

TEST(ExpandCall)
{
	string data = dynamic_string("int x = MyMethod<int>(24);");
	array(location) locations = GetGenericLocations(data);

	IsEqual((size_t)1, locations->Count);

	ExpandGeneric(data, locations->Values[0]);

	string expected = stack_string("int x = MyMethod_int_(24);");

	IsTrue(strings.Equals(expected, data));

	return true;
}

TEST(GetGenericArguments)
{
	string data = stack_string("<T>");
	location genericLocation = {
		.AlligatorStartIndex = 0,
		.AlligatorEndIndex = data->Count - 1
	};

	array(string) expected = stack_array(string, stack_string("T"));
	array(string) actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);
	IsTrue(strings.Equals(actual->Values[0], expected->Values[0]));

	data = stack_string("<T,U,V>");
	genericLocation = (location){
		.AlligatorStartIndex = 0,
		.AlligatorEndIndex = data->Count - 1
	};

	expected = stack_array(string, stack_string("T"), stack_string("U"), stack_string("V"));
	actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);

	IsTrue(strings.Equals(actual->Values[0], expected->Values[0]));
	IsTrue(strings.Equals(actual->Values[1], expected->Values[1]));
	IsTrue(strings.Equals(actual->Values[2], expected->Values[2]));


	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(FlattenArgumentToCName)
	APPEND_TEST(ExpandCall)
	APPEND_TEST(GetGenericArguments)
);

void RunExpansionTests()
{
	RunUnitTests();
}