#include "singine/config.h"
#include "singine/file.h"
#include "string.h"
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

static bool TryLoadConfig(const char* path, const ConfigDefinition, void* state);
static bool TryLoadConfigStream(File stream, const ConfigDefinition, void* state);

const struct _configMethods Configs = {
	.TryLoadConfig = &TryLoadConfig,
	.TryLoadConfigStream = &TryLoadConfigStream
};

static bool TryLoadConfigStream(File stream, const ConfigDefinition config, void* state)
{
	char buffer[BUFFER_SIZE];

	size_t bufferLength = BUFFER_SIZE;

	size_t lineLength;
	while (Files.TryReadLine(stream, buffer, 0, bufferLength, &lineLength))
	{
		// ignore comments
		if (buffer[0] is config->CommentCharacter)
		{
			continue;
		}

		if (config->AbortToken isnt null)
		{
			const char* token = config->AbortToken;
			const size_t tokenLength = max(config->AbortTokenLength - 1, 0);

			if (buffer[0] is token[0])
			{
				// compare the whole token, if the abort token was found abort
				if (memcmp(buffer, token, min(tokenLength, lineLength)) is 0)
				{
					break;
				}
			}
		}

		for (size_t i = 0; i < config->Count; i++)
		{
			// check the first character to avoid comparing whole string
			const char* token = config->Tokens[i];
			const size_t tokenLength = max(config->TokenLengths[i] - 1, 0);

			if (buffer[0] is token[0])
			{
				// compare the whole token, if it's valid invoke the callback
				if (memcmp(buffer, token, min(tokenLength, lineLength)) is 0)
				{
					size_t offset = min(tokenLength + 1, lineLength);

					char* subBuffer = buffer + offset;
					size_t subBufferLength = max(lineLength - tokenLength - 1, 0);

					// check if the first character is whitespace, if it is move the subbuffer over
					// I COULD create a more verstatile solution to this but..
					if (isspace(subBuffer[0]))
					{
						subBuffer = buffer + offset + 1;
						--(subBufferLength);
					}

					if (config->OnTokenFound(i, subBuffer, subBufferLength, state) is false)
					{
						return false;
					}

					// once we have found a valid token and invoked OnTokenFound, break and read next line
					break;
				}
			}
		}
	}

	return true;
}

static bool TryLoadConfig(const char* path, const ConfigDefinition config, void* state)
{
	File file;
	if (Files.TryOpen(path, FileModes.ReadBinary, &file) is false)
	{
		return false;
	}

	if (TryLoadConfigStream(file, config, state) is false)
	{
		return false;
	}

	return Files.TryClose(file);
}