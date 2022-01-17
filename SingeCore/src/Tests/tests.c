#include "tests.h"
#include "csharp.h"
#include "singine/memory.h"

#include "Tests/tests_material.c"
static void MaterialTests(void);

#include "Tests/tests_shader.c"
static void ShaderTests(void);

static void RunAll(void);

const struct _testMethods Tests = {
	.RunAll = &RunAll,
	.MaterialTests = &MaterialTests,
	.ShaderTests = &ShaderTests
};

static void RunAll(void)
{
	MaterialTests();
	ShaderTests();
	// reset alloc and free so we get accurate runtime numbers
	ResetAlloc();
	ResetFree();
}