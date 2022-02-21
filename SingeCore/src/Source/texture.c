#include "graphics/texture.h"
#include "singine/memory.h"
#include "singine/guards.h"
#include "helpers/macros.h"
#include "singine/strings.h"
#include "graphics/graphicsDevice.h"
#include "singine/config.h"
#include "singine/parsing.h"
#include <string.h>
#include "singine/reflection.h"

typedef Image CubeMapImages[6];

static void Dispose(Texture);
static bool TryCreateTexture(Image, Texture* out_texture);
static bool TryCreateTextureAdvanced(Image image, Texture* out_texture, const TextureType type, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state));
static Texture InstanceTexture(Texture texture);
static Texture Blank(void);
static void Save(Texture texture, const char* path);
static Texture Load(const char* path);
static TextureFormat GetFormat(Image image);
static bool TryCreateBufferTexture(const TextureType type, const TextureFormat format, const BufferFormat bufferFormat, size_t width, size_t height, Texture* out_texture);

const struct _textureMethods Textures = {
	.Dispose = &Dispose,
	.TryCreateTexture = &TryCreateTexture,
	.TryCreateTextureAdvanced = &TryCreateTextureAdvanced,
	.Instance = &InstanceTexture,
	.Blank = Blank,
	.Load = &Load,
	.Save = &Save,
	.TryCreateBufferTexture = TryCreateBufferTexture
};

// this is only ran when there is only one remaining instance of the texture being disposed
static void OnTextureBufferDispose(Texture texture)
{
	// delete the GDI texture
	GraphicsDevice.DeleteTexture(texture->Handle->Handle);

	// free the string
	SafeFree(texture->Path);
}

static void Dispose(Texture texture)
{
	if (texture is null)
	{
		return;
	}

	// most of the logic is handled by the callback for the handle disposal
	// the handle disposal is ONLY ran when a single instance remains of the provided texture
	SharedHandles.Dispose(texture->Handle, texture, &OnTextureBufferDispose);

	SafeFree(texture);
}

static Texture CreateTexture(bool allocBuffer)
{
	Texture texture = SafeAlloc(sizeof(struct _texture));

	if (allocBuffer)
	{
		texture->Handle = SharedHandles.Create();
	}

	texture->BufferFormat = BufferFormats.None;
	texture->Format = TextureFormats.None;

	return texture;
}

static bool TryGetHandle(unsigned int* out_handle)
{
	*out_handle = GraphicsDevice.CreateTexture(TextureTypes.Default);

	return true;
}

static bool DefaultTryModifyTexture(void* state)
{
	if (state is null)
	{
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MinifyingFilter, DEFAULT_MINIFYING_FILTER);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MagnifyingFilter, DEFAULT_MAGNIFYING_FILTER);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapX, DEFAULT_WRAPX);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapY, DEFAULT_WRAPY);
		GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapZ, DEFAULT_WRAPZ);
	}

	return true;
}

static bool TryCreateCubeMapAdvanced(CubeMapImages images, Texture* out_texture,
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

	Texture texture = CreateTexture(true);

	texture->Height = images[0]->Height;
	texture->Width = images[0]->Width;

	texture->Handle->Handle = handle;

	texture->BufferFormat = bufferFormat;
	texture->Format = 0;

	texture->Type = TextureTypes.CubeMap;

	* out_texture = texture;

	return true;
}

static bool TryCreateTextureAdvanced(Image image, Texture* out_texture, const TextureType type, TextureFormat format,
	BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state))
{
	BoolGuardNotNull(image);

	unsigned int handle = GraphicsDevice.CreateTexture(type);

	GraphicsDevice.LoadTexture(type, format, bufferFormat, image, 0);

	if (TryModifyTexture isnt null)
	{
		TryModifyTexture(state);
	}

	Texture texture = CreateTexture(true);

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

	return true;
}

static bool TryCreateBufferTexture(const TextureType type, const TextureFormat format, const BufferFormat bufferFormat, size_t width, size_t height, Texture* out_texture)
{
	unsigned int handle = GraphicsDevice.CreateTexture(type);

	GraphicsDevice.LoadBufferTexture(type, format, bufferFormat, width, height, 0);

	DefaultTryModifyTexture(null);

	Texture texture = CreateTexture(true);

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

	return true;
}

static TextureFormat GetFormat(Image image)
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

static bool TryCreateTexture(Image image, Texture* out_texture)
{
	// check the channels available, if there are 4 the format is RGBA, if 3 RGB
	TextureFormat format = GetFormat(image);

	return TryCreateTextureAdvanced(image, out_texture, TextureTypes.Default, format, DEFAULT_TEXTURE_BUFFER_FORMAT, null, &DefaultTryModifyTexture);
}

