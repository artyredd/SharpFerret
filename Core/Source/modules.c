#include "core/modules.h"
#include "core/csharp.h"
#include "core/array.h"
#include <windows.h>

#define MAX_PATH_LENGTH 2048

private void* GetMethod(Module, string);
private Module Load(string);
private void Dispose(Module module);

struct _modulesMethods Modules = {
	.LogModuleLoading = true,
	.Load = Load,
	.Find = GetMethod,
	.ModuleLocations = nested_stack_array(string,
		stack_string("assets\\plugins\\")
	),
	.Dispose = Dispose
};

#define log if(Modules.LogModuleLoading)

DEFINE_TYPE_ID(Module);

private Module Create(string name)
{
	REGISTER_TYPE(Module);

	Module module = Memory.Alloc(sizeof(struct _module), ModuleTypeId);

	module->Name = strings.Clone(name);

	return module;
}

private Module Load(string name)
{
	if (name is null or name->Count is 0)
	{
		throw(FailedToOpenFileException);
	}

	Module module = Create(name);

	log{ fprintf(stdout, "Loading module: %s\n", name->Values); }

	module->Handle = LoadLibrary(name->Values);

	if (module->Handle is null)
	{
		log{ fprintf(stderr, "Failed to find module, looking in alternate Modules.ModuleLocations\n"); };

		bool found = false;
		// attempt alternate locations
		for (int i = 0; i < Modules.ModuleLocations->Count; i++)
		{
			const string alternateDirectory = at(Modules.ModuleLocations, i);

			string combined = empty_stack_array(byte, MAX_PATH_LENGTH);

			strings.AppendArray(combined, alternateDirectory);
			strings.AppendArray(combined, name);

			log{ fprintf(stdout, "Loading module: %s\n", combined->Values); }

			module->Handle = LoadLibrary(combined->Values);

			if (module->Handle)
			{
				found = true;
				log{ fprintf_green(stdout, "Loaded library: %s\n", combined->Values); };
				break;
			}
			else
			{
				log{ fprintf_red(stderr, "Failed to load library: %s\n", combined->Values); }
			}
		}

		if (found is false)
		{
			fprintf_red(stderr, "Failed to load library: %s\n", name->Values);
			throw(FailedToLoadLibraryException);
		}
	}

	return module;
}

private void* GetMethod(Module module, string methodName)
{
	if (module->Handle is null or methodName is null or methodName->Count is 0)
	{
		throw(InvalidArgumentException);
	}

	void* result = null;

	result = GetProcAddress(module->Handle, methodName->Values);

	if (result is null)
	{
		fprintf_red(stderr, "Failed to load method %s from library %s\n", methodName->Values, module->Name->Values);
		throw(FailedToLoadMethodException);
	}

	return result;
}

private void Dispose(Module module)
{
	if (module is null)
	{
		return;
	}

	if (module->Handle) {
		FreeLibrary(module->Handle);
	}

	Memory.Free(module, ModuleTypeId);
}