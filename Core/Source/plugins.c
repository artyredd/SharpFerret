#include "core/plugins.h"

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

	if (plugin->OnStart is null
		or plugin->OnClose is null
		or plugin->OnUpdate is null
		or plugin->AfterUpdate is null
		or plugin->OnFixedUpdate is null
		or plugin->AfterFixedUpdate is null
		)
	{
		fprintf_red(stderr, "Failed to load plugin: %s\n", name->Values);
		throw(FailedToLoadPluginException);
	}

	return plugin;
}

private void Dispose(Plugin plugin)
{
	if (plugin is null)
	{
		return;
	}

	strings.Dispose(plugin->Name);

	Memory.Free(plugin, PluginTypeId);
}