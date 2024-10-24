#include "Tokenizer.h"
#include <stdio.h>
#include <string.h>
#include "core/runtime.h"
#include "core/cunit.h"

private Document TokenizeDocument(string name, string data);

const struct _tokenizerMethods Tokenizers = {
	.TokenizeDocument = TokenizeDocument
};

private Token CreateToken(TokenType type, string data)
{
	Token token = new(Token);

	token->Data = data;
	token->Type = type;

	return token;
}

private TokenType CheckChar(byte c)
{
	TokenType currentType = isalnum(c);
	currentType += isspace(c);
	currentType = currentType ? currentType : 3;

	return currentType;
}

private array(Token) GetTokens(string data)
{
	array(Token) tokens = arrays(Token).Create(0);

	if (data->Count is 0)
	{
		return tokens;
	}

	int startIndex = 0;
	int endIndex = 0;
	
	TokenType previousType = CheckChar(at(data, 0));

	for (int i = 0; i < data->Count; i++)
	{
		int previousC = at(data, max(0, i - 1));
		int c = at(data, i);
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		TokenType currentType = CheckChar(c);

		if (currentType != previousType)
		{
			string sub = stack_substring(data, startIndex, endIndex-startIndex);
			Token token = CreateToken(previousType, strings.Clone(sub));
			arrays(Token).Append(tokens, token);
			startIndex = i;
			endIndex = i;
		}

		previousType = currentType;
	}

	return tokens;
}

private Document TokenizeDocument(string name, string data)
{
	Document doc = new(Document);

	doc->Tokens = GetTokens(data);
	doc->Name = strings.Clone(name);

	return doc;
}

TEST(TypeSplitting)
{
	string data = stack_string("int Method();{ int x = 12; return x; }");

	array(Token) tokens = GetTokens(data);

	IsEqual((ulong)20, tokens->Count);

	return true;
}

TEST_SUITE(RunUnitTests,
	APPEND_TEST(TypeSplitting)
);

OnStart(111)
{
	RunUnitTests();
}