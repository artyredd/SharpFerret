#pragma once

#include "core/csharp.h"
#include "memory.h"
#include "pointer.h"
#include "macros.h"
#include <string.h>
#include <core/memory.h>
#include <core/hashing.h>

// TEMPLATE

#define _EXPAND_array(type) type##_array
#define _EXPAND_Arrays(type) type##_array##Arrays

// Creates an array of the given type, remember to define the type if this fails to compile
#define array(type) _EXPAND_array(type)
// Convenience methods that can be used with an array(type)
#define arrays(type) _EXPAND_Arrays(type)

// Represents an array that has no data and only offsets, combine with
// a base pointer from a parent array to create and array
// Useful for when you want to return a stack_array but can't because it wont
// live long enough
// Great for using as a sliding view for a larger array without having
// to create new stack_subarrays
#define partial_array(type) partial_##type##_##array

#define _EXPAND_STRUCT_NAME(type) _array_##type
#define _EXPAND_METHOD_NAME(type, method) _array_##type##_##method

#define _ARRAY_DEFINE_STRUCT(type) struct _EXPAND_STRUCT_NAME(type)\
{\
	/* The pointer to the backing array */\
	type* Values;\
	/* The size, in bytes, of the backing array*/\
	ulong Size;\
	/* The size in bytes between elements of the array\
	// This would typically be the size of the element stored */\
	ulong ElementSize;\
	/* the size in elements that this array can store\
	this is the same as Size/ElementSize */\
	ulong Capacity;\
	/* The number of elements stored in the array */\
	ulong Count;\
	/* The typeid of the element stored in the array */\
	ulong TypeId;\
	/* Whether or not this object was constructed on the stack */\
	bool StackObject;\
	/* Whether or not the array needs to recalculate the hash */\
	bool Dirty;\
	/* The Hash of this array */\
	ulong Hash;\
	/* Whether or not this array is auto hashed, default is true */\
	bool AutoHash;\
}; \
typedef struct _array_##type partial_##type##_##array;\
typedef struct _array_##type* type##_array;

#define explicit_stack_array(type,initialArraySize,count, ...) (array(type))&(struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = (type*)(type[initialArraySize]){ __VA_ARGS__ },\
	.Size = sizeof(type) * count,\
	.ElementSize = sizeof(type),\
	.Capacity = count,\
	.Count = count,\
	.TypeId = 0,\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

#define stack_array(type,count,...) explicit_stack_array(type,count,count,__VA_ARGS__)

#define auto_stack_array(type,...) stack_array(type,VAR_COUNT(__VA_ARGS__),__VA_ARGS__)
#define _stack_array_comma_count(...) (VAR_COUNT(__VA_ARGS__) - 1)/9
#define nested_stack_array(type,...) stack_array(type,_stack_array_comma_count(__VA_ARGS__),__VA_ARGS__)

#define empty_stack_array(type,count)  (array(type))&(struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = (type*)(type[count]){ 0 },\
	.Size = sizeof(type) * count,\
	.ElementSize = sizeof(type),\
	.Capacity = count,\
	.Count = 0,\
	.TypeId = 0,\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

#define _stack_string(string) explicit_stack_array(char,,sizeof(string) - 1, string) 
#define stack_string(string) _stack_string(string) 

#define dynamic_array(type, initialCount) _EXPAND_METHOD_NAME(type,Create)(initialCount);

// checks the corresponding attribute against the provided value
// throws if the value is out of bounds, otherwise returns value
#define guard_array_attribute(arr,attribute,value) ((value) > (arr)->attribute ? throw_in_expression(IndexOutOfRangeException) : (value))
#define guard_array_attribute_index(arr,attribute,value) ((value) >= (arr)->attribute ? throw_in_expression(IndexOutOfRangeException) : (value))

// checks and throws if the value is out of bounds for the array
// returns the value so it can be used in expressions
#define guard_array_count_index(arr, value) guard_array_attribute_index(arr,Count,value) 
#define guard_array_count(arr,value) guard_array_attribute(arr,Count,value)
// checks and throws if the value is out of bounds for the array
// returns the value so it can be used in expressions
#define guard_array_size(arr, value) guard_array_attribute(arr,Size,value) 


