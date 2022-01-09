#include "graphics/recttransform.h"
#include "singine/memory.h"

static void SetTransform(Transform, Anchor anchor, Anchor pivot, float x, float y, float width, float height);

const struct _rectTransformMethods RectTransforms = {
	.SetTransform = &SetTransform
};

static void SetTransform(Transform transform, Anchor anchor, Anchor pivot, float x, float y, float width, float height)
{
	Transforms.SetScales(transform, width, height, 1);

	float posx = (anchor.x / width) + (x - pivot.x);
	float posy = (anchor.y / height) + (y - pivot.y);

	Transforms.SetPositions(transform, posx, posy, 0);
}