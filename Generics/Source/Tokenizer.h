#include <core/csharp.h>
#include <core/array.h>

typedef uint TokenType;

static const struct _TokenTypes
{
	TokenType None;
	TokenType Word;
	TokenType Whitespace;
	TokenType Control;
} TokenTypes = {
	.None = 0,
	.Word = 1,
	.Whitespace = 2,
	.Control = 3
};

struct _Token;

typedef struct _Token* Token;

struct _Token
{
	TokenType Type;
	string Data;
};

DEFINE_CONTAINERS(Token);

struct _Document
{
	array(Token) Tokens;
	string Data;
	string Name;
};

typedef struct _Document* Document;

DEFINE_CONTAINERS(Document);

struct _tokenizerMethods {
	Document(*TokenizeDocument)(string name, string data);
};

extern const struct _tokenizerMethods Tokenizers;