#pragma once

#include "graphics/material.h"

struct _drawing
{
	void (*SetScene)(Scene scene);
	void (*DrawTriangle)(const float* triangle, Material material);
};

extern const struct _drawing Drawing;