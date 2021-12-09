#pragma once

#include "singine/generics.h"

/// <summary>
/// Pointer to an item structure that represents a link within a doubly linked list
/// </summary>
typedef struct _item* Item;

struct _item {
	Item Next;
	Item Previous;
	Generic Value;
	TypeCode Type;
};

Item CreateItem();