#include "graphics/text.h"
#include "singine/memory.h"
#include <string.h>
#include "helpers/quickmask.h"

static Text Create(void);
static Text CreateText(Font font, char* string, size_t size);
static void Dispose(Text);
static void Draw(Text, Scene);
static void SetDefaultFont(Font);
static Font GetDefaultFont(void);
static void SetCharacter(Text, size_t index, unsigned int newCharacter);
static void SetText(Text, char* string, size_t size);
static void SetFont(Text, Font font);
static Text CreateEmpty(Font font, size_t size);

const struct _textMethods Texts = {
	.Create = &Create,
	.CreateText = &CreateText,
	.Dispose = &Dispose,
	.Draw = &Draw,
	.SetDefaultFont = &SetDefaultFont,
	.GetDefaultFont = &GetDefaultFont,
	.SetCharacter = &SetCharacter,
	.SetText = &SetText,
	.SetFont = &SetFont,
	.CreateEmpty = &CreateEmpty
};

#define CharactersModifiedFlag FLAG_0
#define WholeStringChangedFlag FLAG_1
#define FontChangedFlag FLAG_2

Font DefaultFont = null;

static void SetDefaultFont(Font font) { DefaultFont = font; }

static Font GetDefaultFont(void) { return DefaultFont; }

TYPE_ID(Text);

static Text Create(void)
{
	Memory.RegisterTypeName(nameof(Text), &TextTypeId);

	Text text = Memory.Alloc(sizeof(struct _text), TextTypeId);

	text->Count = 0;
	text->Length = 0;
	text->Font = null;
	text->GameObject = null;
	text->Text = null;
	text->AlignTop = true;

	SetFlag(text->State.Modified, WholeStringChangedFlag);

	return text;
}

static char* DuplicateString(char* string, size_t size)
{
	char* result = Memory.Alloc(size + 1, Memory.String);

	for (size_t i = 0; i < size + 1; i++)
	{
		result[i] = string[i];
	}

	result[size] = '\0';

	return result;
}

static Text CreateEmpty(Font font, size_t size)
{
	Text text = Create();

	text->Font = Fonts.Instance(font);

	text->Text = Memory.Alloc(size + 1, Memory.String);

	text->Count = text->Length = size;

	text->GameObject = GameObjects.CreateEmpty(size);

	// becuase create empty produces an empty(except when size is provided) gameobject we need to manually create a transform
	text->GameObject->Transform = Transforms.Create();

	GameObjects.SetMaterial(text->GameObject, font->Material);

	return text;
}

static Text CreateText(Font font, char* string, size_t size)
{
	Text text = Create();

	text->Font = Fonts.Instance(font);

	text->Text = DuplicateString(string, size);

	text->Count = text->Length = size;

	text->GameObject = GameObjects.CreateEmpty(size);
	// becuase create empty produces an empty(except when size is provided) gameobject we need to manually create a transform
	text->GameObject->Transform = Transforms.Create();

	return text;
}

static void Dispose(Text text)
{
	if (text is null)
	{
		return;
	}

	Memory.Free(text->Text, Memory.String);

	GameObjects.Destroy(text->GameObject);

	Fonts.Dispose(text->Font);

	Memory.Free(text, TextTypeId);
}

static void CreateGameObject(Text text)
{
	text->GameObject = GameObjects.CreateEmpty(text->Length);
	// becuase create empty produces an empty(except when size is provided) gameobject we need to manually create a transform
	text->GameObject->Transform = Transforms.Create();
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

	float newLineAmount = text->AlignTop ? -text->Font->LineHeight : text->Font->LineHeight;

	size_t line = 0;

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
				++(line);
				Transforms.TranslateY(instance->Transform, line * newLineAmount);

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
	char* buffer = text->Text;
	GameObject gameobject = text->GameObject;

	Transform previousTransform = text->GameObject->Transform;

	RenderMesh previousMesh = null;

	float advance = 0.0f;
	bool previousCharacterChanged = false;

	float newLineAmount = text->AlignTop ? -text->Font->LineHeight : text->Font->LineHeight;

	// abc
	// agc
	size_t line = 0;
	for (size_t i = 0; i < text->Count; i++)
	{
		unsigned int c = buffer[i];

		RenderMesh currentMesh = gameobject->Meshes[i];

		// if the character is wrong regenerate it
		FontCharacter character = Fonts.GetCharacter(text->Font, c);

		// check to see if the correct character is already in that spot
		if (currentMesh isnt null && currentMesh->Id is c)
		{
			// check to see if we need to update the position of this character
			if (previousCharacterChanged)
			{
				Transforms.SetParent(currentMesh->Transform, previousTransform);
				Transforms.SetPositions(currentMesh->Transform, advance, 0, 0);
			}

			if (currentMesh->Id is '\n')
			{
				previousTransform = gameobject->Transform;
				advance = 0.0f;
				++(line);
			}
			else
			{
				previousTransform = currentMesh->Transform;
				advance = character->Advance;
			}

			previousMesh = currentMesh;

			previousCharacterChanged = false;

			continue;
		}

		previousCharacterChanged = true;

		RenderMesh instance = RenderMeshes.Duplicate(character->Mesh);

		instance->Id = c;

		if (currentMesh isnt null)
		{
			Transforms.SetParent(currentMesh->Transform, null);
			Transforms.ClearChildren(currentMesh->Transform);
			RenderMeshes.Dispose(currentMesh);
		}

		gameobject->Meshes[i] = instance;

		if (previousMesh isnt null)
		{
			// check to see if we should move horizontally or vertically
			if (previousMesh->Id is '\n')
			{
				Transforms.TranslateY(instance->Transform, line * newLineAmount);

				Transforms.SetParent(instance->Transform, gameobject->Transform);
			}
			else
			{
				Transforms.SetParent(instance->Transform, previousTransform);

				Transforms.TranslateX(instance->Transform, advance);
			}
		}
		else
		{
#pragma warning(disable: 28182) // I broken the compiler with this algo, it thinks im dereferencing previousMesh here?
			Transforms.SetParent(instance->Transform, gameobject->Transform);
#pragma warning(default: 28182)
		}

		previousTransform = instance->Transform;

		previousMesh = instance;

		advance = character->Advance;

		if (c is '\n')
		{
			++(line);
		}
	}

	// if the string we are refreshing to is smaller than our old string that's still in the same buffer we should
	// get rid of the old orphaned ones
	for (size_t i = text->Count; i < text->Length; i++)
	{
		buffer[i] = '\0';

		RenderMesh mesh = gameobject->Meshes[i];

		if (mesh is null)
		{
			break;
		}

		if (mesh->Id isnt 0)
		{
			Transforms.ClearChildren(mesh->Transform);
			RenderMeshes.Dispose(mesh);
		}

		gameobject->Meshes[i] = null;
	}
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

static void Draw(Text text, Scene scene)
{
	RefreshText(text);

	GameObjects.Draw(text->GameObject, scene);
}

static void SetCharacter(Text text, size_t index, unsigned int newCharacter)
{
	SetFlag(text->State.Modified, CharactersModifiedFlag);

	text->Text[index] = (char)newCharacter;
}


static void SetText(Text text, char* string, size_t size)
{
	// check if the pointer were getting is the same as the stored one
	if (string == text->Text)
	{
		// if the count has changed we don't want to draw garbage characters at the end
		text->Count = size;

		//SetFlag(text->State.Modified, WholeStringChangedFlag);
		SetFlag(text->State.Modified, CharactersModifiedFlag);
		return;
	}

	// check to see if we have to resize the array
	if (text->Length < size)
	{
		Memory.Free(text->Text, Memory.String);

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