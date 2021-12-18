#include "math/vectors.h"
#include <stdio.h>
#include "singine/file.h"
#include "cunit.h"


bool TryParseVector3(char* buffer, float* out_vector)
{
	float x, y, z;

	int count = sscanf_s(buffer, "%f %f %f", &x, &y, &z);

	if (count != 3)
	{
		return false;
	}

	out_vector[0] = x;
	out_vector[1] = y;
	out_vector[2] = z;

	return true;
}

bool TryParseVector2(char* buffer, float* out_vector)
{
	float x, y;

	int count = sscanf_s(buffer, "%f %f", &x, &y);

	if (count != 2)
	{
		return false;
	}

	out_vector[0] = x;
	out_vector[1] = y;

	return true;
}

static bool Test_TryGetVector3(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vec3 expected = { -1.0f, 2.4f, 4.90f };

	vec3 actual;

	Assert(TryParseVector3(buffer, actual));

	Assert(Vector3Equals(expected, actual));

	return true;
}

static bool Test_TryGetVector2(File stream)
{
	char* buffer = "-1.0 2.4 4.90";

	vec2 expected = { -1.0f, 2.4f };

	vec2 actual;

	Assert(TryParseVector2(buffer, actual));

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