#include "singine/file.h"
#include "singine/memory.h"
#include "singine/guards.h"

#define NotNull(variableName) if (variableName is null) { fprintf(stderr, #variableName"can not be null"); throw(InvalidArgumentException); }

bool TryOpen(const char* path, FileMode fileMode, File* out_file)
{
	// make sure to set the out value always to it's default value first
	*out_file = null;

	if (path is null || fileMode is null)
	{
		return false;
	}

	File file;

	errno_t error = fopen_s(&file, path, fileMode);

	if (file is null)
	{
		return false;
	}

	*out_file = file;

	return true;
}

File Open(const char* path, FileMode fileMode)
{
	NotNull(path);
	NotNull(fileMode);

	File file;

	errno_t error = fopen_s(&file, path, fileMode);

	if (file is null)
	{
		fprintf(stderr, "Failed to open the file %s ErrorCode: %i"NEWLINE, path, error);
		throw(FileNotFoundException);
	}

	return file;
}

size_t GetFileSize(const File file)
{
	NotNull(file);

	long currentPosition = ftell(file);

	// this isn't the most portable solution but I wrapped this in an abstraction incase I need to diversify the portability later

	// seek to the end of the file and store the length of the file
	fseek(file, 0L, SEEK_END);

	size_t count = ftell(file);

	// if we did not start at the beginning of the file we should return to the original position inside of the file
	if (currentPosition != 0)
	{
		rewind(file);
		fseek(file, currentPosition, SEEK_SET);
	}

	return count;
}

char* ReadFile(const File file)
{
	size_t length = GetFileSize(file);

	char* result = SafeAlloc(length + 1);

	result[length] = '\0'; // add line terminator

	rewind(file);

	for (size_t i = 0; i < length; i++)
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
				SafeFree(result);
				fprintf(stderr, "An error occurred while reading the file at ptr: %llix, Error Code %i", (size_t)file, ferror(file));
				throw(FailedToReadFileException);
			}

			result[i] = '\0';
			break;
		}

		result[i] = c;
	}

	return result;
}

bool TryReadFile(const File file, char** out_data)
{
	*out_data = null;

	size_t length = GetFileSize(file);

	char* result = SafeAlloc(length + 1);

	result[length] = '\0'; // add line terminator

	rewind(file);

	for (size_t i = 0; i < length; i++)
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
				SafeFree(result);
				return false;
			}

			result[i] = '\0';
			break;
		}

		result[i] = c;
	}

	*out_data = result;

	return true;
}

char* ReadAll(const char* path)
{
	File file;
	if (TryOpen(path, FileModes.Read, &file))
	{
		char* data = ReadFile(file);

		if (data is null)
		{
			fprintf(stderr, "Failed to read file %s"NEWLINE, path);
			throw(FailedToReadFileException);
		}

		if (TryClose(file) is false)
		{
			fprintf(stderr, "Failed to close the file %s"NEWLINE, path);
			throw(FailedToCloseFileException);
		}

		return data;
	}

	fprintf(stderr, "Failed to open file %s"NEWLINE, path);
	throw(FailedToOpenFileException);

	// we shouldn't be able to get here, but it's possible if __debugbreak() is continued
	return null;
}

bool TryReadAll(const char* path, char** out_data)
{
	File file;

	if (TryOpen(path, FileModes.Read, &file))
	{
		char* data;

		if (TryReadFile(file, &data))
		{
			*out_data = data;

			return true;
		}

		TryClose(file);
	}

	return false;
}

bool TryReadLine(File file, char* buffer, size_t offset, size_t bufferLength, size_t* out_lineLength)
{
	GuardNotNull(file);

	int c;
	size_t index = 0;

	while ((c = fgetc(file)) != EOF && index < bufferLength)
	{
		if (c is '\0')
		{
			break;
		}
		if (c is '\n')
		{
			break;
		}
		buffer[offset + index++] = c;
	}

	if (ferror(file))
	{
		return false;
	}

	*out_lineLength = index;

	return true;
}

bool TryClose(File file)
{
	return fclose(file) != EOF;
}

void Close(File file)
{
	if (TryClose(file) is false)
	{
		throw(FailedToCloseFileException);
	}
}

#undef NotNull