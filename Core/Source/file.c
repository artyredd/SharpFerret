#include "core/file.h"
#include "core/memory.h"
#include "core/guards.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/os.h"

private bool TryOpen(const string, FileMode fileMode, File* out_file);
private File Open(const string path, FileMode fileMode);
private ulong GetFileSize(const File file);
private array(char) ReadFile(const File file);
private bool TryReadFile(const File file, string* out_data);
private array(char) ReadAll(const string path);
private bool TryReadAll(const string path, string* out_data);
private bool TryReadLine(File file, string buffer, ulong offset, ulong* out_lineLength);
private bool TryGetSequenceCount(File file, const string targetSequence, const string abortSequence, ulong* out_count);
private bool TryClose(File file);
private void Close(File file);
private bool TryVerifyCleanup(void);

const struct _fileMethods Files = {
	.UseAssetDirectories = true,
	.AssetDirectories = nested_stack_array(string,
		stack_string("..\\..\\Singine\\"),
		stack_string("assets\\")
	),
	.TryOpen = &TryOpen,
	.Open = &Open,
	.GetFileSize = &GetFileSize,
	.ReadFile = &ReadFile,
	.TryReadFile = &TryReadFile,
	.ReadAll = &ReadAll,
	.TryReadAll = &TryReadAll,
	.TryReadLine = &TryReadLine,
	.TryGetSequenceCount = &TryGetSequenceCount,
	.TryClose = &TryClose,
	.Close = &Close,
	.TryVerifyCleanup = &TryVerifyCleanup
};

// the number of file handles opened using this file
ulong Global_Files_Opened;
// the number of file handles closed using this file
ulong Global_Files_Closed;

private bool TryOpenInteral(const string path, FileMode fileMode, File* out_file)
{
	// make sure to set the out value always to it's default value first
	*out_file = null;

	if (path is null || fileMode is null || strings.Empty(path))
	{
		return false;
	}

	File file;

#pragma warning(disable: 4189)
	errno_t error = fopen_s(&file, path->Values, fileMode);
#pragma warning(default: 4189)
	if (file is null)
	{
		char buffer[1024];

		strerror_s(buffer, 1024, error);

		fprintf(stderr, "Error opening file %s: %s"NEWLINE, path->Values, buffer);

		return false;
	}

	*out_file = file;

	++(Global_Files_Opened);

	return true;
}


private bool TryOpen(const string path, FileMode fileMode, File* out_file)
{
	if (TryOpenInteral(path, fileMode, out_file))
	{
		return true;
	}


	// check alternate paths
	if (Files.UseAssetDirectories)
	{
		for (int i = 0; i < Files.AssetDirectories->Count; i++)
		{
			const string directory = arrays(string).ValueAt(Files.AssetDirectories, i);

			string newPath = empty_stack_array(char, _MAX_PATH);

			strings.AppendArray(newPath, directory);

			strings.AppendArray(newPath, path);

			if (TryOpenInteral(newPath, fileMode, out_file))
			{
				return true;
			}
		}
	}

	fprintf(stdout, "Is the asset in the right directory? Current working directory: %s"NEWLINE,
		OperatingSystem.ExecutableDirectory()->Values);

	return false;
}


private File Open(const string path, FileMode fileMode)
{
	Guard(strings.Empty(path));
	GuardNotNull(fileMode);

	File file;

	TryOpen(path, fileMode, &file);

	return file;
}

private ulong GetFileSize(const File file)
{
	GuardNotNull(file);

	ulong currentPosition = _ftelli64(file);

	// this isn't the most portable solution but I wrapped this in an abstraction incase I need to diversify the portability later

	// seek to the end of the file and store the length of the file
	if (_fseeki64(file, 0, SEEK_END) != 0)
	{
		throw(FailedToReadFileException);
	}

	ulong count = _ftelli64(file);

	// if we did not start at the beginning of the file we should return to the original position inside of the file
	if (currentPosition != 0)
	{
		rewind(file);

		if (_fseeki64(file, currentPosition, SEEK_SET) is 0)
		{
			throw(FailedToReadFileException);
		}
	}

	return count;
}

private string ReadFile(const File file)
{
	ulong length = GetFileSize(file);

	string result = strings.Create(length + 1);

	result->Values[length] = '\0'; // add line terminator

	rewind(file);

	for (ulong i = 0; i < length; i++)
	{
		int c = fgetc(file);

		if (c is EOF)
		{
			// check to see what error we got
			// if we got 0 then the stream was likely not started at the beginning and we reached EOF early
			int error = ferror(file);

			// if the error we got was actually an error we should return early
			// only attempt rewinding once
			if (error != 0)
			{
				strings.Dispose(result);
				fprintf(stderr, "An error occurred while reading the file at ptr: %llix, Error Code %i", (ulong)file, ferror(file));
				throw(FailedToReadFileException);
			}

			result->Values[i] = '\0';
			safe_increment(result->Count);
			break;
		}

		result->Values[i] = (char)c;
		safe_increment(result->Count);
	}

	return result;
}

