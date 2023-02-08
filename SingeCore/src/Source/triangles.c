
#include "math/triangles.h"
#include "cglm/ray.h"

static bool Intersects(const triangle,const triangle);
static vector3 CalculateNormal(const triangle triangle);
static bool IntersectsSegmentOnTriangle(const triangle left, const vector3 start, const vector3 end);

const struct _triangles Triangles = {
	.Intersects = &Intersects,
	.CalculateNormal = CalculateNormal,
	.SegmentIntersects = IntersectsSegmentOnTriangle
};

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
	vector3 direction = Vector3s.Subtract(end, start);

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

	return intersects && distance <= Vector3s.Distance(start, end);

	//vector3 startVertex;
	//vector3 endVertex;

	//Vectors3CopyTo(start, startVertex);
	//Vectors3CopyTo(end, endVertex);

	//SubtractVectors3(startVertex, triangle[0]);
	//SubtractVectors3(endVertex, triangle[0]);


	//vector3 firstToSecondRay;
	//vector3 firstToThirdRay;

	//Vectors3CopyTo(triangle[1], firstToSecondRay);
	//SubtractVectors3(firstToSecondRay, triangle[0]);

	//Vectors3CopyTo(triangle[2], firstToThirdRay);
	//SubtractVectors3(firstToThirdRay, triangle[0]);

	//vector3 normal;
	//glm_vector3_cross(firstToSecondRay, firstToThirdRay, normal);

	//// x1 x2 nx
	//// y1 y2 ny
	//// z1 z2 nz
	//matrix3 matrix;

	//// GLM IS COLUMN MAJOR
	//// because math i guess
	//Vectors3CopyTo(firstToSecondRay, matrix[0]);
	//Vectors3CopyTo(firstToThirdRay, matrix[1]);
	//Vectors3CopyTo(normal, matrix[2]);

	//glm_matrix3_inv(matrix, matrix);

	//vector3 startInTriangleSpace;
	//glm_matrix3_mulv(matrix, startVertex, startInTriangleSpace);

	//vector3 endInTriangleSpace;
	//glm_matrix3_mulv(matrix, endVertex, endInTriangleSpace);

	//const float zSign = startInTriangleSpace[2] * endInTriangleSpace[2];

	//if (zSign >= 0)
	//{
	//	return false;
	//}

	//const float aX = startInTriangleSpace[0];
	//const float aY = startInTriangleSpace[1];
	//const float aZ = startInTriangleSpace[2];

	//const float bX = endInTriangleSpace[0];
	//const float bY = endInTriangleSpace[1];
	//const float bZ = endInTriangleSpace[2];

	//const float t = bZ / (bZ - aZ);

	//const float negT = (1 - t);

	//const float x = (aX * t) + (negT * bX);
	//const float y = (aY * t) + (negT * bY);

	//return x >= 0.0 && y >= 0 && (x + y) <= 1.0;
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