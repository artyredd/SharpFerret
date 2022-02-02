#include "graphics/texture.h"
#include "singine/memory.h"
#include "singine/guards.h"
#include "helpers/macros.h"
#include "singine/strings.h"
#include "graphics/graphicsDevice.h"
#include "singine/config.h"
#include "singine/parsing.h"
#include <string.h>

static void Dispose(Texture);
static bool TryCreateTexture(Image, Texture* out_texture);
static bool TryCreateTextureAdvanced(Image image, Texture* out_texture, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state));
static void Modify(TextureSetting setting, TextureSettingValue value);
static Texture InstanceTexture(Texture texture);
static Texture Blank(void);
static void Save(Texture texture, const char* path);
static Texture Load(const char* path);

const struct _textureMethods Textures = {
	.Dispose = &Dispose,
	.TryCreateTexture = &TryCreateTexture,
	.TryCreateTextureAdvanced = &TryCreateTextureAdvanced,
	.Instance = &InstanceTexture,
	.Modify = &Modify,
	.Blank = Blank,
	.Load = &Load,
	.Save = &Save
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

static void Modify(TextureSetting setting, TextureSettingValue value)
{
	GraphicsDevice.ModifyTexture(TextureTypes.Default, setting, value);
}

static bool DefaultTryModifyTexture(void* state)
{
	if (state is null)
	{
		Modify(TextureSettings.MinifyingFilter, DEFAULT_MINIFYING_FILTER);
		Modify(TextureSettings.MagnifyingFilter, DEFAULT_MAGNIFYING_FILTER);
	}

	return true;
}

static bool TryCreateTextureAdvanced(Image image, Texture* out_texture, TextureFormat format, BufferFormat bufferFormat, void* state, bool (*TryModifyTexture)(void* state))
{
	BoolGuardNotNull(image);

	unsigned int handle = GraphicsDevice.CreateTexture(TextureTypes.Default);

	GraphicsDevice.LoadTexture(TextureTypes.Default, format, bufferFormat, image);

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

	*out_texture = texture;

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

	return TryCreateTextureAdvanced(image, out_texture, format, DEFAULT_TEXTURE_BUFFER_FORMAT, null, &DefaultTryModifyTexture);
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

TextureSettingValue ClampToEdge;
TextureSettingValue ClampToBorder;
TextureSettingValue MirroredRepeat;
TextureSettingValue Repeat;
TextureSettingValue MirrorClampToEdge;

static const char* Tokens[] = {
	PathToken,
	MinToken,
	MagToken,
	WrapXToken,
	WrapYToken,
	WrapZToken
};

static const size_t TokenLengths[] = {
	sizeof(PathToken),
	sizeof(MinToken),
	sizeof(MagToken),
	sizeof(WrapXToken),
	sizeof(WrapYToken),
	sizeof(WrapZToken),
};

struct _textureInfo {
	char* path;
	TextureSettingValue MinificationFilter;
	TextureSettingValue MagnificationFilter;
	TextureSettingValue WrapX;
	TextureSettingValue WrapY;
	TextureSettingValue WrapZ;
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

	GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MinifyingFilter, state->MinificationFilter);
	GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.MagnifyingFilter, state->MagnificationFilter);
	GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapX, state->WrapX);
	GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapY, state->WrapY);
	GraphicsDevice.ModifyTexture(TextureTypes.Default, TextureSettings.WrapZ, state->WrapZ);

	return true;
}

static bool TryGetFilter(const char* buffer, const size_t length, TextureSettingValue* out_textureSetting)
{
	static const size_t filterCount = sizeof(Filters) / sizeof(TextureSettingValue);

	static const char* filters[sizeof(Filters) / sizeof(TextureSettingValue)] =
	{
		"nearest",
		"linear",
		"nearestNearest",
		"linearNearest",
		"nearestLinear",
		"linearLinear"
	};

	*out_textureSetting = 0;

	for (size_t i = 0; i < filterCount; i++)
	{
		if (Strings.Contains(buffer, length, filters[i], strlen(filters[i])))
		{
			// this is some hacky aliasing, im sorry
			*out_textureSetting = ((TextureSettingValue*)&Filters)[i];
			return true;
		}
	}

	return false;
}

static bool TryGetWrapMode(const char* buffer, const size_t length, TextureSettingValue* out_textureSetting)
{
	static const size_t modeCount = sizeof(WrapModes) / sizeof(TextureSettingValue);

	static const char* wrapModes[sizeof(WrapModes) / sizeof(TextureSettingValue)] =
	{
		"clamped",
		"repeat",
		"mirrored",
		"clampToBorder",
		"mirroredClamped"
	};

	*out_textureSetting = 0;

	for (size_t i = 0; i < modeCount; i++)
	{
		if (Strings.Contains(buffer, length, wrapModes[i], strlen(wrapModes[i])))
		{
			// this is some hacky aliasing, im sorry
			*out_textureSetting = ((TextureSettingValue*)&WrapModes)[i];
			return true;
		}
	}

	return false;
}

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _textureInfo* state)
{
	switch (index)
	{
	case 0: // path
		return TryParseString(buffer, length, MAX_PATH_SIZE, &state->path);
	case 1: // min filter
		return TryGetFilter(buffer, length, &state->MinificationFilter);
	case 2: // mag filter
		return TryGetFilter(buffer, length, &state->MagnificationFilter);
	case 3: // wrap x
		return TryGetWrapMode(buffer, length, &state->WrapX);
	case 4: // wrap y
		return TryGetWrapMode(buffer, length, &state->WrapY);
	case 5: // wrap z
		return TryGetWrapMode(buffer, length, &state->WrapZ);
	default:
		return false;
	}
}

static Texture Load(const char* path)
{
	Texture texture = null;

	struct _textureInfo state = {
		.path = null,
		.MinificationFilter = DEFAULT_MINIFYING_FILTER,
		.MagnificationFilter = DEFAULT_MAGNIFYING_FILTER,
		.WrapX = DEFAULT_WRAPX,
		.WrapY = DEFAULT_WRAPY,
		.WrapZ = DEFAULT_WRAPZ
	};

	if (Configs.TryLoadConfig(path, &TextureConfigDefinition, &state))
	{
		Image image = Images.LoadImage(state.path);

		if (image is null)
		{
			throw(FileNotFoundException);
		}

		TextureFormat format = GetFormat(image);

		if (TryCreateTextureAdvanced(image, &texture, format, DEFAULT_TEXTURE_BUFFER_FORMAT, &state, &ModifyLoadedTexture) is false)
		{
			throw(FailedToLoadTextureException);
		}

		Images.Dispose(image);
	}

	SafeFree(state.path);

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
	fprintf(stream, "#"TokenFormat, MinToken);
	fprintf(stream, "%s\n", "linear");

	fprintf(stream, CommentFormat, MagTokenComment);
	fprintf(stream, "#"TokenFormat, MagToken);
	fprintf(stream, "%s\n", "linear");

	fprintf(stream, CommentFormat, WrapXTokenComment);
	fprintf(stream, "#"TokenFormat, WrapYToken);
	fprintf(stream, "%s\n", "clamp");

	fprintf(stream, CommentFormat, WrapYTokenComment);
	fprintf(stream, "#"TokenFormat, WrapYToken);
	fprintf(stream, "%s\n", "clamp");

	fprintf(stream, CommentFormat, WrapZTokenComment);
	fprintf(stream, "#"TokenFormat, WrapZToken);
	fprintf(stream, "%s\n", "clamp");

	if (Files.TryClose(stream) is false)
	{
		throw(FailedToCloseFileException);
	}
}