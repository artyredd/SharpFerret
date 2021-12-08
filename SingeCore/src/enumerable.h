
typedef struct _item* Item;
struct _item {
	Item Next;
	Item Previous;
	union {
		size_t Value;
		void* Value;
	};
};

typedef struct _enumerable* Enumerable;

struct _enumerable {
	/// <summary>
	/// The head of the doubly linked list
	/// </summary>
	Item Head;
	/// <summary>
	/// The tail of the doubly linked list
	/// </summary>
	Item Tail;
	/// <summary>
	/// The number of items within this enumerable
	/// </summary>
	size_t Count;
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
	/// Whether or not this Enumerable is circular. 
	/// When this Enumerable is circular and contains at least ONE item 
	/// TryGetNext() and TryGetPrevious() will never return false unless an error occurs.
	/// Otherwise TryGetPrevious() will return false when Current is the head, and TryGetNext() will return false when Current is the tail.
	/// </summary>
	bool Circular;
};