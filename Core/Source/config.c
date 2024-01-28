#include "singine/config.h"
#include "singine/file.h"
#include "string.h"
#include "singine/strings.h"
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

static bool TryLoadConfig(const char* path, const ConfigDefinition, void* state);
static bool TryLoadConfigStream(File stream, const ConfigDefinition, void* state);
static void SaveConfigStream(File stream, const ConfigDefinition config, void* state);
static void SaveConfig(const char* path, const ConfigDefinition config, void* state);

const struct _configMethods Configs = {
	.TryLoadConfig = &TryLoadConfig,
	.TryLoadConfigStream = &TryLoadConfigStream,
	.SaveConfig = SaveConfig,
	.SaveConfigStream = SaveConfigStream
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

		if (config->AbortToken.Token isnt null)
		{
			const char* token = config->AbortToken.Token;
			const size_t tokenLength = max(config->AbortToken.Length - 1, 0);

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
			const struct _configToken* token = &config->Tokens[i];

			const size_t tokenLength = min(token->Length, max(token->Length - 1, 0));

			if (buffer[0] is token->Token[0])
			{
				// compare the whole token, if it's valid invoke the callback
				const int indexOfColon = Strings.IndexOf(buffer, lineLength, ':');

				// a colon is required
				if (indexOfColon < 0)
				{
					// colon not found
					return false;
				}

				// specularMap: should not be considered specular:
				if (indexOfColon > (tokenLength + 1))
				{
					continue;
				}

				if (memcmp(buffer, token->Token, min(tokenLength, lineLength)) is 0)
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

					if (token->TokenLoad(subBuffer, subBufferLength, state) is false)
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

static void SaveConfigStream(File stream, const ConfigDefinition config, void* state)
{
	for (size_t i = 0; i < config->Count; i++)
	{
		const struct _configToken token = config->Tokens[i];

		fprintf(stream, "%s"NEWLINE, token.Description);

		token.TokenSave(stream, state);

		fprintf(stream, NEWLINE);
	}
}

static void SaveConfig(const char* path, const ConfigDefinition config, void* state)
{
	File file;
	if (Files.TryOpen(path, FileModes.Create, &file) is false)
	{
		throw(FailedToOpenFileException);
	}

	SaveConfigStream(file, config, state);

	if (Files.TryClose(file) is false)
	{
		throw(FailedToCloseFileException);
	}
}