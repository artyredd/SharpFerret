#include "graphics/texture.h"
#include "GL/glew.h"
#include "singine/memory.h"
#include "singine/guards.h"
#include "helpers/macros.h"

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
	.WrapZ = GL_TEXTURE_WRAP_R
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

static void Dispose(Texture);
static bool TryCreateTexture(Image, Texture out_texture);
static bool TryCreateTextureAdvanced(Image image, Texture out_texture, TextureFormat format, BufferFormat bufferFormat, bool (*TryModifyTexture)(unsigned int handle));
static void Modify(TextureSetting setting, TextureSettingValue value);
Texture InstanceTexture(Texture texture);

const struct _textureMethods Textures = {
	.Dispose = &Dispose,
	.TryCreateTexture = &TryCreateTexture,
	.TryCreateTextureAdvanced = &TryCreateTextureAdvanced,
	.InstanceTexture = &InstanceTexture,
	.Modify = &Modify
};

static void OnTextureBufferDispose(unsigned int handle)
{
	glDeleteTextures(1, &handle);
}

static void Dispose(Texture texture)
{
	SharedBuffers.Dispose(texture->Buffer, &OnTextureBufferDispose);

	SafeFree(texture);
}

static Texture CreateTexture(bool allocBuffer)
{
	Texture texture = SafeAlloc(sizeof(struct _texture));


	if (allocBuffer)
	{
		texture->Buffer = CreateSharedBuffer();
	}

	texture->Height = 0;
	texture->Width = 0;

	texture->BufferFormat = BufferFormats.None;
	texture->Format = TextureFormats.None;

	return texture;
}

static bool TryGetHandle(unsigned int* out_handle)
{
	GLuint handle;
	glGenTextures(1, &handle);

	glBindTexture(GL_TEXTURE_2D, handle);

	*out_handle = handle;

	return true;
}

static void Modify(TextureSetting setting, TextureSettingValue value)
{
	glTexParameteri(GL_TEXTURE_2D, setting, value);
}

static bool DefaultTryModifyTexture(unsigned int handle)
{
	Modify(TextureSettings.MinifyingFilter, DEFAULT_MINIFYING_FILTER);
	Modify(TextureSettings.MagnifyingFilter, DEFAULT_MAGNIFYING_FILTER);

	return true;
}

static bool TryCreateTextureAdvanced(Image image, Texture out_texture, TextureFormat format, BufferFormat bufferFormat, bool (*TryModifyTexture)(unsigned int handle))
{
	BoolGuardNotNull(image);

	unsigned int handle;
	if (TryGetHandle(&handle) is false)
	{
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, image->Width, image->Height, 0, format, bufferFormat, image->Pixels);

	if (TryModifyTexture isnt null)
	{
		TryModifyTexture(handle);
	}

	Texture texture = CreateTexture(true);

	texture->Height = image->Height;
	texture->Width = image->Width;

	texture->Buffer->Handle = handle;

	texture->BufferFormat = bufferFormat;
	texture->Format = format;

	out_texture = texture;

	return true;
}

static bool TryCreateTexture(Image image, Texture out_texture)
{
	return TryCreateTextureAdvanced(image, out_texture, DEFAULT_TEXTURE_FORMAT, DEFAULT_TEXTURE_BUFFER_FORMAT, &DefaultTryModifyTexture);
}

static Texture InstanceTexture(Texture texture)
{
	Texture newTexture = CreateTexture(false);

	CopyMember(texture, newTexture, Height);
	CopyMember(texture, newTexture, Width);

	CopyMember(texture, newTexture, Buffer);

	// increment the number of instances that are active for the texture so when dispose is called on the texture the underlying
	// buffer handle does not get disposed until the last texture that references it is diposed
	++(texture->Buffer->ActiveInstances);

	CopyMember(texture, newTexture, BufferFormat);
	CopyMember(texture, newTexture, Format);

	return newTexture;
}