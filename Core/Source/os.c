#include "core/os.h"

// problematic fucker
#include "windows.h"
// >:(

private array(char) ExecutableDirectory(void);
private array(char) ExecutableDirectorylINUX(void);

extern const struct _osMethods OperatingSystem = {
#ifdef LINUX
	.ExecutableDirectory = ExecutableDirectoryLINUX
#else
	.ExecutableDirectory = ExecutableDirectory
#endif
};

array(char) ExecutableDirectory(void)
{
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);

	array(char) result = Arrays(char).Create(length);

	for (size_t i = 0; i < safe_add(length, 1); i++)
	{
		*Arrays(char).At(result, i) = buffer[i];
	}

	return result;
}