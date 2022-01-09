#include "graphics/text.h"
#include "singine/memory.h"
#include <string.h>
#include "helpers/quickmask.h"

static Text Create(void);
static Text CreateText(Font font, char* string, size_t size);
static void Dispose(Text);
static void Draw(Text, Camera);
static void SetDefaultFont(Font);
static Font GetDefaultFont(void);
static void SetCharacter(Text, size_t index, unsigned int newCharacter);
static void SetText(Text, char* string, size_t size);
static void SetFont(Text, Font font);

const struct _textMethods Texts = {
	.Create = &Create,
	.CreateText = &CreateText,
	.Dispose = &Dispose,
	.Draw = &Draw,
	.SetDefaultFont = &SetDefaultFont,
	.GetDefaultFont = &GetDefaultFont,
	.SetCharacter = &SetCharacter,
	.SetText = &SetText,
	.SetFont = &SetFont
};

#define CharactersModifiedFlag FLAG_0
#define WholeStringChangedFlag FLAG_1
#define FontChangedFlag FLAG_2

Font DefaultFont = null;

static void SetDefaultFont(Font font) { DefaultFont = font; }

static Font GetDefaultFont(void) { return DefaultFont; }

static Text Create(void)
{
	Text text = SafeAlloc(sizeof(struct _text));

	text->Count = 0;
	text->Length = 0;
	text->Font = null;
	text->GameObject = null;
	text->Text = null;

	SetFlag(text->State.Modified, WholeStringChangedFlag);

	return text;
}

static char* DuplicateString(char* string, size_t size)
{
	char* result = SafeAlloc(size + 1);

	for (size_t i = 0; i < size + 1; i++)
	{
		result[i] = string[i];
	}

	result[size] = '\0';

	return result;
}

static Text CreateText(Font font, char* string, size_t size)
{
	Text text = Create();

	text->Font = Fonts.Instance(font);

	text->Text = DuplicateString(string, size);

	text->Count = text->Length = size;

	text->GameObject = GameObjects.CreateEmpty(size);

	return text;
}

static void Dispose(Text text)
{
	if (text is null)
	{
		return;
	}

	SafeFree(text->Text);

	GameObjects.Destroy(text->GameObject);

	Fonts.Dispose(text->Font);

	SafeFree(text);
}

static void CreateGameObject(Text text)
{
	text->GameObject = GameObjects.CreateEmpty(text->Length);
	text->GameObject->Material = Materials.Instance(text->Font->Material);
}

static void ClearGameObject(Text text)
{
	GameObject gameobject = text->GameObject;

	if (gameobject is null)
	{
		CreateGameObject(text);
		return;
	}

	GameObjects.Clear(gameobject);
}

static void GenerateCharacters(Text text)
{
	char* buffer = text->Text;
	GameObject gameobject = text->GameObject;

	FontCharacter previousCharacter = null;
	Transform previousTransform = text->GameObject->Transform;

	for (size_t i = 0; i < text->Count; i++)
	{
		unsigned int c = buffer[i];

		FontCharacter character = Fonts.GetCharacter(text->Font, c);

		RenderMesh instance = RenderMeshes.Duplicate(character->Mesh);

		instance->Id = c;

		gameobject->Meshes[i] = instance;

		if (previousCharacter isnt null)
		{
			// check to see if we should move horizontally or vertically
			if (previousCharacter->Id is '\n')
			{
				Transforms.TranslateY(instance->Transform, text->Font->LineHeight);

				Transforms.SetParent(instance->Transform, gameobject->Transform);
			}
			else
			{
				Transforms.SetParent(instance->Transform, previousTransform);

				Transforms.TranslateX(instance->Transform, previousCharacter->Advance);
			}
		}
		else
		{
			Transforms.SetParent(instance->Transform, gameobject->Transform);
		}

		previousTransform = instance->Transform;

		previousCharacter = character;
	}
}

static void RefreshCharacters(Text text)
{

}

static void RefreshText(Text text)
{
	// make a copy of the state
	unsigned int state = text->State.Modified;

	// if nothing changed return and render
	if (state is 0)
	{
		return;
	}

	// reset the state first so we dont have to set it multiple times
	ResetFlags(text->State.Modified);

	// if the font has changed all the kerning and meshes are invalidated
	// redraw the whole thing
	if (HasFlag(state, FontChangedFlag))
	{
		// wipe all old characters
		ClearGameObject(text);

		// make sure the material on the gameobject matches the font
		GameObjects.SetMaterial(text->GameObject, text->Font->Material);

		// regenerate the characters
		GenerateCharacters(text);

		return;
	}

	// if the entire string changed we need to redraw the whole thing but resize to the new array size
	if (HasFlag(state, WholeStringChangedFlag))
	{
		// if the whole reference for the string has changed we create a new rendermesh array from scratch
		GameObjects.Resize(text->GameObject, text->Length);

		// regenerate the entire string
		GenerateCharacters(text);

		return;
	}

	if (HasFlag(state, CharactersModifiedFlag))
	{
		// only regenerate the changed characters within the string
		RefreshCharacters(text);

		return;
	}

	throw(UnexpectedOutcomeException);
}

static void Draw(Text text, Camera camera)
{
	RefreshText(text);

	GameObjects.Draw(text->GameObject, camera);
}

static void SetCharacter(Text text, size_t index, unsigned int newCharacter)
{
	SetFlag(text->State.Modified, CharactersModifiedFlag);
}


static void SetText(Text text, char* string, size_t size)
{
	// check if the pointer were getting is the same as the stored one
	if (string == text->Text)
	{
		// if the count has changed we don't want to draw garbage characters at the end
		text->Count = size;

		SetFlag(text->State.Modified, CharactersModifiedFlag);
		return;
	}

	// check to see if we have to resize the array
	if (text->Length < size)
	{
		SafeFree(text->Text);

		text->Text = DuplicateString(string, size);

		text->Count = text->Length = size;
	}
	else
	{
		strcpy_s(text->Text, size + 1, string);
		text->Count = size;
	}

	SetFlag(text->State.Modified, WholeStringChangedFlag);
}

static void SetFont(Text text, Font font)
{
	text->Font = Fonts.Instance(font);
	GameObjects.SetMaterial(text->GameObject, font->Material);

	SetFlag(text->State.Modified, FontChangedFlag);
}