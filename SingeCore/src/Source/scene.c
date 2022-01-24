#include "graphics/scene.h"
#include "singine/memory.h"

static Scene Create(void);
static void Dispose(Scene);

const struct _sceneMethods Scenes = {
	.Create= &Create,
	.Dispose = &Dispose
};

static Scene Create(void)
{
	return SafeAlloc(sizeof(struct _scene));
}

static void Dispose(Scene scene)
{
	SafeFree(scene);
}