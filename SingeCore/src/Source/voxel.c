#include "math/voxel.h"
#include "singine/memory.h"
#include "helpers/quickmask.h"

private voxelTree Create(Mesh mesh);
private bool Intersects(Voxel, Voxel);
private bool IntersectsTree(voxelTree, voxelTree);
private void Dispose(VoxelTree);

const struct _voxelMethods Voxels = {
	.Create = Create,
	.Intersects = Intersects,
	.Dispose = Dispose,
	.IntersectsTree = IntersectsTree
};

static quadrant GetQuadrant(vector3 center, vector3 point)
{
	const bool upper = point.y >= center.y;
	const bool north = point.z <= center.z;
	const bool east = point.x >= center.x;

	quadrant result = 0;

	result |= upper ? 0 : FLAG_2;
	result |= north ? 0 : FLAG_1;
	result |= east ? FLAG_0 : 0;

	return result;
}

private void SortNewVoxel(Voxel root, Voxel voxel);

private void SortOrAssignVoxel(Voxel* destination, Voxel voxel)
{
	if (*destination isnt null)
	{
		SortNewVoxel(*destination, voxel);
		return;
	}
	*destination = voxel;
}

private void SortNewVoxel(Voxel root, Voxel voxel)
{
	// first rule of tree club, don't touch root >:(
	if (root == voxel)
	{
		return;
	}

	const vector3 center = voxel->BoundingBox.Center;
	const quadrant quadrant = GetQuadrant(root->BoundingBox.Center, center);

	// expand the parents bounding box if needed
	root->BoundingBox = Cuboids.Join(root->BoundingBox, voxel->BoundingBox);

	// check to see if we have children
	if (quadrant is Octants.Upper.North.East)
	{
		SortOrAssignVoxel(&root->Upper.North.East, voxel);
	}
	else
	if(quadrant is Octants.Upper.North.West)
	{
		SortOrAssignVoxel(&root->Upper.North.West, voxel);
	}
	else
	if(quadrant is Octants.Upper.South.East)
	{
		SortOrAssignVoxel(&root->Upper.South.East, voxel);
	}
	else
	if(quadrant is Octants.Upper.South.West)
	{
		SortOrAssignVoxel(&root->Upper.South.West, voxel);
	}
	else
	if(quadrant is Octants.Lower.North.East)
	{
		SortOrAssignVoxel(&root->Lower.North.East, voxel);
	}
	else
	if(quadrant is Octants.Lower.North.West)
	{
		SortOrAssignVoxel(&root->Lower.North.West, voxel);
	}
	else
	if(quadrant is Octants.Lower.South.East)
	{
		SortOrAssignVoxel(&root->Lower.South.East, voxel);
	}
	else
	if(quadrant is Octants.Lower.South.West)
	{
		SortOrAssignVoxel(&root->Lower.South.West, voxel);
	}
}

private vector3 GetGlobalCenter(const Voxel array, const size_t count)
{
	float x = 0;
	float y = 0;
	float z = 0;

	for (size_t i = 0; i < count; i++)
	{
		const vector3 vector = array[i].BoundingBox.Center;

		x += vector.x;
		y += vector.y;
		z += vector.z;
	}

	return (vector3) {
		x / count,
		y / count,
		z / count
	};
}

TYPE_ID(voxel);
TYPE_ID(VoxelTree);

static voxelTree Create(Mesh mesh) 
{
	const size_t voxelCount = mesh->VertexCount / 3;

	// we'll need a voxel for each triangle so just allocate the amount of triangles we need right off the bat
	REGISTER_TYPE(voxel);
	Voxel voxels = Memory.Alloc((sizeof(voxel) * voxelCount) + 1, voxelTypeId);

	// create the root
	// the root's center is the center of the mesh
	// calculated from the centroids of all the triangles
	voxel root = voxels[0];

	// get the center of all the points in the model
	// this will be the center of the root voxel tree
	const vector3 globalCentroid = Vector3s.MeanArray(mesh->Vertices, mesh->VertexCount);
	
	// ensure that when the cuboid is joined
	// the root always resizes to be the first cuboid
	root.BoundingBox = Cuboids.Minimum;

	// centers dont get changed when we join two cuboids
	root.BoundingBox.Center = globalCentroid;

	for (size_t i = 1; i < voxelCount; i++)
	{
		// assign the triangles
		Voxel destinationVoxel = &voxels[i];

		const triangle triangle = struct_cast(struct triangle)mesh->Vertices[i * 3];

		destinationVoxel->Triangle = triangle;

		// generate the bounding box for the cuboid
		const cuboid boundingBox = Cuboids.Create(triangle);

		destinationVoxel->BoundingBox = boundingBox;

		SortNewVoxel(&root, destinationVoxel);
	}

	voxelTree result = {
		.Voxels = voxels,
		.Count = voxelCount
	};

	return result;
}

private void Dispose(voxelTree tree)
{
	Memory.Free(tree.Voxels, voxelTypeId);
}

static bool Intersects(const Voxel left, const Voxel right, Voxel* out_voxel)
{
	// set out variable first
	*out_voxel = null;

	// check to see if the two cuboids intersect first
	const bool cuboidsIntersect = Cuboids.Intersects(left->BoundingBox, right->BoundingBox);

	if (cuboidsIntersect is false)
	{
		return false;
	}

	// just because the cuboids intersect doesn't necessarily mean the voxels intersect
	// check to see which child its intersecting
	if (left->Upper.North.East isnt null)
	{
		if (Intersects(right, left->Upper.North.East, out_voxel))
		{
			return true;
		}
	}

	// since we have no children we're a leaf voxel and contain a triangle
	// check to see if the two triangles collide
	const bool trianglesIntersect = Triangles.Intersects(left->Triangle, right->Triangle);

	*out_voxel = left;

	return trianglesIntersect;
}

private bool IntersectsTree(const voxelTree left, const voxelTree right)
{

}