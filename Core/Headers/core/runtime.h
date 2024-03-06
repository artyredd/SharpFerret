#pragma once

#include <stdio.h>
#pragma warning(disable : 4075)

typedef void(__cdecl* OnStartMethod)(void);

#pragma section(".srt$a", read)
__declspec(allocate(".srt$a")) __declspec(selectany)  const OnStartMethod GLOBAL_InitSegStart = (OnStartMethod)1;

#pragma section(".srt$z",read)
__declspec(allocate(".srt$z")) __declspec(selectany)  const OnStartMethod GLOBAL_InitSegEnd = (OnStartMethod)1;

#define _EXPAND_STRING(value) #value
#define _STRING(value) _EXPAND_STRING(value)
#define _ONSTART_SECTION(id) _STRING(.srt$a ## id)
#define _ONSTART_NAME(id, name) name ## id
#define _ONSTART_METHOD_NAME(id) _OnStartBody ## id
#define _ONSTART_METHOD_ADDRESS(id) _OnStartAddress ## id

#define _ON_START(id) static void _ONSTART_METHOD_NAME(id)(void);\
__pragma(section(_ONSTART_SECTION(id), read)); \
__declspec(allocate(_ONSTART_SECTION(id))) const OnStartMethod _ONSTART_METHOD_ADDRESS(id) = (OnStartMethod)_ONSTART_METHOD_NAME(id); \
static void _ONSTART_METHOD_NAME(id)(void)

// Creates a method that gets called when the application starts
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define OnStart(order) _ON_START(order)

static void RunOnStartMethods() {
	const OnStartMethod* x = &GLOBAL_InitSegStart;
	for (++x; x < &GLOBAL_InitSegEnd; ++x)
		if (*x) (*x)();
}
