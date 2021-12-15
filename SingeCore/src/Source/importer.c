#include "csharp.h"
#include "stdlib.h"
#include "singine/file.h"
#include "modeling/importer.h"
#include "singine/guards.h"

static bool VerifyFormat(FileFormat format)
{
	for (size_t i = 0; i < sizeof(SupportedFormats) / sizeof(FileFormat); i++)
	{
		if (format == SupportedFormats[i])
		{
			return true;
		}
	}
	return false;
}

static bool TryImportModel(char* path, FileFormat format)
{
	// make sure the format is supported
	if (VerifyFormat(format) is false)
	{
		return false;
	}

	// open the path
	File model;
	if (TryOpen(path, FileModes.Read, &model) is false)
	{
		return false;
	}



	return true;
}

Mesh ImportModel(char* path, FileFormat format)
{
	return null;
}