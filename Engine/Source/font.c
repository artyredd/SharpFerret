#include "engine/graphics/font.h"
#include "core/memory.h"
#include <string.h>
#include <float.h>
#include <ctype.h>
#include "engine/modeling/importer.h"

static Font Create(Model);
static void Dispose(Font);
static Font Import(char* path, FileFormat format);
static void Draw(Font font, unsigned short character, Scene scene);
static void SetMaterial(Font, Material);
static FontCharacter GetFontCharacter(Font, unsigned int character);
static Font Instance(Font);

const struct _fontMethods Fonts = {
	.Draw = &Draw,
	.Create = &Create,
	.Dispose = &Dispose,
	.Import = &Import,
	.SetMaterial = &SetMaterial,
	.Instance = &Instance,
	.GetCharacter = &GetFontCharacter
};

static FontCharacter CreateCharacter(Mesh);
static void DisposeCharacter(FontCharacter);

const struct _fontCharacterMethods FontCharacters = {
	.Create = &CreateCharacter,
	.Dispose = &DisposeCharacter
};

DEFINE_TYPE_ID(FontCharacter);
DEFINE_TYPE_ID(Font)

static FontCharacter CreateWhiteSpaceCharacter(Font font, size_t width)
{
	Memory.RegisterTypeName(nameof(FontCharacter), &FontCharacterTypeId);

	FontCharacter newCharacter = Memory.Alloc(sizeof(struct _fontCharacter), FontCharacterTypeId);

	RenderMesh emptyMesh = RenderMeshes.Create();

	newCharacter->Advance = width * font->SpaceWidth;

	newCharacter->MinY = font->Characters['H']->MinY;
	newCharacter->MaxY = font->Characters['H']->MaxY;
	newCharacter->LeftBearing = 0;
	newCharacter->RightBearing = newCharacter->Advance;

	newCharacter->Mesh = emptyMesh;

	Transforms.TranslateX(emptyMesh->Transform, newCharacter->Advance);

	return newCharacter;
}

static void AssignWhiteSpaceCharacters(Font font)
{
	font->Characters['\t'] = CreateWhiteSpaceCharacter(font, 4);
	font->Characters['\t']->Id = '\t';

	font->Characters[' '] = CreateWhiteSpaceCharacter(font, 1);
	font->Characters[' ']->Id = ' ';

	font->Characters['\n'] = CreateWhiteSpaceCharacter(font, 1);
	font->Characters['\n']->Id = '\n';

	// becuase this is a small character and not visible it's likely that it's undefined within a charset
	// and becuase we dispose of a font using the start character we should make sure to move the start index
	// to '\t' if it's not already so we dispose of it correctly
	if ('\t' < font->StartCharacter)
	{
		font->StartCharacter = '\t';
	}
}

static void CalculateAndAssignFontSizes(Font font)
{
	font->LineHeight = font->MaxY - font->MinY;

	// the space width should be 1/4th the EM
	// the EM is 0.7 the height of H
	FontCharacter hCharacter = font->Characters['H'];

	if (hCharacter isnt null)
	{
		font->EmSize = hCharacter->MaxY - hCharacter->MinY;

		font->SpaceWidth = 0.35f * font->EmSize;
	}
	else
	{
		font->SpaceWidth = 0.125f;
	}
}

static void CreateCharactersFromModel(Font font, Model model)
{
	bool assignedStart = false;
	for (size_t i = 0; i < model->Count; i++)
	{
		Mesh mesh = model->Meshes[i];

		FontCharacter character = FontCharacters.Create(mesh);

		if (character isnt null)
		{
			font->Characters[character->Id] = character;

			if (assignedStart is false)
			{
				font->StartCharacter = character->Id;
				assignedStart = true;
			}
			font->EndCharacter = character->Id;

			if (font->MinY > character->MinY)
			{
				font->MinY = character->MinY;
			}

			if (font->MaxY < character->MaxY)
			{
				font->MaxY = character->MaxY;
			}
		}
	}

	font->Count = model->Count;
}



