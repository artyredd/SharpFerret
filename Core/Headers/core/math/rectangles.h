#pragma once

#include "core/csharp.h"
#include "core/math/vectors.h"
#include "core/array.h"

// Top left oriented system
// x,y represents top left corner of square
struct irectangle
{
	int Height;
	int Width;
	ivector2;
};

typedef struct irectangle irectangle;

DEFINE_CONTAINERS(irectangle);

extern const struct _rectangleMethods
{
	// Returns true when the first rectangle contains the second
	bool (*Contains)(irectangle, irectangle);
	// Returns true when the first triangle can fit the )second within its bounds
	bool (*CanFit)(irectangle, irectangle);
	tuple(irectangle, irectangle) (*BisectRectAlongXAxis)(irectangle rect, int xBisection);
	tuple(irectangle, irectangle) (*BisectRectAlongYAxis)(irectangle rect, int xBisection);
	tuple(irectangle, irectangle) (*SubtractRectangle)(irectangle source, irectangle value);
	void (*RunUnitTests)();
} Rectangles;