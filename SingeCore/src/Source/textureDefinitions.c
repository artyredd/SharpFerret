#include "graphics/textureDefinitions.h"
#include "GL/glew.h"

struct _textureTypes TextureTypes = {
	.Default = {.Name = "2d", .Value = GL_TEXTURE_2D },
	.CubeMap = {.Name = "cubemap", .Value = GL_TEXTURE_CUBE_MAP },
	.CubeMapFace = {.Name = "cubemap", .Value = GL_TEXTURE_CUBE_MAP_POSITIVE_X }
};

const struct _textureFormats TextureFormats = {
	.Red = GL_RED,
	.RG = GL_RG,
	.RGB = GL_RGB,
	.BGR = GL_BGR,
	.RGBA = GL_RGBA,
	.BGRA = GL_BGRA,
	.RedInteger = GL_RED_INTEGER,
	.RGInteger = GL_RG_INTEGER,
	.RGBInteger = GL_RGB_INTEGER,
	.BGRInteger = GL_BGR_INTEGER,
	.RGBAInteger = GL_RGBA_INTEGER,
	.BGRAInteger = GL_BGRA_INTEGER,
	.StencilIndex = GL_STENCIL_INDEX,
	.DepthComponent = GL_DEPTH_COMPONENT,
	.DepthStencil = GL_DEPTH_STENCIL,
	.Depth24Stencil8 = GL_DEPTH24_STENCIL8
};

const struct _bufferFormat BufferFormats =
{
	.UByte = GL_UNSIGNED_BYTE ,
	.Byte = GL_BYTE ,
	.UShort = GL_UNSIGNED_SHORT,
	.Short = GL_SHORT ,
	.UInt = GL_UNSIGNED_INT,
	.Int = GL_INT ,
	.Float = GL_FLOAT ,
	.UByte332 = GL_UNSIGNED_BYTE_3_3_2 ,
	.UByte233Rev = GL_UNSIGNED_BYTE_2_3_3_REV ,
	.UShort565 = GL_UNSIGNED_SHORT_5_6_5 ,
	.UShort565Rev = GL_UNSIGNED_SHORT_5_6_5_REV ,
	.UShort4444 = GL_UNSIGNED_SHORT_4_4_4_4,
	.UShort4444Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV ,
	.UShort5551 = GL_UNSIGNED_SHORT_5_5_5_1 ,
	.UShort1555Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV ,
	.UInt8888 = GL_UNSIGNED_INT_8_8_8_8 ,
	.UInt8888Rev = GL_UNSIGNED_INT_8_8_8_8_REV ,
	.UInt1010102 = GL_UNSIGNED_INT_10_10_10_2 ,
	.UInt2101010Rev = GL_UNSIGNED_INT_2_10_10_10_REV,
	.UInt248 = GL_UNSIGNED_INT_24_8
};

const struct _textureSettings TextureSettings = {
	.DepthStencilMode = GL_DEPTH_STENCIL_TEXTURE_MODE,
	.BaseLevel = GL_TEXTURE_BASE_LEVEL,
	.CompareFunction = GL_TEXTURE_COMPARE_FUNC,
	.CompareMode = GL_TEXTURE_COMPARE_MODE,
	.LevelOfDetailBias = GL_TEXTURE_LOD_BIAS,
	.MinimumLevelOfDetail = GL_TEXTURE_MIN_LOD,
	.MaximumLevelOfDetail = GL_TEXTURE_MAX_LOD,
	.MinifyingFilter = GL_TEXTURE_MIN_FILTER,
	.MagnifyingFilter = GL_TEXTURE_MAG_FILTER,
	.SwizzleR = GL_TEXTURE_SWIZZLE_R,
	.SwizzleG = GL_TEXTURE_SWIZZLE_G,
	.SwizzleB = GL_TEXTURE_SWIZZLE_B,
	.SwizzleA = GL_TEXTURE_SWIZZLE_A,
	.SwizzleRGBA = GL_TEXTURE_SWIZZLE_RGBA,
	.WrapX = GL_TEXTURE_WRAP_S,
	.WrapY = GL_TEXTURE_WRAP_T,
	.WrapZ = GL_TEXTURE_WRAP_R,
	.BorderColor = GL_TEXTURE_BORDER_COLOR
};

struct _textureFilters FilterTypes = {
	.Nearest = {.Name = "nearest", .Value = GL_NEAREST},
	.Linear = {.Name = "linear", .Value = GL_LINEAR},
	.NearestMipMapNearest = {.Name = "nearestNearest", .Value = GL_NEAREST_MIPMAP_NEAREST},
	.LinearMipMapNearest = {.Name = "linearNearest", .Value = GL_LINEAR_MIPMAP_NEAREST},
	.NearestMipMapLinear = {.Name = "nearestLinear", .Value = GL_NEAREST_MIPMAP_LINEAR},
	.LinearMipMapLinear = {.Name = "linearLinear", .Value = GL_LINEAR_MIPMAP_LINEAR},
};

struct _textureWrapModes WrapModes = {
	.ClampToEdge = {.Name = "clamp", .Value = GL_CLAMP_TO_EDGE },
	.ClampToBorder = {.Name = "clampToBorder", .Value = GL_CLAMP_TO_BORDER },
	.MirroredRepeat = {.Name = "mirroredRepeat", .Value = GL_MIRRORED_REPEAT},
	.Repeat = {.Name = "repeat", .Value = GL_REPEAT},
	.MirrorClampToEdge = {.Name = "mirrored", .Value = GL_MIRROR_CLAMP_TO_EDGE},
};