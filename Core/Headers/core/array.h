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

#define new_array(type) arrays(type).Create(0)


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

#define explicit_stack_array(type,initialArraySize,count, sizeOffset , ...) (array(type))&(struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = (type*)(type[initialArraySize]){ __VA_ARGS__ },\
	.Size = (sizeof(type) * count) + sizeOffset,\
	.ElementSize = sizeof(type),\
	.Capacity = count,\
	.Count = count,\
	.TypeId = 0,\
	.StackObject = true,\
	.Dirty = true,\
	.Hash = 0,\
	.AutoHash = true\
}

#define stack_array(type,count,...) explicit_stack_array(type,count,count,0,__VA_ARGS__)

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

#define _stack_string(string) explicit_stack_array(byte,,sizeof(string)-1,1, string) 
#define stack_string(string) _stack_string(string) 

#define _dynamic_array(type, initialCount) _EXPAND_METHOD_NAME(type,Create)(initialCount) 
#define dynamic_array(type, initialCount) _dynamic_array(type, initialCount)

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

#define __at(arr,index) (arr->Values[index]) 
#define _at(arr,index) __at(arr,guard_array_count_index(arr, index))
// Gets the element at the given index within the array
// Will throw if out of bounds
#define at(arr,index) _at(arr,index)
// Gets the element at the given index within the array
// Ignores bounds
#define unsafe_at(arr,index) __at(arr,index)

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
	.Values = &at(arr,startIndex),\
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
	.Values = arr->Count >= (ulong)partial.Values ? (arr->Values + (ulong)partial.Values) : (byte*)(ulong)throw_in_expression(IndexOutOfRangeException),\
	.Size = guard_array_size(arr, partial.Size),\
	.ElementSize = guard_array_attribute(arr,ElementSize,partial.ElementSize),\
	.Capacity = guard_array_attribute(arr, Capacity, partial.Capacity),\
	.Count = guard_array_count(arr,(partial).Count),\
	.TypeId = partial.TypeId,\
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
private array(type) _EXPAND_METHOD_NAME(type,Append)(array(type)array, type value)\
{\
Arrays.Append((Array)array, &value); \
return array;\
}\
private void _EXPAND_METHOD_NAME(type,RemoveIndex)(array(type) array, ulong index)\
{\
Arrays.RemoveIndex((Array)array, index); \
}\
private type* _EXPAND_METHOD_NAME(type, At)(array(type) array, ulong index)\
{\
return (type*)Arrays.At((Array)array, index); \
}\
private array(type) _EXPAND_METHOD_NAME(type, Clear)(array(type)array)\
{\
Arrays.Clear((Array)array); \
return array;\
}\
private array(type) _EXPAND_METHOD_NAME(type, RemoveRange)(array(type) array, ulong index, ulong count)\
{\
	/* { a, b, c, d, e } */\
	/* RemoveRange(2,2) */\
	/* { a, b, e } */\
	const ulong safeCount = guard_array_count(array, count);\
	const ulong safeIndex = guard_array_count_index(array, index);\
	type* destination = &array->Values[safeIndex];\
	const ulong maybeSafeEndIndex = safe_add(safeCount, safeIndex);\
	if(maybeSafeEndIndex >= array->Count){ \
		memset(destination, 0, safeCount * array->ElementSize);\
	}else{\
		const ulong safeEndIndex = guard_array_count_index(array, maybeSafeEndIndex); \
		type* source = &array->Values[safeEndIndex]; \
		const ulong safeRemainingCount = safe_subtract(array->Count, safeEndIndex); \
		memmove(destination, source, safeRemainingCount* array->ElementSize); \
		/* null terminate at the end to make debugbing strings easier in the debugger */\
		memset(destination + safeRemainingCount, '\0', 1);\
	}\
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
private type _EXPAND_METHOD_NAME(type, ValueAt)(array(type) array, ulong index)\
{\
return at(array,index); \
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
	array->Hash = Hashing.HashSafe((byte*)array->Values, array->Count * array->ElementSize);\
}\
return array->Hash; \
}\
private bool _EXPAND_METHOD_NAME(type, Equals)(array(type) left, array(type) right)\
{\
return Arrays.Equals((Array)left,(Array)right);\
}\
private bool _EXPAND_METHOD_NAME(type, BeginsWith)(array(type) array, array(type) value)\
{\
ulong safeCount = safe_subtract(min(value->Count,array->Count),1);\
array(type) sub = stack_subarray_front(type, array, safeCount);\
return Arrays.Equals((Array)sub, (Array)value); \
}\
private array(type) _EXPAND_METHOD_NAME(type, Fill)(array(type) array, type value)\
{\
	for(int i = 0; i < array->Count; i++)\
	{\
		at(array, i) = value;\
	}array->Dirty = true;array->Hash = 0;\
	return array;\
}\
private type _EXPAND_METHOD_NAME(type, Last)(array(type)array)\
{\
	return at(array, array->Count - 1);\
}\
private type _EXPAND_METHOD_NAME(type, Pop)(array(type)array)\
{\
	type result = at(array, array->Count - 1);\
	safe_decrement(array->Count);\
	array->Dirty = true;\
	array->Hash = 0;\
	return result;\
}\
private int _EXPAND_METHOD_NAME(type, IndexOf)(array(type) array, type value)\
{\
for (int i = 0; i < array->Count; i++)\
{\
if (memcmp(&value, &(at(array, i)), array->ElementSize) is 0)\
{\
return i; \
}\
}\
return -1; \
}\
private void _EXPAND_METHOD_NAME(type, Print)(void* stream, array(type) array)\
{\
fprintf(stream, "{ "); \
for (int i = 0; i < array->Count; i++)\
{\
type value = at(array, i); \
if (IsTypeof(value, struct _array_byte*) is true) { fprintf(stream, "%s", (char*)((*(array(type)*)(void*) & value)->Values)); }\
else { fprintf(stream, FormatCType(value), value); }\
if (i isnt array->Count - 1) { fprintf(stream, ", "); }\
}\
fprintf(stream, " }"); \
}\
private int _EXPAND_METHOD_NAME(type, IndexWhere)(array(type) array, void* state, bool(*expression)(type, void*))\
{\
for (int i = 0; i < array->Count; i++)\
{\
if (expression(at(array, i), state)) { return i; }\
}\
return -1; \
}\
private type _EXPAND_METHOD_NAME(type, Select)(array(type) array, void* state, bool(*expression)(type, void*))\
{\
int index = _EXPAND_METHOD_NAME(type, IndexWhere)(array, state, expression); \
if (index is - 1) { return (type) { 0 }; }\
return at(array, index); \
}\
private array(type) _EXPAND_METHOD_NAME(type, Any)(array(type) arr, void* state, bool(*expression)(type, void*))\
{\
array(type) result = null; \
for (int i = 0; i < arr->Count; i++)\
{\
type item = at(arr, i); \
if (expression(item, state))\
{\
if (result) { result = _EXPAND_METHOD_NAME(type, Create)(1); }\
_EXPAND_METHOD_NAME(type, Append)(result, item); \
}\
}\
return null; \
}\
const static struct _array_##type##_methods\
{\
array(type) (*Create)(ulong count); \
void (*AutoResize)(array(type)); \
void (*Resize)(array(type), ulong newCount); \
array(type) (*Append)(array(type), type); \
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
array(type) (*Clear)(array(type)); \
void (*Foreach)(array(type), void(*method)(type*)); \
void (*ForeachWithContext)(array(type), void* context, void(*method)(void*, type*)); \
int (*IndexOf)(array(type), type); \
int (*IndexWhere)(array(type), void* state, bool(*Expression)(type value, void* state)); \
type(*Select)(array(type), void* state, bool(*Expression)(type value, void* state)); \
array(type) (*Any)(array(type), void* state, bool(*Expression)(type value, void* state)); \
array(type) (*Clone)(array(type)); \
ulong(*Hash)(array(type)); \
array(type) (*Fill)(array(type), type); \
type(*Pop)(array(type)); \
array(type) (*Push)(array(type), type); \
type (*Last)(array(type));\
void (*Print)(void* stream, array(type)); \
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
.Fill = _EXPAND_METHOD_NAME(type, Fill), \
.Pop = _EXPAND_METHOD_NAME(type, Pop), \
.Push = _EXPAND_METHOD_NAME(type, Append), \
.Last = _EXPAND_METHOD_NAME(type, Last), \
.IndexOf = _EXPAND_METHOD_NAME(type, IndexOf), \
.IndexWhere = _EXPAND_METHOD_NAME(type, IndexWhere), \
.Select = _EXPAND_METHOD_NAME(type, Select), \
.Any = _EXPAND_METHOD_NAME(type, Any), \
.Print = _EXPAND_METHOD_NAME(type, Print), \
.Dispose = _EXPAND_METHOD_NAME(type, Dispose)\
};

