#include "GenericExpansion.h"
#include "core/cunit.h"

#include "Compiler.h"
#include "core/file.h"
#include "core/guards.h"
#include "core/runtime.h"
#include "core/strings.h"
#include "helpers.c"
#include <ctype.h>
#include <string.h>

#define MAX_ARG_LENGTH 4096

// Reduces any arument to a CName
// buffer should be 4 times len of arg
// ex.                        int* | int_ptr
//		struct vec2{int x; int y;} | struct_vec2_int_x_int_y_
//		              MACRO(thing) | MACRO_thing_
// ARG(struct MAKE_NAME(2){ int**** First; float second; })
// ARG_struct_MAKE_NAME_2_int_ptr_ptr_ptr_ptr_First_float_second_
string FlattenArgumentToCName(string arg, string stack_buffer) {
	// due to * expanding to _ptr
	// empty buffer should be larger than the provided string
	// incase of <int************************************>
	Guard(stack_buffer->Capacity >= (arg->Count * 4));

	string result = stack_buffer;

	// prevent int_______float___________name____macro
	for (int i = 0; i < arg->Count; i++) {
		const int c = at(arg, i);

		if (IsValidNameCharacter(c)) {
			strings.Append(result, c);
			continue;
		}

		if (c is '*') {
			// do stuff
			strings.AppendCArray(result, "_ptr", sizeof("_ptr") - 1);
			continue;
		}

		if (unsafe_at(result, safe_subtract(result->Count, 1)) != '_') {
			strings.Append(result, '_');
			continue;
		}
	}

	return stack_buffer;
}

array(string) GetGenericArguments(string data, location location) {
	array(string) result = dynamic_array(string, 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;
	int parenDepth = 0;

	int nameStartIndex = 1;

	string subdata = stack_substring(data, location.AlligatorStartIndex,
		location.AlligatorEndIndex -
		location.AlligatorStartIndex + 1);

	for (int i = 1; i < subdata->Count; i++) {
		int previousC = at(subdata, max(0, i - 1));
		int c = at(subdata, i);
		// we can do this since arrays have an invisible \0 at the end so we can't
		// overflow
		int nextC = unsafe_at(subdata, i + 1);

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false) {
			// look back and check to see if it's delimited
			if (previousC isnt '\\') {
				inString = !inString;
				continue;
			}
		}

		// check for macros
		if (c is '#' && !inMacro and inString is false and
			inSingleComment is false and inMultiComment is false) {
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and
			inString is false) {
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false) {
			inMultiComment = false;
			continue;
		}

		// check to see if we're at a newline
		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and
			inString is false) {
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false) {
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false) {
			inMultiComment = true;
			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro) {
			continue;
		}

		if (inSingleComment or inMultiComment) {
			continue;
		}

		// dont read if we're in a string
		if (inString) {
			continue;
		}

		if (c is '{') {
			++depth;
			continue;
		}
		if (c is '}') {
			--depth;
			continue;
		}
		if (c is '(') {
			parenDepth++;
			continue;
		}
		if (c is ')') {
			parenDepth--;
			continue;
		}

		// if we're in the middle of defining a struct
		// or calling a macro skip
		//          \/
		// <struct { }>
		// <MACRO(13 )>
		if (depth or parenDepth) {
			continue;
		}

		// end of
		if (c is ',') {
			string stackName =
				stack_substring(subdata, nameStartIndex, i - nameStartIndex);
			stackName = combine_partial_string(stackName, Strings.TrimAll(stackName));
			string name = empty_dynamic_string(stackName->Count);
			strings.AppendArray(name, stackName);
			arrays(string).Append(result, name);
			// the next index is the start of the next type name
			nameStartIndex = i + 1;
		}

		if (i is subdata->Count - 1 and nameStartIndex > -1) {
			string stackName =
				stack_substring(subdata, nameStartIndex, i - nameStartIndex);
			stackName = combine_partial_string(stackName, Strings.TrimAll(stackName));
			string name = empty_dynamic_string(stackName->Count);
			strings.AppendArray(name, stackName);
			arrays(string).Append(result, name);
		}
	}

	// check for single argument
	// or no argument
	if (result->Count is 0) {
		if (subdata->Count is 2) {
			fprintf_red(
				stderr,
				"A minium of one type argument is required when using generics. %s", "");
			throw(MissingGenericArgumentException);
		}

		// assume a single argument
		string stackName = stack_substring(subdata, 1, subdata->Count - 2);
		stackName = combine_partial_string(stackName, Strings.TrimAll(stackName));
		string name = empty_dynamic_string(stackName->Count);
		strings.AppendArray(name, stackName);
		arrays(string).Append(result, name);
	}

	return result;
}

