#pragma once

#include "math/vectors.h"
#include "math/triangles.h"
#include "modeling/mesh.h"
#include "graphics/transform.h"
#include "cuboid.h"

typedef unsigned char quadrant;

struct _quadrantHemiLayer
{
	quadrant East;
	quadrant West;
};

struct _quadrantLayer
{
	struct _quadrantHemiLayer North;
	struct _quadrantHemiLayer South;
};

static struct _octants {
	struct _quadrantLayer Upper;
	struct _quadrantLayer Lower;
} Octants = {
	.Upper = {
		0b000,
		0b001,
		0b010,
		0b011
	},
	.Lower = {
		0b100,
		0b101,
		0b110,
		0b111
	}
};

typedef struct _voxel* Voxel;

struct _voxelHemiLayer
{
	// Pointer to the East (+X) Subvoxels
	Voxel East;

	// Pointer to the East (-X) Subvoxels
	Voxel West;
};

struct _voxelLayer
{
	// Pointers to the north (+Z) Subvoxels
	struct _voxelHemiLayer North;

	// Pointers to the south (-Z) Subvoxels
	struct _voxelHemiLayer South;
};


typedef struct _voxel voxel;

struct _voxel
{
	// pointer to the voxel that owns this object
	Voxel Parent;

	cuboid BoundingBox;

	// the triangle this voxel is representing;
	triangle Triangle;

	// pointer collection for all the voxels
	// located above the y axis of this voxel
	struct _voxelLayer Upper;

	// pointer collection for all the voxels
	// located below the y axis of this voxel
	struct _voxelLayer Lower;
};

typedef struct voxelTree voxelTree;

struct voxelTree
{
	// Array of voxels in this tree
	Voxel Voxels;
	
	// The number of voxels in this tree
	size_t Count;

	// The transform referenced by this tree
	Transform Transform;
};

struct _voxelMethods
{
	voxelTree(*Create)(Mesh mesh);
	void (*Dispose)(voxelTree);
	bool (*IntersectsTree)(const voxelTree, const voxelTree);
};

extern const struct _voxelMethods Voxels;

void RunVoxelUnitTests(void);