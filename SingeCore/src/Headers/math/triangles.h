#pragma once

#include "math/vectors.h"

typedef vec3 triangle[3];

struct _triangles
{
	// calculates the determinant between the point and the triangle
	double (*Determinant)(triangle triangle, vec3 point);

	// determins whether or not the triangles intersect one another in 3d space
	bool (*Intersects)(triangle left, triangle right);
};

extern const struct _triangles Triangles;