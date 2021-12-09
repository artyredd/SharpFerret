#include "singine/generics.h"
#include "singine/memory.h"

static void Dispose(KeyValuePair pair)
{
	SafeFree(pair->Key);
	SafeFree(pair->Value);
	SafeFree(pair);
}

KeyValuePair CreateKeyValuePair(TypeCode key, TypeCode value)
{
	KeyValuePair pair = SafeAlloc(sizeof(struct _keyValuePair));

	pair->KeyType = TypeCodes.none;
	pair->ValueType = TypeCodes.none;

	pair->Key = SafeAlloc(sizeof(union _generic));
	pair->Value = SafeAlloc(sizeof(union _generic));

	pair->Dispose = &Dispose;

	return pair;
}