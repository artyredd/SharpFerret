#include "graphics/font.h"
#include "singine/memory.h"
#include <string.h>

static Font Create(Model);
static void Dispose(Font);
static Font Import(char* path, FileFormat format);
static void Draw(Font font, unsigned short character, Camera camera);
static void SetMaterial(Font, Material);
static GameObject CreateLine(Font, char* buffer, size_t bufferLength);

const struct _fontMethods Fonts = {
	.Draw = &Draw,
	.Create = &Create,
	.Dispose = &Dispose,
	.Import = &Import,
	.SetMaterial = &SetMaterial,
	.CreateLine = &CreateLine
};

static FontCharacter CreateCharacter(Mesh);
static void DisposeCharacter(FontCharacter);

const struct _fontCharacterMethods FontCharacters = {
	.Create = &CreateCharacter,
	.Dispose = &DisposeCharacter
};

static Font Create(Model model)
{
	Font font = SafeAlloc(sizeof(struct _font));

	font->Material = null;

	// memset null into the entire character array so consumers can rely on unavailable characters being null and not garbage
	//memset(font->Characters, 0, MAX_SUPPORTED_CHARACTERS);
	// apprently memset doesn't set over 10,000 characters on my system so i did it manually
	for (size_t i = 0; i < MAX_SUPPORTED_CHARACTERS; i++)
	{
		font->Characters[i] = null;
	}

	// instead of trusting the models count as chaarcter size count as they are processed
	// since not all characters are gaurunteed to have all attriute required to draw them and are thrown out

	size_t count = 0;
	Mesh mesh = model->Head;

	bool assignedStart = false;

	while (mesh != null)
	{
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

			++(count);
		}

		mesh = mesh->Next;
	}

	if (count == 0)
	{
		Fonts.Dispose(font);
		return null;
	}

	return font;
}

static void Dispose(Font font)
{
	if (font is null)
	{
		return;
	}

	Materials.Dispose(font->Material);

	for (size_t i = font->StartCharacter; i < font->EndCharacter + 1; i++)
	{
		FontCharacters.Dispose(font->Characters[i]);
	}

	SafeFree(font);
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
		model->Dispose(model);
		throw(FailedToImportModelException);
	}

	Font font = Create(model);

	model->Dispose(model);

	return font;
}

static void Draw(Font font, unsigned short character, Camera camera)
{
	FontCharacter c = font->Characters[character];
	if (c isnt null)
	{
		Materials.Draw(font->Material, c->Mesh, camera);
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

	//  create the new character
	FontCharacter character = SafeAlloc(sizeof(struct _fontCharacter));

	character->Mesh = null;

	if (TryLoadCharacterSize(mesh, character) is false)
	{
		fprintf_s(stderr, "Failed to load the character size for the character: %s", mesh->Name);

		RenderMeshes.Dispose(newMesh);
		SafeFree(character);

		return null;
	}

	float height = character->MaxY - character->MinY;

	Transforms.SetPositions(newMesh->Transform, -(character->Advance / 2), -height / 2, 0);

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

	SafeFree(character);
}

static GameObject CreateLine(Font font, char* buffer, size_t bufferLength)
{
	GameObject line = GameObjects.CreateWithMaterial(font->Material);

	line->Meshes = SafeAlloc(sizeof(RenderMesh) * bufferLength);
	line->Count = bufferLength;

	// track where we should place the next character
	double cursor = 0.0;

	for (size_t i = 0; i < bufferLength; i++)
	{
		unsigned int c = buffer[i];

		FontCharacter character = font->Characters[c];

		// default any missing characters to a question mark this is standard
		if (character is null)
		{
			character = font->Characters['?'];

			if (character is null)
			{
				throw(MissingCharacterException);
			}
		}

		RenderMesh instance = RenderMeshes.Duplicate(character->Mesh);

		line->Meshes[i] = instance;

		Transforms.SetParent(instance->Transform, line->Transform);

		// move the character over to the cursor position
		Transforms.TranslateX(instance->Transform, (float)cursor);

		// move the cursor position over by the width of the character
		cursor += character->Advance;
	}

	return line;
}