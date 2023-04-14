#include "cunit.h"
#include "math/triangles.h"
#include "csharp.h"
#include "cglm/ray.h"
#include "math/cuboid.h"

TEST(NonIntersecting)
{
	// this is two triangles that are exactly the same but one behind
	// the other and not intersecting
	triangle left = {
		{ 0,0,0 },
		{ 0,1,0 },
		{ 1,0,0 }
	};
	triangle right = {
		{ 0,0,1 },
		{ 0,1,1 },
		{ 1,0,1 }
	};

	bool expected = false;
	bool actual = Triangles.Intersects(left, right);

	IsTrue(expected == actual);

	return true;
}

TEST(IntersectingNonCoplanar)
{
	// this is two triangles that are exactly the same but one behind
	// the other and not intersecting
	triangle left = {
		{ 0,0,0 },
		{ 0,1,0 },
		{ 1,0,0 }
	};
	triangle right = {
		{ 0,0.5,0.5 },
		{ 1,0.5,0.5 },
		{ 0.5,0.5,-0.5 }
	};

	bool expected = true;
	bool actual = Triangles.Intersects(left, right);

	IsTrue(expected == actual);

	left.Point1 = (vector3){ 0, 3, 0 };
	left.Point2 = (vector3){ 0, 4, 0 };
	left.Point3 = (vector3){ 4, 3, 0 };

	expected = false;
	actual = Triangles.Intersects(left, right);

	IsTrue(expected == actual);

	return true;
}

TEST(NonIntersectingCoplanar)
{
	triangle left = {
		{ 0,0,0 },
		{ 0,1,0 },
		{ 1,0,0 }
	};
	triangle right = {
		{ 2,2,0 },
		{ 2.5,3,0 },
		{ 3,2,0 }
	};

	bool expected = true;
	bool actual = Triangles.Intersects(left, right);

	IsTrue(expected == actual);

	return true;
}

TEST(IntersectingCoplanar)
{
	triangle left = {
		{ 0,0,0 },
		{ 0.5,0.5,0 },
		{ 1,0,0 }
	};
	triangle right = {
		{ 0,4,0 },
		{ 0.5f,0.4f,0 },
		{ 1,4,0 }
	};

	bool expected = true;
	bool actual = Triangles.Intersects(left, right);

	IsTrue(expected == actual);

	return true;
}

TEST(CuboidIntersectsWorks)
{
	cuboid left = {
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	cuboid right = {
		.StartVertex = {1,1,1},
		.EndVertex = {2,2,2}
	};

	// intersection is inclusive
	IsTrue(Cuboids.Intersects(left, right));

	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {1.001f,1.001f,1.001f},
		.EndVertex = {2,2,2}
	};

	IsFalse(Cuboids.Intersects(left, right));

	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {-1,-1,-1},
		.EndVertex = {0,0,0}
	};

	IsTrue(Cuboids.Intersects(left, right));

	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {-1,-1,-1},
		.EndVertex = {-.001f,-.001f,-.001f}
	};

	IsFalse(Cuboids.Intersects(left, right));

	// completely contained
	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {0.001f,0.001f,0.001f},
		.EndVertex = {0.999f,0.999f,0.999f}
	};

	IsTrue(Cuboids.Intersects(left, right));

	// completely contained degenerate
	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {0.001f,0.001f,0.001f},
		.EndVertex = {0.001f,0.001f,0.001f}
	};

	IsTrue(Cuboids.Intersects(left, right));

	// completely contained perfect
	left = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};
	right = (cuboid){
		.StartVertex = {0,0,0},
		.EndVertex = {1,1,1}
	};

	IsTrue(Cuboids.Intersects(left, right));

	return true;
}

TEST_SUITE(TriangleTests, 
	APPEND_TEST(NonIntersecting);
	APPEND_TEST(IntersectingNonCoplanar);
	APPEND_TEST(NonIntersectingCoplanar);
	APPEND_TEST(IntersectingCoplanar);
	APPEND_TEST(CuboidIntersectsWorks);
);