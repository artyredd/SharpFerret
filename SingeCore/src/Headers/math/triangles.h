#pragma once

#include "math/vectors.h"

typedef vec3 triangle[3];

struct _triangles
{
	// calculates the determinant between the point and the triangle
	double (*Determinant)(triangle triangle, vec3 point);

	// calculates the normal for the given triangle
	// returns false when the triangle is degenerate
	void (*CalculateNormal)(const triangle triangle, vec3 out_normal);

	// determins whether or not the triangles intersect one another in 3d space
	bool (*Intersects)(const triangle left, const triangle right);

	// determines whether or not a line segment intersects with the given triangle
	bool (*SegmentIntersects)(const triangle left, const vec3 start, const vec3 end);

	// winds the triangle
	void (*WindTriangle)(triangle triangle);
};

extern const struct _triangles Triangles;