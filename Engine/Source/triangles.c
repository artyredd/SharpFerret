
#include "math/triangles.h"
#include "cglm/ray.h"

private bool Intersects(const triangle,const triangle);
private vector3 CalculateNormal(const triangle triangle);
private bool IntersectsSegmentOnTriangle(const triangle left, const vector3 start, const vector3 end);
private vector3 Centroid(triangle triangle);

const struct _triangles Triangles = {
	.Intersects = &Intersects,
	.CalculateNormal = CalculateNormal,
	.SegmentIntersects = IntersectsSegmentOnTriangle,
	.Centroid = Centroid
};

private vector3 Centroid(triangle triangle)
{
	return (vector3) {
		(triangle.Point1.x + triangle.Point2.x + triangle.Point3.x) / 3,
		(triangle.Point1.y + triangle.Point2.y + triangle.Point3.y) / 3,
		(triangle.Point1.z + triangle.Point2.z + triangle.Point3.z) / 3,
	};
}

static vector3 CalculateNormal(const triangle triangle)
{
	// normal of a triangle is the cross product of the two rays
	// formed from two sides of the triangle
	vector3 left = Vector3s.Subtract(triangle.Point2, triangle.Point1);

	vector3 right = triangle.Point3;
	right = Vector3s.Subtract(right, triangle.Point1);

	return Vector3s.Cross(left, right);
}

#define positive(number) ((number) > 0)
#define negative(number) ((number) < 0)
#define zero(number) ((number) is 0)

static bool IntersectsSegmentOnTriangle(const triangle triangle, const vector3 start, const vector3 end)
{
	/*vector3 direction = Vector3s.Subtract(end, start);

	glm_normalize((float*)&direction);

	float distance;
	const bool intersects = glm_ray_triangle(
		(float*)&start, 
		(float*)&direction,
		(float*)&triangle.Point1,
		(float*)&triangle.Point2,
		(float*)&triangle.Point3,
		&distance
	);

	return intersects && distance <= Vector3s.Distance(start, end);*/

	vector3 startVertex = Vector3s.Subtract(start, triangle.Point1);
	vector3 endVertex = Vector3s.Subtract(end, triangle.Point1);

	vector3 firstToSecondRay = Vector3s.Subtract(triangle.Point2, triangle.Point1);
	vector3 firstToThirdRay = Vector3s.Subtract(triangle.Point3, triangle.Point1);

	vector3 normal = Vector3s.Cross(firstToSecondRay, firstToThirdRay);

	// x1 x2 nx
	// y1 y2 ny
	// z1 z2 nz

	// GLM IS COLUMN MAJOR
	// because math i guess
	matrix3 matrix = 
	{
		firstToSecondRay,
		firstToThirdRay,
		normal
	};

	matrix = Matrix3s.Inverse(matrix);

	vector3 startInTriangleSpace = Matrix3s.MultiplyVector3(matrix, startVertex);

	vector3 endInTriangleSpace = Matrix3s.MultiplyVector3(matrix, endVertex);

	const float zSign = startInTriangleSpace.z * endInTriangleSpace.z;

	if (zSign >= 0)
	{
		return false;
	}

	const float aX = startInTriangleSpace.x;
	const float aY = startInTriangleSpace.y;
	const float aZ = startInTriangleSpace.z;

	const float bX = endInTriangleSpace.x;
	const float bY = endInTriangleSpace.y;
	const float bZ = endInTriangleSpace.z;

	const float t = bZ / (bZ - aZ);

	const float negT = (1 - t);

	const float x = (aX * t) + (negT * bX);
	const float y = (aY * t) + (negT * bY);

	return x >= 0.0 && y >= 0 && (x + y) <= 1.0;
}

static bool Intersects(const triangle left, const triangle right)
{
	return 
		IntersectsSegmentOnTriangle(left, right.Point1, right.Point2) || 
		IntersectsSegmentOnTriangle(left, right.Point2, right.Point3) || 
		IntersectsSegmentOnTriangle(left, right.Point3, right.Point1) || 
		IntersectsSegmentOnTriangle(right, left.Point1, left.Point2) ||
		IntersectsSegmentOnTriangle(right, left.Point2, left.Point3) ||
		IntersectsSegmentOnTriangle(right, left.Point3, left.Point1);
}