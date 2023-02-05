#include "math/vectors.h"
#include <stdio.h>
#include "singine/file.h"
#include "cunit.h"
#include "string.h"
#include "cglm/cam.h"
#include "cglm/mat3.h"

static bool TryParseVector3(const char* buffer, const size_t length, float* out_vector3);
static bool TrySerializeVec3(char* buffer, const size_t length, const float* vector);
static bool TrySerializeVec3Stream(File stream, const float* vector);
static void Vector3Set(vec3, float x, float y, float z);
static void Vector3CopyTo(const vec3 source, vec3 destination);
static void Cross(const vec3 left, const vec3 right, vec3 out_result);
static void Multiply(const vec3 left, const vec3 right, vec3 destination);
static void Scale(const vec3 vector, const float value, vec3 destination);
static void Add(const vec3 left, const vec3 right, vec3 destination);
static void Subtract(const vec3 left, const vec3 right, vec3 destination);

const struct _vector3Methods Vector3s = {
	.TryDeserialize = &TryParseVector3,
	.TrySerialize = &TrySerializeVec3,
	.TrySerializeStream = &TrySerializeVec3Stream,
	.Set = Vector3Set,
	.CopyTo = Vector3CopyTo,
	.Cross = &Cross,
	.Multiply = Multiply,
	.Scale  = Scale,
	.Add = Add,
	.Subtract = Subtract
};

static bool TryParseVector2(const char* buffer, const size_t length, float* out_vector2);
static bool TrySerializeVec2(char* buffer, const size_t length, const float* vector);

const struct _vector2Methods Vector2s = {
	.TryDeserialize = &TryParseVector2,
	.TrySerialize = &TrySerializeVec2
};

static bool TryDeserializeVec4(const char* buffer, const size_t length, float* out_vector2);
static bool TrySerializeVec4(char* buffer, const size_t length, const float* vector);
static bool TrySerializeVec4Stream(File stream, const float* vector);

const struct _vector4Methods Vector4s = {
	.TryDeserialize = &TryDeserializeVec4,
	.TrySerialize = &TrySerializeVec4,
	.TrySerializeStream = &TrySerializeVec4Stream
};

static void LookAt(mat4 matrix, vec3 position, vec3 target, vec3 upDirection);
static float Determinant(mat3 matrix);

const struct _matrixMethods Matrices = {
	.LookAt = LookAt,
	.Determinant = Determinant
};

#define Vector2SerializationFormat "%f %f"
#define Vector3SerializationFormat "%f %f %f"
#define Vector4SerializationFormat "%f %f %f %f"

static bool TryParseVector3(const char* buffer, size_t length, float* out_vector3)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, "%f %f %f", &out_vector3[0],
		&out_vector3[1],
		&out_vector3[2]);

	return count == 3;
}

static bool TryParseVector2(const char* buffer, size_t length, float* out_vector2)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, "%f %f",
		&out_vector2[0],
		&out_vector2[1]);

	return count == 2;
}

static void Vector3Set(vec3 vector, float x, float y, float z)
{
	vector[0] = x;
	vector[1] = y;
	vector[2] = z;
}

static void Vector3CopyTo(const vec3 source, vec3 destination)
{
	Vectors3CopyTo(source, destination);
}

static void Cross(const vec3 left, const vec3 right, vec3 destination)
{
#pragma warning (disable : 4090)
	glm_cross(left, right, destination);
#pragma warning (default : 4090)
}

static void Multiply(const vec3 left, const vec3 right, vec3 destination)
{
#pragma warning (disable : 4090)
	glm_vec3_mul(left, right, destination);
#pragma warning (default : 4090)
}

static void Add(const vec3 left, const vec3 right, vec3 destination)
{
	destination[0] = left[0] + right[0];
	destination[1] = left[1] + right[1];
	destination[2] = left[2] + right[2];
}

static void Subtract(const vec3 left, const vec3 right, vec3 destination)
{
	destination[0] = left[0] - right[0];
	destination[1] = left[1] - right[1];
	destination[2] = left[2] - right[2];
}

static void Scale(const vec3 vector, float value, vec3 destination)
{
	destination[0] = vector[0] * value;
	destination[1] = vector[1] * value;
	destination[2] = vector[2] * value;
}

static bool TrySerializeVec3Stream(File stream, const float* vector)
{
	if (stream is null)
	{
		return false;
	}

	int result = fprintf_s(stream, Vector3SerializationFormat,
		vector[0],
		vector[1],
		vector[2]);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec3(char* buffer, const size_t length, const float* vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector3SerializationFormat, vector[0], vector[1], vector[2]);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec2(char* buffer, const size_t length, const float* vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector2SerializationFormat, vector[0], vector[1]);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TryDeserializeVec4(const char* buffer, const size_t length, float* out_vector4)
{
	if (length is 0)
	{
		return false;
	}

	int count = sscanf_s(buffer, Vector4SerializationFormat, &out_vector4[0],
		&out_vector4[1],
		&out_vector4[2],
		&out_vector4[3]);

	return count == 4;
}

static bool TrySerializeVec4Stream(File stream, const float* vector)
{
	if (stream is null)
	{
		return false;
	}

	int result = fprintf_s(stream, Vector4SerializationFormat,
		vector[0],
		vector[1],
		vector[2],
		vector[3]);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool TrySerializeVec4(char* buffer, const size_t length, const float* vector)
{
	if (length is 0)
	{
		return false;
	}

	int result = sprintf_s(buffer, length, Vector4SerializationFormat,
		vector[0],
		vector[1],
		vector[2],
		vector[3]);

	// 0 is runtime error, negative is encoding error
	return result > 0;
}

static bool Test_TryGetVector3(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vec3 expected = { -1.0f, 2.4f, 4.90f };

	vec3 actual;

	Assert(TryParseVector3(buffer, strlen(buffer), actual));

	Assert(Vector3Equals(expected, actual));

	return true;
}

static bool Test_TryGetVector2(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vec2 expected = { -1.0f, 2.4f };

	vec2 actual;

	Assert(TryParseVector2(buffer, strlen(buffer), actual));

	Assert(Vector2Equals(expected, actual));

	return true;
}

bool RunVectorUnitTests()
{
	TestSuite suite = CreateSuite(__FILE__);

	suite->Append(suite, "TryGetVector3", Test_TryGetVector3);
	suite->Append(suite, "TryGetVector2", Test_TryGetVector2);

	bool pass = suite->Run(suite);

	suite->Dispose(suite);

	return pass;
}

static float Determinant(mat3 matrix)
{
	return glm_mat3_det(matrix);
}

static void LookAt(mat4 matrix, vec3 position, vec3 target, vec3 upDirection)
{
	glm_look(position, target, upDirection, matrix);
}