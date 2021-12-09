#pragma once

#include "csharp.h"
#include "singine/enumerableItem.h"

typedef struct _enumerable* Enumerable;

struct _enumerable {
	/// <summary>
	/// The number of items within this enumerable
	/// </summary>
	size_t Count;
	/// <summary>
	/// Whether or not this Enumerable is circular. 
	/// When this Enumerable is circular and contains at least ONE item 
	/// TryGetNext() and TryGetPrevious() will never return false unless an error occurs.
	/// Otherwise TryGetPrevious() will return false when Current is the head, and TryGetNext() will return false when Current is the tail.
	/// </summary>
	bool Circular;
	/// <summary>
	/// The head of the doubly linked list
	/// </summary>
	Item Head;
	/// <summary>
	/// The tail of the doubly linked list
	/// </summary>
	Item Tail;
	/// <summary>
	/// The current item
	/// </summary>
	Item Current;
	/// <summary>
	/// Attempts to get the next item within the Enumerable, returns true if out_item was set with the next item, otherwise false
	/// </summary>
	bool (*TryGetNext)(Enumerable, Item* out_item);
	/// <summary>
	/// Attempts to get the previous item within the Enumerable, returns true if out_item was set with the previous item, otherwise false
	/// </summary>
	bool (*TryGetPrevious)(Enumerable, Item* out_item);
	/// <summary>
	/// Inserts a new item as the previous item
	/// </summary>
	Item(*InsertPrevious)(Enumerable);
	/// <summary>
	/// Inserts a new item as the next item
	/// </summary>
	Item(*InsertNext)(Enumerable);
	/// <summary>
	/// Appends the item to the end of the Enumerable, it will become the new end of the Enumerable
	/// </summary>
	Item(*Append)(Enumerable);
	/// <summary>
	/// Resets the current item back to the head of the enumerable(if it exists)
	/// </summary>
	void(*Reset)(Enumerable);
	/// <summary>
	/// Removes the current item from the Enumerable
	/// </summary>
	Item(*Remove)(Enumerable);
	/// <summary>
	/// Disposes of this object and all item contained
	/// </summary>
	void(*Dispose)(Enumerable);
};

/// Creates a new enumerable object implemented as a linked list
Enumerable CreateEnumerable();

bool RunTests();