#define DEFINE_ARRAY(type) _EXPAND_DEFINE_ARRAY(type)

#define DEFINE_COMPARATOR(type, name, operation) private bool name(type* left, type* right){return *(left) operation *(right);} 

// define common types
DEFINE_ARRAY(bool);
DEFINE_ARRAY(byte);
DEFINE_ARRAY(int);
DEFINE_ARRAY(float);
DEFINE_ARRAY(double);
DEFINE_ARRAY(ulong);

DEFINE_ARRAY(array(byte));

#define string array(byte)
#define strings arrays(byte)
#define stack_substring(arr, index, count) stack_subarray(byte, arr, index, count)
#define stack_substring_front(arr, endIndexInclusive) stack_subarray_front(byte, arr, endIndexInclusive)
#define stack_substring_back(arr, startIndexInclusive) stack_subarray_back(byte, arr, startIndexInclusive)
#define partial_string partial_array(byte)
#define partial_substring(arr,index,count) partial_subarray(byte,arr,index,count)
#define partial_substring_front(arr, endIndexInclusive) partial_subarray_front(byte, arr, endIndexInclusive)
#define partial_substring_back(arr, startIndexInclusive) partial_subarray_back(byte, arr, startIndexInclusive)
#define combine_partial_string(arr, partial) combine_partial_array(byte, arr, partial)
#define dynamic_string(cString) strings.AppendArray(strings.Create(sizeof(cString)-1), stack_string(cString))
#define empty_dynamic_string(count) strings.Create(count)

