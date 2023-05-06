#include "singine/array.h"
#include "singine/memory.h"
#include <memory.h>
#include <string.h>

private Array Create(size_t elementSize, size_t count, size_t typeId);
private size_t GetNextAvailableIndex(Array);
private void AutoResize(Array);
private void Resize(Array, size_t newCount);
private void Dispose(Array);
private void Append(Array, void*);
private void RemoveIndex(Array, size_t index);
private void InsertionSort(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
private void Swap(Array, size_t firstIndex, size_t secondIndex);

const struct _arrayMethods Arrays = {
	.Create = Create,
	.AutoResize = AutoResize,
	.Resize = Resize,
	.Append = Append,
	.InsertionSort = InsertionSort,
	.Swap = Swap,
	.Dispose = Dispose
};

TYPE_ID(Array);

private Array Create(size_t elementSize, size_t count, size_t typeId)
{
	REGISTER_TYPE(Array);

	ARRAY(void) array = Memory.Alloc(sizeof(struct _array_void), ArrayTypeId);

	if (count)
	{
		array->Values = Memory.Alloc(elementSize * count, typeId);
	}

	array->Count = count;
	array->ElementSize = elementSize;
	array->TypeId = typeId;
	array->Size = elementSize * count;
	array->Capacity = count;

	return array;
}

private void Append(Array array, void* value)
{
	// check to see if we need to resize or not
	if (array->Count < array->Capacity)
	{
		size_t offset = array->Count * array->ElementSize;
		memcpy((char*)array->Values + offset, value, array->ElementSize);
		array->Count = safe_add(array->Count, 1);
	}
	else
	{
		AutoResize(array);
		Append(array, value);
	}
}

private void AutoResize(Array array)
{
	size_t newSize = array->Size < 8192 ? max(array->Size << 1, 1) : (size_t)(array->Size * 1.2);

	if ((newSize % array->ElementSize) isnt 0)
	{
		newSize = safe_add(newSize, (newSize % array->ElementSize));
	}

	if (array->Size is 0)
	{
		array->Values = Memory.Alloc(newSize, array->TypeId);
	}
	else
	{
		Memory.ReallocOrCopy(&array->Values, array->Size, newSize, array->TypeId);
	}

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

	Memory.ReallocOrCopy(&array->Values, array->Size, newSize, array->TypeId);

	array->Size = newSize;
	array->Count = newCount;
}

private void RemoveIndex(Array array, size_t index)
{
	if (index >= array->Capacity)
	{
		throw(IndexOutOfRangeException);
	}

	size_t destinationOffset = index * array->ElementSize;
	size_t startOffset = safe_add(index, 1) * array->ElementSize;

	if (destinationOffset > array->Size || startOffset > array->Size)
	{
		throw(IndexOutOfRangeException);
	}

	size_t size = safe_subtract(array->Size, startOffset);

	memmove((char*)array->Values + destinationOffset, (char*)array->Values + startOffset, size);

	safe_decrement(array->Count);
}

private void Swap(Array array, size_t firstIndex, size_t secondIndex)
{
	char* firstSourcePointer = (char*)array->Values + (firstIndex * array->ElementSize);
	char* secondSourcePointer = (char*)array->Values + (secondIndex * array->ElementSize);

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
		size_t offset = array->ElementSize * i;

		memcpy(temporaryMemoryBlock, (char*)array->Values + offset, array->ElementSize);

		j = i - 1;

		size_t jOffset = array->ElementSize * j;

		char* jPointer = (char*)array->Values + jOffset;

		while (j >= 0 && comparator(jPointer, temporaryMemoryBlock)) {
			Swap(array, j + 1, j);

			j = j - 1;

			jOffset = array->ElementSize * j;

			jPointer = (char*)array->Values + jOffset;
		}

		memcpy((char*)array->Values + ((j + 1) * array->ElementSize), temporaryMemoryBlock, array->ElementSize);
	}

	Memory.Free(temporaryMemoryBlock, Memory.GenericMemoryBlock);
}

private void Dispose(Array array)
{
	Memory.Free(array->Values, array->TypeId);
	Memory.Free(array, ArrayTypeId);
}