#include "core/os.h"

// problematic fucker
#include "windows.h"
// >:(

private string ExecutableDirectory(void);
private string ExecutableDirectorylINUX(void);

extern const struct _osMethods OperatingSystem = {
#ifdef LINUX
	.ExecutableDirectory = ExecutableDirectoryLINUX
#else
	.ExecutableDirectory = ExecutableDirectory
#endif
};

array(byte) ExecutableDirectory(void)
{
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	array(byte) result = arrays(byte).Create(length);

	for (ulong i = 0; i < safe_add(length, 1); i++)
	{
		*arrays(byte).At(result, i) = buffer[i];
	}

	return result;
}