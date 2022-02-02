#pragma once
#include "graphics/sharedBuffer.h"
#include "graphics/texture.h"

typedef struct _cubeMap* CubeMap;

struct _cubeMap {
	/// <summary>
	/// The actual handle to the cubemap texture on the graphics device, this is shared between all instanced of this cube map
	/// it is only actually disposed when the number of instances is less than or equal to 1
	/// </summary>
	SharedHandle Handle;
};

struct _cubeMapMethods {
	CubeMap(*Create)(void);
	CubeMap(*Instance)(CubeMap);
	void (*Dispose)(CubeMap);
};

extern const struct _cubeMapMethods CubeMaps;