#pragma once
#include "core/csharp.h"

struct _testMethods
{
	void (*RunAll)(void);
	void (*MaterialTests)(void);
	void (*ShaderTests)(void);
	void (*TriangleTests)(void);
};

extern const struct _testMethods Tests;