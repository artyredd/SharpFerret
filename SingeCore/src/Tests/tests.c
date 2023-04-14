#include "tests.h"
#include "csharp.h"
#include "singine/memory.h"

#include "Tests/tests_material.c"
static void MaterialTests(void);

#include "Tests/tests_shader.c"
static void ShaderTests(void);

#include "Tests/tests_triangles.c"
static void TriangleTests(void);

#include "math/voxel.h"

static void RunAll(void);

const struct _testMethods Tests = {
	.RunAll = &RunAll,
	.MaterialTests = &MaterialTests,
	.ShaderTests = &ShaderTests,
	.TriangleTests = &TriangleTests
};

static void RunAll(void)
{
	MaterialTests();
	ShaderTests();
	TriangleTests();
	RunVoxelUnitTests();
	// reset alloc and free so we get accurate runtime numbers
	ResetAlloc();
	ResetFree();
}