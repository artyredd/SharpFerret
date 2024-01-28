#pragma once

/// <summary>
/// A rectangle who's fields are floats
/// </summary>
typedef struct _rectangle rect;

struct _rectangle {
	float x;
	float y;
	float Width;
	float Height;
};

/// <summary>
/// A rectangle who's fields are size_t
/// </summary>
typedef struct _size_rectangle size_rect;

struct _size_rectangle {
	size_t x;
	size_t y;
	size_t Width;
	size_t Height;
};

/// <summary>
/// A rectangle who's fields are int
/// </summary>
typedef struct _int_rectangle int_rect;

struct _int_rectangle {
	int x;
	int y;
	int Width;
	int Height;
};