private void InstantiateIntArray(array(int)* arr) { *arr = dynamic_array(int, 0); }

// returns a list of arrays of indices
// each index of the returned array matches the index of the provided typeNames
// each array represents a list of indices where that typename is used
//
GenericTypeInfo GetGenericArgumentsWithinBody(string data, location location,
	array(string) typeNames) {
	array(array(int)) result = dynamic_array(array(int), typeNames->Count);
	result->Count = typeNames->Count;
	arrays(array(int)).Foreach(result, InstantiateIntArray);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;

	string subdata =
		stack_substring(data, location.StartScopeIndex,
			location.EndScopeIndex - location.StartScopeIndex + 1);

	bool previousbyteWasPuncutation = true;

	for (int i = 0; i < subdata->Count; i++) {
		int previousC = at(subdata, max(0, i - 1));
		int c = at(subdata, i);
		// we can do this since arrays have an invisible \0 at the end so we can't
		// overflow
		int nextC = unsafe_at(subdata, i + 1);

		if (c is '"' and inSingleComment is false and inMultiComment is false) {
			// look back and check to see if it's delimited
			if (previousC isnt '\\') {
				inString = !inString;
				continue;
			}
		}

		if (c is '#' && !inMacro and inString is false and
			inSingleComment is false and inMultiComment is false) {
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and
			inString is false) {
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false) {
			inMultiComment = false;
			continue;
		}

		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and
			inString is false) {
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false) {
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false) {
			inMultiComment = true;
			continue;
		}

		if (inMacro or inSingleComment or inMultiComment or inString) {
			continue;
		}

		// ignore non name byteacters, NO type can start with one
		if (IsValidNameCharacter(c) is false) {
			previousbyteWasPuncutation = true;
			continue;
		}

		if (previousbyteWasPuncutation) {
			// check to see if this is the start
			// of one of our arguments
			const int indexOfFoundArg =
				BeginsWithAny(stack_substring_back(subdata, i), typeNames);
			if (indexOfFoundArg isnt - 1) {
				// just because it begins with a type token
				// doesnt mean its a type token
				// make sure a NON valid type byteacter follows the typename
				string substr = stack_substring_back(subdata, i);
				const byte cAfterToken =
					at(substr, at(typeNames, indexOfFoundArg)->Count);
				if (IsValidNameCharacter(cAfterToken) is false) {
					arrays(int).Append(at(result, indexOfFoundArg), i);
				}
			}

			previousbyteWasPuncutation = false;
		}
	}

	return (GenericTypeInfo) { .TypeNames = typeNames, .TypeLocations = result };
}

private void SumCount(int* out_count, array(int)* arr_ptr) {
	*out_count += (*arr_ptr)->Count;
}

private bool BigToSmallComparator(tuple(string, int)* left,
	tuple(string, int)* right) {
	return (*right).Second > (*left).Second;
}

// sorts the provided generic type info into a list
// ids and indices that are sorted so that the type locations
// that occur last are first in the second array
private array(tuple(string, int)) SortGenericTypeInfo(GenericTypeInfo info) {
	// the total number of elements in all the int arrays
	int totalCount = 0;
	arrays(array(int))
		.ForeachWithContext(info.TypeLocations, &totalCount, SumCount);

	array(tuple(string, int)) result =
		dynamic_array(tuple(string, int), totalCount);

	for (int typeIndex = 0; typeIndex < info.TypeLocations->Count; typeIndex++) {
		string name = at(info.TypeNames, typeIndex);
		array(int) indices = at(info.TypeLocations, typeIndex);

		for (int i = 0; i < indices->Count; i++) {
			arrays(tuple(string, int))
				.Append(result, (tuple(string, int)) { name, at(indices, i) });
		}
	}

	arrays(tuple(string, int)).InsertionSort(result, BigToSmallComparator);

	info.SortedTypeLocations = result;

	return result;
}