private bool TryReadFile(const File file, string* out_data)
{
	*out_data = null;

	const ulong length = GetFileSize(file);

	string result = strings.Create(length + 1);

	result->Values[length] = '\0'; // add line terminator

	rewind(file);

	for (ulong i = 0; i < length; i++)
	{
		int c = fgetc(file);

		if (c is EOF)
		{
			// check to see what error we got
			// if we got 0 then the stream was likely not started at the beginning and we reached EOF early
			int error = ferror(file);

			// if the error we got was actually an error we should return early
			// only attempt rewinding once
			if (error != 0)
			{
				strings.Dispose(result);
				return false;
			}

			result->Values[i] = '\0';
			break;
		}

		result->Values[i] = (char)c;
	}

	*out_data = result;

	return true;
}

private string ReadAll(const string path)
{
	File file;
	if (TryOpen(path, FileModes.Read, &file))
	{
		string data = ReadFile(file);

		if (strings.Empty(data))
		{
			fprintf(stderr, "Failed to read file %s"NEWLINE, path->Values);
			throw(FailedToReadFileException);
		}

		if (TryClose(file) is false)
		{
			fprintf(stderr, "Failed to close the file %s"NEWLINE, path->Values);
			throw(FailedToCloseFileException);
		}

		return data;
	}

	fprintf(stderr, "Failed to open file %s"NEWLINE, path->Values);
	throw(FailedToOpenFileException);

	// we shouldn't be able to get here, but it's possible if __debugbreak() is continued
	return null;
}

private bool TryReadAll(const string path, string* out_data)
{
	File file;

	if (TryOpen(path, FileModes.Read, &file))
	{
		string data;

		if (TryReadFile(file, &data))
		{
			*out_data = data;

			return TryClose(file);
		}

		TryClose(file);
	}

	return false;
}

private bool TryReadLine(File file, string buffer, ulong offset, ulong* out_lineLength)
{
	GuardNotNull(file);

	char* result = fgets(buffer->Values + offset, (int)(buffer->Capacity - offset), file);

	// if eof is encountered before any characters
	if (result is null || ferror(file))
	{
		return false;
	}

	ulong length = strlen(result);

	if (result[length - 1] is '\n' || result[length - 1] is '\r')
	{
		--(length);
		result[length] = '\0';
	}

	if (result[length - 1] is '\n' || result[length - 1] is '\r')
	{
		--(length);
		result[length] = '\0';
	}

	length = max(length, 0);

	*out_lineLength = length;

	return true;
}

private bool TryGetSequenceCount(File file, const string targetSequence, const string abortSequence, ulong* out_count)
{
	GuardNotNull(targetSequence);
	GuardNotNull(abortSequence);

	// store the position that the file currently is in
	const long previousPosition = ftell(file);

	if (previousPosition is - 1L)
	{
		return false;
	}

	// read until either we encounter abortSequence or EOF
	ulong count = 0;
	ulong abortIndex = 0;
	ulong targetIndex = 0;

	int c;
	while ((c = fgetc(file)) != EOF)
	{
		// check to see if we are at the start of the target sequence
		if (c is abortSequence->Values[abortIndex])
		{
			++abortIndex;

			// check to see if we have found the entire sequence
			if (abortIndex >= abortSequence->Count)
			{
				break;
			}
		}
		else
		{
			// if the current character is not part of the target reset 
			abortIndex = 0;
		}

		// check to see if we are at the start of the target sequence
		if (c is targetSequence->Values[targetIndex])
		{
			++targetIndex;

			// check to see if we have found the entire sequence
			if (targetIndex >= targetSequence->Count)
			{
				++count;
				targetIndex = 0;
			}
		}
		else
		{
			// if the current character is not part of the target reset 
			targetIndex = 0;
		}
	}

	if (ferror(file))
	{
		return false;
	}

	// return to beginning
	if (fseek(file, previousPosition, SEEK_SET) != 0)
	{
		return false;
	}

	*out_count = count;

	return true;
}

static bool TryClose(File file)
{
	++(Global_Files_Closed);
	return fclose(file) != EOF;
}

private void Close(File file)
{
	if (TryClose(file) is false)
	{
		throw(FailedToCloseFileException);
	}
}

private bool TryVerifyCleanup(void)
{
	if (Global_Files_Opened != Global_Files_Closed)
	{
		fprintf(stderr, "The number of the files that were opened did not match the number of file handles that were closed: %lli/%lli"NEWLINE, Global_Files_Opened, Global_Files_Closed);
	}

	return Global_Files_Opened == Global_Files_Closed;
}