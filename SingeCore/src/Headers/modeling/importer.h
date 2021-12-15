#pragma once

#include "modeling/mesh.h"

typedef const char* FileFormat;

static const struct _formats {
	const FileFormat Obj;
} FileFormats = {
	".obj"
};

static const FileFormat SupportedFormats[1] = { ".obj" };

Mesh ImportModel(char* path, FileFormat format);