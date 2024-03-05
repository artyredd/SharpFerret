#pragma once
#include <stdlib.h>
#include "engine/graphics/sharedBuffer.h"
#include "engine/graphics/imaging.h"
#include "engine/graphics/textureDefinitions.h"
#include "core/array.h"

#define DEFAULT_MINIFYING_FILTER FilterTypes.Linear
#define DEFAULT_MAGNIFYING_FILTER FilterTypes.Linear
#define DEFAULT_WRAPX WrapModes.ClampToEdge
#define DEFAULT_WRAPY WrapModes.ClampToEdge
#define DEFAULT_WRAPZ WrapModes.ClampToEdge
#define DEFAULT_TEXTURE_FORMAT TextureFormats.RGBA
#define DEFAULT_TEXTURE_BUFFER_FORMAT BufferFormats.UByte
#define DEFAULT_TEXTURE_TYPE TextureTypes.Default

typedef struct _rawTexture* RawTexture;

struct _rawTexture {
	/// <summary>
	/// The path of this texture
	/// </summary>
	char* Path;
	SharedHandle Handle;
	ulong Height;
	ulong Width;
	BufferFormat BufferFormat;
	TextureFormat Format;
	TextureType Type;
	FilterType MinificationFilter;
	FilterType MagnificationFilter;
	WrapMode WrapX;
	WrapMode WrapY;
	WrapMode WrapZ;
};

DEFINE_CONTAINERS(RawTexture);

struct _rawTextureMethods
{
	/// <summary>
	/// Returns a blank texture with a width and height of 1 px with the color white
	/// </summary>
	RawTexture(*Blank)(void);
	/// <summary>
	/// Attempts to create a new texture from the provided image and invokes the given method after the texture has been bound so modifiers such as 
	/// filters can be applied in-line
	/// </summary>
	bool (*TryCreateTextureAdvanced)(Image image, RawTexture* out_texture, const TextureType type, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state));
	/// <summary>
	/// Attempts to create a new texture from the provided image
	/// </summary>
	bool (*TryCreateTexture)(Image, RawTexture* out_texture);
	/// <summary>
	/// Create a texture on the graphics device that holds no data and can be used as a buffer
	/// </summary>
	bool (*TryCreateBufferTexture)(const TextureType type, const TextureFormat format, const BufferFormat bufferFormat, ulong width, ulong height, RawTexture* out_texture);
	/// <summary>
	/// Instances a new copy of the provided texture
	/// </summary>
	RawTexture(*Instance)(RawTexture texture);
	/// <summary>
	/// Disposed the provided texture
	/// </summary>
	void (*Dispose)(RawTexture);
	/// <summary>
	/// Loads a texture definition(.texture) from the provided path
	/// </summary>
	RawTexture(*Load)(const string path);
	/// <summary>
	/// Saves the provided texture to a given path
	/// </summary>
	void (*Save)(RawTexture texture, const string path);
};

extern const struct _rawTextureMethods RawTextures;