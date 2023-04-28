#include "math/voxel.h"
#include "singine/memory.h"
#include "helpers/quickmask.h"
#include "graphics/drawing.h"
#include "tests/cunit.h"

private voxelTree Create(Mesh mesh);
private bool Intersects(
	const Voxel left, const Transform leftTransform, 
	const Voxel right, const Transform rightTransform,
	Voxel* out_voxel);
private bool IntersectsTree(const voxelTree, const voxelTree);
private void Dispose(voxelTree);

const struct _voxelMethods Voxels = {
	.Create = Create,
	.Dispose = Dispose,
	.IntersectsTree = IntersectsTree
};

static quadrant GetQuadrant(vector3 center, vector3 point)
{
	const bool upper = point.y >= center.y;
	const bool north = point.z >= center.z;
	const bool east = point.x >= center.x;

	quadrant result = 0;

	result |= upper ? 0 : FLAG_2;
	result |= north ? 0 : FLAG_1;
	result |= east  ? 0 : FLAG_0;

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
	Voxel root = &voxels[0];

	// get the center of all the points in the model
	// this will be the center of the root voxel tree
	const vector3 globalCentroid = Vector3s.MeanArray(mesh->Vertices, mesh->VertexCount);
	
	// ensure that when the cuboid is joined
	// the root always resizes to be the first cuboid
	root->BoundingBox = Cuboids.Minimum;

	// centers dont get changed when we join two cuboids
	root->BoundingBox.Center = globalCentroid;

	for (size_t i = 0; i < voxelCount; i++)
	{
		// assign the triangles
		Voxel destinationVoxel = &voxels[i + 1];

		const triangle triangle = struct_cast(struct triangle)mesh->Vertices[i * 3];

		destinationVoxel->Triangle = triangle;

		// generate the bounding box for the cuboid
		const cuboid boundingBox = Cuboids.Create(triangle);

		destinationVoxel->BoundingBox = boundingBox;

		SortNewVoxel(root, destinationVoxel);
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

private bool RecurseIntersects(const Voxel left, const Transform leftTransform, const Voxel right, const Transform rightTransform, Voxel* out_voxel)
{
#define RECURSE(octant)\
	if (left->##octant isnt null)\
	{\
		if (Intersects(right, rightTransform, left->##octant, leftTransform, out_voxel))\
		{\
			return true;\
		}\
	}

#pragma warning (disable:5103)
	RECURSE(Upper.North.East);
	RECURSE(Upper.North.West);
	RECURSE(Upper.South.East);
	RECURSE(Upper.South.West);
	RECURSE(Lower.North.East);
	RECURSE(Lower.North.West);
	RECURSE(Lower.South.East);
	RECURSE(Lower.South.West);
#pragma warning (default:5103)

#undef RECURSE
	return false;
}

private bool Intersects(const Voxel left, const Transform leftTransform, const Voxel right, const Transform rightTransform, Voxel* out_voxel)
{
	// set out variable first
	*out_voxel = null;

	// check to see if the two cuboids intersect first
	const cuboid leftBounding = Cuboids.AddOffset(left->BoundingBox, leftTransform->Position);
	const cuboid rightBounding = Cuboids.AddOffset(right->BoundingBox, rightTransform->Position);

	const bool cuboidsIntersect = Cuboids.Intersects(leftBounding, rightBounding);

	if (cuboidsIntersect is false)
	{
		return false;
	}

	// just because the cuboids intersect doesn't necessarily mean the voxels intersect
	// check to see which child its intersecting
	if (RecurseIntersects(left, leftTransform, right, rightTransform, out_voxel))
	{
		return true;
	}

	// since we have no children we're a leaf voxel and contain a triangle
	// check to see if the two triangles collide
	const bool trianglesIntersect = Triangles.Intersects(left->Triangle, right->Triangle);

	*out_voxel = left;

	return trianglesIntersect;
}

private bool IntersectsTree(const voxelTree left, const voxelTree right)
{
	Voxel intersection;
	return Intersects(left.Voxels, left.Transform, right.Voxels, right.Transform, &intersection);
}

TEST(GetQuadrantWorks)
{
	const vector3 origin = {0,0,0};
	vector3 vertex = {0,0,0};

	IsEqual(Octants.Upper.North.East, GetQuadrant(origin, vertex),"%i");

	vertex = (vector3){0,-1,0};
	IsEqual(Octants.Lower.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 0,1,0 };
	IsEqual(Octants.Upper.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 1,0,0 };
	IsEqual(Octants.Upper.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ -1,0,0 };
	IsEqual(Octants.Upper.North.West, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 1,1,0 };
	IsEqual(Octants.Upper.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ -1,1,0 };
	IsEqual(Octants.Upper.North.West, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 0,0,0 };
	IsEqual(Octants.Upper.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 0,1,-1 };
	IsEqual(Octants.Upper.South.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 0,-1,1 };
	IsEqual(Octants.Lower.North.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ 1, 1,-1 };
	IsEqual(Octants.Upper.South.East, GetQuadrant(origin, vertex), "%i");

	vertex = (vector3){ -1, -1,-1 };
	IsEqual(Octants.Lower.South.West, GetQuadrant(origin, vertex), "%i");

	return true;
}

TEST(VoxelGeneratesCorrectly)
{
	vector3 vertices[24] = {
		{-1,-1,-1},
		{-1,-1,-1},
		{-1,-1,-1},
		{-1,1,-1},
		{-1,1,-1},
		{-1,1,-1},
		{1,-1,-1},
		{1,-1,-1},
		{1,-1,-1},
		{1,1,-1},
		{1,1,-1},
		{1,1,-1},
		{1,-1,1},
		{1,-1,1},
		{1,-1,1},
		{1,1,1},
		{1,1,1},
		{1,1,1},
		{-1,-1,1},
		{-1,-1,1},
		{-1,-1,1},
		{-1,1,1},
		{-1,1,1},
		{-1,1,1}
	};

	struct _mesh mesh = {
		.Name = "Voxel",
		.Vertices = vertices,
		.VertexCount = sizeof(vertices)/sizeof(vector3)
	};

	struct _transform transform1 = {
		.Position = {0,0,0}
	};

	voxelTree left = Create(&mesh);
	left.Transform = &transform1;
	
	return true;
}

TEST_SUITE(
	VoxelTests,
	APPEND_TEST(VoxelGeneratesCorrectly)
	APPEND_TEST(GetQuadrantWorks)
);

void RunVoxelUnitTests(void)
{
	VoxelTests();
}