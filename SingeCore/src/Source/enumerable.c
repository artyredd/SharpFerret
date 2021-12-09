#include "singine/enumerable.h"
#include "singine/memory.h"
#include "cunit.h"

static bool TryGetNext(Enumerable, Item* out_item);
static bool TryGetPrevious(Enumerable, Item* out_item);
static Item InsertPrevious(Enumerable);
static Item InsertNext(Enumerable);
static Item Append(Enumerable);
static void Reset(Enumerable);
static Item Remove(Enumerable);

static void Dispose(Enumerable collection);

Enumerable CreateEnumerable()
{
	Enumerable list = SafeAlloc(sizeof(struct _enumerable));

	// define fields
	list->Count = 0;
	list->Head = list->Tail = list->Current = null;
	list->Circular = false;

	// define methods
	list->TryGetNext = &TryGetNext;
	list->TryGetPrevious = &TryGetPrevious;
	list->InsertNext = &InsertNext;
	list->InsertPrevious = &InsertPrevious;
	list->Append = &Append;
	list->Reset = &Reset;
	list->Remove = &Remove;
	list->Dispose = &Dispose;

	return list;
}

static bool TryGetNext(Enumerable collection, Item* out_item)
{
	// default to no out value
	*out_item = null;

	if (collection is null || out_item is null)
	{
		return false;
	}

	Item current = collection->Current;

	if (current is null)
	{
		return false;
	}

	// in a circular list the tail will always point to the head and not be null
	if (current->Next is null)
	{
		return false;
	}

	collection->Current = current->Next;

	return current;
}

static bool TryGetPrevious(Enumerable collection, Item* out_item)
{
	// default to no out value
	*out_item = null;


	if (collection is null || out_item is null)
	{
		return false;
	}

	Item current = collection->Current;

	if (current is null)
	{
		return false;
	}

	// in a circular list the tail will always point to the head and not be null
	if (current->Previous is null)
	{
		return false;
	}

	collection->Current = current->Previous;

	return current;
}

static Item InsertPrevious(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	++(collection->Count);

	Item item = CreateItem();

	Item current = collection->Current;

	if (current is null)
	{
		collection->Current = collection->Head = collection->Tail = item;

		return item;
	}

	// shift the previous link left
	Item previous = current->Previous;

	current->Previous = item;

	previous->Next = item;

	return item;
}

static Item InsertNext(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	++(collection->Count);

	Item item = CreateItem();

	Item current = collection->Current;

	if (current is null)
	{
		collection->Current = collection->Head = collection->Tail = item;

		return item;
	}

	// shift the previous link left
	Item next = current->Next;

	current->Next = item;

	next->Previous = item;

	return item;
}

static Item Append(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	++(collection->Count);

	Item item = CreateItem();

	if (collection->Tail is null)
	{
		collection->Current = collection->Head = collection->Tail = item;

		if (collection->Circular)
		{
			item->Next = item;
			item->Previous = item;
		}

		return item;
	}

	collection->Tail->Next = item;

	item->Previous = collection->Tail;

	collection->Tail = item;

	if (collection->Circular)
	{
		collection->Head->Previous = item;
		item->Next = collection->Head;
	}

	return item;
}

static void Reset(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	collection->Current = collection->Head;
}

static Item Remove(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	--(collection->Count);

	Item current = collection->Current;

	// move the closest link to the current location
	if (current->Next != null)
	{
		collection->Current = current->Next;
	}
	else if (current->Previous != null)
	{
		collection->Current = current->Previous;
	}
	else
	{
		collection->Current = null;
	}

	// remove the link
	if (current->Previous != null)
	{
		current->Previous->Next = current->Next;
	}
	if (current->Next != null)
	{
		current->Next->Previous = current->Previous;
	}

	// make sure we don't lose the list
	if (current is collection->Head)
	{
		collection->Head = collection->Current;
	}
	if (current is collection->Tail)
	{
		collection->Tail = collection->Current;
	}

	return current;
}

static void Dispose(Enumerable collection)
{
	if (collection is null)
	{
		fprintf(stderr, "collection was null"NEWLINE);
		throw(InvalidArgumentException);
	}

	// free the links
	collection->Reset(collection);

	Item item;
	while (collection->TryGetNext(collection, &item))
	{
		// if we got to the tail
		if (item is collection->Tail)
		{
			SafeFree(item->Value);
			SafeFree(item);
			break;
		}

		SafeFree(item->Value);
		SafeFree(item);
	}

	SafeFree(collection);
}

// START UNIT TESTS

static bool InstantiatesCorrectly(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	Equals(collection->Count, (size_t)0, "%lli");

	Equals(collection->Circular, false, "%i");

	IsNull(collection->Head);
	IsNull(collection->Tail);
	IsNull(collection->Current);

	NotNull(collection->Append);
	NotNull(collection->Reset);
	NotNull(collection->InsertNext);
	NotNull(collection->InsertPrevious);
	NotNull(collection->TryGetNext);
	NotNull(collection->TryGetPrevious);
	NotNull(collection->Dispose);
	NotNull(collection->Remove);

	collection->Dispose(collection);

	return true;
}

static bool DisposesCorrectly(FILE* stream)
{
	ResetAlloc();
	ResetFree();

	Enumerable collection = CreateEnumerable();

	NotZero(AllocCount());

	collection->Dispose(collection);

	NotZero(FreeCount());

	Equals(AllocCount(), FreeCount(), "%lli");

	ResetAlloc();
	ResetFree();

	return true;
}

static bool AppendWorks(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	IsZero(collection->Count);

	Item item = collection->Append(collection);

	NotZero(collection->Count);

	Equals((size_t)1, collection->Count, "%lli");

	Assert(item == collection->Head);

	Assert(item == collection->Tail);

	// since circular is false the tail should NOT point to the head
	Assert(item->Next != item);

	collection->Dispose(collection);

	return true;
}

static bool RemoveWorks(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	IsZero(collection->Count);

	Item item = collection->Append(collection);

	Equals((size_t)1, collection->Count, "%lli");

	Item removed = collection->Remove(collection);

	Assert(item == removed);

	IsZero(collection->Count);

	IsNull(collection->Head);
	IsNull(collection->Tail);
	IsNull(collection->Current);

	return 1;
}

bool RunTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "Instantiation", &InstantiatesCorrectly);
	suite->Append(suite, "DisposesCorrectly", &DisposesCorrectly);
	suite->Append(suite, "DisposesCorrectly", &AppendWorks);
	suite->Append(suite, "DisposesCorrectly", &RemoveWorks);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}