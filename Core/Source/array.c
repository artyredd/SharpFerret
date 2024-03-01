#include "core/array.h"
#include "core/memory.h"
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
private Array AppendArray(Array array, Array appendedValue);
private void* At(Array array, size_t index);
private void Clear(Array array);
private void Foreach(Array array, void(*method)(void* item));
private void ForeachWithContext(Array array, void* context, void(*method)(void* context, void* item));
private Array InsertArray(Array dest, Array src, size_t index);

const struct _arrayMethods Arrays = {
	.Create = Create,
	.AutoResize = AutoResize,
	.Resize = Resize,
	.Append = Append,
	.InsertionSort = InsertionSort,
	.RemoveIndex = RemoveIndex,
	.Swap = Swap,
	.Dispose = Dispose,
	.AppendArray = AppendArray,
	.InsertArray = InsertArray,
	.At = At,
	.Clear = Clear,
	.Foreach = Foreach,
	.ForeachWithContext = ForeachWithContext
};

private Array Create(size_t elementSize, size_t count, size_t typeId)
{
	REGISTER_TYPE(Array);

	array(void) array = Memory.Alloc(sizeof(struct _array_void), ArrayTypeId);

	if (count)
	{
		// since alloc gets a zeroed block of memory and 0 is '\0'
		// alloc one more byte of space to ensure all of our arrays are
		// always terminated, regardless if their strings or not
		array->Values = Memory.Alloc((elementSize * count) + 1, typeId);
	}

	array->Count = 0;
	array->ElementSize = elementSize;
	array->TypeId = typeId;
	array->Size = (elementSize * count) + 1;
	array->Capacity = count;

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
}

private void AutoResize(Array array)
{
	// if a user allocs a array on the stack they can't modify past the given
	// memory block without overwriting stack stuff
	if (array->StackObject)
	{
		throw(StackObjectModifiedException);
	}

	size_t newSize = max(array->Size << 1, 1);

	if ((newSize % array->ElementSize) isnt 0)
	{
		newSize = safe_add(newSize, (newSize % array->ElementSize));
	}

	if (array->Size is 0)
	{
		// alloc one more byte so its terminated
		array->Values = Memory.Alloc(newSize + 1, array->TypeId);
	}
	else
	{
		// alloc one more byte so its terminated
		Memory.ReallocOrCopy(&array->Values, array->Size, newSize + 1, array->TypeId);
	}

	array->Capacity = newSize / array->ElementSize;
	array->Size = newSize + 1;
}

private void Resize(Array array, size_t newCount)
{
	// if a user allocs a array on the stack they can't modify past the given
		// memory block without overwriting stack stuff
	if (array->StackObject)
	{
		throw(StackObjectModifiedException);
	}

	if (newCount is 0)
	{
		Memory.Free(array->Values, array->TypeId);
		array->Values = null;
		array->Size = 0;
		array->Count = 0;
		return;
	}

	size_t newSize = array->ElementSize * newCount;

	Memory.ReallocOrCopy(&array->Values, array->Size, newSize + 1, array->TypeId);

	array->Size = newSize + 1;
	array->Count = newCount;
}

private void RemoveIndex(Array array, size_t index)
{
	if (index >= array->Count)
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

private Array AppendArray(Array destinationArray, Array values)
{
	for (size_t i = 0; i < values->Count; i++)
	{
		Arrays.Append(destinationArray, At(values, i));
	}

	return destinationArray;
}

private Array InsertArray(Array destination, Array source, size_t index)
{
	guard_array_count(destination, index);

	if (destination->Capacity >= (destination->Count + source->Count))
	{
		// make space for the new array
		char* sourcePtr = At(destination, index);
		char* destinationPtr = At(destination, index + source->Count);

		memmove(destinationPtr, sourcePtr, destination->Count - index);

		memcpy(sourcePtr, source->Values, source->Count * source->ElementSize);

		destination->Count = safe_add(destination->Count, source->Count);

		return destination;
	}

	AutoResize(destination);
	return InsertArray(destination, source, index);
}

private void Clear(Array array)
{
	memset(array->Values, 0, array->Size);
	array->Count = 0;
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

private void Dispose(Array array)
{
	// avoid crazyness
	if (array->StackObject)
	{
		throw(StackObjectModifiedException);
	}

	Memory.Free(array->Values, array->TypeId);
	Memory.Free(array, ArrayTypeId);
}