static Font Create(Model model)
{
	Memory.RegisterTypeName(nameof(Font), &FontTypeId);

	Font font = Memory.Alloc(sizeof(struct _font), FontTypeId);

	font->Material = null;
	font->MinY = FLT_MAX;
	font->MaxY = FLT_MIN;
	font->ActiveInstances = 1;

	// memset null into the entire character array so consumers can rely on unavailable characters being null and not garbage
	memset(font->Characters, 0, MAX_SUPPORTED_CHARACTERS * sizeof(FontCharacter));

	// instead of trusting the models count as chaarcter size count as they are processed
	// since not all characters are gaurunteed to have all attriute required to draw them and are thrown out
	CreateCharactersFromModel(font, model);

	// if we were not able to get any characters from the model we should return null
	if (font->Count is 0)
	{
		Dispose(font);
		return null;
	}

	CalculateAndAssignFontSizes(font);

	AssignWhiteSpaceCharacters(font);

	return font;
}

static void Dispose(Font font)
{
	if (font is null)
	{
		return;
	}

	if (font->ActiveInstances > 1)
	{
		--(font->ActiveInstances);
		return;
	}

	Materials.Dispose(font->Material);

	for (size_t i = font->StartCharacter; i < font->EndCharacter + 1; i++)
	{
		FontCharacters.Dispose(font->Characters[i]);
	}

	Memory.Free(font, FontTypeId);
}

static Font Import(char* path, FileFormat format)
{
	Model model;
	if (Importers.TryImport(path, format, &model) is false)
	{
		throw(FailedToReadFileException);
	}

	if (model->Count is 0)
	{
		Models.Dispose(model);
		throw(FailedToImportModelException);
	}

	Font font = Create(model);

	Models.Dispose(model);

	return font;
}

static void Draw(Font font, unsigned short character, Scene scene)
{
	FontCharacter c = font->Characters[character];
	if (c isnt null)
	{
		Materials.Draw(font->Material, c->Mesh, scene);
	}
}

static void SetMaterial(Font font, Material material)
{
	Materials.Dispose(font->Material);
	font->Material = Materials.Instance(material);
}

static bool TryLoadCharacterSize(Mesh mesh, FontCharacter character)
{
	// example:
	// o symbol U+0021 glyph 4 xadv 0.545 lsb 0.192 rsb 0.189  ymin -0.025 ymax 0.790
	// %*s      U+%x   %*s %*i xadv %lf    lsb %lf    rsb %lf  ymin %f     ymax %f
	size_t count = sscanf_s(mesh->Name, "%*s U+%hx %*s %*i xadv %f lsb %f rsb %f ymin %f ymax %f",
		&character->Id,
		&character->Advance,
		&character->LeftBearing,
		&character->RightBearing,
		&character->MinY,
		&character->MaxY);

	return count == 6;
}

static FontCharacter CreateCharacter(Mesh mesh)
{
	RenderMesh newMesh;
	if (RenderMeshes.TryBindMesh(mesh, &newMesh) is false)
	{
		throw(FailedToBindMeshException);
	}

	Memory.RegisterTypeName(nameof(FontCharacter), &FontCharacterTypeId);
	//  create the new character
	FontCharacter character = Memory.Alloc(sizeof(struct _fontCharacter), FontCharacterTypeId);

	character->Mesh = null;

	if (TryLoadCharacterSize(mesh, character) is false)
	{
		fprintf_s(stderr, "Failed to load the character size for the character: %s", mesh->Name);

		RenderMeshes.Dispose(newMesh);
		Memory.Free(character, FontCharacterTypeId);

		return null;
	}

	//Transforms.SetPositions(newMesh->Transform, -(character->Advance / 2), 0, 0);

	character->Mesh = newMesh;

	return character;
}

static void DisposeCharacter(FontCharacter character)
{
	if (character is null)
	{
		return;
	}

	RenderMeshes.Dispose(character->Mesh);

	Memory.Free(character, FontCharacterTypeId);
}

static FontCharacter GetFontCharacter(Font font, unsigned int desiredCharacter)
{
	FontCharacter character = font->Characters[desiredCharacter];

	// default any missing characters to a question mark this is standard
	if (character is null)
	{
		if (isspace(desiredCharacter))
		{
			// if it's generic whitespace that's not space or tab just return space
			return font->Characters[' '];
		}

		character = font->Characters['?'];

		if (character is null)
		{
			throw(MissingCharacterException);
		}
	}

	return character;
}

static Font Instance(Font font)
{
	if (font is null)
	{
		return null;
	}

	Font newFont = font;

	++(font->ActiveInstances);

	return newFont;
}