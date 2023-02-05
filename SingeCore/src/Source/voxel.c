#include "math/voxel.h"

static voxel Create(vec3 point);
static quadrant GetQuandrant(vec3 centroid, vec3 point);
static bool Intersects(voxel, voxel);

const struct _voxelMethods Voxels = {
	.Create = Create,
	.GetQuandrant = GetQuandrant,
	.Intersects = Intersects
};

static voxel Create(vec3 point) 
{

}

static quadrant GetQuandrant(vec3 centroid, vec3 point) 
{

}

static bool CubeIntersects(const voxel left, const voxel right)
{
	if (right.StartVertex)
	{
	
	}
}

static bool Intersects(const voxel left , const voxel right)
{
	
}