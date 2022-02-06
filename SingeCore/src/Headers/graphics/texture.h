#pragma once
#include <stdlib.h>
#include "graphics/sharedBuffer.h"
#include "graphics/imaging.h"
#include "graphics/textureDefinitions.h"

#define DEFAULT_MINIFYING_FILTER FilterTypes.Linear
#define DEFAULT_MAGNIFYING_FILTER FilterTypes.Linear
#define DEFAULT_WRAPX WrapModes.ClampToEdge
#define DEFAULT_WRAPY WrapModes.ClampToEdge
#define DEFAULT_WRAPZ WrapModes.ClampToEdge
#define DEFAULT_TEXTURE_FORMAT TextureFormats.RGBA
#define DEFAULT_TEXTURE_BUFFER_FORMAT BufferFormats.UByte
#define DEFAULT_TEXTURE_TYPE TextureTypes.Default

typedef struct _texture* Texture;

struct _texture {
	/// <summary>
	/// The path of this texture
	/// </summary>
	char* Path;
	SharedHandle Handle;
	size_t Height;
	size_t Width;
	BufferFormat BufferFormat;
	TextureFormat Format;
	TextureType* Type;
	FilterType* MinificationFilter;
	FilterType* MagnificationFilter;
	WrapMode* WrapX;
	WrapMode* WrapY;
	WrapMode* WrapZ;
};

struct _textureMethods
{
	/// <summary>
	/// Returns a blank texture with a width and height of 1 px with the color white
	/// </summary>
	Texture(*Blank)(void);
	/// <summary>
	/// Attempts to create a new texture from the provided image and invokes the given method after the texture has been bound so modifiers such as 
	/// filters can be applied in-line
	/// </summary>
	bool (*TryCreateTextureAdvanced)(Image image, Texture* out_texture, const TextureType type, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state));
	/// <summary>
	/// Attempts to create a new texture from the provided image
	/// </summary>
	bool (*TryCreateTexture)(Image, Texture* out_texture);
	/// <summary>
	/// Instances a new copy of the provided texture
	/// </summary>
	Texture(*Instance)(Texture texture);
	/// <summary>
	/// Disposed the provided texture
	/// </summary>
	void (*Dispose)(Texture);
	/// <summary>
	/// Loads a texture definition(.texture) from the provided path
	/// </summary>
	Texture(*Load)(const char* path);
	/// <summary>
	/// Saves the provided texture to a given path
	/// </summary>
	void (*Save)(Texture texture, const char* path);
};

extern const struct _textureMethods Textures;