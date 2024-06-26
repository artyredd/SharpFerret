#include "core/os.h"

// problematic fucker
#include "windows.h"
// >:(

private string ExecutableDirectory(void);
private string ExecutableDirectorylINUX(void);

private array(string) GetFilesInDirectory(string path, bool recursive);
private bool IsDirectory(string path);
private int ThreadCount();
private void PrintLastError(void* stream);

extern const struct _osMethods OperatingSystem = {
#ifdef LINUX
	.ExecutableDirectory = ExecutableDirectoryLINUX
#else
	.ExecutableDirectory = ExecutableDirectory,
	.GetFilesInDirectory = GetFilesInDirectory,
	.IsDirectory = IsDirectory,
	.ThreadCount = ThreadCount,
	.PrintLastError = PrintLastError
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

private void PrintError(int error)
{
#ifdef LINUX
	/* Do nothing for now */
#else
	switch (error)
	{
	case 0x7B /* 123 */:
		fprintf(stderr, "The filename, directory name, or volume label syntax is incorrect.\n");
	default:
		fprintf(stderr, "Error reading file attributes. Error code: %d\n", GetLastError());
	}
#endif
}

private bool IsDirectory(const string path)
{
#ifdef LINUX
	/* Do nothing for now */
#else
	DWORD attributes = GetFileAttributes(path->Values);

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		PrintError(GetLastError());
		throw(FailedToOpenFileException);
	}

	return (attributes & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

private string MaybeAppendCharacter(string path, byte c)
{
	if (path->Count is 0)
	{
		strings.Append(path, c);
		return path;
	}

	if (at(path, path->Count - 1) isnt c)
	{
		strings.Append(path, c);
	}

	return path;
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
	MaybeAppendCharacter(searchPath, '\\');
	MaybeAppendCharacter(searchPath, '*');

	// Start searching for files
	hFind = FindFirstFile(searchPath->Values, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error searching directory. Error code: %d\n", GetLastError());
		throw(FailedToReadFileException);
	}

	// Loop through files in directory
	do {
		if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0) {
			string fileOrDir = empty_stack_array(byte, MAX_PATH << 1);

			strings.AppendArray(fileOrDir, path);
			MaybeAppendCharacter(fileOrDir, '\\');
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
				arrays(string).Append(result, strings.Clone(fileOrDir));
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	// Close handle
	FindClose(hFind);

	return result;
}

static SYSTEM_INFO SystemInfo;
static bool CachedSystemInfo;

private int ThreadCount()
{
	if (CachedSystemInfo is false)
	{
		GetSystemInfo(&SystemInfo);
		CachedSystemInfo = true;
	}

	return SystemInfo.dwNumberOfProcessors;
}

private void PrintLastError(void* stream)
{
	// Retrieve the last error code
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0) {
		// No error
		return;
	}

	// Allocate a buffer for the error message
	LPSTR messageBuffer = NULL;

	// Format the error message
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorMessageID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer,
		0,
		NULL
	);

	// Print the error message to stdout
	if (size) {
		fprintf(stream, "Error: %s\n", messageBuffer);
	}
	else {
		fprintf(stream, "Error code: %lu\n", errorMessageID);
	}

	// Free the buffer allocated by FormatMessage
	if (messageBuffer) {
		LocalFree(messageBuffer);
	}
}