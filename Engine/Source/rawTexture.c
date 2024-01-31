#include "engine/graphics/rawTexture.h"
#include "core/memory.h"
#include "core/guards.h"
#include "core/macros.h"
#include "core/strings.h"
#include "engine/graphics/graphicsDevice.h"
#include "core/config.h"
#include "core/parsing.h"
#include <string.h>
#include "core/reflection.h"
#include <engine/graphics/colors.h>

typedef Image CubeMapImages[6];

private void Dispose(RawTexture);
private bool TryCreateTexture(Image, RawTexture* out_texture);
private bool TryCreateTextureAdvanced(Image image, RawTexture* out_texture, const TextureType type, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state));
private RawTexture InstanceTexture(RawTexture texture);
private RawTexture Blank(void);
private void Save(RawTexture texture, const char* path);
private RawTexture Load(const char* path);
private TextureFormat GetFormat(Image image);
private bool TryCreateBufferTexture(const TextureType type, const TextureFormat format, const BufferFormat bufferFormat, size_t width, size_t height, RawTexture* out_texture);

const struct _rawTextureMethods RawTextures = {
	.Dispose = &Dispose,
	.TryCreateTexture = &TryCreateTexture,
	.TryCreateTextureAdvanced = &TryCreateTextureAdvanced,
	.Instance = &InstanceTexture,
	.Blank = Blank,
	.Load = &Load,
	.Save = &Save,
	.TryCreateBufferTexture = TryCreateBufferTexture
};

DEFINE_TYPE_ID(RawTexture);

// this is only ran when there is only one remaining instance of the texture being disposed
private void OnTextureBufferDispose(RawTexture texture)
{
	// delete the GDI texture
	GraphicsDevice.DeleteTexture(texture->Handle->Handle);

	// free the string
	Memory.Free(texture->Path, Memory.String);
}

private void Dispose(RawTexture texture)
{
	if (texture is null)
	{
		return;
	}

	// most of the logic is handled by the callback for the handle disposal
	// the handle disposal is ONLY ran when a single instance remains of the provided texture
	SharedHandles.Dispose(texture->Handle, texture, &OnTextureBufferDispose);

	Memory.Free(texture, typeid(RawTexture));
}

private RawTexture CreateTexture(bool allocBuffer)
{
	REGISTER_TYPE(RawTexture);

	RawTexture texture = Memory.Alloc(sizeof(struct _rawTexture), typeid(RawTexture));

	if (allocBuffer)
	{
		texture->Handle = SharedHandles.Create();
	}

	texture->BufferFormat = BufferFormats.None;
	texture->Format = TextureFormats.None;

	return texture;
}

private bool TryGetHandle(unsigned int* out_handle)
{
	*out_handle = GraphicsDevice.CreateTexture(TextureTypes.Default);

	return true;
}

private bool DefaultTryModifyTexture(void* state)
{
	if (state is null)
	{
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MinifyingFilter, DEFAULT_MINIFYING_FILTER);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MagnifyingFilter, DEFAULT_MAGNIFYING_FILTER);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapX, DEFAULT_WRAPX);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapY, DEFAULT_WRAPY);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapZ, DEFAULT_WRAPZ);
		GraphicsDevice.ModifyTextureProperty(TextureTypes.Default, TextureSettings.BorderColor, Colors.White);
	}

	return true;
}

private bool TryCreateCubeMapAdvanced(CubeMapImages images, RawTexture* out_texture,
	BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state))
{
	BoolGuardNotNull(images);

	unsigned int handle = GraphicsDevice.CreateTexture(TextureTypes.CubeMap);

	for (size_t i = 0; i < 6; i++)
	{
		Image image = images[i];

		TextureFormat format = GetFormat(image);

		GraphicsDevice.LoadTexture(TextureTypes.CubeMapFace, format, bufferFormat, images[i], (unsigned int)i);
	}

	if (TryModifyTexture isnt null)
	{
		TryModifyTexture(state);
	}

	RawTexture texture = CreateTexture(true);

	texture->Height = images[0]->Height;
	texture->Width = images[0]->Width;

	texture->Handle->Handle = handle;

	texture->BufferFormat = bufferFormat;
	texture->Format = 0;

	texture->Type = TextureTypes.CubeMap;

	*out_texture = texture;

	GraphicsDevice.ClearTexture(TextureTypes.Default);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMap);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMapFace);

	return true;
}

