#include "graphics/imaging.h"
#include "csharp.h"
#include "singine/memory.h"
#include "singine/strings.h"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#include "stb_image.h"
#endif

static void Dispose(Image image);
static bool TryLoadImage(const char* path, Image* out_image);
static Image LoadImage(const char* path);
static bool TryGetImageInfo(const char* path, Image* out_info);
static Image CreateImage(void);

const struct _imageMethods Images = {
	.CreateImage = &CreateImage,
	.TryLoadImage = &TryLoadImage,
	.TryGetImageInfo = &TryGetImageInfo,
	.LoadImage = &LoadImage,
	.Dispose = &Dispose
};

static bool TryLoadImage(const char* path, Image* out_image)
{
	*out_image = null;

	if (path is null)
	{
		return false;
	}

	Image image = LoadImage(path);

	if (image is null)
	{
		return false;
	}

	*out_image = image;

	return true;
}

static Image LoadImage(const char* path)
{
	Image image = CreateImage();

	image->Path = Strings.DuplicateTerminated(path);

	image->Pixels = stbi_load(path, &image->Width, &image->Height, &image->Channels, 0);

	if (image->Pixels is null)
	{
		SafeFree(image);
		return null;
	}

	return image;
}

static bool TryGetImageInfo(const char* path, Image* out_info)
{
	*out_info = null;

	if (path is null)
	{
		return false;
	}

	Image image = Images.CreateImage();

	int width = 0;
	int height = 0;
	int channels = 0;

	bool result = stbi_load(path, &width, &height, &channels, 0);

	image->Width = (size_t)width;
	image->Height = (size_t)height;
	image->Channels = (size_t)channels;

	return result;
}

static void Dispose(Image image)
{
	if (image is null)
	{
		return;
	}

	SafeFree(image->Path);
	SafeFree(image->Pixels);
	SafeFree(image);
}

static Image CreateImage()
{
	return SafeAlloc(sizeof(struct _image));
}