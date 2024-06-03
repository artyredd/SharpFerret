#pragma once

#include "array.h"

struct _osMethods {
	// Gets the current running executable directory
	string(*ExecutableDirectory)(void);
	array(string) (*GetFilesInDirectory)(string path, bool recursive);
	bool (*IsDirectory)(string path);
	// Gets the logical processor count (thread count) from the system
	int (*ThreadCount)();
};

extern const struct _osMethods OperatingSystem;