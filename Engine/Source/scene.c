#include "engine/graphics/scene.h"
#include "core/memory.h"
#include "core/guards.h"

static Scene Create(void);
static void Dispose(Scene);
static void AddLight(Scene, Light);

const struct _sceneMethods Scenes = {
	.Create = &Create,
	.Dispose = &Dispose,
	.AddLight = &AddLight
};

DEFINE_TYPE_ID(Scene);
DEFINE_TYPE_ID(Lights);

static Scene Create(void)
{
	Memory.RegisterTypeName(nameof(Scene), &SceneTypeId);
	Memory.RegisterTypeName("Scene_Lights", &LightsTypeId);

	return Memory.Alloc(sizeof(struct _scene), SceneTypeId);
}

static void Dispose(Scene scene)
{
	Memory.Free(scene->Lights, LightsTypeId);
	Memory.Free(scene, SceneTypeId);
}

static void AddLight(Scene scene, Light light)
{
	GuardNotNull(scene);
	GuardNotNull(light);

	// make sure there is enough room
	size_t previousSize = scene->LightCount * sizeof(Light);
	size_t newSize = (scene->LightCount + 1) * sizeof(Light);

	Memory.ReallocOrCopy((void**)&scene->Lights, previousSize, newSize, LightsTypeId);

	scene->Lights[scene->LightCount] = light;

	++(scene->LightCount);
}