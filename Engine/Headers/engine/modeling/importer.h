#pragma once

#include "engine/modeling/mesh.h"
#include "engine/modeling/model.h"
#include "core/array.h"

typedef const char* FileFormat;

static const struct _formats {
	const FileFormat Obj;
} FileFormats = {
	".obj"
};

static const FileFormat SupportedFormats[1] = { ".obj" };

struct _modelImporterMethods {
	bool (*TryImport)(string path, FileFormat format, Model* out_model);

	Model(*Import)(string path, FileFormat format);
};

extern const struct _modelImporterMethods Importers;