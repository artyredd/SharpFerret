#pragma once

#include "core/array.h"

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

array(location) GetGenericLocations(string data);
location IdentifyGenericCall(string data, int depth, int openAlligatorIndex, int lastMacroEndIndex);


// Reads the tokens from the provided file
void RunTests();
