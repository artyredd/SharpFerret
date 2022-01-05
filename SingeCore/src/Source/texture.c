#include "graphics/texture.h"
#include "GL/glew.h"

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
	.DepthStencil = GL_DEPTH_STENCIL
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
	.UInt2101010Rev = GL_UNSIGNED_INT_2_10_10_10_REV
};

const struct _textureFilters Filters = {
	.Nearest = GL_NEAREST,
	.Linear = GL_LINEAR,
	.NearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
	.LinearMipMapNearest = GL_LINEAR_MIPMAP_NEAREST,
	.NearestMipMapLinear = GL_NEAREST_MIPMAP_LINEAR,
	.LinearMipMapLinear = GL_LINEAR_MIPMAP_LINEAR,
};

const struct _textureWrapModes WrapModes = {
	.ClampToEdge = GL_CLAMP_TO_EDGE,
	.ClampToBorder = GL_CLAMP_TO_BORDER,
	.MirroredRepeat = GL_MIRRORED_REPEAT,
	.Repeat = GL_REPEAT,
	.MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
};