#pragma once

struct _physics
{
	void (*Update)(double deltaTime);
};

extern struct _physics Physics;