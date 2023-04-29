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

const struct _arrayMethods Arrays = {
	.Create = Create,
	.AutoResize = AutoResize,
	.Resize = Resize,
	.Append = Append,
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

private void Dispose(Array array)
{
	Memory.Free(array->Values, array->TypeId);
	Memory.Free(array, ArrayTypeId);
}