private bool TryCreateTextureAdvanced(Image image, RawTexture* out_texture, const TextureType type, TextureFormat format,
	BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state))
{
	BoolGuardNotNull(image);

	unsigned int handle = GraphicsDevice.CreateTexture(type);

	GraphicsDevice.LoadTexture(type, format, bufferFormat, image, 0);

	if (TryModifyTexture isnt null)
	{
		TryModifyTexture(state);
	}

	RawTexture texture = CreateTexture(true);

	texture->Height = image->Height;
	texture->Width = image->Width;

	texture->Handle->Handle = handle;

	texture->BufferFormat = bufferFormat;
	texture->Format = format;

	texture->Path = Strings.DuplicateTerminated(image->Path);

	// ignore const violation, we are intentionally passing a const value here
	// this is known to be problematic
#pragma warning(disable: 4090)
	texture->Type = type;
#pragma warning(default: 4090)

	* out_texture = texture;

	// make sure we don't have a dangling texture
	GraphicsDevice.ClearTexture(TextureTypes.Default);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMap);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMapFace);

	return true;
}

private bool TryCreateBufferTexture(const TextureType type, const TextureFormat format, const BufferFormat bufferFormat, size_t width, size_t height, RawTexture* out_texture)
{
	unsigned int handle = GraphicsDevice.CreateTexture(type);

	// if it's a depth map we should use CubeMapFace 6 times instead of a single texture with the Depth type
	if (type.Value.AsUInt is TextureTypes.CubeMap.Value.AsUInt)
	{
		for (unsigned int i = 0; i < 6; i++)
		{
			GraphicsDevice.LoadBufferTexture(TextureTypes.CubeMapFace, format, bufferFormat, width, height, i);
		}
	}
	else
	{
		// default to using a 2d texture
		GraphicsDevice.LoadBufferTexture(type, format, bufferFormat, width, height, 0);
	}

	DefaultTryModifyTexture(null);

	RawTexture texture = CreateTexture(true);

	texture->Height = height;
	texture->Width = width;

	texture->Handle->Handle = handle;

	texture->BufferFormat = bufferFormat;
	texture->Format = format;

	texture->Path = null;

	// ignore const violation, we are intentionally passing a const value here
	// this is known to be problematic
#pragma warning(disable: 4090)
	texture->Type = type;
#pragma warning(default: 4090)

	* out_texture = texture;

	GraphicsDevice.ClearTexture(TextureTypes.Default);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMap);
	GraphicsDevice.ClearTexture(TextureTypes.CubeMapFace);

	return true;
}

private TextureFormat GetFormat(Image image)
{
	// check the channels available, if there are 4 the format is RGBA, if 3 RGB
	TextureFormat format = DEFAULT_TEXTURE_FORMAT;

	if (image->Channels is 3)
	{
		format = TextureFormats.RGB;
	}
	else if (image->Channels is 4)
	{
		format = TextureFormats.RGBA;
	}

	return format;
}

private bool TryCreateTexture(Image image, RawTexture* out_texture)
{
	// check the channels available, if there are 4 the format is RGBA, if 3 RGB
	TextureFormat format = GetFormat(image);

	return TryCreateTextureAdvanced(image, out_texture, TextureTypes.Default, format, DEFAULT_TEXTURE_BUFFER_FORMAT, null, &DefaultTryModifyTexture);
}

private RawTexture InstanceTexture(RawTexture texture)
{
	if (texture is null)
	{
		return null;
	}

	RawTexture newTexture = CreateTexture(false);

	// value types
	CopyMember(texture, newTexture, Height);
	CopyMember(texture, newTexture, Width);
	CopyMember(texture, newTexture, BufferFormat);
	CopyMember(texture, newTexture, Format);

	CopyMember(texture, newTexture, MinificationFilter);
	CopyMember(texture, newTexture, MagnificationFilter);
	CopyMember(texture, newTexture, WrapX);
	CopyMember(texture, newTexture, WrapY);
	CopyMember(texture, newTexture, WrapZ);
	CopyMember(texture, newTexture, Type);

	// reference types
	CopyMember(texture, newTexture, Path);

	newTexture->Handle = SharedHandles.Instance(texture->Handle);

	return newTexture;
}

struct _image BlankImage = {
	.Height = 1,
	.Width = 1,
	.Channels = 4, // RGBA, 4 components
	.Pixels = (unsigned char*)"\255\255\255\255", // a white pixel
	.Path = null,
};