// Creates a stack array that is a subarray of the given array
// Uses the given arrays pointers so changes to the stack array
// affect the given array
// example: 
// array = { a, b, c, d, e, f, g, h }
// sub = stack_subarray(array, 4, 2)
// sub { e, f }
// ! will throw IndexOutOfRangeException if start index or count are out of bounds
#define stack_subarray(type, arr, startIndex, count) (array(type))&(struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = &arr->Values[guard_array_count_index(arr, startIndex)],\
	.Size = guard_array_size(arr, sizeof(type) * count),\
	.ElementSize = sizeof(type),\
	.Capacity = guard_array_count(arr, count),\
	.Count = guard_array_count(arr, count),\
	.TypeId = 0,\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

// Creates a stack array that is a subarray of the given array
// Returns an array that contains the values inclusive between
// the given index and the end
// example: 
// array = { a, b, c, d, e, f, g, h }
// sub = stack_subarray_back(array, 4)
// sub { e, f, g, h }
#define stack_subarray_back(type, arr, startIndex) stack_subarray(type, arr, startIndex, safe_subtract(arr->Count, startIndex))

// Creates a stack array that is a subarray of the given array
// Returns an array that contains the values inclusive between
// the front and the the given index
// example: 
// array = { a, b, c, d, e, f, g, h }
// sub = stack_subarray_back(array, 3)
// sub { a, b, c, d }
#define stack_subarray_front(type, arr, endIndexInclusive) stack_subarray(type, arr, 0, safe_add(endIndexInclusive, 1))

// VALUE TYPE; Creates a persistent array that is a subarray of the given array
// This array represents only offsets and contains no data and must be
// combined with a pointer to provide an actual array.
// USE THIS FOR: when you want to return a stack_array, but the stack_array
// can't live past the current stack frame. Useful for avoiding memory
// allocations.
#define partial_subarray(type, arr, startIndex, count) (struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = arr->Count >= startIndex ? (type*)(void*)(ulong)startIndex : (type*)(void*)(ulong)throw_in_expression(IndexOutOfRangeException),\
	.Size = guard_array_size(arr, sizeof(type) * count),\
	.ElementSize = sizeof(type),\
	.Capacity = guard_array_count(arr, count),\
	.Count = guard_array_count(arr, count),\
	.TypeId = 0,\
	/* This is a value type object that isn't a stack object */\
	/* But we want to avoid trying to free an offset */\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

// Creates a stack array that is a subarray of the given array
// Returns an array that contains the values inclusive between
// the given index and the end
// example: 
// array = { a, b, c, d, e, f, g, h }
// sub = stack_subarray_back(array, 4)
// sub { e, f, g, h }
#define partial_subarray_back(type, arr, startIndex) partial_subarray(type, arr, startIndex, safe_subtract(arr->Count, startIndex))

// Creates a stack array that is a subarray of the given array
// Returns an array that contains the values inclusive between
// the front and the the given index
// example: 
// array = { a, b, c, d, e, f, g, h }
// sub = stack_subarray_back(array, 3)
// sub { a, b, c, d }
#define partial_subarray_front(type, arr, endIndexInclusive) partial_subarray(type, arr, 0, safe_add(endIndexInclusive, 1))


