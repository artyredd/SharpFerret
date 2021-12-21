#pragma once

#include "modeling/mesh.h"
#include "modeling/model.h"

typedef const char* FileFormat;

static const struct _formats {
	const FileFormat Obj;
} FileFormats = {
	".obj"
};

static const FileFormat SupportedFormats[1] = { ".obj" };

bool TryImportModel(char* path, FileFormat format, Model* out_model);

Model ImportModel(char* path, FileFormat format);