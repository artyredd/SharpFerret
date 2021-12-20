#pragma once

#include "mesh.h"

typedef struct _model* Model;

struct _model {
	char* Name;
	/// <summary>
	/// The number of meshes that this model contains
	/// </summary>
	size_t Count;
	Mesh Head;
	Mesh Tail;
	void(*Dispose)(Model);
};