void RepackRecalculatedTypeLocations(MethodInfo info)
{
	// clear old values
	for (int i = 0; i < info.TypeLocations->Count; i++)
	{
		arrays(int).Clear(at(info.TypeLocations, i));
	}

	for (int i = 0; i < info.SortedTypeLocations->Count; i++)
	{
		tuple(string, int) pair = at(info.SortedTypeLocations, i);

		int nameIndex = arrays(string).IndexOf(info.TypeNames, pair.First);

		if (nameIndex is - 1)
		{
			// can't repack a typename that we didnt originally unpack
			throw(ItemNotFoundInCollectionException);
		}

		arrays(int).Append(at(info.TypeLocations, nameIndex), pair.Second);
	}
}

private string RemoveTypesFromGenericMethodBody(string data, GenericTypeInfo info)
{
	string result = strings.Clone(data);

	// iterate backwards deleting all the generic types
	// so we can fill in concrete types later
	for (size_t forwardIndex = 0; forwardIndex < info.SortedTypeLocations->Count; forwardIndex++)
	{
		string name = at(info.SortedTypeLocations, forwardIndex).First;
		int index = at(info.SortedTypeLocations, forwardIndex).Second;

		strings.RemoveRange(result, index, name->Count);

		// recalc indices for any values that come after this
		int backIndex = forwardIndex;
		while (backIndex-- > 0) {
			at(info.SortedTypeLocations, backIndex).Second -= name->Count;
		}
	}

	return result;
}

void ExpandGenericArgumentsWithinBody(string data, location location,
	GenericTypeInfo info) {}

MethodInfo GetMethodInfo(string data, location location) {

	MethodInfo info = {
		.Data = null,
		.TypeLocations = null,
		.TypeNames = GetGenericArguments(data,location),
		.SortedTypeLocations = null,
		.Name = null
	};

	GenericTypeInfo genericInfo = GetGenericArgumentsWithinBody(data, location, info.TypeNames);

	info.TypeLocations = genericInfo.TypeLocations;
	info.TypeNames = genericInfo.TypeNames;
	info.SortedTypeLocations = SortGenericTypeInfo(genericInfo);
	genericInfo.SortedTypeLocations = info.SortedTypeLocations;

	info.Data = RemoveTypesFromGenericMethodBody(data, genericInfo);

	string name = combine_partial_string(data, GetMethodName(stack_substring_front(data, location.AlligatorStartIndex - 1)));

	info.Name = strings.Clone(name);

	// because we removed the generic type names
	// the type locations array is invalidated
	RepackRecalculatedTypeLocations(info);

	return info;
}

GenericTypeInfo GetGenericTokens(string data, location location) {
	return (GenericTypeInfo) { 0 };
}

void ExpandCall(string data, location location) {
	// calls just get replaced with a valid C name
	// and the preprocessor and compiler will do lookup
	const size_t start = location.AlligatorStartIndex;
	const size_t count = location.AlligatorEndIndex - start + 1;

	string buffer = empty_stack_array(byte, MAX_ARG_LENGTH);

	string arguments = stack_substring(data, start, count);

	string flattened = FlattenArgumentToCName(arguments, buffer);

	// we dont know for sure the flattend name is the same length
	// it may be shorter or longer, shift old text left over it
	strings.RemoveRange(data, start, count);
	strings.InsertArray(data, flattened, start);
}

void ExpandMethodDeclaration(string data, location location) {
	// T MyMethod<T>(T, T);
}

