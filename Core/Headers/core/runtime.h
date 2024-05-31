#pragma once

#include <stdio.h>
#include "core/csharp.h"
#include "core/array.h"

#define _EXPAND_STRING(value) #value
#define _STRING(value) _EXPAND_STRING(value)

#pragma warning(disable : 4075)

typedef void(__cdecl* _VoidMethod)(void);

DEFINE_CONTAINERS(_VoidMethod);
DEFINE_CONTAINERS(array(_VoidMethod));

#define SECTION_METHOD_MARKER(sectionHeader, sectionName) \
__declspec(allocate(sectionHeader)) __declspec(selectany)  const _VoidMethod sectionName = (_VoidMethod)1;

#define _ONSTART_SECTION(id) _STRING(.srt$a ## id)
#define _ONSTART_NAME(id, name) name ## id
#define _METHOD_NAME(sectionName,id) _On##sectionName##Body ## id
#define _METHOD_ADDRESS(sectionName,id) _On##sectionName##Address ## id

#define _ON_START(sectionId,name,id) static void _METHOD_NAME(name,id)(void);\
__pragma(section(_STRING(sectionId##id), read)); \
__declspec(allocate(_STRING(sectionId##id))) const _VoidMethod _METHOD_ADDRESS(name,id) = (_VoidMethod) _METHOD_NAME(name,id); \
static void _METHOD_NAME(name,id)(void)

#define DEFINE_SECTION_METHOD_RUNNER(sectionName,sectionHeaderName,sectionFooterName)\
__declspec(dllexport) inline void RunOn##sectionName##Methods() {\
	const _VoidMethod* x = &sectionHeaderName;\
	for (++x; x < &sectionFooterName; ++x)\
		if (*x) (*x)();\
};

#define START_SECTION_HEADER ".srt$a"
#define START_SECTION_FOOTER ".srt$z"

#pragma section(START_SECTION_HEADER, read)
SECTION_METHOD_MARKER(START_SECTION_HEADER, GLOBAL_InitSegStart)

#pragma section(START_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(START_SECTION_FOOTER, GLOBAL_InitSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(Start, GLOBAL_InitSegStart, GLOBAL_InitSegEnd);
#endif // !RUNTIME_METHODS_DEFINED


// Creates a method that gets called when the application starts
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define OnStart(order) _ON_START(.srt$a,Start, order)


#define UPDATE_SECTION_HEADER ".upd$a"
#define UPDATE_SECTION_FOOTER ".upd$z"

#pragma section(UPDATE_SECTION_HEADER, read)
SECTION_METHOD_MARKER(UPDATE_SECTION_HEADER, GLOBAL_UpdateSegStart)

#pragma section(UPDATE_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(UPDATE_SECTION_FOOTER, GLOBAL_UpdateSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(Update, GLOBAL_UpdateSegStart, GLOBAL_UpdateSegEnd);
#endif // !RUNTIME_METHODS_DEFINED


// Creates a method that gets called every frame
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define OnUpdate(order) _ON_START(.upd$a,Update, order)

#define AFTER_UPDATE_SECTION_HEADER ".aft$a"
#define AFTER_UPDATE_SECTION_FOOTER ".aft$z"

#pragma section(AFTER_UPDATE_SECTION_HEADER, read)
SECTION_METHOD_MARKER(AFTER_UPDATE_SECTION_HEADER, GLOBAL_AfterUpdateSegStart)

#pragma section(AFTER_UPDATE_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(AFTER_UPDATE_SECTION_FOOTER, GLOBAL_AfterUpdateSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(AfterUpdate, GLOBAL_AfterUpdateSegStart, GLOBAL_AfterUpdateSegEnd);
#endif // !RUNTIME_METHODS_DEFINED


// Creates a method that gets called AFTER every frame
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define AfterUpdate(order) _ON_START(.aft$a,AfterUpdate, order)

#define FIXED_UPDATE_SECTION_HEADER ".fup$a"
#define FIXED_UPDATE_SECTION_FOOTER ".fup$z"

#pragma section(FIXED_UPDATE_SECTION_HEADER, read)
SECTION_METHOD_MARKER(FIXED_UPDATE_SECTION_HEADER, GLOBAL_FixedUpdateSegStart)

#pragma section(FIXED_UPDATE_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(FIXED_UPDATE_SECTION_FOOTER, GLOBAL_FixedUpdateSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(FixedUpdate, GLOBAL_FixedUpdateSegStart, GLOBAL_FixedUpdateSegEnd);
#endif // !RUNTIME_METHODS_DEFINED


// Creates a method that gets called at the same real
// time between calls
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define OnFixedUpdate(order) _ON_START(.fup$a,FixedUpdate, order)

#define AFTER_FIXED_UPDATE_SECTION_HEADER ".afu$a"
#define AFTER_FIXED_UPDATE_SECTION_FOOTER ".afu$z"

#pragma section(AFTER_FIXED_UPDATE_SECTION_HEADER, read)
SECTION_METHOD_MARKER(AFTER_FIXED_UPDATE_SECTION_HEADER, GLOBAL_AfterFixedUpdateSegStart)

#pragma section(AFTER_FIXED_UPDATE_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(AFTER_FIXED_UPDATE_SECTION_FOOTER, GLOBAL_AfterFixedUpdateSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(AfterFixedUpdate, GLOBAL_AfterFixedUpdateSegStart, GLOBAL_AfterFixedUpdateSegEnd);
#endif // !RUNTIME_METHODS_DEFINED

// Creates a method that gets called at the same real
// time between calls
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define AfterFixedUpdate(order) _ON_START(.afu$a,AfterFixedUpdate, order)

#define CLOSE_SECTION_HEADER ".cls$a"
#define CLOSE_SECTION_FOOTER ".cls$z"

#pragma section(CLOSE_SECTION_HEADER, read)
SECTION_METHOD_MARKER(CLOSE_SECTION_HEADER, GLOBAL_CloseSegStart)

#pragma section(CLOSE_SECTION_FOOTER, read)
SECTION_METHOD_MARKER(CLOSE_SECTION_FOOTER, GLOBAL_CloseSegEnd)

#ifndef RUNTIME_METHODS_DEFINED
DEFINE_SECTION_METHOD_RUNNER(Close, GLOBAL_CloseSegStart, GLOBAL_CloseSegEnd);
#endif // !RUNTIME_METHODS_DEFINED

#define RUNTIME_METHODS_DEFINED

// Creates a method that gets called when the application starts
// order is a UNIQUE letter or number globally
// to determine the order this method gets run against the rest
#define OnClose(order) _ON_START(.cls$a,Close, order)

typedef byte RuntimeEventType;

static const struct _runtimeEventTypes
{
	RuntimeEventType None;
	RuntimeEventType Start;
	RuntimeEventType Close;
	RuntimeEventType Update;
	RuntimeEventType AfterUpdate;
	RuntimeEventType FixedUpdate;
	RuntimeEventType AfterFixedUpdate;
} RuntimeEventTypes = {
	.None = 0,
	.Start = 1,
	.Close = 2,
	.Update = 3,
	.AfterUpdate = 4,
	.FixedUpdate = 5,
	.AfterFixedUpdate = 6
};

extern struct _Application {
	// Closes the application
	void (*Close)(void);
	// starts the application and begin the runtime
	void (*Start)();
	// Sets the method that should be used to retrieve time
	// every frame
	void (*SetTimeProvider)(double(*Provider)());
	// the interval in seconds that Fixed update should be run
	double FixedUpdateTimeInterval;
	void (*AppendEvent)(RuntimeEventType, void(*Method)(void));
	void (*RemoveEvent)(RuntimeEventType, void(*Method)(void));
	void (*SetParentApplication)(struct _Application* parent);
	struct _state {
		// Flag that when non-zero sigals
		// the application runtime to exit the
		// application normally
		int CloseApplicationFlag;
		// the method that should be called every loop to check
		// the time and compare it to the interval
		double(*TimeProvider)();
		// Stored last real time of engine
		double PreviousTime;
		// whether or not the runtime is started
		bool RuntimeStarted;
		// whether or not OnStart methods have already started
		// executing
		bool StartMethodsBegan;
		// Pointer to the parent process application struct
		// this is non-null if the application currently
		// running is loaded as a plugin in another
		// ferret module
		struct _Application* ParentProcessApplication;
		array(array(_VoidMethod)) (*GlobalEvents)();
	} InternalState;
} Application;