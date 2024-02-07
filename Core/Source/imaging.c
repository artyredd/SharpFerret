#include "core/imaging/imaging.h"
#include "core/csharp.h"
#include "core/memory.h"
#include "core/strings.h"

#ifndef STBI_INCLUDE_STB_IMAGE_H
#include "stb_image.h"
#endif

private void Dispose(Image image);
private bool TryLoadImage(const ARRAY(char) path, Image* out_image);
private Image LoadImage(const ARRAY(char) path);
private bool TryGetImageInfo(const ARRAY(char) path, Image* out_info);
private Image CreateImage(void);
private Image LoadImageFromCString(const char* path);

const struct _imageMethods Images = {
	.CreateImage = &CreateImage,
	.TryLoadImage = &TryLoadImage,
	.TryGetImageInfo = &TryGetImageInfo,
	.LoadImage = &LoadImage,
	.LoadImageFromCString = LoadImageFromCString,
	.Dispose = &Dispose
};

private bool TryLoadImage(const ARRAY(char) path, Image* out_image)
{
	*out_image = null;

	if (path is null)
	{
		return false;
	}

	*out_image = LoadImage(path);

	return *out_image isnt null;
}

// An array of innstanced pointers to image
// Only dispose of them if they have no references
ARRAY(Image) GLOBAL_ImageCache = null;

private Image LoadImageFromCString(const char* path)
{
	ARRAY(char) tmp = ARRAYS(char).CreateFromCArray(path, strlen(path));

	Image image = LoadImage(tmp);

	ARRAYS(char).Dispose(tmp);

	return image;
}

private Image LoadImage(const ARRAY(char) path)
{
	if (GLOBAL_ImageCache is null)
	{
		GLOBAL_ImageCache = ARRAYS(Image).Create(0);
	}

	for (size_t i = 0; i < GLOBAL_ImageCache->Count; i++)
	{
		Image image = *ARRAYS(Image).At(GLOBAL_ImageCache, i);

		if (ARRAYS(char).Equals(path, image->Path))
		{
			return image;
		}
	}

	Image image = CreateImage();

	image->Path = ARRAYS(char).Clone(path);

	image->Pixels = stbi_load(path->Values, &image->Width, &image->Height, &image->Channels, 0);

	if (image->Pixels is null)
	{
		Dispose(image);

		return null;
	}

	ARRAYS(Image).Append(GLOBAL_ImageCache, image);

	return image;
}

private bool TryGetImageInfo(const ARRAY(char) path, Image* out_info)
{
	*out_info = null;

	if (path is null || path->Values is null)
	{
		return false;
	}

	Image image = Images.CreateImage();

	int width = 0;
	int height = 0;
	int channels = 0;

	bool result = stbi_load(path->Values, &width, &height, &channels, 0);

	image->Width = (size_t)width;
	image->Height = (size_t)height;
	image->Channels = (size_t)channels;

	*out_info = image;

	return result;
}

DEFINE_TYPE_ID(Image);

private void Dispose(Image image)
{
	if (image is null)
	{
		return;
	}

	for (size_t i = 0; i < GLOBAL_ImageCache->Count; i++)
	{
		Image cachedImage = *ARRAYS(Image).At(GLOBAL_ImageCache, i);

		if (cachedImage == image)
		{
			return;
		}
	}

	ARRAYS(char).Dispose(image->Path);

	// these pixels are provided by stbi and are freed using free() instead to keep acccurate Memory.Alloc stats
	free(image->Pixels);
	Memory.Free(image, ImageTypeId);
}

private Image CreateImage()
{
	Memory.RegisterTypeName(nameof(Image), &ImageTypeId);

	return Memory.Alloc(sizeof(struct _image), ImageTypeId);
}