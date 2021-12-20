#pragma once

#include "mesh.h"

typedef struct _model* Model;

struct _model {
	char* Name;
	Mesh Head;
	Mesh Tail;
	void(*Dispose)(Model);
};