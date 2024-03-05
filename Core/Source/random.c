#pragma once

#include "core/random.h"
#include <float.h>
//#include "core/time.h"

private float NextFloat();
private float BetweenFloat(float lower, float upper);

private bool NextBool(void);
private float NextFloat(void);
private float BetweenFloat(float lower, float upper);
private int NextInt(void);
private int BetweenInt(int lower, int upper);
private ulong Nextulong(void);
private ulong Betweenulong(ulong lower, ulong upper);
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
	.Nextulong = Nextulong,
	.BetweenFloat = BetweenFloat,
	.BetweenInt = BetweenInt,
	.Betweenulong = Betweenulong,
	.Chance = Chance
};

bool GLOBAL_HasInitializedRandomNumberGenerator = false;

ulong GLOBAL_PreviousRandSeed = 0;

private bool NextBool(void)
{
	return (NextUInt() % 2) == 0;
}

private float NextFloat(void)
{
	return ((float)NextInt() / (float)(RAND_MAX));
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
			Random.Seed = (ulong)(1.0 / /*Time.Time()*/ 42.0 * DBL_MAX);
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

private ulong Nextulong(void)
{
	int result = NextInt();

	result |= (ulong)NextInt() << ((sizeof(ulong) / sizeof(int)) * sizeof(char));

	return result;
}

private ulong Betweenulong(ulong lower, ulong upper)
{
	ulong range = safe_subtract(upper, lower);

	ulong offset = Nextulong() % range;

	return lower + offset;
}

private bool Chance(float chance)
{
	return NextFloat() <= chance;
}