#include "singine/os.h"

// problematic fucker
#include "windows.h"
// >:(

private ARRAY(char) ExecutableDirectory(void);

extern const struct _osMethods OperatingSystem = {
	.ExecutableDirectory = ExecutableDirectory
};

ARRAY(char) ExecutableDirectory(void)
{
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	ARRAY(char) result = ARRAYS(char).Create(length);

	for (size_t i = 0; i < safe_add(length, 1); i++)
	{
		*ARRAYS(char).At(result, i) = buffer[i];
	}

	return result;
}