void ExpandGeneric(string data, location location) {
	Guard(data->Count > 0);
	Guard(location.Call or location.Declaration or location.Definition or
		location.Struct);

	if (location.Call) {
		ExpandCall(data, location);
	}
	else if (location.Declaration) {
		// ExpandDeclaration(data, location);
	}
	else if (location.Definition) {
		// ExpandDefinition(data, location);
	}
	else if (location.Struct) {
		// ExpandStruct(data, location);
	}
}

void ExpandGenerics(string data, array(location) locations) {
	// expand backwards so the indexes dont change
	int i = locations->Count;
	while (i-- > 0) {
		location location = at(locations, i);

		ExpandGeneric(data, location);
	}
}

TEST(FlattenArgumentToCName) {
	string data = stack_string("");
	string result = empty_stack_array(byte, 1024);

	IsTrue(
		strings.Equals(stack_string(""), FlattenArgumentToCName(data, result)));

	data = stack_string("int");
	result = empty_stack_array(byte, 1024);

	IsTrue(strings.Equals(stack_string("int"),
		FlattenArgumentToCName(data, result)));

	data = stack_string("int*");
	result = empty_stack_array(byte, 1024);

	IsTrue(strings.Equals(stack_string("int_ptr"),
		FlattenArgumentToCName(data, result)));

	data = stack_string("int**");
	result = empty_stack_array(byte, 1024);

	IsTrue(strings.Equals(stack_string("int_ptr_ptr"),
		FlattenArgumentToCName(data, result)));

	data = stack_string("struct vec2{ int x; int y; }");
	result = empty_stack_array(byte, 1024);

	IsTrue(strings.Equals(stack_string("struct_vec2_int_x_int_y_"),
		FlattenArgumentToCName(data, result)));

	data = stack_string("<T,U,V,X,Y,Z>");
	result = empty_stack_array(byte, 1024);

	IsTrue(strings.Equals(stack_string("_T_U_V_X_Y_Z_"),
		FlattenArgumentToCName(data, result)));

	return true;
}

TEST(ExpandCall) {
	string data = dynamic_string("int x = MyMethod<int>(24);");
	array(location) locations = GetGenericLocations(data);

	IsEqual((size_t)1, locations->Count);

	ExpandGeneric(data, at(locations, 0));

	string expected = stack_string("int x = MyMethod_int_(24);");

	IsTrue(strings.Equals(expected, data));

	return true;
}

TEST(GetGenericArguments) {
	string data = stack_string("<T>");
	location genericLocation = { .AlligatorStartIndex = 0,
								.AlligatorEndIndex = data->Count - 1 };

	array(string) expected = nested_stack_array(string, stack_string("T"));
	array(string) actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);
	IsTrue(strings.Equals(at(actual, 0), at(expected, 0)));

	data = stack_string("<T,U,V>");
	genericLocation = (location){ .AlligatorStartIndex = 0,
								 .AlligatorEndIndex = data->Count - 1 };

	expected = nested_stack_array(string, stack_string("T"), stack_string("U"),
		stack_string("V"));
	actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);

	IsTrue(strings.Equals(at(actual, 0), at(expected, 0)));
	IsTrue(strings.Equals(at(actual, 1), at(expected, 1)));
	IsTrue(strings.Equals(at(actual, 2), at(expected, 2)));

	data = stack_string("<T,struct tuple_int{int x; int y;},V>");
	genericLocation = (location){ .AlligatorStartIndex = 0,
								 .AlligatorEndIndex = data->Count - 1 };

	expected = nested_stack_array(string, stack_string("T"),
		stack_string("struct tuple_int{int x; int y;}"),
		stack_string("V"));
	actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);

	IsTrue(strings.Equals(at(actual, 0), at(expected, 0)));
	IsTrue(strings.Equals(at(actual, 1), at(expected, 1)));
	IsTrue(strings.Equals(at(actual, 2), at(expected, 2)));

	data = stack_string("<\nT,\nU,\nV\n>");
	genericLocation = (location){ .AlligatorStartIndex = 0,
								 .AlligatorEndIndex = data->Count - 1 };

	expected = nested_stack_array(string, stack_string("T"), stack_string("U"),
		stack_string("V"));
	actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);

	IsTrue(strings.Equals(at(actual, 0), at(expected, 0)));
	IsTrue(strings.Equals(at(actual, 1), at(expected, 1)));
	IsTrue(strings.Equals(at(actual, 2), at(expected, 2)));

	data = stack_string("<T,\nstruct triplet_int{/* comment here */int x;// "
		"other comment\n int y;},\nV\n>");
	genericLocation = (location){ .AlligatorStartIndex = 0,
								 .AlligatorEndIndex = data->Count - 1 };

	expected =
		nested_stack_array(string, stack_string("T"),
			stack_string("struct triplet_int{/* comment here "
				"*/int x;// other comment\n int y;}"),
			stack_string("V"));
	actual = GetGenericArguments(data, genericLocation);

	IsEqual(expected->Count, actual->Count);

	IsTrue(strings.Equals(at(actual, 0), at(expected, 0)));
	IsTrue(strings.Equals(at(actual, 1), at(expected, 1)));
	IsTrue(strings.Equals(at(actual, 2), at(expected, 2)));

	return true;
}

