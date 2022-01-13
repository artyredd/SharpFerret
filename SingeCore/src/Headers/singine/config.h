#pragma once
#include "csharp.h"

typedef struct _configDefinition* ConfigDefinition;

struct _configDefinition {
	/// <summary>
	/// The array of tokens that should be searched for within the config
	/// </summary>
	const char** Tokens;
	/// <summary>
	/// The length of each token in the tokens array
	/// </summary>
	const size_t* TokenLengths;
	/// <summary>
	/// The number of tokens within the token array
	/// </summary>
	const size_t Count;
	/// <summary>
	/// The character that delineates that a line of the config can be discarded(ie. comment lines that start with # for example)
	/// </summary>
	const int CommentCharacter;
	/// <summary>
	/// The method that should be invoked when a token is found, the buffer received does NOT include the token and only the data for the token
	/// </summary>
	bool (*OnTokenFound)(size_t tokenIndex, const char* buffer, size_t length, void* state);
};

struct _configMethods {
	/// <summary>
	/// Attempts to open the given path and locate each token within the config definition, once found OnTokenFound within the definition is invoked
	/// with the index of the token within the token array and the data for that token, along with the state pointer originally passed to the method,
	/// this method returns true when all calls to OnTokenFound return true and no file error occurs, otherwise false
	/// </summary>
	bool(*TryLoadConfig)(const char* path, const ConfigDefinition, void* state);
};

extern const struct _configMethods Configs;