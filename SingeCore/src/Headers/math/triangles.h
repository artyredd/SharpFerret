#pragma once

#include "math/vectors.h"

typedef struct triangle triangle;

struct triangle {
	vector3 Point1;
	vector3 Point2;
	vector3 Point3;
};

struct _triangles
{
	// calculates the determinant between the point and the triangle
	double (*Determinant)(const triangle triangle,const vector3 point);

	// calculates the normal for the given triangle
	// returns false when the triangle is degenerate
	vector3 (*CalculateNormal)(const triangle triangle);

	// determins whether or not the triangles intersect one another in 3d space
	bool (*Intersects)(const triangle left, const triangle right);

	// determines whether or not a line segment intersects with the given triangle
	bool (*SegmentIntersects)(const triangle left, const vector3 start, const vector3 end);
};

extern const struct _triangles Triangles;