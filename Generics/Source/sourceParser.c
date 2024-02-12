#pragma once

#include "core/array.h"
#include "sourceParser.h"
#include "core/file.h"

private array(string) ReadTokensFromStream(File file)
{
	array(string) result = Arrays(string).Create(0);


	return result;
}

// Reads the tokens from the provided file
private array(string) ReadTokens(string path)
{
	File file;
	if (Files.TryOpen(path, FileModes.ReadBinary, &file) is false)
	{
		return null;
	}

	array(string) result = ReadTokensFromStream(file);

	Files.Close(file);

	return result;
}