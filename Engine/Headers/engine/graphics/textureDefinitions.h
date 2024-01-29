#pragma once

#include "core/reflection.h" 

// Specifies the target texture.
typedef parsableValue TextureType;

struct _textureTypes {
	/// <summary>
	/// Default texture type is texture 2d
	/// </summary>
	TextureType Default;
	/// <summary>
	/// A cubemap texture object
	/// </summary>
	TextureType CubeMap;
	// A cubemap texture face, add with integer to set the nth face order: +x,-x, +y, -y, +z, -z
	TextureType CubeMapFace;
};

extern struct _textureTypes TextureTypes;

#define MAX_TEXTURE_TYPES sizeof(TextureTypes)/sizeof(TextureType)

#define TryGetTextureTypeName(value, out_name) ParsableValues.TryGetName(&TextureTypes, MAX_TEXTURE_TYPES, value, out_name)
#define TryGetTextureTypeValue(buffer, length, out_value) ParsableValues.TryGetInt(&TextureTypes, MAX_TEXTURE_TYPES, buffer, length, out_value)
#define TryGetTextureType(buffer, length, out_value) ParsableValues.TryGetMemberByName(&TextureTypes, MAX_TEXTURE_TYPES, buffer, length, out_value)

// Specifies the format of the pixel data.
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
	TextureFormat Depth24Stencil8;
};

extern const struct _textureFormats TextureFormats;

// Specifies the data type of the pixel data.
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
	BufferFormat UInt248;
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
	TextureSetting BorderColor;
};

extern const struct _textureSettings TextureSettings;

typedef parsableValue TextureValue;

/// <summary>
/// A parsable value object with it's value union interpreted as an int
/// </summary>
typedef TextureValue FilterType;

struct _textureFilters {
	FilterType Nearest;
	FilterType Linear;
	FilterType NearestMipMapNearest;
	FilterType LinearMipMapNearest;
	FilterType NearestMipMapLinear;
	FilterType LinearMipMapLinear;
};

extern struct _textureFilters FilterTypes;

#define MAX_FILTER_TYPES sizeof(FilterTypes)/sizeof(FilterType)

#define TryGetFilterName(value, out_name) ParsableValues.TryGetName(&FilterTypes, MAX_FILTER_TYPES, value, out_name)
#define TryGetFilterValue(buffer, length, out_value) ParsableValues.TryGetInt(&FilterTypes, MAX_FILTER_TYPES, buffer, length, out_value)
#define TryGetFilterType(buffer, length, out_value) ParsableValues.TryGetMemberByName(&FilterTypes, MAX_FILTER_TYPES, buffer, length, out_value)

/// <summary>
/// A parsable value object with it's value union interpreted as an int
/// </summary>
typedef TextureValue WrapMode;

struct _textureWrapModes {
	WrapMode ClampToEdge;
	WrapMode ClampToBorder;
	WrapMode MirroredRepeat;
	WrapMode Repeat;
	WrapMode MirrorClampToEdge;
};

extern struct _textureWrapModes WrapModes;

#define MAX_WRAP_MODES sizeof(WrapModes)/sizeof(WrapMode)

#define TryGetWrapModeName(value, out_name) ParsableValues.TryGetName(&WrapModes, MAX_WRAP_MODES, value, out_name)
#define TryGetWrapModeValue(buffer, length, out_value) ParsableValues.TryGetInt(&WrapModes, MAX_WRAP_MODES, buffer, length, out_value)
#define TryGetWrapModeType(buffer, length, out_value) ParsableValues.TryGetMemberByName(&WrapModes, MAX_WRAP_MODES, buffer, length, out_value)