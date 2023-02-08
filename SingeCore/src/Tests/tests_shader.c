#include "tests.h"
#include "cunit.h"
#include "graphics/shadercompiler.h"
#include "graphics/shaders.h"
#include "singine/memory.h"

// ignore param warning
#pragma warning(disable: 4100)
static bool Test_Shader_Leak_Create(File stream)
{
	ResetAlloc();
	ResetFree();

	Shader shader = Shaders.CreateEmpty();

	Shaders.Dispose(shader);

	IsEqual(FreeCount(), AllocCount(), "%lli");

	shader = Shaders.Create();

	Shaders.Dispose(shader);

	IsEqual(FreeCount(), AllocCount(), "%lli");

	return true;
}

static bool Test_Shader_Leak_Instance(File stream)
{
	ResetAlloc();
	ResetFree();

	Shader shader = Shaders.CreateEmpty();

	for (size_t i = 0; i < 100; i++)
	{
		Shader instance = Shaders.Instance(shader);

		Shaders.Dispose(instance);
	}

	Shaders.Dispose(shader);

	IsEqual(FreeCount(), AllocCount(), "%lli");

	shader = Shaders.Create();

	for (size_t i = 0; i < 100; i++)
	{
		Shader instance = Shaders.Instance(shader);

		Shaders.Dispose(instance);
	}

	Shaders.Dispose(shader);

	IsEqual(FreeCount(), AllocCount(), "%lli");

	return true;
}

static void ShaderTests(void)
{
	TestSuite suite = CreateSuite("ShaderTests");

	suite->Append(suite, "Create", &Test_Shader_Leak_Create);
	suite->Append(suite, "Instance", &Test_Shader_Leak_Instance);

	suite->Run(suite);

	suite->Dispose(suite);
}
#pragma warning(default: 4100)