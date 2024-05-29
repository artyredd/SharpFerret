#pragma once

#include "core/array.h"
#include "core/modules.h"

struct _plugin
{
	string Name;
	Module Module;
	void (*OnStart)(void);
	void (*OnUpdate)(void);
	void (*AfterUpdate)(void);
	void (*OnFixedUpdate)(void);
	void (*AfterFixedUpdate)(void);
	void (*OnClose)(void);
};

typedef struct _plugin* Plugin;

extern struct _pluginMethods
{
	// loads the given plugin
	// loads the module and hooks up any 
	// ferret runtimes if they exist
	Plugin(*Load)(string);
	void (*Dispose)(Plugin);
} Plugins;