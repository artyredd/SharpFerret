#pragma once

typedef struct _size_vec2 size_vec2;

struct _size_vec2 {
	size_t x;
	size_t y;
};

static const struct _Vector2Constants {
	size_vec2 Empty;
} Vector2 = {
	{0,0}
};