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
/// A rectangle who's fields are ulong
/// </summary>
typedef struct _size_rectangle size_rect;

struct _size_rectangle {
	ulong x;
	ulong y;
	ulong Width;
	ulong Height;
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