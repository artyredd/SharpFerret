#pragma once

struct vector3
{
	float x;
	float y;
	float z;
};

typedef unsigned char quadrant;

typedef struct _quadrantLayer quadrantLayer;

struct _quadrantLayer
{
	quadrant NorthWest;
	quadrant NorthEast;
	quadrant SouthWest;
	quadrant SouthEast;
};

struct _octants {
	quadrantLayer Upper;
	quadrantLayer Lower;
} Octants = {
	.Upper = {
		1,
		2,
		3,
		4
	},
	.Lower = {
		5,
		6,
		7,
		8
	}
};

typedef struct voxel* Voxel;

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

struct  _voxel
{
	vec3 Centriod;
	vec3 StartVertex;
	vec3 EndVertex;

	// pointer collection for all the voxels
	// located above the y axis of this voxel
	struct _voxelLayer Upper;

	// pointer collection for all the voxels
	// located below the y axis of this voxel
	struct _voxelLayer Lower;
};

struct _voxelMethods
{
	voxel (*Create)(vec3 point);
	quadrant (*GetQuandrant)(vec3 centroid, vec3 point);
	bool (*Intersects)(voxel, voxel);
};

extern const struct _voxelMethods Voxels;