static string GlobalData = stack_string(
	"V Method<T,U,V>(T left,U right){ if(((T)right).Thing && "
	"((U)left).Thing){V result = (V)left + (V)right; return result; }}");
static location GlobalLocation = { .AlligatorStartIndex = 8,
									   .AlligatorEndIndex = 14,
									   .StartScopeIndex = 0,
									   .EndScopeIndex = 120 };
array(string) GlobalTypeNames = nested_stack_array(
	string, stack_string("T"), stack_string("U"), stack_string("V"));

array(array(int)) GlobalTypeLocations =
stack_array(array(int), 3, auto_stack_array(int, 9, 16, 38),
	auto_stack_array(int, 11, 23, 58),
	auto_stack_array(int, 0, 13, 73, 85, 95));

TEST(GetGenericArgumentsWithinBody) {
	string data = GlobalData;

	location dataLocation = GlobalLocation;

	array(string) typeNames = GlobalTypeNames;

	array(array(int)) expectedArray = GlobalTypeLocations;

	GenericTypeInfo typeInfo =
		GetGenericArgumentsWithinBody(data, dataLocation, typeNames);

	IsTrue(arrays(string).Equals(typeNames, typeInfo.TypeNames));

	array(array(int)) actualArray = typeInfo.TypeLocations;

	IsEqual(expectedArray->Count, actualArray->Count);

	for (int arrayIndex = 0;
		arrayIndex < min(actualArray->Count, expectedArray->Count);
		arrayIndex++) {
		array(int) actualValues = at(actualArray, arrayIndex);
		array(int) expectedValues = at(expectedArray, arrayIndex);

		IsEqual(expectedValues->Count, actualValues->Count);

		for (int valueIndex = 0;
			valueIndex < min(actualValues->Count, expectedValues->Count);
			valueIndex++) {
			int actual = at(actualValues, valueIndex);
			int expected = at(expectedValues, valueIndex);

			IsEqual(expected, actual);
		}
	}

	return true;
}

array(tuple(string, int)) GlobalSortedTypeLocations = stack_array(tuple(string, int), 11,
	{ stack_string("V"), 95 },
	{ stack_string("V"), 85 },
	{ stack_string("V"), 73 },
	{ stack_string("U"), 58 },
	{ stack_string("T"), 38 },
	{ stack_string("U"), 23 },
	{ stack_string("T"), 16 },
	{ stack_string("V"), 13 },
	{ stack_string("U"), 11 },
	{ stack_string("T"), 9 },
	{ stack_string("V"), 0 }
);

