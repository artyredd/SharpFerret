#pragma once

#include "engine/gameobject.h"
#include "engine/modeling/importer.h"

GameObject CreateGameObjectFromMesh(Mesh mesh);
GameObject CreateFromRenderMesh(RenderMesh mesh);
GameObject LoadGameObjectFromModel(char* path, FileFormat format);