#pragma once
#include <core/array.h>


typedef struct {
	// list of type names within the 
	array(string) TypeNames;
	// index matches type name indices
	// TypeLocations[i] is an array of start indexes where TypeName[i] appears
	array(array(int)) TypeLocations;
} GenericTypeInfo;

typedef struct {
	string name;
	string data;
	GenericTypeInfo;
} MethodInfo;

DEFINE_CONTAINERS(MethodInfo);
DEFINE_CONTAINERS(GenericTypeInfo);