#pragma once

// Specifies the target texture.
typedef unsigned int TextureType;

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

extern const struct _textureTypes TextureTypes;

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