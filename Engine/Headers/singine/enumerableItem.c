#include "singine/enumerableItem.h"
#include "singine/memory.h"
#include "csharp.h"

Item CreateItem()
{
	Item item = SafeAlloc(sizeof(struct _item));

	item->Value = SafeAlloc(sizeof(union _generic));

	item->Type = TypeCodes.none;
	item->Next = null;
	item->Previous = null;

	return item;
}