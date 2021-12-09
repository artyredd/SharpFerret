#include "csharp.h"

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
	/// The width in pixels of the image
	/// </summary>
	size_t Width;
	/// <summary>
	/// The height in pixels of the image
	/// </summary>
	size_t Height;
	/// <summary>
	/// The number of color channels in the image
	/// </summary>
	int Channels;
	/// <summary>
	/// The underlying pixels of the image
	/// </summary>
	unsigned char* Pixels;
	/// <summary>
	/// Disposes the image
	/// </summary>
	void(*Dispose)(Image image);
};

/// Attempts to load the provided image, returns true if it was successfully loaded, otherwise false
/// Use Dispose to free the object
static bool TryLoadImage(const char* path, Image* out_image);

/// Attempts to load the image's information, returns true if the image was located and info obtained, otherwise false
/// Use Dispose to free the object
static bool TryGetImageInfo(const char* path, Image* out_info);

/// Allocates a new image and returns a pointer to it
static Image CreateImage();