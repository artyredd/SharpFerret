#pragma once

#include "singine/random.h"
#include <float.h>
#include "singine/time.h"

private float NextFloat();
private float BetweenFloat(float lower, float upper);

private bool NextBool(void);
private float NextFloat(void);
private float BetweenFloat(float lower, float upper);
private int NextInt(void);
private int BetweenInt(int lower, int upper);
private size_t NextSize_t(void);
private size_t BetweenSize_t(size_t lower, size_t upper);
private unsigned int NextUInt(void);
private bool Chance(float chance);

struct _randomMethods Random =
{
	.RandomSeedOnStart = true,
	.Seed = 42,
	.NextBool = NextBool,
	.NextFloat = NextFloat,
	.NextInt = NextInt,
	.NextUInt = NextUInt,
	.NextSize_t = NextSize_t,
	.BetweenFloat = BetweenFloat,
	.BetweenInt = BetweenInt,
	.BetweenSize_t = BetweenSize_t
};

bool GLOBAL_HasInitializedRandomNumberGenerator = false;

size_t GLOBAL_PreviousRandSeed = 0;

private bool NextBool(void)
{
	return (NextUInt() % 2) == 0;
}

private float NextFloat(void)
{
	return (float)NextInt() / (float)INT_MAX;
}

private float BetweenFloat(float lower, float upper)
{
	float range = upper - lower;

	return lower + (NextFloat() * range);
}

private unsigned int NextUInt(void)
{
	if (GLOBAL_PreviousRandSeed != Random.Seed)
	{
		if (Random.RandomSeedOnStart && GLOBAL_HasInitializedRandomNumberGenerator is false)
		{
			Random.Seed = (size_t)(1.0 / Time.Time() * DBL_MAX);
		}

		srand(Random.Seed % UINT_MAX);

		GLOBAL_PreviousRandSeed = Random.Seed;

		GLOBAL_HasInitializedRandomNumberGenerator = true;
	}

	return (unsigned int)rand();
}

private int NextInt(void)
{
	return NextBool() ? NextUInt() : -(int)NextUInt();
}

private int BetweenInt(int lower, int upper)
{
	int range = upper - lower;

	int offset = (int)(NextFloat() * (float)range);

	return lower + offset;
}

private size_t NextSize_t(void)
{
	int result = NextInt();

	result |= (size_t)NextInt() << ((sizeof(size_t) / sizeof(int)) * sizeof(char));

	return result;
}

private size_t BetweenSize_t(size_t lower, size_t upper)
{
	size_t range = safe_subtract(upper, lower);

	size_t offset = (size_t)(NextFloat() * (float)range);

	return lower + offset;
}

private bool Chance(float chance)
{
	return NextFloat() <= chance;
}