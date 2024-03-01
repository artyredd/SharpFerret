#include "GenericExpansion.h"
#include "core/cunit.h"

#include "core/file.h"
#include "core/strings.h"
#include <string.h>
#include <ctype.h>
#include "core/guards.h"
#include "helpers.c"

#define MAX_ARG_LENGTH 4096

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

TEST_SUITE(RunUnitTests,
	APPEND_TEST(FlattenArgumentToCName)
	APPEND_TEST(ExpandCall)
);

void RunExpansionTests()
{
	RunUnitTests();
}