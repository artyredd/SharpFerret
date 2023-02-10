#include "math/voxel.h"

static voxel Create(vector3 point);
static quadrant GetQuandrant(vector3 centroid, vector3 point);
static bool Intersects(voxel, voxel);

const struct _voxelMethods Voxels = {
	.Create = Create,
	.GetQuandrant = GetQuandrant,
	.Intersects = Intersects
};

static voxel Create(vector3 point) 
{
	 
}

static quadrant GetQuandrant(vector3 centroid, vector3 point) 
{

}

static bool CubeIntersects(const voxel left, const voxel right)
{
}

static bool Intersects(const voxel left , const voxel right)
{
	
}