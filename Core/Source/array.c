#include "core/array.h"
#include "core/memory.h"
#include <memory.h>
#include <string.h>
#include "core/cunit.h"

private Array Create(ulong elementSize, ulong count, ulong typeId);
private ulong GetNextAvailableIndex(Array);
private void AutoResize(Array);
private void Resize(Array, ulong newCount);
private void Dispose(Array);
private void Append(Array, void*);
private void RemoveIndex(Array, ulong index);
private void InsertionSort(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
private void Swap(Array, ulong firstIndex, ulong secondIndex);
private Array AppendArray(Array array, Array appendedValue);
private void* At(Array array, ulong index);
private void Clear(Array array);
private void Foreach(Array array, void(*method)(void* item));
private void ForeachWithContext(Array array, void* context, void(*method)(void* context, void* item));
private Array InsertArray(Array dest, Array src, ulong index);
private bool Equals(Array left, Array right);

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
	.ForeachWithContext = ForeachWithContext,
	.Equals = Equals
};

private Array Create(ulong elementSize, ulong count, ulong typeId)
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
	array->Dirty = true;
	array->AutoHash = true;
	array->StackObject = false;
	array->Hash = 0;

	return array;
}

private ulong HashArray(Array array)
{
	array->Hash = Hashing.HashSafe(array->Values, array->Count * array->ElementSize);
	array->Dirty = false;

	return array->Hash;
}

private void Append(Array array, void* value)
{
	// check to see if we need to resize or not
	if (array->Count < array->Capacity)
	{
		memcpy(At(array, array->Count), value, array->ElementSize);

		array->Count = safe_add(array->Count, 1);


		if (array->AutoHash)
		{
			// we only have to hash the new value
			if (array->Dirty is false and array->AutoHash)
			{
				array->Hash = Hashing.ChainHashSafe(value, array->ElementSize, array->Hash);
			}
			else
			{
				HashArray(array);
			}
		}
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

	ulong newSize = max(array->Size << 1, 1);

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

private void Resize(Array array, ulong newCount)
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

	ulong newSize = array->ElementSize * newCount;

	Memory.ReallocOrCopy(&array->Values, array->Size, newSize + 1, array->TypeId);

	array->Size = newSize + 1;
	array->Count = newCount;
}

private void RemoveIndex(Array array, ulong index)
{
	if (index >= array->Count)
	{
		throw(IndexOutOfRangeException);
	}

	ulong size = safe_subtract(array->Size, safe_add(index, 1) * array->ElementSize);

	memmove(At(array, index), At(array, safe_add(index, 1)), size);

	safe_decrement(array->Count);

	array->Dirty = true;
}

private void Swap(Array array, ulong firstIndex, ulong secondIndex)
{
	char* firstSourcePointer = At(array, firstIndex);
	char* secondSourcePointer = At(array, secondIndex);

	for (ulong i = 0; i < array->ElementSize / sizeof(char); i++)
	{
		ulong offset = (sizeof(char) * i);

		char* firstPointer = firstSourcePointer + offset;
		char* secondPointer = secondSourcePointer + offset;

		char tmp = *firstPointer;
		*firstPointer = *secondPointer;
		*secondPointer = tmp;
	}

	array->Dirty = true;
}

private void InsertionSort(Array array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock))
{
	// chat gpt generated insertion sort cuz im lazy
	// and i got punished for it because this creates a buffer
	// overrun somwhere that i now have to debug
	char* tempPointer = Memory.Alloc(array->ElementSize, Memory.GenericMemoryBlock);

	int leftIndex = 0;

	for (int rightIndex = 1; rightIndex < array->Count; rightIndex++) {
		memcpy(tempPointer, At(array, rightIndex), array->ElementSize);

		leftIndex = rightIndex - 1;

		char* leftPointer = At(array, leftIndex);

		while (leftIndex >= 0 && comparator(leftPointer, tempPointer)) {
			Swap(array, leftIndex + 1, leftIndex);

			leftIndex = leftIndex - 1;

			leftPointer = At(array, leftIndex);
		}

		memcpy(At(array, leftIndex + 1), tempPointer, array->ElementSize);
	}

	Memory.Free(tempPointer, Memory.GenericMemoryBlock);

	array->Dirty = true;
}

// Gets a pointer to the value contained at index
private void* At(Array array, ulong index)
{
	return (char*)array->Values + (index * array->ElementSize);
}

private Array AppendArray(Array destinationArray, Array values)
{
	for (ulong i = 0; i < values->Count; i++)
	{
		Arrays.Append(destinationArray, At(values, i));
	}

	return destinationArray;
}

private Array InsertArray(Array destination, Array source, ulong index)
{
	guard_array_count_index(destination, index);

	if (destination->Capacity >= (destination->Count + source->Count))
	{
		// make space for the new array
		char* sourcePtr = At(destination, index);
		char* destinationPtr = At(destination, index + source->Count);

		memmove(destinationPtr, sourcePtr, destination->Count - index);

		memcpy(sourcePtr, source->Values, source->Count * source->ElementSize);

		destination->Count = safe_add(destination->Count, source->Count);

		destination->Dirty = true;

		return destination;
	}

	AutoResize(destination);
	return InsertArray(destination, source, index);
}

