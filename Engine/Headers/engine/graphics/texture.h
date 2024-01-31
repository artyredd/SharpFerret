#pragma once

#include "core/math/rectangles.h"
#include "engine/graphics/rawTexture.h"

struct _texture {
	RawTexture Texture;

};

typedef struct _texture Texture;

DEFINE_CONTAINERS(Texture);

extern struct _textureMethods {
	Texture(*Create)();
	void (*Dispose)();
} Textures;