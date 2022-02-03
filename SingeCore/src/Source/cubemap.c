#include "graphics/cubemap.h"
#include "graphics/graphicsDevice.h"
#include "singine/memory.h"

static CubeMap Create(void);
static CubeMap Instance(CubeMap);
static void Dispose(CubeMap);

const struct _cubeMapMethods CubeMaps = {
	.Create = Create,
	.Instance = Instance,
	.Dispose = Dispose,
};

typedef Texture CubeMapTextureArray[6];

static CubeMap Create(void)
{
	return SafeAlloc(sizeof(struct _cubeMap));
}

static CubeMap Instance(CubeMap map)
{
	CubeMap newMap = Create();

	newMap->Handle = SharedHandles.Instance(map->Handle);

	return newMap;
}

void OnHandleDisposed(CubeMap map)
{
	GraphicsDevice.DeleteTexture(map->Handle->Handle);
}

static void Dispose(CubeMap map)
{
	if (map is null) { return; }

	SharedHandles.Dispose(map->Handle, map, &OnHandleDisposed);

	SafeFree(map);
}