private void Clear(Array array)
{
	memset(array->Values, 0, array->Size);
	array->Count = 0;
	array->Dirty = false;
}

private void Foreach(Array array, void(*method)(void* item))
{
	for (ulong i = 0; i < array->Count; i++)
	{
		method(At(array, i));
	}
}

private void ForeachWithContext(Array array, void* context, void(*method)(void* context, void* item))
{
	for (ulong i = 0; i < array->Count; i++)
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

private bool Equals(Array left, Array right)
{
	if (left is right)
	{
		return true;
	}

	if ((left is null && right isnt null) || (left isnt null && right is null))
	{
		return false;
	}

	if ((left->Values is null && right->Values isnt null) || (left->Values isnt null && right->Values is null))
	{
		return false;
	}

	if (left->Count != right->Count)
	{
		return false;
	}

	if (left->Values is right->Values)
	{
		return true;
	}

	if (left->AutoHash && right->AutoHash)
	{

		if (left->Dirty is false and right->Dirty is false)
		{
			return left->Hash is right->Hash;
		}
		else if (left->Dirty ^ right->Dirty)
		{
			// only one needs to be hashed
			ulong rightOrLeft = left->Dirty ? right->Hash : left->Hash;
			ulong leftOrRight = left->Dirty ? HashArray(left) : HashArray(right);

			return rightOrLeft is leftOrRight;
		}

		// we calc the hashes in the next method
		right->Dirty = false;
		left->Dirty = false;

		return Memory.CompareMemoryAndHash(
			left->Values,
			left->Count * left->ElementSize,
			right->Values,
			right->Count * right->ElementSize,
			&left->Hash, &right->Hash) is 0;
	}

	return memcmp(left->Values,
		right->Values,
		left->ElementSize * left->Count) is 0;
}

#include "core/runtime.h"

TEST(Equals)
{
	string left = dynamic_string("ABCDEF");
	string right = dynamic_string("ABCDEF");
	string third = dynamic_string("UWUOWO");

	IsTrue(strings.Equals(left, left));
	IsTrue(strings.Equals(null, null));
	IsFalse(strings.Equals(left, null));
	IsFalse(strings.Equals(null, left));

	char* ptr = left->Values;
	left->Values = null;

	IsFalse(strings.Equals(left, right));

	left->Values = ptr;
	ptr = right->Values;
	right->Values = null;

	IsFalse(strings.Equals(left, right));

	right->Values = ptr;

	safe_increment(right->Count);

	IsFalse(strings.Equals(left, right));

	safe_decrement(right->Count);

	IsTrue(strings.Equals(left, right));

	ptr = right->Values;
	right->Values = left->Values;

	IsTrue(strings.Equals(left, right));

	right->Values = ptr;

	IsTrue(strings.Equals(left, right));

	// test memcmp branch
	left->AutoHash = false;
	right->AutoHash = false;
	third->AutoHash = false;
	IsTrue(strings.Equals(left, right));
	IsFalse(strings.Equals(left, third));
	IsFalse(strings.Equals(right, third));

	// test hashing branch
	left->AutoHash = true;
	right->AutoHash = true;
	third->AutoHash = true;

	left->Hash = strings.Hash(left);
	right->Hash = strings.Hash(right);

	IsEqual(left->Hash, right->Hash);
	IsFalse(left->Dirty);
	IsFalse(right->Dirty);

	IsTrue(strings.Equals(left, right));

	right->Dirty = true;
	right->Hash = 0;

	IsTrue(strings.Equals(left, right));
	IsFalse(left->Dirty);
	IsFalse(right->Dirty);
	IsEqual(left->Hash, right->Hash);

	left->Dirty = true;
	left->Hash = 0;

	IsTrue(strings.Equals(left, right));
	IsFalse(left->Dirty);
	IsFalse(right->Dirty);
	IsEqual(left->Hash, right->Hash);

	left->Dirty = true;
	left->Hash = 0;

	right->Dirty = true;
	right->Hash = 0;

	IsTrue(strings.Equals(left, right));
	IsFalse(left->Dirty);
	IsFalse(right->Dirty);
	IsEqual(left->Hash, right->Hash);

	return true;
}

TEST(Hashing)
{
	string data = stack_string("The Quick Brown Fox");
	int hash = strings.Hash(data);

	IsNotZero(hash);

	string other = dynamic_string("The Quick Brown Fox");

	IsTrue(strings.Equals(data, other));

	int otherHash = strings.Hash(other);

	IsEqual(hash, otherHash);

	return true;
}

DEFINE_COMPARATOR(byte, GreaterThan, > );

TEST(Sorting)
{
	string data = stack_string("21875098126354912839");

	strings.InsertionSort(data, GreaterThan);

	string expected = stack_string("01112223345567888999");

	IsStringEqual(expected, data);

	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(Hashing)
	APPEND_TEST(Equals)
	APPEND_TEST(Sorting)
);

OnStart(11)
{
	RunUnitTests();
}