#define _EXPAND_tuple(left,right) left##_##right##_tuple
#define tuple(left,right) _EXPAND_tuple(left,right)

#define _DEFINE_TUPLE_FULL(left,leftname,right,rightname) struct _##left##_##right##_tuple{ left leftname;right rightname;}; typedef struct _##left##_##right##_tuple left##_##right##_tuple; 
#define DEFINE_TUPLE_FULL(left,leftname,right,rightname) _DEFINE_TUPLE_FULL(left,leftname,right,rightname)
#define DEFINE_TUPLE(left,right) DEFINE_TUPLE_FULL(left,First,right,Second); DEFINE_ARRAY(tuple(left,right));

#define DEFINE_TUPLE_BOTH_WAYS(T1,T2) DEFINE_TUPLE(T1,T2); DEFINE_TUPLE(T2,T1);
#define DEFINE_TUPLE_ALL(major,T1,T2,T3,T4,T5,T6,T7) DEFINE_TUPLE(major,major);\
DEFINE_TUPLE(major, T1);\
DEFINE_TUPLE(major, T2);\
DEFINE_TUPLE(major, T3);\
DEFINE_TUPLE(major, T4);\
DEFINE_TUPLE(major, T5);\
DEFINE_TUPLE(major, T6);\
DEFINE_TUPLE(major, T7);

DEFINE_TUPLE_ALL(bool, byte, int, long, ulong, float, double, string)
DEFINE_TUPLE_ALL(byte, bool, int, long, ulong, float, double, string)
DEFINE_TUPLE_ALL(int, bool, byte, long, ulong, float, double, string)
DEFINE_TUPLE_ALL(long, bool, byte, int, ulong, float, double, string)
DEFINE_TUPLE_ALL(ulong, bool, byte, int, long, float, double, string)
DEFINE_TUPLE_ALL(float, bool, byte, int, long, ulong, double, string)
DEFINE_TUPLE_ALL(double, bool, byte, int, long, ulong, float, string)
DEFINE_TUPLE_ALL(string, bool, byte, int, long, ulong, float, double)


#define DEFINE_TUPLE_ALL_INTRINSIC(type)\
DEFINE_TUPLE_BOTH_WAYS(type,bool);\
DEFINE_TUPLE_BOTH_WAYS(type,byte);\
DEFINE_TUPLE_BOTH_WAYS(type,int);\
DEFINE_TUPLE_BOTH_WAYS(type,long);\
DEFINE_TUPLE_BOTH_WAYS(type,ulong);\
DEFINE_TUPLE_BOTH_WAYS(type,float);\
DEFINE_TUPLE_BOTH_WAYS(type,double);\
DEFINE_TUPLE_BOTH_WAYS(type,string);

#define _DEFINE_CONTAINERS(type) __pragma(warning(disable:4113)); DEFINE_ARRAY(type); DEFINE_TUPLE(type,type); DEFINE_TUPLE_ALL_INTRINSIC(type); DEFINE_POINTER(type); __pragma(warning(default:4113))
#define DEFINE_CONTAINERS(type) _DEFINE_CONTAINERS(type)

#define FORWARD_CONTAINER_TYPE(type) struct _EXPAND_STRUCT_NAME(type); typedef struct _array_##type* type##_array;

DEFINE_TUPLE_INTRINSIC(string);

typedef void* void_ptr;
DEFINE_CONTAINERS(void_ptr);
DEFINE_CONTAINERS(array(int));
DEFINE_TUPLE(array(string), array(int));
//DEFINE_CONTAINERS(tuple(string, int));