// creates a stack_array by combining a partial array with a regular array
// STACK OBJECT!
#define combine_partial_array(type, arr, partial) (array(type))&(partial_array(type))\
{\
	.Values = arr->Count >= (ulong)partial.Values ? (arr->Values + (ulong)partial.Values) : (char*)(ulong)throw_in_expression(IndexOutOfRangeException),\
	.Size = guard_array_size(arr, partial.Size),\
	.ElementSize = guard_array_attribute(arr,ElementSize,partial.ElementSize),\
	.Capacity = guard_array_attribute(arr, Capacity, partial.Capacity),\
	.Count = guard_array_count(arr,(partial).Count),\
	.TypeId = partial.TypeId != arr->TypeId ? throw_in_expression(TypeMismatchException) : partial.TypeId,\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

_ARRAY_DEFINE_STRUCT(void);
typedef array(void) Array;

DEFINE_TYPE_ID(Array);

struct _arrayMethods
{
	Array(*Create)(ulong elementSize, ulong count, ulong typeId);
	void (*AutoResize)(Array);
	void (*Resize)(Array, ulong newCount);
	// Appends the given item to the end of the array
	void (*Append)(Array, void*);
	// Removes the given index, moving all contents to the left
	// in its place
	void (*RemoveIndex)(Array, ulong index);
	// Swaps the positions of two elements
	void (*Swap)(Array, ulong firstIndex, ulong secondIndex);
	// Insertion sorts given the provided comparator Func
	void (*InsertionSort)(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
	// Gets a pointer to the value contained at index
	void* (*At)(Array, ulong index);
	// Appends the given value array to the end of the given array
	Array(*AppendArray)(Array array, Array appendedValue);
	Array(*InsertArray)(Array dest, Array src, ulong index);
	void (*Clear)(Array array);
	bool (*Equals)(Array left, Array right);
	void (*Foreach)(Array, void(*method)(void*));
	void (*ForeachWithContext)(Array, void* context, void(*method)(void* context, void* item));
	void (*Dispose)(Array);
};

extern const struct _arrayMethods Arrays;


// TEMPLATE FOR METHODS
#define _EXPAND_DEFINE_ARRAY(type) _ARRAY_DEFINE_STRUCT(type)\
DEFINE_TYPE_ID(type##_array); \
private array(type) _EXPAND_METHOD_NAME(type,Create)(ulong count)\
{\
REGISTER_TYPE(type##_array); \
return (array(type))Arrays.Create(sizeof(type), count, type##_arrayTypeId); \
}\
private void _EXPAND_METHOD_NAME(type,AutoResize)(array(type) array)\
{\
Arrays.AutoResize((Array)array); \
}\
private void _EXPAND_METHOD_NAME(type,Resize)(array(type) array, ulong newCount)\
{\
Arrays.Resize((Array)array, newCount); \
}\
private void _EXPAND_METHOD_NAME(type,Append)(array(type)array, type value)\
{\
Arrays.Append((Array)array, &value); \
}\
private void _EXPAND_METHOD_NAME(type,RemoveIndex)(array(type) array, ulong index)\
{\
Arrays.RemoveIndex((Array)array, index); \
}\
private array(type) _EXPAND_METHOD_NAME(type, RemoveRange)(array(type) array, ulong index, ulong count)\
{\
	/* { a, b, c, d, e } */\
	/* RemoveRange(2,2) */\
	/* { a, b, e } */\
	const ulong safeIndex = guard_array_count_index(array, index);\
	const ulong safeCount = guard_array_count(array, count);\
	type* destination = &array->Values[safeIndex];\
	array(type) source = stack_subarray_back(type, array, safeIndex + safeCount);\
	memmove(destination,source->Values,source->Count * source->ElementSize );\
	array->Count = safe_subtract(array->Count, safeCount);\
	array->Dirty = true;\
	return array; \
}\
private bool _EXPAND_METHOD_NAME(type, Empty)(array(type) array)\
{\
if (array is null || array->Values is null)\
{\
return true; \
}\
if (array->Capacity is 0 || array->Count is 0)\
{\
return true; \
}\
return false; \
}\
private void _EXPAND_METHOD_NAME(type, Swap)(array(type) array, ulong firstIndex, ulong secondIndex)\
{\
Arrays.Swap((Array)array, firstIndex, secondIndex); \
}\
private void _EXPAND_METHOD_NAME(type, InsertionSort)(array(type) array, bool(comparator)(type* leftMemoryBlock, type* rightMemoryBlock))\
{\
Arrays.InsertionSort((Array)array, comparator); \
}\
private type* _EXPAND_METHOD_NAME(type, At)(array(type) array, ulong index)\
{\
return (type*)Arrays.At((Array)array, index); \
}\
private type _EXPAND_METHOD_NAME(type, ValueAt)(array(type) array, ulong index)\
{\
return array->Values[guard_array_count_index(array,index)]; \
}\
private array(type) _EXPAND_METHOD_NAME(type, AppendArray)(array(type) array, array(type) appendedValue)\
{\
return (array(type))Arrays.AppendArray((Array)array, (Array)appendedValue); \
}\
private array(type) _EXPAND_METHOD_NAME(type, InsertArray)(array(type) destination, array(type) source, ulong index)\
{\
	return (array(type))Arrays.InsertArray((Array)destination, (Array)source, index);\
}\
private void _EXPAND_METHOD_NAME(type, AppendCArray)(array(type) array, const type* carray, const ulong arraySize)\
{\
for (ulong cIndex = 0; cIndex < arraySize; ++cIndex)\
{\
type value = carray[cIndex]; \
\
_EXPAND_METHOD_NAME(type, Append)(array, value); \
}\
}\
private void _EXPAND_METHOD_NAME(type, Clear)(array(type)array)\
{\
Arrays.Clear((Array)array); \
}\
private void _EXPAND_METHOD_NAME(type, Foreach)(array(type)array, void(*method)(type*))\
{\
Arrays.Foreach((Array)array, method); \
}\
private void _EXPAND_METHOD_NAME(type, ForeachWithContext)(array(type)array, void* context, void(*method)(void* context, type* item))\
{\
Arrays.ForeachWithContext((Array)array, context, method); \
}\
private array(type) _EXPAND_METHOD_NAME(type, Clone)(array(type) array)\
{\
array(type) result = _EXPAND_METHOD_NAME(type, Create)(array->Capacity); \
_EXPAND_METHOD_NAME(type, AppendArray)(result, array); \
return result; \
}\
private void _EXPAND_METHOD_NAME(type, Dispose)(array(type)array)\
{\
Arrays.Dispose((Array)array); \
}\
private ulong _EXPAND_METHOD_NAME(type, Hash)(array(type) array)\
{\
if(array->Dirty)\
{\
	array->Dirty = false;\
	array->Hash = Hashing.HashSafe((char*)array->Values, array->Count * array->ElementSize);\
}\
return array->Hash; \
}\
private bool _EXPAND_METHOD_NAME(type, Equals)(array(type) left, array(type) right)\
{\
return Arrays.Equals((Array)left,(Array)right);\
}\
private bool _EXPAND_METHOD_NAME(type, BeginsWith)(array(type) array, array(type) value)\
{\
ulong safeCount = min(value->Count,array->Count);\
array(type) sub = stack_subarray_front(type, array, safeCount);\
return Arrays.Equals((Array)sub, (Array)value); \
}\
const static struct _array_##type##_methods\
{\
array(type) (*Create)(ulong count); \
void (*AutoResize)(array(type)); \
void (*Resize)(array(type), ulong newCount); \
void (*Append)(array(type), type); \
void (*RemoveIndex)(array(type), ulong index); \
array(type) (*RemoveRange)(array(type), ulong startIndex, ulong count); \
bool (*Empty)(array(type)); \
void (*Swap)(array(type), ulong firstIndex, ulong secondIndex); \
void (*InsertionSort)(array(type), bool(comparator)(type* left, type* right)); \
type* (*At)(array(type), ulong index); \
type(*ValueAt)(array(type), ulong index); \
array(type) (*AppendArray)(array(type), const array(type) appendedValue); \
array(type) (*InsertArray)(array(type) destination, array(type) values, ulong index); \
void (*AppendCArray)(array(type), const type* carray, const ulong count); \
bool (*Equals)(array(type), array(type)); \
bool (*BeginsWith)(array(type), array(type)); \
void (*Clear)(array(type)); \
void (*Foreach)(array(type), void(*method)(type*)); \
void (*ForeachWithContext)(array(type), void* context, void(*method)(void*, type*)); \
array(type) (*Clone)(array(type)); \
ulong(*Hash)(array(type)); \
void (*Dispose)(array(type)); \
} type##_array##Arrays = \
{\
.Create = _EXPAND_METHOD_NAME(type, Create), \
.AutoResize = _EXPAND_METHOD_NAME(type, AutoResize), \
.Resize = _EXPAND_METHOD_NAME(type, Resize), \
.Append = _EXPAND_METHOD_NAME(type, Append), \
.RemoveIndex = _EXPAND_METHOD_NAME(type, RemoveIndex), \
.RemoveRange = _EXPAND_METHOD_NAME(type, RemoveRange), \
.Empty = _EXPAND_METHOD_NAME(type, Empty), \
.Swap = _EXPAND_METHOD_NAME(type, Swap), \
.InsertionSort = _EXPAND_METHOD_NAME(type, InsertionSort), \
.At = _EXPAND_METHOD_NAME(type, At), \
.ValueAt = _EXPAND_METHOD_NAME(type, ValueAt), \
.AppendArray = _EXPAND_METHOD_NAME(type, AppendArray), \
.InsertArray = _EXPAND_METHOD_NAME(type, InsertArray), \
.AppendCArray = _EXPAND_METHOD_NAME(type, AppendCArray), \
.Equals = _EXPAND_METHOD_NAME(type, Equals), \
.Clear = _EXPAND_METHOD_NAME(type, Clear), \
.Foreach = _EXPAND_METHOD_NAME(type, Foreach), \
.ForeachWithContext = _EXPAND_METHOD_NAME(type, ForeachWithContext), \
.Clone = _EXPAND_METHOD_NAME(type, Clone), \
.Hash = _EXPAND_METHOD_NAME(type, Hash), \
.BeginsWith = _EXPAND_METHOD_NAME(type, BeginsWith), \
.Dispose = _EXPAND_METHOD_NAME(type, Dispose)\
};

#define DEFINE_ARRAY(type) _EXPAND_DEFINE_ARRAY(type)

// define common types
DEFINE_ARRAY(bool);
DEFINE_ARRAY(char);
DEFINE_ARRAY(int);
DEFINE_ARRAY(float);
DEFINE_ARRAY(double);
DEFINE_ARRAY(ulong);

DEFINE_ARRAY(array(char));

#define string array(char)
#define strings arrays(char)
#define stack_substring(arr, index, count) stack_subarray(char, arr, index, count)
#define stack_substring_front(arr, endIndexInclusive) stack_subarray_front(char, arr, endIndexInclusive)
#define stack_substring_back(arr, startIndexInclusive) stack_subarray_back(char, arr, startIndexInclusive)
#define partial_string partial_array(char)
#define partial_substring(arr,index,count) partial_subarray(char,arr,index,count)
#define partial_substring_front(arr, endIndexInclusive) partial_subarray_front(char, arr, endIndexInclusive)
#define partial_substring_back(arr, startIndexInclusive) partial_subarray_back(char, arr, startIndexInclusive)
#define combine_partial_string(arr, partial) combine_partial_array(char, arr, partial)
#define dynamic_string(cString) strings.AppendArray(strings.Create(sizeof(cString)-1), stack_string(cString))
#define empty_dynamic_string(count) strings.Create(count)

#define _EXPAND_tuple(left,right) left##_##right##_tuple
#define tuple(left,right) _EXPAND_tuple(left,right)

#define _DEFINE_TUPLE_FULL(left,leftname,right,rightname) struct _##left##_##right##_tuple{ left leftname;right rightname;}; typedef struct _##left##_##right##_tuple left##_##right##_tuple; 
#define DEFINE_TUPLE_FULL(left,leftname,right,rightname) _DEFINE_TUPLE_FULL(left,leftname,right,rightname)
#define DEFINE_TUPLE(left,right) DEFINE_TUPLE_FULL(left,First,right,Second); DEFINE_ARRAY(tuple(left,right));

#define DEFINE_TUPLE_BOTH_WAYS(T1,T2) DEFINE_TUPLE(T1,T2); DEFINE_TUPLE(T2,T1);
#define DEFINE_TUPLE_ALL(major,T1,T2,T3,T4,T5,T6) DEFINE_TUPLE(major,major);DEFINE_TUPLE(major, T1);DEFINE_TUPLE(major, T2);DEFINE_TUPLE(major, T3);DEFINE_TUPLE(major, T4);DEFINE_TUPLE(major, T5);DEFINE_TUPLE(major, T6);

DEFINE_TUPLE_ALL(bool, char, int, long, ulong, float, double)
DEFINE_TUPLE_ALL(char, bool, int, long, ulong, float, double)
DEFINE_TUPLE_ALL(int, bool, char, long, ulong, float, double)
DEFINE_TUPLE_ALL(long, bool, char, int, ulong, float, double)
DEFINE_TUPLE_ALL(ulong, bool, char, int, long, float, double)
DEFINE_TUPLE_ALL(float, bool, char, int, long, ulong, double)
DEFINE_TUPLE_ALL(double, bool, char, int, long, ulong, float)

#define DEFINE_TUPLE_ALL_INTRINSIC(type)\
DEFINE_TUPLE_BOTH_WAYS(type,bool);\
DEFINE_TUPLE_BOTH_WAYS(type,char);\
DEFINE_TUPLE_BOTH_WAYS(type,int);\
DEFINE_TUPLE_BOTH_WAYS(type,long);\
DEFINE_TUPLE_BOTH_WAYS(type,ulong);\
DEFINE_TUPLE_BOTH_WAYS(type,float);\
DEFINE_TUPLE_BOTH_WAYS(type,double);\

#define _DEFINE_CONTAINERS(type) __pragma(warning(disable:4113)); DEFINE_ARRAY(type); DEFINE_TUPLE(type,type); DEFINE_TUPLE_ALL_INTRINSIC(type); DEFINE_POINTER(type); __pragma(warning(default:4113))
#define DEFINE_CONTAINERS(type) _DEFINE_CONTAINERS(type)

DEFINE_TUPLE_INTRINSIC(string);

#pragma warning(disable:4113)
DEFINE_POINTER(_Bool);
DEFINE_POINTER(char);
DEFINE_POINTER(int);
DEFINE_POINTER(short);
DEFINE_POINTER(long);
DEFINE_POINTER(ulong);
DEFINE_POINTER(float);
DEFINE_POINTER(double);
#pragma warning(default: 4113)

DEFINE_CONTAINERS(array(int));
