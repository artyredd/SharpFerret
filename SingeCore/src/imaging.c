#include "Graphics/imaging.h"
#include "csharp.h"
#include "memory.h"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#include "stb_image.h"
#endif

static void Dispose(Image image);

bool TryLoadImage(const char* path, Image* out_image)
{
	*out_image = null;

	if (path is null)
	{
		return false;
	}

	Image image = CreateImage();

	int width = 0;
	int height = 0;
	int channels = 0;

	image->Pixels = stbi_load(path, &width, &height, &channels, 0);

	if (image->Pixels is null)
	{
		return false;
	}

	image->Width = (size_t)width;
	image->Height = (size_t)height;
	image->Channels = (size_t)channels;

	return true;
}

bool TryGetImageInfo(const char* path, Image* out_info)
{
	*out_info = null;

	if (path is null)
	{
		return false;
	}

	Image image = CreateImage();

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
	SafeFree(image->Pixels);
	SafeFree(image);
}

Image CreateImage()
{
	Image img = SafeAlloc(sizeof(struct _image));

	img->Dispose = &Dispose;

	img->Pixels = null;

	return img;
}