static Texture InstanceTexture(Texture texture)
{
	if (texture is null)
	{
		return null;
	}

	Texture newTexture = CreateTexture(false);

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

static Texture Blank(void)
{
	Image blankImage = &BlankImage;

	Texture texture;

	if (TryCreateTexture(blankImage, &texture))
	{
		return texture;
	}

	return null;
}

#define MAX_PATH_SIZE 512

#define CommentFormat "%s\n"
#define TokenFormat "%s: "

#define PathTokenComment "# the path to the texture"
#define PathToken "path"

#define MinTokenComment "# the minification filter to use for the texture"NEWLINE"# nearest, linear, nearestNearest, linearNearest, nearestLinear, linearLinear"
#define MinToken "minificationFilter"

#define MagTokenComment "# the maginification filter to use for the texture"NEWLINE"# nearest, linear, nearestNearest, linearNearest, nearestLinear, linearLinear"
#define MagToken "magnificationFilter"

#define WrapXTokenComment "# How to wrap the texture along the x (S) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"
#define WrapXToken "wrapX"

#define WrapYTokenComment "# How to wrap the texture along the y (T) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"
#define WrapYToken "wrapY"

#define WrapZTokenComment "# How to wrap the texture along the z (R) axis"NEWLINE"# clamped, repeat, mirrored, clampToBorder, mirroredClamped"
#define WrapZToken "wrapZ"

#define TypeTokenComment "# the type of texture this represents"NEWLINE"# 2d, cubemap"
#define TypeToken "type"

#define LeftTokenComment "# the path for the left face of the cube map"
#define RightTokenComment "# the path for the right face of the cube map"
#define UpTokenComment "# the path for the up face of the cube map"
#define DownTokenComment "# the path for the down face of the cube map"
#define ForwardTokenComment "# the path for the forward face of the cube map"
#define BackTokenComment "# the path for the back face of the cube map"

#define LeftToken "left"
#define RightToken "right"
#define UpToken "up"
#define DownToken "down"
#define ForwardToken "forward"
#define BackToken "back"

static const char* Tokens[] = {
	PathToken,
	MinToken,
	MagToken,
	WrapXToken,
	WrapYToken,
	WrapZToken,
	TypeToken,
	LeftToken,
	RightToken,
	UpToken,
	DownToken,
	ForwardToken,
	BackToken
};

static const size_t TokenLengths[] = {
	sizeof(PathToken),
	sizeof(MinToken),
	sizeof(MagToken),
	sizeof(WrapXToken),
	sizeof(WrapYToken),
	sizeof(WrapZToken),
	sizeof(TypeToken),
	sizeof(LeftToken),
	sizeof(RightToken),
	sizeof(UpToken),
	sizeof(DownToken),
	sizeof(ForwardToken),
	sizeof(BackToken)
};

struct _textureInfo {
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

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _textureInfo* state);

struct _configDefinition TextureConfigDefinition = {
	.Tokens = (const char**)&Tokens,
	.TokenLengths = (const size_t*)&TokenLengths,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(char*),
	.OnTokenFound = &OnTokenFound
};

static bool ModifyLoadedTexture(struct _textureInfo* state)
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

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _textureInfo* state)
{
	switch (index)
	{
	case 0: // path
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->path);
	case 1: // min filter
		return TryGetFilterType(buffer, length, &state->MinificationFilter);
	case 2: // mag filter
		return TryGetFilterType(buffer, length, &state->MagnificationFilter);
	case 3: // wrap x
		return TryGetWrapModeType(buffer, length, &state->WrapX);
	case 4: // wrap y
		return TryGetWrapModeType(buffer, length, &state->WrapY);
	case 5: // wrap z
		return TryGetWrapModeType(buffer, length, &state->WrapZ);
	case 6: // type
		return TryGetTextureType(buffer, length, &state->Type);
	case 7: // left path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->left);
	case 8: // right path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->right);
	case 9: // up path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->up);
	case 10: // down path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->down);
	case 11: // forward path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->forward);
	case 12: // back path of cubemap
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->back);
	default:
		return false;
	}
}

static void Load2dTexture(struct _textureInfo state, Texture* out_texture)
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

static void VerifyCubeMapImages(CubeMapImages images)
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

static void LoadCubemap(struct _textureInfo state, Texture* out_texture)
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

static Texture Load(const char* path)
{
	Texture texture = null;

	// ignore const violation for type default
#pragma warning(disable:4090)
	struct _textureInfo state = {
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

	SafeFree(state.path);
	SafeFree(state.left);
	SafeFree(state.right);
	SafeFree(state.forward);
	SafeFree(state.back);
	SafeFree(state.up);
	SafeFree(state.down);

	return texture;
}

static void Save(Texture texture, const char* path)
{
	GuardNotNull(texture);
	GuardNotNull(path);

	File stream;
	if (Files.TryOpen(path, FileModes.Create, &stream) is false)
	{
		throw(FailedToOpenFileException);
	}

	fprintf(stream, CommentFormat, PathTokenComment);
	fprintf(stream, TokenFormat, PathToken);
	fprintf(stream, "%s\n", texture->Path);

	fprintf(stream, CommentFormat, MinTokenComment);
	fprintf(stream, TokenFormat, MinToken);
	fprintf(stream, "%s\n", texture->MinificationFilter.Name);

	fprintf(stream, CommentFormat, MagTokenComment);
	fprintf(stream, TokenFormat, MagToken);
	fprintf(stream, "%s\n", texture->MagnificationFilter.Name);

	fprintf(stream, CommentFormat, WrapXTokenComment);
	fprintf(stream, TokenFormat, WrapXToken);
	fprintf(stream, "%s\n", texture->WrapX.Name);

	fprintf(stream, CommentFormat, WrapYTokenComment);
	fprintf(stream, TokenFormat, WrapYToken);
	fprintf(stream, "%s\n", texture->WrapY.Name);

	fprintf(stream, CommentFormat, WrapZTokenComment);
	fprintf(stream, TokenFormat, WrapZToken);
	fprintf(stream, "%s\n", texture->WrapZ.Name);

	if (Files.TryClose(stream) is false)
	{
		throw(FailedToCloseFileException);
	}
}