private RawTexture Blank(void)
{
	Image blankImage = &BlankImage;

	RawTexture texture;

	if (TryCreateTexture(blankImage, &texture))
	{
		return texture;
	}

	return null;
}

#define MAX_PATH_SIZE 512

struct _textureState {
	char* path;
	TextureType Type;
	FilterType MinificationFilter;
	FilterType MagnificationFilter;
	WrapMode WrapX;
	WrapMode WrapY;
	WrapMode WrapZ;
	// cube map
	char* left;
	char* right;
	char* up;
	char* down;
	char* forward;
	char* back;
};

TOKEN_LOAD(path, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->path);
}

TOKEN_SAVE(path, RawTexture)
{
	fprintf(stream, "%s", state->Path);
}

TOKEN_LOAD(minificationFilter, struct _textureState*)
{
	return TryGetFilterType(buffer, length, &state->MinificationFilter);
}

TOKEN_SAVE(minificationFilter, RawTexture)
{
	fprintf(stream, "%s", state->MinificationFilter.Name);
}

TOKEN_LOAD(magnificationFilter, struct _textureState*)
{
	return TryGetFilterType(buffer, length, &state->MagnificationFilter);
}

TOKEN_SAVE(magnificationFilter, RawTexture)
{
	fprintf(stream, "%s", state->MagnificationFilter.Name);
}

TOKEN_LOAD(wrapX, struct _textureState*)
{
	return TryGetWrapModeType(buffer, length, &state->WrapX);
}

TOKEN_SAVE(wrapX, RawTexture)
{
	fprintf(stream, "%s", state->WrapX.Name);
}

TOKEN_LOAD(wrapY, struct _textureState*)
{
	return TryGetWrapModeType(buffer, length, &state->WrapY);
}

TOKEN_SAVE(wrapY, RawTexture)
{
	fprintf(stream, "%s", state->WrapY.Name);
}

TOKEN_LOAD(wrapZ, struct _textureState*)
{
	return TryGetWrapModeType(buffer, length, &state->WrapZ);
}

TOKEN_SAVE(wrapZ, RawTexture)
{
	fprintf(stream, "%s", state->WrapZ.Name);
}

TOKEN_LOAD(type, struct _textureState*)
{
	return TryGetTextureType(buffer, length, &state->Type);
}

TOKEN_SAVE(type, RawTexture)
{
	fprintf(stream, "%s", state->Type.Name);
}

TOKEN_LOAD(left, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->left);
}

TOKEN_SAVE(left, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}

TOKEN_LOAD(right, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->right);
}

TOKEN_SAVE(right, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}

TOKEN_LOAD(up, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->up);
}

TOKEN_SAVE(up, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}

TOKEN_LOAD(down, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->down);
}

TOKEN_SAVE(down, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}

TOKEN_LOAD(forward, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->forward);
}

TOKEN_SAVE(forward, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}

TOKEN_LOAD(back, struct _textureState*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_SIZE, &state->back);
}

TOKEN_SAVE(back, RawTexture)
{
	ignore_unused(stream);
	ignore_unused(state);
}


TOKENS(13) {
	TOKEN(path, "# the path to the texture"),
		TOKEN(minificationFilter, "# the minification filter to use for the texture"NEWLINE"# nearest, linear, nearestNearest, linearNearest, nearestLinear, linearLinear"),
		TOKEN(magnificationFilter, "# the maginification filter to use for the texture"NEWLINE"# nearest, linear, nearestNearest, linearNearest, nearestLinear, linearLinear"),
		TOKEN(wrapX, "# How to wrap the texture along the x (S) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"),
		TOKEN(wrapY, "# How to wrap the texture along the y (T) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"),
		TOKEN(wrapZ, "# How to wrap the texture along the z (R) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"),
		TOKEN(type, "# the type of texture this represents"NEWLINE"# 2d, cubemap"),
		TOKEN(left, "# the path for the left face of the cube map"),
		TOKEN(right, "# the path for the right face of the cube map"),
		TOKEN(up, "# the path for the up face of the cube map"),
		TOKEN(down, "# the path for the down face of the cube map"),
		TOKEN(forward, "# the path for the forward face of the cube map"),
		TOKEN(back, "# the path for the back face of the cube map")
};

struct _configDefinition TextureConfigDefinition = {
	.Tokens = Tokens,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(struct _configToken)
};

