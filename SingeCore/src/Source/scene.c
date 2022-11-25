#include "graphics/scene.h"
#include "singine/memory.h"
#include "singine/guards.h"

static Scene Create(void);
static void Dispose(Scene);
static void AddLight(Scene, Light);

const struct _sceneMethods Scenes = {
	.Create= &Create,
	.Dispose = &Dispose,
	.AddLight = &AddLight
};

static Scene Create(void)
{
	return Memory.Alloc(sizeof(struct _scene));
}

static void Dispose(Scene scene)
{
	Memory.Free(scene->Lights);
	Memory.Free(scene);
}

static void AddLight(Scene scene, Light light)
{
	GuardNotNull(scene);
	GuardNotNull(light);

	// make sure there is enough room
	size_t previousSize = scene->LightCount * sizeof(Light);
	size_t newSize = (scene->LightCount + 1) * sizeof(Light);
	
	Memory.ReallocOrCopy((void**)&scene->Lights, previousSize, newSize);

	scene->Lights[scene->LightCount] = light;

	++(scene->LightCount);
}