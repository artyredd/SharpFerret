#pragma once
#include "core/csharp.h"
#include "core/file.h"

struct _configToken
{
	const char* Token;
	const size_t Length;
	bool (*TokenLoad)(const char* buffer, size_t length, void* state);
	void (*TokenSave)(File stream, void* state);
	const char* Description;
};

static bool __AbortTokenReturnTrue(const char* buffer, const size_t length, void* state)
{
	ignore_unused(buffer);
	ignore_unused(length);
	ignore_unused(state);

	return true;
}

#define TOKEN( token, description ) { .Token = #token, .Length = sizeof(#token), .TokenLoad = &Token##token##Load, .TokenSave = &Token##token##Save, .Description = description }

#define ABORT_TOKEN( token ) { .Token = #token, .Length = sizeof(#token), .TokenLoad = &__AbortTokenReturnTrue, .Description = "" }

#define TOKENS( count ) static const struct _configToken Tokens[count] = 

#define TOKEN_LOAD( token, stateType ) static bool Token##token##Load(const char* buffer, const size_t length, stateType state)

#define TOKEN_SAVE( token, stateType ) static void Token##token##Save(File stream, stateType state)

typedef const struct _configDefinition* ConfigDefinition;

struct _configDefinition {
	// list of tokens
	const struct _configToken* Tokens;
	/// <summary>
	/// The number of tokens within the token array
	/// </summary>
	const size_t Count;
	/// <summary>
	/// The character that delineates that a line of the config can be discarded(ie. comment lines that start with # for example)
	/// </summary>
	const int CommentCharacter;
	/// <summary>
	/// The token that when encountered should signify continued config reading should be aborted
	/// </summary>
	struct _configToken AbortToken;
};

#define DEFINE_CONFIG(name,optionalBody) static const struct _configDefinition name##ConfigDefinition = {\
	.Tokens = Tokens,\
	.CommentCharacter = '#',\
	.Count = sizeof(Tokens) / sizeof(struct _configToken),\
	optionalBody\
}

#define CONFIG(name) DEFINE_CONFIG(name, )

struct _configMethods {
	/// <summary>
	/// Attempts to open the given path and locate each token within the config definition, once found OnTokenFound within the definition is invoked
	/// with the index of the token within the token array and the data for that token, along with the state pointer originally passed to the method,
	/// this method returns true when all calls to OnTokenFound return true and no file error occurs, otherwise false
	/// </summary>
	bool(*TryLoadConfig)(const char* path, const ConfigDefinition, void* state);

	/// <summary>
	/// Attempts to locate each token within the config definition, once found OnTokenFound within the definition is invoked
	/// with the index of the token within the token array and the data for that token, along with the state pointer originally passed to the method,
	/// this method returns true when all calls to OnTokenFound return true and no file error occurs, otherwise false
	/// </summary>
	bool (*TryLoadConfigStream)(File stream, const ConfigDefinition, void* state);

	void (*SaveConfig)(const char* path, const ConfigDefinition, void* state);
	void (*SaveConfigStream)(File stream, const ConfigDefinition, void* state);
};

extern const struct _configMethods Configs;