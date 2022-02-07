#pragma once

#include "csharp.h"

// When always binary is defined all read and write modes are post-fixed with "b" to enable binary
#define ALWAYS_BINARY

#ifdef ALWAYS_BINARY
#define AlwaysBinaryPostfix "b"
#else
#define AlwaysBinaryPostfix ""
#endif

typedef const char* FileMode;

static struct _fileModes {
	FileMode Read;
	FileMode Create;
	FileMode Append;
	FileMode ReadWrite;
	FileMode ReadAppend;
	FileMode ReadBinary;
} FileModes = {
	.Read = "r"AlwaysBinaryPostfix,
	.Create = "w"AlwaysBinaryPostfix,
	.Append = "a"AlwaysBinaryPostfix,
	.ReadWrite = "r+"AlwaysBinaryPostfix,
	.ReadAppend = "a+"AlwaysBinaryPostfix,
	.ReadBinary = "rb"
};

typedef FILE* File;

struct _fileMethods {
	/// <summary>
	/// Attemps to open the file at the provided path with the requested file permissions
	/// </summary>
	/// <param name="name">The file path of the file that you want to be opened</param>
	/// <param name="fileMode">The FileMode that you want the file opened with see file.h/FileModes for options(standard C options)</param>
	/// <param name="out_file">OUT PARAMETER: the variable your want to be set with the opened file, or null if it was not opened</param>
	/// <returns>true if the file was opened successfully, otherwise false</returns>
	bool (*TryOpen)(const char* path, FileMode fileMode, File* out_file);

	File(*Open)(const char* path, FileMode fileMode);

	size_t(*GetFileSize)(const File file);

	char* (*ReadFile)(const File file);

	bool (*TryReadFile)(const File file, char** out_data);

	char* (*ReadAll)(const char* path);

	bool (*TryReadAll)(const char* path, char** out_data);

	bool (*TryReadLine)(File file, char* buffer, size_t offset, size_t bufferLength, size_t* out_lineLength);

	/// <summary>
	/// Attempts to look forwards into the file and count the number of times targetSequence is encountered, counting is stopped when end of file is reached
	/// or when the abortsequence is encountered
	/// </summary>
	/// <returns></returns>
	bool (*TryGetSequenceCount)(File file, const char* targetSequence, const size_t targetLength, const char* abortSequence, const size_t abortLength, size_t* out_count);

	bool (*TryClose)(File file);

	void (*Close)(File file);

	/// <summary>
	/// Determines if all files that were opened were closed appropriately
	/// </summary>
	bool (*TryVerifyCleanup)(void);
};

extern const struct _fileMethods Files;

