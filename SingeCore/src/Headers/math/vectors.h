#pragma once
#include "csharp.h"

#ifndef cglm_h
typedef float vec4[4];
typedef float vec3[3];
typedef float vec2[2];
#endif // !cglm_h

typedef int intvec4[4];
typedef int intvec3[3];
typedef int intvec2[2];

typedef struct _face* Face;

#define Vector2Equals(left,right) (left[0] ==  right[0] && left[1] == right[1])

#define Vector3Equals(left,right) (left[0] ==  right[0] && left[1] == right[1] && left[2] == right[2])

static void CopyTo(const float* source, float* destination, const size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		destination[i] = source[i];
	}
}

#define Vector3CopyTo(source,destination) CopyTo(source,destination, 3)
#define Vector2CopyTo(source,destination) CopyTo(source,destination, 2)

bool TryParseVector3(char* buffer, vec3 out_vector);

bool TryParseVector2(char* buffer, vec3 out_vector);

bool RunVectorUnitTests();