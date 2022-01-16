#pragma once
#include "csharp.h"

struct _testMethods
{
	void (*RunAll)(void);
	void (*MaterialTests)(void);
	void (*ShaderTests)(void);
};

extern const struct _testMethods Tests;