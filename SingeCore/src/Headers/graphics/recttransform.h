#pragma once

#include "graphics/transform.h"

typedef struct _vec2 Anchor;
typedef struct _vec2 Pivot;

struct _vec2 {
	float x;
	float y;
};

static struct _anchors {
	Anchor Center;
	Anchor Left;
	Anchor Right;
	Anchor Up;
	Anchor Down;
	Anchor UpperRight;
	Anchor UpperLeft;
	Anchor LowerRight;
	Anchor LowerLeft;
} Anchors = {
	.Center = { 0,0 },
	.Left = { -1,0 },
	.Right = { 1,0 },
	.Up = { 0,1 },
	.Down = { 0,-1 },
	.UpperRight = { 1,1 },
	.UpperLeft = { -1,1 },
	.LowerRight = { 1,-1 },
	.LowerLeft = { -1,-1 }
};

static struct _anchors Pivots = {
	.Center = { 0,0 },
	.Left = { -1,0 },
	.Right = { 1,0 },
	.Up = { 0,1 },
	.Down = { 0,-1 },
	.UpperRight = { 1,1 },
	.UpperLeft = { -1,1 },
	.LowerRight = { 1,-1 },
	.LowerLeft = { -1,-1 }
};

struct _rectTransformMethods {
	void (*SetTransform)(Transform, Anchor anchor, Pivot pivot, float x, float y, float width, float height);
};

extern const struct _rectTransformMethods RectTransforms;