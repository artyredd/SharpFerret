#pragma once

#include "modeling/mesh.h"

typedef const char* FileFormat;

static const struct _formats {
	const FileFormat Obj;
} SupportedFormats = {
	".obj"
};

Mesh ImportModel(char* path);