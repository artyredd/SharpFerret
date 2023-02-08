#include "cunit.h"
#include "math/triangles.h"
#include "csharp.h"
#include "cglm/ray.h"

TEST(Determinant)
{
	vector3 point = {
		0,0,0
	};

	triangle triangle = {
		{1,2,3},
		{0,4,5},
		{0,0,6}
	};

	const double expected = 24;

	const double actual = Triangles.Determinant(triangle, point);

	IsEqual(expected,actual,"%lf");

	return true;
}

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

static void TriangleTests(void)
{
	TestSuite suite = CreateSuite("TriangleTests");

	APPEND_TEST(Determinant);
	APPEND_TEST(NonIntersecting);
	APPEND_TEST(IntersectingNonCoplanar);
	APPEND_TEST(NonIntersectingCoplanar);
	APPEND_TEST(IntersectingCoplanar);

	suite->Run(suite);

	suite->Dispose(suite);
}