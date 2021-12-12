#pragma once

#include "csharp.h"

typedef const char* FileMode;

static struct _fileModes {
	FileMode Read;
	FileMode Create;
	FileMode Append;
	FileMode ReadWrite;
	FileMode ReadAppend;
} FileModes = {
	"r", "w", "a", "r+", "a+"
};

typedef FILE* File;

/// <summary>
/// Attemps to open the file at the provided path with the requested file permissions
/// </summary>
/// <param name="name">The file path of the file that you want to be opened</param>
/// <param name="fileMode">The FileMode that you want the file opened with see file.h/FileModes for options(standard C options)</param>
/// <param name="out_file">OUT PARAMETER: the variable your want to be set with the opened file, or null if it was not opened</param>
/// <returns>true if the file was opened successfully, otherwise false</returns>
bool TryOpen(const char* path, FileMode fileMode, File* out_file);

File Open(const char* path, FileMode fileMode);

size_t GetFileSize(const File file);

char* ReadFile(const File file);

char* ReadAll(char* path);

bool TryClose(File file);