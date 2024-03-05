#pragma once
#include "core/csharp.h"
#include "core/file.h"

typedef struct vector2 vector2;

struct vector2
{
	float x;
	float y;
};

typedef vector2 vector2;

typedef struct vector3 vector3;

struct vector3
{
	vector2;
	float z;
};

typedef vector3 vector3;

typedef struct vector4 vector4;

_declspec(align(16))
struct vector4
{
	vector3;
	float w;
};

typedef struct vector4 vector4;


typedef struct matrix3 matrix3;

_declspec(align(16))
struct matrix3 {
	vector3 Column1;
	vector3 Column2;
	vector3 Column3;
};

typedef struct matrix4 matrix4;

_declspec(align(16))
struct matrix4
{
	vector4 Column1;
	vector4 Column2;
	vector4 Column3;
	vector4 Column4;
};

// Enum of names used to denote a direction
typedef unsigned int Direction;

static struct _directionIndex {
	Direction Zero;
	Direction Left;
	Direction Right;
	Direction Up;
	Direction Down;
	Direction Forward;
	Direction Back;
} Directions = {
	.Zero = 0,
	.Left = 1,
	.Right = 2,
	.Up = 3,
	.Down = 4,
	.Forward = 5,
	.Back = 6
};

static const struct _vectorDirections {
	// { 0, 0, 0 }
	const vector3 Zero;
	// { -1, 0, 0 }
	const vector3 Left;
	// { 1, 0, 0 }
	const vector3 Right;
	// { 0, 1, 0 }
	const vector3 Up;
	// { 0, -1, 0 }
	const vector3 Down;
	// { 0, 0, 1 }
	const vector3 Forward;
	// { 0, 0, -1 }
	const vector3 Back;
} Vector3 = {
	.Zero = { 0, 0, 0 },
	.Left = { -1, 0, 0 },
	.Right = { 1, 0, 0 },
	.Up = { 0, 1, 0 },
	.Down = { 0, -1, 0 },
	.Forward = { 0, 0, 1 },
	.Back = { 0, 0, -1 }
};

struct _vector2Methods {
	bool (*TryDeserialize)(const char* buffer, const ulong length, vector2* out_vector2);
	bool (*TrySerialize)(char* buffer, const ulong length, const vector2 vector);
	bool (*Equals)(const vector2 left, const vector2 right);
	bool (*Close)(const vector2 left, const vector2 right, float epsilon);
};

extern const struct _vector2Methods Vector2s;

struct _vector3Methods {
	bool (*TryDeserialize)(const char* buffer, const ulong length, vector3* out_vector3);
	bool (*TrySerialize)(char* buffer, const ulong length, const vector3 vector);
	bool (*TrySerializeStream)(File stream, const vector3 vector);
	vector3(*Cross)(vector3 left, vector3 right);
	vector3(*Multiply)(const vector3 left, const vector3 right);
	vector3(*Scale)(const vector3 vector, const float value);
	vector3(*Add)(const vector3 left, const vector3 right);
	vector3(*Subtract)(const vector3 left, const vector3 right);
	vector3(*Mean)(const vector3 left, const vector3 right);
	vector3(*MeanArray)(const vector3* array, const ulong count);
	float (*Distance)(const vector3 left, const vector3 right);
	bool (*Equals)(const vector3 left, const vector3 right);
	bool (*Close)(const vector3 left, const vector3 right, float epsilon);
};

extern const struct _vector3Methods Vector3s;

struct _vector4Methods {
	bool (*TryDeserialize)(const char* buffer, const ulong length, vector4* out_vector4);
	bool (*TrySerialize)(char* buffer, const ulong length, const vector4 vector);
	bool (*TrySerializeStream)(File stream, const vector4 vector);
	bool (*Equals)(const vector4 left, const vector4 right);
	bool (*Close)(const vector4 left, const vector4 right, float epsilon);
};

extern const struct _vector4Methods Vector4s;

static struct _matrixConstants {
	matrix4 Identity;
	matrix4 Zero;
} Matrix4 = {
	.Identity = {
		{ 1.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 1.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f},
		{ 0.0f, 0.0f, 0.0f, 1.0f}
	},
	.Zero = {
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 0.0f, 0.0f, 0.0f}
	}
};

struct _mat3Methods
{
	float (*Determinant)(matrix3 matrix);
	matrix3(*Inverse)(matrix3);
	vector3(*MultiplyVector3)(matrix3, vector3);
};

extern const struct _mat3Methods Matrix3s;

struct _mat4Methods {
	matrix4(*LookAt)(vector3 position, vector3 target, vector3 upDirection);
	matrix4(*Inverse)(matrix4);
	matrix4(*Scale)(matrix4, vector3 scale);
	matrix4(*Translate)(matrix4, vector3 position);
	matrix4(*Multiply)(matrix4, matrix4);
	vector3(*MultiplyVector3)(matrix4, vector3, float w);
};

extern const struct _mat4Methods Matrix4s;

typedef struct ivector2 ivector2;

struct ivector2
{
	int x;
	int y;
};

typedef ivector2 ivector2;

typedef struct ivector3 ivector3;

struct ivector3
{
	ivector2;
	int z;
};

typedef ivector3 ivector3;

typedef struct ivector4 ivector4;

_declspec(align(16))
struct ivector4
{
	ivector3;
	int w;
};

typedef struct ivector4 ivector4;
