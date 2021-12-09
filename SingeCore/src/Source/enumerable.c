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
	if (collection is null)
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

	Item next = current->Next;

	collection->Current = next;

	*out_item = next;

	return true;
}

static bool TryGetPrevious(Enumerable collection, Item* out_item)
{
	if (collection is null)
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

	Item previous = current->Previous;

	collection->Current = previous;

	*out_item = previous;

	return true;
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

	if (previous is collection->Tail)
	{
		collection->Tail = item;
	}
	if (previous is collection->Head)
	{
		collection->Head = item;
	}

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

	if (next is collection->Tail)
	{
		collection->Tail = item;
	}
	if (next is collection->Head)
	{
		collection->Head = item;
	}

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

	if (collection->Head is null)
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

	// break any circular links
	if (collection->Tail != null)
	{
		collection->Tail->Next = null;
	}

	Item head = collection->Head;
	while (head != null)
	{
		Item tmp = head;

		head = head->Next;

		SafeFree(tmp->Value);
		SafeFree(tmp);
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

	collection->Dispose(collection);

	return 1;
}

static bool TryGetNextWorks(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	for (size_t i = 0; i < 10; i++)
	{
		collection->Append(collection)->Value->AsSizeT = i;
	}

	Assert(collection->Current->Value->AsSizeT == 0);

	Item item = collection->Current;

	Equals((size_t)0, item->Value->AsSizeT, "%lli");

	for (size_t i = 1; i < 10; i++)
	{
		item = null;

		IsTrue(collection->TryGetNext(collection, &item));

		NotNull(item);

		Equals(i, item->Value->AsSizeT, "%lli");
	}

	item = null;

	IsFalse(collection->TryGetNext(collection, &item));

	Equals(collection->Count, (size_t)10, "%lli");

	collection->Dispose(collection);

	return true;
}

static bool TryGetPreviousWorks(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	collection->Circular = true;

	for (size_t i = 0; i < 10; i++)
	{
		collection->Append(collection)->Value->AsSizeT = i;
	}

	Item item = collection->Current;

	Equals((size_t)0, item->Value->AsSizeT, "%lli");

	for (size_t i = 9; i > 0; i--)
	{
		item = null;

		IsTrue(collection->TryGetPrevious(collection, &item));

		NotNull(item);

		Equals(i, item->Value->AsSizeT, "%lli");
	}

	item = null;

	Equals(collection->Count, (size_t)10, "%lli");

	collection->Dispose(collection);

	return true;
}

static bool ResetWorks(FILE* stream)
{
	Enumerable collection = CreateEnumerable();

	for (size_t i = 0; i < 10; i++)
	{
		collection->Append(collection)->Value->AsSizeT = i;
	}

	Assert(collection->Current == collection->Head);

	Item item;

	IsTrue(collection->TryGetNext(collection, &item));

	Assert(item->Previous is collection->Head);

	Assert(collection->Current != collection->Head);

	collection->Reset(collection);

	Assert(collection->Current == collection->Head);

	collection->Dispose(collection);

	return true;
}

bool RunTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "Instantiation", &InstantiatesCorrectly);
	suite->Append(suite, "DisposesCorrectly", &DisposesCorrectly);
	suite->Append(suite, "AppendWorks", &AppendWorks);
	suite->Append(suite, "RemoveWorks", &RemoveWorks);
	suite->Append(suite, "TryGetNextWorks", &TryGetNextWorks);
	suite->Append(suite, "TryGetPreviousWorks", &TryGetPreviousWorks);
	suite->Append(suite, "ResetWorks", &ResetWorks);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}