private bool ModifyLoadedTexture(struct _textureState* state)
{
	if (state is null)
	{
		return false;
	}

	GraphicsDevice.ModifyTexture(state->Type, TextureSettings.MinifyingFilter, state->MinificationFilter);
	GraphicsDevice.ModifyTexture(state->Type, TextureSettings.MagnifyingFilter, state->MagnificationFilter);
	GraphicsDevice.ModifyTexture(state->Type, TextureSettings.WrapX, state->WrapX);
	GraphicsDevice.ModifyTexture(state->Type, TextureSettings.WrapY, state->WrapY);
	GraphicsDevice.ModifyTexture(state->Type, TextureSettings.WrapZ, state->WrapZ);

	return true;
}

private void Load2dTexture(struct _textureState state, RawTexture* out_texture)
{
	Image image = Images.LoadImage(state.path);

	if (image is null)
	{
		throw(FileNotFoundException);
	}

	TextureFormat format = GetFormat(image);

	if (TryCreateTextureAdvanced(image, out_texture, TextureTypes.Default, format, DEFAULT_TEXTURE_BUFFER_FORMAT, &state, &ModifyLoadedTexture) is false)
	{
		throw(FailedToLoadTextureException);
	}

	Images.Dispose(image);
}

private void VerifyCubeMapImages(CubeMapImages images)
{
	for (size_t i = 0; i < 6; i++)
	{
		Image image = images[i];

		if (image is null)
		{
			fprintf(stderr, "Failed to load image within cube map at index = %lli", i);
			throw(FailedToLoadTextureException);
		}
	}
}

private void LoadCubemap(struct _textureState state, RawTexture* out_texture)
{
	CubeMapImages images = {
		Images.LoadImage(state.left),
		Images.LoadImage(state.right),
		Images.LoadImage(state.up),
		Images.LoadImage(state.down),
		Images.LoadImage(state.back),
		Images.LoadImage(state.forward)
	};

	VerifyCubeMapImages(images);

	if (TryCreateCubeMapAdvanced(images, out_texture, DEFAULT_TEXTURE_BUFFER_FORMAT, &state, &ModifyLoadedTexture) is false)
	{
		for (size_t i = 0; i < 6; i++)
		{
			Images.Dispose(images[i]);
		}
		throw(FailedToLoadTextureException);
	}

	for (size_t i = 0; i < 6; i++)
	{
		Images.Dispose(images[i]);
	}
}

private RawTexture Load(const char* path)
{
	RawTexture texture = null;

	// ignore const violation for type default
#pragma warning(disable:4090)
	struct _textureState state = {
		.path = null,
		.Type = DEFAULT_TEXTURE_TYPE,
		.MinificationFilter = DEFAULT_MINIFYING_FILTER,
		.MagnificationFilter = DEFAULT_MAGNIFYING_FILTER,
		.WrapX = DEFAULT_WRAPX,
		.WrapY = DEFAULT_WRAPY,
		.WrapZ = DEFAULT_WRAPZ,
		.left = null,
		.right = null,
		.up = null,
		.down = null,
		.forward = null,
		.back = null
	};
#pragma warning(default:4090)

	if (Configs.TryLoadConfig(path, &TextureConfigDefinition, &state))
	{
		if (state.Type.Value.AsUInt is TextureTypes.CubeMap.Value.AsUInt)
		{
			LoadCubemap(state, &texture);

			// since a cubemap has multiple source textures it's path is not set when we create the texture
			// asign the path name to the texture instead of any one source
			texture->Path = Strings.DuplicateTerminated(path);
		}
		else
		{
			Load2dTexture(state, &texture);
		}

		texture->MagnificationFilter = state.MagnificationFilter;
		texture->MinificationFilter = state.MinificationFilter;
		texture->WrapX = state.WrapX;
		texture->WrapY = state.WrapY;
		texture->WrapZ = state.WrapZ;
		texture->Type = state.Type;
	}

	Memory.Free(state.path, Memory.String);
	Memory.Free(state.left, Memory.String);
	Memory.Free(state.right, Memory.String);
	Memory.Free(state.forward, Memory.String);
	Memory.Free(state.back, Memory.String);
	Memory.Free(state.up, Memory.String);
	Memory.Free(state.down, Memory.String);

	return texture;
}

private void Save(RawTexture texture, const char* path)
{
	GuardNotNull(texture);
	GuardNotNull(path);

	Configs.SaveConfig(path, &TextureConfigDefinition, texture);
}