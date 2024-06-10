#pragma once

#include "core/csharp.h"
#include "array.h"

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
	// whether or not to automatically check backup directories if opening a file fails
	bool UseAssetDirectories;
	// Directories to automatically check when opening a file fails
	array(string) AssetDirectories;
	/// <summary>
	/// Attemps to open the file at the provided path with the requested file permissions
	/// </summary>
	/// <param name="name">The file path of the file that you want to be opened</param>
	/// <param name="fileMode">The FileMode that you want the file opened with see file.h/FileModes for options(standard C options)</param>
	/// <param name="out_file">OUT PARAMETER: the variable your want to be set with the opened file, or null if it was not opened</param>
	/// <returns>true if the file was opened successfully, otherwise false</returns>
	bool (*TryOpen)(const string path, FileMode fileMode, File* out_file);

	File(*Open)(const string path, FileMode fileMode);

	ulong(*GetFileSize)(const File file);

	string(*ReadFile)(const File file);
	int (*ReadUntil)(const File file, string output, byte target);

	// reads the provided fle stream into a NEW string and sets the out
	// values to the string.
	bool (*TryReadFile)(const File file, string* out_data);

	string(*ReadAll)(const string path);

	// writes the given string to the provided path, creates the file
	// if non existing, otherwise overwrites file contents
	void (*WriteAll)(const string path, const string data);

	// reads the provided path into a NEW string and sets the out value
	// to the string
	bool (*TryReadAll)(const string path, string* out_data);

	// reads the file into buffer at the given offset
	// DOES NOT MODIFY buffer count while reading
	// returns true when successful and outputs line length that was read
	// (if you want to add to the buffer count manually)
	bool (*TryReadLine)(File file, string buffer, ulong offset, ulong* out_lineLength);

	/// <summary>
	/// Attempts to look forwards into the file and count the number of times targetSequence is encountered, counting is stopped when end of file is reached
	/// or when the abortsequence is encountered
	/// </summary>
	/// <returns></returns>
	bool (*TryGetSequenceCount)(File file, const string targetSequence, const string abortSequence, ulong* out_count);

	bool (*TryClose)(File file);

	// closes and disposes the file
	void (*Dispose)(File);

	void (*Close)(File file);

	/// <summary>
	/// Determines if all files that were opened were closed appropriately
	/// </summary>
	bool (*TryVerifyCleanup)(void);
};

extern const struct _fileMethods Files;

