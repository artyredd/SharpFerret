#include "math/vectors.h"
#include <stdio.h>
#include "singine/file.h"
#include "cunit.h"
#include "string.h"

static bool TryParseVector3(const char* buffer, const size_t length, float* out_vector3);
static bool TrySerializeVec3(char* buffer, const size_t length, const float* vector);
static bool TrySerializeVec3Stream(File stream, const float* vector);

const struct _vector3Methods Vector3s = {
	.TryDeserialize = &TryParseVector3,
	.TrySerialize = &TrySerializeVec3,
	.TrySerializeStream = &TrySerializeVec3Stream
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