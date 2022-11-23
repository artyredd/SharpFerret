#include "types.h"
#include "singine/memory.h"


static Pointer Create(void);
static void SetValue(Pointer, void* value, size_t size);
static bool GetValue(Pointer, void* out_value);
static void Dispose(Pointer);

const struct _pointerMethods Pointers =
{
	.Create = &Create,
	.SetValue = &SetValue,
	.GetValue = &GetValue,
	.Dispose = &Dispose
};

static Pointer Create(void)
{
	Pointer result = SafeAlloc(sizeof(Pointer));

	result->BlockSize = 0;
	result->Pointer = (size_t)null;

	return result;
}

static void SetValue(Pointer pointer, void* value, size_t size)
{
	if (pointer == false || value)
	{
		throw(NullReferenceException);
	}

	if (value)
	{
		if (pointer->Pointer)
		{
		
		}
		else
		{
			pointer->Pointer = SafeAlloc();
		}
	}
	else
	{
		throw(InvalidArgumentException);
	}
}

static bool GetValue(Pointer pointer, void* out_value)
{
	if (pointer && out_value)
	{
	
	}
	return true;
}

static void Dispose(Pointer pointer)
{
	SafeFree(pointer);
}