#pragma once

#include "math/vectors.h"
#include "math/triangles.h"

typedef struct _cuboid cuboid;

struct _cuboid {
	// the point at the center of the cube used for sorting
	vector3 Center;
	// The Vertex who's coordinates are generated from the minimum x,y,z values from a triangles points
	vector3 StartVertex;
	// The vertex whos coordinates are generated from the maximum x,y,z values from a triangles points
	vector3 EndVertex;
};

struct _cuboidMethods {
	// An inverted maximum sized cuboid;
	cuboid Minimum;
	cuboid (*Create)(triangle);
	// checks to see if the cuboids intersect inclusive
	bool (*Intersects)(cuboid, cuboid);
	bool (*Contains)(cuboid, vector3 point);
	cuboid(*AddOffset)(cuboid left, vector3 offset);
	cuboid(*Join)(cuboid, cuboid);
};

extern const struct _cuboidMethods Cuboids;