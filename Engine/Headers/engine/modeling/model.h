#pragma once

#include "mesh.h"

typedef struct _model* Model;

struct _model {
	char* Name;
	/// <summary>
	/// The number of meshes that this model contains
	/// </summary>
	ulong Count;
	Mesh* Meshes;
};

struct _modelMethods {
	void (*Dispose)(Model);
	Model(*Create)(void);
};

extern const struct _modelMethods Models;