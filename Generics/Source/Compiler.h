#pragma once
#include <core/array.h>


typedef struct {
	// list of type names within the 
	array(string) TypeNames;
	// index matches type name indices
	// TypeLocations[i] is an array of start indexes where TypeName[i] appears
	array(array(int)) TypeLocations;
	// Sorted last to first type locations
	array(tuple(string, int)) SortedTypeLocations;
} GenericTypeInfo;

typedef struct _CompileUnit* CompileUnit;

typedef struct {
	CompileUnit Parent;
	string Name;
	string Data;
	GenericTypeInfo;
} MethodInfo;


DEFINE_CONTAINERS(MethodInfo);
DEFINE_CONTAINERS(GenericTypeInfo);


struct _GenericMethodInstance
{
	MethodInfo Info;
	array(string) Arguments;
	CompileUnit Parent;
};

typedef struct _GenericMethodInstance GenericMethodInstance;

DEFINE_CONTAINERS(GenericMethodInstance);

typedef struct _Assembly* Assembly;

struct _CompileUnit
{
	Assembly Parent;
	array(MethodInfo) Methods;
	array(GenericMethodInstance) MethodInstances;
};


DEFINE_CONTAINERS(CompileUnit);

struct _Assembly
{
	array(CompileUnit) CompileUnits;
	array(MethodInfo) Methods;
	array(GenericMethodInstance) MethodInstances;
};
