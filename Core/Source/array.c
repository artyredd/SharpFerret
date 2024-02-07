#include "core/array.h"
#include "core/memory.h"
#include <memory.h>
#include <core/hashing.h>
#include <string.h>

private Array Create(size_t elementSize, size_t count, size_t typeId);
private Array CreateFromCArray(const void* cArray, size_t elementSize, size_t count, size_t typeId);
private size_t GetNextAvailableIndex(Array);
private void AutoResize(Array);
private void Resize(Array, size_t newCount);
private void Dispose(Array);
private void Append(Array, void*);
private void RemoveIndex(Array, size_t index);
private void InsertionSort(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
private void Swap(Array, size_t firstIndex, size_t secondIndex);
private void AppendArray(Array array, Array appendedValue);
private void* At(Array array, size_t index);
private void Clear(Array array);
private void Foreach(Array array, void(*method)(void* item));
private void ForeachWithContext(Array array, void* context, void(*method)(void* context, void* item));
private Array Clone(Array);
private bool Equals(Array, Array);
private size_t Hash(Array);

const struct _arrayMethods Arrays = {
	.Create = Create,
	.CreateFromCArray = CreateFromCArray,
	.AutoResize = AutoResize,
	.Resize = Resize,
	.Append = Append,
	.InsertionSort = InsertionSort,
	.RemoveIndex = RemoveIndex,
	.Swap = Swap,
	.Dispose = Dispose,
	.AppendArray = AppendArray,
	.At = At,
	.Clear = Clear,
	.Foreach = Foreach,
	.ForeachWithContext = ForeachWithContext,
	.Clone = Clone,
	.Equals = Equals,
	.Hash = Hash
};

DEFINE_TYPE_ID(Array);

private Array CreateFromCArray(const void* cArray, size_t elementSize, size_t count, size_t typeId)
{
	Array array = Create(elementSize, count, typeId);

	memcpy(array->Values, cArray, count * elementSize);

	return array;
}

private Array Create(size_t elementSize, size_t count, size_t typeId)
{
	REGISTER_TYPE(Array);

	ARRAY(void) array = Memory.Alloc(sizeof(struct _array_void), ArrayTypeId);

	if (count)
	{
		array->Values = Memory.Alloc((elementSize * count) + 1, typeId);

		// null terminate memory block for legacy reasons
		((char*)array->Values)[elementSize * count] = '\0';
	}

	array->Count = count;
	array->ElementSize = elementSize;
	array->TypeId = typeId;
	// size is +1 to fit the null terminator
	array->Size = (elementSize * count) + 1;
	array->Capacity = count;
	array->Hashcode = 0;

	return array;
}

private void Append(Array array, void* value)
{
	// check to see if we need to resize or not
	if (array->Count < array->Capacity)
	{
		memcpy(At(array, array->Count), value, array->ElementSize);

		array->Count = safe_add(array->Count, 1);
	}
	else
	{
		AutoResize(array);
		Append(array, value);
	}

	if (array->Hashcode is 0)
	{
		Hash(array->Values);
	}
	else
	{
		array->Hashcode = Hashing.ChainHash(value, array->Hashcode);
	}
}

private void AutoResize(Array array)
{
	size_t newSize = max(array->Size << 1, 1);

	if ((newSize % array->ElementSize) isnt 0)
	{
		newSize = safe_add(newSize, (newSize % array->ElementSize));
	}

	// add one to add null terminator for legacy cstr things
	newSize = newSize + 1;

	if (array->Size is 0)
	{
		array->Values = Memory.Alloc(newSize, array->TypeId);
	}
	else
	{
		Memory.ReallocOrCopy(&array->Values, array->Size, newSize, array->TypeId);
	}

	((char*)array->Values)[newSize - 1] = '\0';

	array->Capacity = array->Size / array->ElementSize;
	array->Size = newSize;
}

private void Resize(Array array, size_t newCount)
{
	if (newCount is 0)
	{
		Memory.Free(array->Values, array->TypeId);
		array->Values = null;
		array->Size = 0;
		array->Count = 0;
		return;
	}

	size_t newSize = array->ElementSize * newCount;

	// add one to add null terminator for legacy cstr things
	newSize = newSize + 1;

	Memory.ReallocOrCopy(&array->Values, array->Size, newSize, array->TypeId);

	// set null terminator
	((char*)array->Values)[newSize - 1] = '\0';

	array->Size = newSize;
	array->Count = newCount;
}

private void RemoveIndex(Array array, size_t index)
{
	if (index >= array->Capacity)
	{
		throw(IndexOutOfRangeException);
	}

	size_t size = safe_subtract(array->Size, safe_add(index, 1) * array->ElementSize);

	memmove(At(array, index), At(array, safe_add(index, 1)), size);

	safe_decrement(array->Count);
}

private void Swap(Array array, size_t firstIndex, size_t secondIndex)
{
	char* firstSourcePointer = At(array, firstIndex);
	char* secondSourcePointer = At(array, secondIndex);

	for (size_t i = 0; i < array->ElementSize / sizeof(char); i++)
	{
		size_t offset = (sizeof(char) * i);

		char* firstPointer = firstSourcePointer + offset;
		char* secondPointer = secondSourcePointer + offset;

		char tmp = *firstPointer;
		*firstPointer = *secondPointer;
		*secondPointer = tmp;
	}
}

private void InsertionSort(Array array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock))
{
	// chat gpt generated insertion sort cuz im lazy
	size_t j = 0;

	char* temporaryMemoryBlock = Memory.Alloc(array->ElementSize, Memory.GenericMemoryBlock);

	for (size_t i = 1; i < array->Count; i++) {

		memcpy(temporaryMemoryBlock, At(array, i), array->ElementSize);

		j = i - 1;

		char* jPointer = At(array, j);

		while (j >= 0 && comparator(jPointer, temporaryMemoryBlock)) {
			Swap(array, j + 1, j);

			j = j - 1;

			jPointer = At(array, j);
		}

		memcpy(At(array, j + 1), temporaryMemoryBlock, array->ElementSize);
	}

	Memory.Free(temporaryMemoryBlock, Memory.GenericMemoryBlock);
}

