#pragma once

#include "core/math/rectangles.h"
#include "engine/graphics/rawTexture.h"

struct _texture {
	// A pointer to the larger raw texture this texture is a part of
	Pointer(RawTexture) RawTexture;
	// The rectangle that this texture consumes within the larger RawTexture 
	irectangle Rect;
};

typedef struct _texture* Texture;

DEFINE_CONTAINERS(Texture);

extern struct _textureMethods {
	bool (*TryCreate)(Image, Texture* out_texture);
	void (*Dispose)(Texture);
} Textures;