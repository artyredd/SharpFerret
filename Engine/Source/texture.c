#include "engine/graphics/texture.h"

#include "engine/graphics/rawTexture.h"

private bool TryCreate(Image, Texture* out_texture);
private void Dispose(Texture);

struct _textureMethods Textures = {
	.TryCreate = TryCreate,
	.Dispose = Dispose
};

DEFINE_TYPE_ID(Texture);

private bool TryCreate(Image image, Texture* out_texture)
{
	REGISTER_TYPE(Texture);

	return true;
}

private void Dispose(Texture texture)
{
	Memory.Free(texture, typeid(Texture));
}