#pragma once

#include "core/array.h"

struct _module
{
	string Name;
	void* Handle;
};

typedef struct _module* Module;

extern struct _modulesMethods {
	// whether or not loading and closing modules are logged to stdout
	bool LogModuleLoading;
	array(string) ModuleLocations;
	// Finds the item within the given module using the given name
	void* (*Find)(Module, string name);
	// Loads the module with the given name
	Module(*Load)(string name);
	void (*Dispose)(Module);
} Modules;