TEST(SortGenericTypeInfo) {
	string data = GlobalData;
	location dataLocation = GlobalLocation;
	array(string) typeNames = GlobalTypeNames;

	GenericTypeInfo typeInfo =
		GetGenericArgumentsWithinBody(data, dataLocation, typeNames);

	array(tuple(string, int)) expectedArray = GlobalSortedTypeLocations;

	array(tuple(string, int)) actualArray = SortGenericTypeInfo(typeInfo);

	IsEqual(expectedArray->Count, actualArray->Count);

	for (int i = 0; i < min(expectedArray->Count, actualArray->Count); i++)
	{
		tuple(string, int) expected = at(expectedArray, i);
		tuple(string, int) actual = at(actualArray, i);

		IsEqual(expected.First, actual.First);
		IsEqual(expected.Second, actual.Second);
	}

	return true;
}

string GlobalTypelessData = stack_string(" Method<,,>( left, right){ if((()right).Thing && (()left).Thing){ result = ()left + ()right; return result; }}");


TEST(RemoveTypesFromGenericMethodBody)
{
	string data = GlobalData;
	location dataLocation = GlobalLocation;
	array(string) typeNames = GlobalTypeNames;

	GenericTypeInfo typeInfo =
		GetGenericArgumentsWithinBody(data, dataLocation, typeNames);

	array(tuple(string, int)) sortedInfo = SortGenericTypeInfo(typeInfo);

	typeInfo.SortedTypeLocations = sortedInfo;

	string actual = RemoveTypesFromGenericMethodBody(data, typeInfo);

	string expected = GlobalTypelessData;

	IsEqual(expected, actual);

	return true;
}

array(array(int)) GlobalRepackedTypeLocations =
stack_array(array(int), 3, auto_stack_array(int, 32, 12, 8),
	auto_stack_array(int, 51, 18, 9),
	auto_stack_array(int, 85, 76, 65, 10, 0));

TEST(GetMethodInfo)
{
	string data = GlobalData;
	location dataLocation = GlobalLocation;

	MethodInfo expected = {
		.Name = stack_string("Method"),
		.Data = GlobalTypelessData,
		.TypeNames = GlobalTypeNames,
		.TypeLocations = GlobalRepackedTypeLocations,
		.SortedTypeLocations = GlobalSortedTypeLocations
	};

	MethodInfo actual = GetMethodInfo(data, dataLocation);

	NotNull(actual.Name);
	NotNull(actual.Data);
	NotNull(actual.SortedTypeLocations);
	NotNull(actual.TypeLocations);
	NotNull(actual.TypeNames);

	IsEqual(actual.Data, expected.Data);
	IsEqual(actual.Name, expected.Name);

	for (int i = 0; i < min(actual.TypeNames->Count, expected.TypeNames->Count); i++)
	{
		string expectedType = at(expected.TypeNames, i);
		string actualType = at(actual.TypeNames, i);
		IsEqual(expectedType, actualType);
	}

	for (int i = 0; i < min(expected.TypeLocations->Count, actual.TypeLocations->Count); i++)
	{
		array(int) actualInts = at(actual.TypeLocations, i);

		ulong previousHash = actualInts->Hash;
		ulong newHash = arrays(int).Hash(actualInts);

		IsEqual(previousHash, newHash);

		array(int) expectedInts = at(expected.TypeLocations, i);

		ulong expectedHash = arrays(int).Hash(expectedInts);

		IsEqual(expectedHash, newHash);

		ulong manualHash = Hashing.HashSafe((char*)expectedInts->Values, expectedInts->Count * sizeof(int));

		IsEqual(manualHash, expectedHash);

		/*AutoPrint("Expected:\n");
		arrays(int).Print(stdout, expectedInts);
		AutoPrint("\nActual:\n");
		arrays(int).Print(stdout, actualInts);
		AutoPrint((byte)'\n');*/

		IsTrue(arrays(int).Equals(actualInts, expectedInts));
	}

	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(FlattenArgumentToCName) APPEND_TEST(ExpandCall)
	APPEND_TEST(GetGenericArguments)
	APPEND_TEST(GetGenericArgumentsWithinBody)
	APPEND_TEST(SortGenericTypeInfo)
	APPEND_TEST(RemoveTypesFromGenericMethodBody)
	APPEND_TEST(GetMethodInfo)
);

OnStart(2) { RunUnitTests(); }
