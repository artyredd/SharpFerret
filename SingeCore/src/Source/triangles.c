
#include "math/triangles.h"
#include "cglm/vec3.h"
#include "cglm/mat3.h"
#include "cglm/ray.h"

static double Determinant(const triangle, const vec3);
static bool Intersects(const triangle,const triangle);
static void CalculateNormal(const triangle triangle, vec3 out_normal);
static void WindTriangle(triangle triangle);
static bool IntersectsSegmentOnTriangle(const triangle left, const vec3 start, const vec3 end);

const struct _triangles Triangles = {
	.Determinant = &Determinant,
	.Intersects = &Intersects,
	.CalculateNormal = CalculateNormal,
	.WindTriangle = WindTriangle,
	.SegmentIntersects = IntersectsSegmentOnTriangle
};

static void CalculateNormal(const triangle triangle, vec3 out_normal)
{
	// normal of a triangle is the cross product of the two rays
	// formed from two sides of the triangle
	vec3 left;
	Vectors3CopyTo(triangle[1], left);
	SubtractVectors3(left, triangle[0]);

	vec3 right;
	Vectors3CopyTo(triangle[2], right);
	SubtractVectors3(right, triangle[0]);

	glm_vec3_crossn(left, right, out_normal);
}

// calculates the determinant of a point and a triangle
// This can be used to determine if a point is within a triangle or not
// if the value returned is positive the point exists within the triangle
// if the value returned is negative the point is outside the triangle
// if the value is retruned the point lies on an edge or matches a vertice of the triangle
static inline double Determinant(const triangle triangle, const vec3 point)
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

static inline bool InsideTriangle(const triangle triangle,const vec3 point)
{
	return Determinant(triangle, point) >= 0;
}

static inline bool AnyPointExistsWithinTriangle(const triangle left, const triangle right)
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
static inline bool GetDeterminants(const triangle left, const triangle right, struct determinant* out_determinants)
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
	if ( negative(orientation) )
	{
		return true;
	}

	if ( positive(orientation) )
	{
		vec3 tmp;

		Vectors3CopyTo(triangle[1], tmp);
		Vectors3CopyTo(triangle[2], triangle[1]);
		Vectors3CopyTo(tmp, triangle[2]);

		return true;
	}

	return false;
}

static void WindTriangle(triangle triangle)
{
	OrientCounterClockwise(triangle);
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

static bool CheckPlaneIntersectionIntervals(const triangle left, const triangle right)
{
	triangle tmp;
	Vectors3CopyTo(left[0], tmp[0]);
	Vectors3CopyTo(left[1], tmp[1]);
	Vectors3CopyTo(right[0], tmp[2]);

	const bool firstDeterminant = Determinant(tmp, right[1]) <= 0;

	Vectors3CopyTo(left[0], tmp[0]);
	Vectors3CopyTo(left[2], tmp[1]);
	Vectors3CopyTo(right[2], tmp[2]);

	const bool secondDeterminant = Determinant(tmp, right[0]) <= 0;

	return firstDeterminant == secondDeterminant;
}

static bool IntersectsSegmentOnTriangle(const triangle left, const vec3 start, const vec3 end)
{
	/*vec3 direction;
	Vector3s.Subtract(end, start, direction);

	float distance;
	return glm_ray_triangle(start, direction, left[0],left[1], left[2], &distance);*/

	vec3 startVertex;
	vec3 endVertex;

	Vectors3CopyTo(start, startVertex);
	Vectors3CopyTo(end, endVertex);

	SubtractVectors3(startVertex, left[0]);
	SubtractVectors3(endVertex, left[0]);


	vec3 firstToSecondRay;
	vec3 firstToThirdRay;

	Vectors3CopyTo(left[1], firstToSecondRay);
	SubtractVectors3(firstToSecondRay, left[0]);

	Vectors3CopyTo(left[2], firstToThirdRay);
	SubtractVectors3(firstToThirdRay, left[0]);

	vec3 normal;
	glm_vec3_cross(firstToSecondRay, firstToThirdRay, normal);

	// x1 x2 nx
	// y1 y2 ny
	// z1 z2 nz
	mat3 matrix;

	// GLM IS COLUMN MAJOR
	// because math i guess
	Vectors3CopyTo(firstToSecondRay, matrix[0]);
	Vectors3CopyTo(firstToThirdRay, matrix[1]);
	Vectors3CopyTo(normal, matrix[2]);

	glm_mat3_inv(matrix, matrix);

	vec3 startInTriangleSpace;
	glm_mat3_mulv(matrix, startVertex, startInTriangleSpace);

	vec3 endInTriangleSpace;
	glm_mat3_mulv(matrix, endVertex, endInTriangleSpace);

	const float zSign = startInTriangleSpace[2] * endInTriangleSpace[2];

	if (zSign >= 0)
	{
		return false;
	}

	const float aX = startInTriangleSpace[0];
	const float aY = startInTriangleSpace[1];
	const float aZ = startInTriangleSpace[2];

	const float bX = endInTriangleSpace[0];
	const float bY = endInTriangleSpace[1];
	const float bZ = endInTriangleSpace[2];

	const float t = bZ / (bZ - aZ);

	const float negT = (1 - t);

	const float x = (aX * t) + (negT * bX);
	const float y = (aY * t) + (negT * bY);

	return x >= 0.0 && y >= 0 && (x + y) <= 1.0;
}

static bool Intersects(const triangle left, const triangle right)
{
	return 
		IntersectsSegmentOnTriangle(left, right[0], right[1]) || 
		IntersectsSegmentOnTriangle(left, right[1], right[2]) || 
		IntersectsSegmentOnTriangle(left, right[2], right[0]) || 
		IntersectsSegmentOnTriangle(right, left[0], left[1]) ||
		IntersectsSegmentOnTriangle(right, left[1], left[2]) ||
		IntersectsSegmentOnTriangle(right, left[2], left[0]);
}