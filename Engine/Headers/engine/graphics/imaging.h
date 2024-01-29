#pragma once

#include "core/csharp.h"

/// <summary>
/// Types of supported formats
/// </summary>
enum ImageFormat {
	none,
	JPEG,
	PNG,
	TGA,
	BMP,
	PSD,
	GIF,
	HDR,
	PIC,
	PNM
};

typedef struct _image* Image;

struct _image {
	/// <summary>
	/// The path that was used to load the image
	/// </summary>
	char* Path;
	/// <summary>
	/// The width in pixels of the image
	/// </summary>
	int Width;
	/// <summary>
	/// The height in pixels of the image
	/// </summary>
	int Height;
	/// <summary>
	/// The number of color channels in the image
	/// </summary>
	int Channels;
	/// <summary>
	/// The underlying pixels of the image
	/// </summary>
	unsigned char* Pixels;
};

struct _imageMethods {
	/// Allocates a new image and returns a pointer to it
	Image(*CreateImage)(void);
	/// <summary>
	/// Attempts to load the provided image, returns the loaded image if it was loaded, otherwise null
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	Image(*LoadImage)(const char* path);
	/// Attempts to load the image's information, returns true if the image was located and info obtained, otherwise false
	/// Use Dispose to free the object
	bool (*TryGetImageInfo)(const char* path, Image* out_info);
	/// Attempts to load the provided image, returns true if it was successfully loaded, otherwise false
	/// Use Dispose to free the object
	bool (*TryLoadImage)(const char* path, Image* out_image);
	/// <summary>
	/// Disposes the image
	/// </summary>
	void(*Dispose)(Image image);
};

extern const struct _imageMethods Images;