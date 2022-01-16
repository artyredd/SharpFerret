#include "graphics/material.h"
#include "cunit.h"
#include "singine/memory.h"

// ignore param warning
#pragma warning(disable: 4100)
static bool Test_LeakBasic_Create(File stream)
{
	ResetAlloc();
	ResetFree();

	Material material = Materials.Create(null, null);

	Materials.Dispose(material);

	Equals(FreeCount(), AllocCount(), "%lli");

	return true;
}

static bool Test_Leak_Instance(File stream)
{
	ResetAlloc();
	ResetFree();

	Material material = Materials.CreateMaterial();

	//no matter how many copies we should not leak
	for (size_t i = 0; i < 100; i++)
	{
		Material instance = Materials.Instance(material);

		Materials.Dispose(instance);
	}

	Materials.Dispose(material);

	Equals(FreeCount(), AllocCount(), "%lli");

	return true;
}

static void MaterialTests(void)
{
	TestSuite suite = CreateSuite("MaterialTests");

	suite->Append(suite, "Create", &Test_LeakBasic_Create);
	suite->Append(suite, "Instance", &Test_Leak_Instance);

	suite->Run(suite);

	suite->Dispose(suite);
}
#pragma warning(default: 4100)