// Gets a pointer to the value contained at index
private void* At(Array array, size_t index)
{
	return (char*)array->Values + (index * array->ElementSize);
}

private void AppendArray(Array array, Array appension)
{
	for (size_t i = 0; i < appension->Count; i++)
	{
		Arrays.Append(array, At(appension, i));
	}
}

private void Clear(Array array)
{
	memset(array->Values, 0, array->Size);

	array->Count = 0;

	// set null terminator
	((char*)array->Values)[array->Size - 1] = '\0';
}

private void Foreach(Array array, void(*method)(void* item))
{
	for (size_t i = 0; i < array->Count; i++)
	{
		method(At(array, i));
	}
}

private void ForeachWithContext(Array array, void* context, void(*method)(void* context, void* item))
{
	for (size_t i = 0; i < array->Count; i++)
	{
		method(context, At(array, i));
	}
}

private Array Clone(Array array)
{
	Array newArray = Create(array->ElementSize, array->Count, array->TypeId);

	memcpy(newArray->Values, array->Values, array->Size);

	return newArray;
}

private bool Equals(Array left, Array right)
{
	// if they point to the same memory block they must be equal
	if (left->Values == right->Values)
	{
		return true;
	}

	if (left->Count != right->Count)
	{
		return false;
	}

	if (left->ElementSize != right->ElementSize)
	{
		return false;
	}

	if (left->Hashcode && right->Hashcode)
	{
		return left->Hashcode == right->Hashcode;
	}

	if (left->Hashcode is 0)
	{
		Hash(left);
	}

	if (right->Hashcode is 0)
	{
		Hash(right);
	}

	return left->Hashcode == right->Hashcode;
}

private size_t Hash(Array array)
{
	size_t hash = Hashing.Hash(array->Values);

	array->Hashcode = hash;

	return hash;
}

private void Dispose(Array array)
{
	Memory.Free(array->Values, array->TypeId);
	Memory.Free(array, ArrayTypeId);
}