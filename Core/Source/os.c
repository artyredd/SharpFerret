#include "core/os.h"

// problematic fucker
#include "windows.h"
// >:(

private string ExecutableDirectory(void);
private string ExecutableDirectorylINUX(void);

private array(string) GetFilesInDirectory(string path, bool recursive);
private bool IsDirectory(string path);

extern const struct _osMethods OperatingSystem = {
#ifdef LINUX
	.ExecutableDirectory = ExecutableDirectoryLINUX
#else
	.ExecutableDirectory = ExecutableDirectory,
	.GetFilesInDirectory = GetFilesInDirectory,
	.IsDirectory = IsDirectory
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

private bool IsDirectory(const string path)
{
	DWORD attributes = GetFileAttributes(path);

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		printf(stderr, "Error reading file attributes. Error code: %d\n", GetLastError());

		throw(FailedToOpenFileException);
	}

	return (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

private array(string) GetFilesInDirectory(string path, bool recursive)
{
	array(string) result = arrays(string).Create(0);

	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	string searchPath = empty_stack_array(byte, MAX_PATH << 1);

	if (!IsDirectory(path))
	{
		return result;
	}

	// Construct search path
	// From: D:\Files\Directory
	// To__: D:\Files\Directory\*
	strings.AppendArray(searchPath, path);
	strings.AppendCArray(searchPath, "\\*", sizeof("\\*") - 1);

	// Start searching for files
	hFind = FindFirstFile(searchPath->Values, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Error searching directory. Error code: %d\n", GetLastError());
		return;
	}

	// Loop through files in directory
	do {
		if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
			string fileOrDir = empty_stack_array(byte, MAX_PATH << 1);

			strings.AppendArray(fileOrDir, path);
			strings.Append(fileOrDir, '\\');
			strings.AppendCArray(fileOrDir, findFileData.cFileName, strlen(findFileData.cFileName));

			// If it's a directory, recurse into it
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && recursive)
			{
				array(string) recursiveResults = GetFilesInDirectory(fileOrDir, recursive);
				arrays(string).AppendArray(result, recursiveResults);
				arrays(string).Dispose(recursiveResults);
			}
			else
			{
				arrays(string).Append(result, fileOrDir);
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	// Close handle
	FindClose(hFind);

	return result;
}