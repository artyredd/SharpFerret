#include "core/config.h"
#include "core/file.h"
#include "string.h"
#include "core/strings.h"
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

static bool TryLoadConfig(const string path, const ConfigDefinition, void* state);
static bool TryLoadConfigStream(File stream, const ConfigDefinition, void* state);
static void SaveConfigStream(File stream, const ConfigDefinition config, void* state);
static void SaveConfig(const string path, const ConfigDefinition config, void* state);

const struct _configMethods Configs = {
	.TryLoadConfig = &TryLoadConfig,
	.TryLoadConfigStream = &TryLoadConfigStream,
	.SaveConfig = SaveConfig,
	.SaveConfigStream = SaveConfigStream
};

static bool TryLoadConfigStream(File stream, const ConfigDefinition config, void* state)
{
	string buffer = empty_stack_array(byte, BUFFER_SIZE);

	ulong bufferLength = BUFFER_SIZE;

	ulong lineLength;
	while (buffer->Count = 0, Files.TryReadLine(stream, buffer, 0, &lineLength))
	{
		if (buffer->Count is 0)
		{
			continue;
		}

		// ignore comments
		if (at(buffer, 0) is config->CommentCharacter)
		{
			continue;
		}

		if (config->AbortToken.Token isnt null)
		{
			const char* token = config->AbortToken.Token;
			const ulong tokenLength = max(config->AbortToken.Length - 1, 0);

			if (at(buffer, 0) is token[0])
			{
				// compare the whole token, if the abort token was found abort
				if (memcmp(buffer->Values, token, min(tokenLength, lineLength)) is 0)
				{
					break;
				}
			}
		}

		for (ulong i = 0; i < config->Count; i++)
		{
			// check the first character to avoid comparing whole string
			const struct _configToken* token = &config->Tokens[i];

			const ulong tokenLength = safe_subtract(token->Length, 1);

			if (at(buffer, 0) is token->Token[0])
			{
				// compare the whole token, if it's valid invoke the callback
				const int indexOfColon = Strings.IndexOf(buffer->Values, lineLength, ':');

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

				if (memcmp(buffer->Values, token->Token, min(tokenLength, lineLength)) is 0)
				{
					ulong offset = min(tokenLength + 1, lineLength);

					array(byte) subBuffer = stack_subarray_back(byte, buffer, offset);

					// check if the first character is whitespace, if it is move the subbuffer over
					// I COULD create a more verstatile solution to this but..
					if (isspace(at(subBuffer, 0)))
					{
						subBuffer = stack_subarray_back(byte, subBuffer, 1);
					}

					if (token->TokenLoad(subBuffer->Values, subBuffer->Count, state) is false)
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

static bool TryLoadConfig(const string path, const ConfigDefinition config, void* state)
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
	for (ulong i = 0; i < config->Count; i++)
	{
		const struct _configToken token = config->Tokens[i];

		fprintf(stream, "%s"NEWLINE, token.Description);

		token.TokenSave(stream, state);

		fprintf(stream, NEWLINE);
	}
}

static void SaveConfig(const string path, const ConfigDefinition config, void* state)
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