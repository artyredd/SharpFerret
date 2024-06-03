#include "core/plugins.h"
#include "core/runtime.h"
#include "core/csharp.h"

private Plugin Load(string);
private void Dispose(Plugin);

struct _pluginMethods Plugins = {
	.Load = Load,
	.Dispose = Dispose
};

DEFINE_TYPE_ID(Plugin);

private Plugin Create(string name)
{
	REGISTER_TYPE(Plugin);

	Plugin plugin = Memory.Alloc(sizeof(struct _plugin), PluginTypeId);

	plugin->Name = strings.Clone(name);

	return plugin;
}

private struct _Application* GetChildApplication(Plugin plugin)
{
	struct _Application* (*GetApplication)(void) = Modules.Find(plugin->Module, stack_string("GetApplication"));

	if (GetApplication is null)
	{
		fprintf_red(stdout, "Plugin: %s does not have method: struct _Application* GetApplication(void);\n", plugin->Name->Values);
		throw(FailedToLoadMethodException);
	}

	struct _Application* result = GetApplication();

	if (result is null)
	{
		fprintf_red(stdout, "Plugin: %s GetApplication(void) method did not return a valid pointer to the plugins Application object\n", plugin->Name->Values);

		throw(FailedToLoadPluginException);
	}

	return result;
}

private void HookPluginMethodsIntoRuntime(Plugin plugin)
{
	// assign the events
	Application.AppendEvent(RuntimeEventTypes.Start, plugin->OnStart);
	Application.AppendEvent(RuntimeEventTypes.Close, plugin->OnClose);
	Application.AppendEvent(RuntimeEventTypes.Update, plugin->OnUpdate);
	Application.AppendEvent(RuntimeEventTypes.AfterUpdate, plugin->AfterUpdate);
	Application.AppendEvent(RuntimeEventTypes.FixedUpdate, plugin->OnFixedUpdate);
	Application.AppendEvent(RuntimeEventTypes.AfterFixedUpdate, plugin->AfterFixedUpdate);
	Application.AppendEvent(RuntimeEventTypes.Render, plugin->OnRender);
	Application.AppendEvent(RuntimeEventTypes.AfterRender, plugin->AfterRender);
}

private void UnHookPluginMethodsFromRuntime(Plugin plugin)
{
	// assign the events
	Application.RemoveEvent(RuntimeEventTypes.Start, plugin->OnStart);
	Application.RemoveEvent(RuntimeEventTypes.Close, plugin->OnClose);
	Application.RemoveEvent(RuntimeEventTypes.Update, plugin->OnUpdate);
	Application.RemoveEvent(RuntimeEventTypes.AfterUpdate, plugin->AfterUpdate);
	Application.RemoveEvent(RuntimeEventTypes.FixedUpdate, plugin->OnFixedUpdate);
	Application.RemoveEvent(RuntimeEventTypes.AfterFixedUpdate, plugin->AfterFixedUpdate);
	Application.AppendEvent(RuntimeEventTypes.Render, plugin->OnRender);
	Application.RemoveEvent(RuntimeEventTypes.AfterRender, plugin->AfterRender);
}

private Plugin Load(string name)
{
	Plugin plugin = Create(name);

	plugin->Module = Modules.Load(name);

	plugin->OnStart = Modules.Find(plugin->Module, stack_string("RunOnStartMethods"));
	plugin->OnClose = Modules.Find(plugin->Module, stack_string("RunOnCloseMethods"));
	plugin->OnUpdate = Modules.Find(plugin->Module, stack_string("RunOnUpdateMethods"));
	plugin->AfterUpdate = Modules.Find(plugin->Module, stack_string("RunOnAfterUpdateMethods"));
	plugin->OnFixedUpdate = Modules.Find(plugin->Module, stack_string("RunOnFixedUpdateMethods"));
	plugin->AfterFixedUpdate = Modules.Find(plugin->Module, stack_string("RunOnAfterFixedUpdateMethods"));
	plugin->OnRender = Modules.Find(plugin->Module, stack_string("RunOnOnRenderMethods"));
	plugin->AfterRender = Modules.Find(plugin->Module, stack_string("RunOnAfterRenderMethods"));

	if (plugin->OnStart is null
		or plugin->OnClose is null
		or plugin->OnUpdate is null
		or plugin->AfterUpdate is null
		or plugin->OnFixedUpdate is null
		or plugin->AfterFixedUpdate is null
		or plugin->OnRender is null
		or plugin->AfterRender is null
		)
	{
		fprintf_red(stderr, "Failed to load plugin: %s\n", name->Values);
		throw(FailedToLoadPluginException);
	}

	// take over the child plugins application
	// so their events hook into ours
	struct _Application* application = GetChildApplication(plugin);

	application->SetParentApplication(&Application);

	// attach the plugin's events to our runtime
	HookPluginMethodsIntoRuntime(plugin);

	return plugin;
}

private void Dispose(Plugin plugin)
{
	if (plugin is null)
	{
		return;
	}

	// make sure the runtime doesn't attempt to call the plugins methods
	UnHookPluginMethodsFromRuntime(plugin);

	strings.Dispose(plugin->Name);

	Memory.Free(plugin, PluginTypeId);
}