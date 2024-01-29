#pragma once

#include "engine/graphics/renderMesh.h"

struct _renderModel {
	/// The transform of this model this is the parent transform for all child meshes of this object
	Transform Transform;
	// An array of render meshes that this model controls
	RenderMesh* Meshes;
	// The number of meshes this model controls
	size_t Count;
};