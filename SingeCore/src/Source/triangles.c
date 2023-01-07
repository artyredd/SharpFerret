
#include "math/triangles.h"

static double Determinant(triangle, vec3);
static bool Intersects(triangle,triangle);

const struct _triangles Triangles = {
	.Determinant = &Determinant,
	.Intersects = &Intersects
};

// calculates the determinant of a point and a triangle
// This can be used to determine if a point is within a triangle or not
// if the value returned is positive the point exists within the triangle
// if the value returned is negative the point is outside the triangle
// if the value is retruned the point lies on an edge or matches a vertice of the triangle
static inline double Determinant(triangle triangle, vec3 point)
{
#define a ( triangle[0][0] - point[0] )
#define b ( triangle[0][1] - point[1] )
#define c ( triangle[0][2] - point[2] )
#define d ( triangle[1][0] - point[0] )
#define e ( triangle[1][1] - point[1] )
#define f ( triangle[1][2] - point[2] )
#define g ( triangle[2][0] - point[0] )
#define h ( triangle[2][1] - point[1] )
#define i ( triangle[2][2] - point[2] )
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
}

static inline bool InsideTriangle(triangle triangle, vec3 point)
{
	return Determinant(triangle, point) >= 0;
}

static inline bool AnyPointExistsWithinTriangle(triangle left, triangle right)
{
	return 
		InsideTriangle(left, right[0]) ||
		InsideTriangle(left, right[1]) ||
		InsideTriangle(left, right[2]) ||
		InsideTriangle(right, left[0]) ||
		InsideTriangle(right, left[1]) ||
		InsideTriangle(right, left[2]);
}

// positive: the points are arranged in a counterclockwise orientation. 
// negative: the points are arranged in a clockwise orientation. 
// zero: the points are collinear.
static inline float VerticeOrientation(triangle triangle)
{
	return Matrices.Determinant(triangle);
}

#define positive(number) ((number) > 0)
#define negative(number) ((number) < 0)
#define zero(number) ((number) is 0)

struct determinant
{
	double xDeterminant;
	double yDeterminant;
	double zDeterminant;
};

// Calculates the determinants of the given triangles and sets the 
// the values of the given out_determinant to the caculated values
// returns true: when the determinants are valid and an intersection MAY
// be possible and requires further calculations
// returns false: when no intersection is possible between both triangles
static inline bool GetDeterminants(triangle left, triangle right, struct determinant* out_determinants)
{
	const double firstDeterminant = Determinant(left, right[0]);
	const double secondDeterminent = Determinant(left, right[1]);
	const double thirdDeterminent = Determinant(left, right[2]);

	out_determinants->xDeterminant = firstDeterminant;
	out_determinants->yDeterminant = secondDeterminent;
	out_determinants->zDeterminant = thirdDeterminent;

	const bool allPositiveNonZero = positive(firstDeterminant) && positive(secondDeterminent) && positive(thirdDeterminent);
	const bool allNegativeNonZero = negative(firstDeterminant) && negative(secondDeterminent) && negative(thirdDeterminent);

	return (allPositiveNonZero || allNegativeNonZero) == false;
}

// orients the provided triangle to be counter clockwise
// returns true: when the triangle was oriented successfully
// otherwise returns false: notable when all 3 vertices are colinear
static bool OrientCounterClockwise(triangle triangle)
{
	const float orientation = VerticeOrientation(triangle);

	// the vertices are already counter clock wise in orientation
	if ( positive(orientation) )
	{
		return true;
	}

	if ( negative(orientation) )
	{
		vec3 tmp;

		Vectors3CopyTo(triangle[1], tmp);
		Vectors3CopyTo(triangle[2], triangle[1]);
		Vectors3CopyTo(tmp, triangle[2]);

		return true;
	}

	return false;
}

static float SignedArea(vec3 point, vec3 lineStart, vec3 lineEnd)
{
	vec3 ray;
	Vectors3CopyTo(lineStart, ray);
	SubtractVectors3(ray, lineEnd);

#define x point[0]
#define y point[1]
#define z point[2]
#define vx ray[0]
#define vy ray[1]
#define vz ray[2]
	return ((y* vz - z * vy) - (x * vz - z * vx) + (x * vy - y * vx)) / 2;
#undef x
#undef y
#undef z
#undef vx
#undef vy
#undef vz
}

static bool CheckPlaneIntersectionIntervals(triangle left, triangle right)
{
	triangle tmp;
	Vectors3CopyTo(left[0], tmp[0]);
	Vectors3CopyTo(left[1], tmp[1]);
	Vectors3CopyTo(right[0], tmp[0]);

	const bool firstDeterminant = Determinant(tmp, right[1]) <= 0;

	Vectors3CopyTo(left[0], tmp[0]);
	Vectors3CopyTo(left[2], tmp[1]);
	Vectors3CopyTo(right[2], tmp[0]);

	const bool secondDeterminant = Determinant(tmp, right[0]) <= 0;

	return firstDeterminant == secondDeterminant;
}

static bool Intersects(triangle left, triangle right)
{
	// Olivier Devillers, Philippe Guigue.Faster Triangle - Triangle Intersection Tests.RR - 4488, INRIA.
	//	2002. inria - 00072100
	struct determinant determinants;
	if (GetDeterminants( left,right, &determinants) == false)
	{
		return false;
	}

	const bool allZero = zero(determinants.xDeterminant) && zero(determinants.yDeterminant) && zero(determinants.zDeterminant);

	// when the dertminant is all zero then the triangles are coplanar and 
	// we must then determine if they are intersecting in a 2d space
	if (allZero)
	{
		// 2d calc
		return AnyPointExistsWithinTriangle(left, right);
	}
	else
	{
		// possible intersection occurs, check the other triangle as well to confirm
		return GetDeterminants(right,left, &determinants) && CheckPlaneIntersectionIntervals(left, right);
	}
}