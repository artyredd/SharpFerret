#pragma once
#include "csharp.h"

#ifndef cglm_types_h
typedef _declspec(align(16)) float vec4[4];
typedef float vec3[3];
typedef float vec2[2];
#endif // !cglm_h

#ifndef cglm_mat_h
#define GLM_MAT4_IDENTITY_INIT  {{1.0f, 0.0f, 0.0f, 0.0f},                    \
                                 {0.0f, 1.0f, 0.0f, 0.0f},                    \
                                 {0.0f, 0.0f, 1.0f, 0.0f},                    \
                                 {0.0f, 0.0f, 0.0f, 1.0f}}

#define GLM_MAT4_ZERO_INIT      {{0.0f, 0.0f, 0.0f, 0.0f},                    \
                                 {0.0f, 0.0f, 0.0f, 0.0f},                    \
                                 {0.0f, 0.0f, 0.0f, 0.0f},                    \
                                 {0.0f, 0.0f, 0.0f, 0.0f}}
typedef _declspec(align(16)) vec4 mat4[4];
#endif // !cglm_mat_h

#define Vector2Equals(left,right) (left[0] ==  right[0] && left[1] == right[1])

#define Vector3Equals(left,right) (left[0] ==  right[0] && left[1] == right[1] && left[2] == right[2])

#define Vector4Equals(left,right) (left[0] ==  right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3])

#define Vector3MembersEqual(vector,x,y,z) (vector[0] ==  x && vector[1] == y && vector[2] == z)

#define SetX(vector, x) vector[0] = x
#define SetY(vector, y) vector[1] = y
#define SetZ(vector, z) vector[2] = z
#define SetW(vector, w) vector[3] = w

#define AddX(vector, x) vector[0] += x
#define AddY(vector, y) vector[1] += y
#define AddZ(vector, z) vector[2] += z
#define AddW(vector, w) vector[3] += w

#define SetVector2(vec2, x, y) vec2[0] = x; vec2[1] = y
#define SetVector3(vec3, x, y, z) vec3[0] = x; vec3[1] = y; vec3[2] = z
#define SetVector4(vec4, x, y, z, w) vec4[0] = x; vec4[1] = y; vec4[2] = z; vec4[3] = w

#define AddVector3(vec3, x, y, z) vec3[0] += x; vec3[1] += y; vec3[2] += z
#define AddVector4(vec4, x, y, z, w) vec4[0] += x; vec4[1] += y; vec4[2] += z; vec4[3] += w

#define SetVectors2(left,right) left[0] = right[0]; left[1] = right[1]
#define SetVectors3(left,right) left[0] = right[0]; left[1] = right[1]; left[2] = right[2]
#define SetVectors4(left,right) left[0] = right[0]; left[1] = right[1]; left[2] = right[2]; left[3] = right[3]

#define Vectors2CopyTo(left,right) SetVectors2((right), (left));
#define Vectors3CopyTo(left,right) SetVectors3((right),(left))
#define Vectors4CopyTo(left,right) SetVectors4((right),(left))

#define SubtractVectors3(left,right) left[0] -= right[0]; left[1] -= right[1]; left[2] -= right[2]

// mutates the left vector with the right's values
#define AddVectors3(left,right) left[0] += right[0]; left[1] += right[1]; left[2] += right[2]
#define AddVectors4(left,right) left[0] += right[0]; left[1] += right[1]; left[2] += right[2]; left[3] += right[3]

#define ScaleVector3(vector,scale) vector[0] *= scale;vector[1] *= scale;vector[2] *= scale

#define MultiplyVectors3(left, right) left[0] *= right[0]; left[1] *= right[1]; left[2] *= right[2]

#define InitializeVector3(vec3) SetVector3(vec3,0,0,0);
#define InitializeVector4(vec3) SetVector4(vec3,0,0,0,0);
#define InitializeMat4(mat4) glm_mat4_identity(mat4)

#define SetMatrices4(left,right) SetVectors4(left[0],right[0]);SetVectors4(left[1],right[1]);SetVectors4(left[2],right[2]);SetVectors4(left[3],right[3])

#define FLOAT_FORMAT "%0.2f"

#define PrintVector3(vector,stream) fprintf(stream, "["FLOAT_FORMAT","FLOAT_FORMAT","FLOAT_FORMAT"]", vector[0], vector[1], vector[2]);

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

static struct _vectorDirections {
	vec3 Zero;
	vec3 Left;
	vec3 Right;
	vec3 Up;
	vec3 Down;
	vec3 Forward;
	vec3 Back;
} Vector3 = {
	.Zero = { 0, 0, 0},
	.Left = { -1, 0, 0},
	.Right = { 1, 0, 0},
	.Up = { 0, 1, 0},
	.Down = { 0, -1, 0},
	.Forward = { 0, 0, 1},
	.Back = { 0, 0, -1}
};

static struct _matrixConstants {
	mat4 Identity;
	mat4 Zero;
} Matrix4 = {
	.Identity = GLM_MAT4_IDENTITY_INIT,
	.Zero = GLM_MAT4_ZERO_INIT
};

bool TryParseVector3(char* buffer, vec3 out_vector);

bool TryParseVector2(char* buffer, vec3 out_vector);

bool RunVectorUnitTests();