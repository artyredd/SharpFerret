#pragma once

#include "singine/gameobject.h"
#include "modeling/importer.h"

GameObject CreateGameObjectFromMesh(Mesh mesh);
GameObject LoadGameObjectFromModel(char* path, FileFormat format);