#pragma once
#include <stdlib.h>
#include "graphics/sharedBuffer.h"
#include "graphics/imaging.h"

#define DEFAULT_MINIFYING_FILTER Filters.Linear
#define DEFAULT_MAGNIFYING_FILTER Filters.Linear
#define DEFAULT_TEXTURE_FORMAT TextureFormats.RGBA
#define DEFAULT_TEXTURE_BUFFER_FORMAT BufferFormats.UByte

typedef unsigned int TextureFormat;

struct _textureFormats {
	TextureFormat None;
	TextureFormat Red;
	TextureFormat RG;
	TextureFormat RGB;
	TextureFormat BGR;
	TextureFormat RGBA;
	TextureFormat BGRA;
	TextureFormat RedInteger;
	TextureFormat RGInteger;
	TextureFormat RGBInteger;
	TextureFormat BGRInteger;
	TextureFormat RGBAInteger;
	TextureFormat BGRAInteger;
	TextureFormat StencilIndex;
	TextureFormat DepthComponent;
	TextureFormat DepthStencil;
};

extern const struct _textureFormats TextureFormats;

typedef unsigned int BufferFormat;

struct _bufferFormat
{
	BufferFormat None;
	BufferFormat UByte;
	BufferFormat Byte;
	BufferFormat UShort;
	BufferFormat Short;
	BufferFormat UInt;
	BufferFormat Int;
	BufferFormat Float;
	BufferFormat UByte332;
	BufferFormat UByte233Rev;
	BufferFormat UShort565;
	BufferFormat UShort565Rev;
	BufferFormat UShort4444;
	BufferFormat UShort4444Rev;
	BufferFormat UShort5551;
	BufferFormat UShort1555Rev;
	BufferFormat UInt8888;
	BufferFormat UInt8888Rev;
	BufferFormat UInt1010102;
	BufferFormat UInt2101010Rev;
};

extern const struct _bufferFormat BufferFormats;

typedef int TextureSetting;

struct _textureSettings {
	TextureSetting DepthStencilMode;
	TextureSetting BaseLevel;
	TextureSetting CompareFunction;
	TextureSetting CompareMode;
	TextureSetting LevelOfDetailBias;
	TextureSetting MinimumLevelOfDetail;
	TextureSetting MaximumLevelOfDetail;
	TextureSetting MinifyingFilter;
	TextureSetting MagnifyingFilter;
	TextureSetting SwizzleR;
	TextureSetting SwizzleG;
	TextureSetting SwizzleB;
	TextureSetting SwizzleA;
	TextureSetting SwizzleRGBA;
	TextureSetting WrapX;
	TextureSetting WrapY;
	TextureSetting WrapZ;
};

extern const struct _textureSettings TextureSettings;

typedef int TextureSettingValue;

struct _textureFilters {
	TextureSettingValue Nearest;
	TextureSettingValue Linear;
	TextureSettingValue NearestMipMapNearest;
	TextureSettingValue LinearMipMapNearest;
	TextureSettingValue NearestMipMapLinear;
	TextureSettingValue LinearMipMapLinear;
};

extern const struct _textureFilters Filters;

struct _textureWrapModes {
	TextureSettingValue ClampToEdge;
	TextureSettingValue ClampToBorder;
	TextureSettingValue MirroredRepeat;
	TextureSettingValue Repeat;
	TextureSettingValue MirrorClampToEdge;
};

extern const struct _textureWrapModes WrapModes;

typedef struct _texture* Texture;

struct _texture {
	SharedHandle Handle;
	size_t Height;
	size_t Width;
	BufferFormat BufferFormat;
	TextureFormat Format;
	/// <summary>
	/// The texture slot that this texture should be bound to in order to be drawn, default is TEXTURE_0 (1), 0 is NO slot at all and the texture can't be drawn
	/// </summary>
	unsigned int Slot;
};

struct _textureMethods
{
	/// <summary>
	/// Modifies the currently bound texture, this should be invoked in the method given to TryCreateTextureAdvanced
	/// </summary>
	void(*Modify)(TextureSetting setting, TextureSettingValue value);
	/// <summary>
	/// Attempts to create a new texture from the provided image and invokes the given method after the texture has been bound so modifiers such as 
	/// filters can be applied in-line
	/// </summary>
	bool (*TryCreateTextureAdvanced)(Image image, Texture* out_texture, TextureFormat format, BufferFormat bufferFormat, bool (*TryModifyTexture)(unsigned int handle));
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
};

extern const struct _textureMethods Textures;