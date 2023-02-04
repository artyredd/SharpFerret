#include "cunit.h"
#include "math/triangles.h"
#include "csharp.h"
#include "cglm/ray.h"

TEST(Determinant)
{
	vec3 point = {
		0,0,0
	};

	triangle triangle = {
		{1,2,3},
		{0,4,5},
		{0,0,6}
	};

	const double expected = 24;

	const double actual = Triangles.Determinant(triangle, point);

	Equals(expected,actual,"%lf");

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

	SetVector3Macro(left[0], 0, 3, 0);
	SetVector3Macro(left[1], 0, 4, 0);
	SetVector3Macro(left[2], 4, 3, 0);

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

TEST(cglm_coplanar_works)
{
	triangle left = {
		{ 0,0,0 },
		{ 0.5,0.5,0 },
		{ 1,0,0 }
	};

	vec3 origin = { 0, 0, 0};

	vec3 ray = { 0.25, 0.25, 0 };

	float distance = 0;

	const bool actual = glm_ray_triangle(origin, ray, left[0], left[1], left[2], &distance);

	const bool expected = true;

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
	APPEND_TEST(cglm_coplanar_works);

	suite->Run(suite);

	suite->Dispose(suite);
}