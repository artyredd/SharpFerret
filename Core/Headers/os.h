#pragma once

#include "array.h"

struct _osMethods {
	// Gets the current running executable directory
	ARRAY(char) (*ExecutableDirectory)(void);
};

extern const struct _osMethods OperatingSystem;