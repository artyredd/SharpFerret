#include "math/cuboid.h"
#include <float.h>

private cuboid Create(triangle);
private bool Intersects(cuboid, cuboid);
private bool Contains(cuboid, vector3 point);
private cuboid Join(cuboid, cuboid);

const struct _cuboidMethods Cuboids = 
{
	.Minimum = {
		.Center = { 0, 0, 0 },
		.StartVertex = { FLT_MAX, FLT_MAX, FLT_MAX },
		.EndVertex = { FLT_MIN, FLT_MIN, FLT_MIN }
	},
	.Contains = Contains,
	.Create = Create,
	.Intersects = Intersects,
	.Join = Join
};

// generates a new cuboid is the sum of both cuboids
static cuboid Join(cuboid left, cuboid right)
{
	return (cuboid) {
		.StartVertex = {
			.x = min(left.StartVertex.x, right.StartVertex.x),
			.y = min(left.StartVertex.y, right.StartVertex.y),
			.z = min(left.StartVertex.z, right.StartVertex.z),
		},
		.EndVertex = {
			.x = max(left.EndVertex.x, right.EndVertex.x),
			.y = max(left.EndVertex.y, right.EndVertex.y),
			.z = max(left.EndVertex.z, right.EndVertex.z),
		}
	};
}

private float LongestFloat(const triangle triangle, const vector3 centroid)
{
	float result = 0.0;

	float distance1 = Vector3s.Distance(triangle.Point1, centroid);
	float distance2 = Vector3s.Distance(triangle.Point2, centroid);
	float distance3 = Vector3s.Distance(triangle.Point3, centroid);

	return max(distance1, max(distance2, distance3));
}

// gets the 3d cartesian, axis aligned bounding box of the triangle
static cuboid Create(triangle triangle)
{
	const vector3 centroid = Triangles.Centroid(triangle);

	const float radius = LongestFloat(triangle, centroid);

	const vector3 start = {
			.x = centroid.x - radius,
			.y = centroid.y - radius,
			.z = centroid.y - radius,
	};

	const vector3 end = {
			.x = centroid.x + radius,
			.y = centroid.y + radius,
			.z = centroid.y + radius,
	};

	return (cuboid) {
		.StartVertex = start,
		.EndVertex = end,
		.Center = centroid
	};
}

static bool Intersects(cuboid left, cuboid right)
{
	const bool xIntersects = (left.StartVertex.x <= right.EndVertex.x && left.EndVertex.x >= right.StartVertex.x);
	const bool yIntersects = xIntersects && (left.StartVertex.y <= right.EndVertex.y && left.EndVertex.y >= right.StartVertex.y);
	const bool zIntersects = yIntersects && (left.StartVertex.z <= right.EndVertex.z && left.EndVertex.z >= right.StartVertex.z);

	return xIntersects;
}

static bool Contains(cuboid cube, vector3 point)
{

}