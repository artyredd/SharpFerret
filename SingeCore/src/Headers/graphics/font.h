#pragma once

#include "singine/gameobject.h"
#include "graphics/material.h"
#include <Headers/modeling/importer.h>

#define MAX_SUPPORTED_CHARACTERS USHRT_MAX

typedef struct _fontCharacter* FontCharacter;

struct _fontCharacter {
	/// <summary>
	/// The character id for the character
	/// </summary>
	unsigned short Id;
	/// <summary>
	/// The horizontal distance from the current pen position to the glyph's left bbox edge.
	/// It is positive for horizontal layouts, and in most cases negative for vertical ones.
	/// </summary>
	float LeftBearing;
	/// <summary>
	/// Only used for horizontal layouts to describe the distance from the bbox's right edge 
	/// to the advance width. In most cases it is a non-negative number:
	/// </summary>
	float RightBearing;
	/// <summary>
	/// The horizontal distance to increment (for left-to-right writing) or decrement 
	/// (for right-to-left writing) the pen position after a glyph has been rendered when 
	/// processing text. It is always positive for horizontal layouts, and zero for vertical 
	/// ones.
	/// </summary>
	float Advance;
	float MinY;
	float MaxY;
	/// <summary>
	/// The render mesh for this character
	/// </summary>
	RenderMesh Mesh;
};

typedef struct _font* Font;

struct _font {
	/// <summary>
	/// The characters for this font, the size of this array is (MAX_SUPPORTED_CHARACTERS)USHRT_MAX to support Unicode, 
	/// though most of the array will be null for any paticular font
	/// </summary>
	FontCharacter Characters[MAX_SUPPORTED_CHARACTERS];
	/// <summary>
	/// The index of the first character of the font
	/// </summary>
	size_t StartCharacter;
	/// <summary>
	/// The index of the last character of the font
	/// </summary>
	size_t EndCharacter;
	/// <summary>
	/// The number of characters this font supports
	/// </summary>
	size_t Count;
	/// <summary>
	/// The font material that should be used to render this font
	/// </summary>
	Material Material;
};

struct _fontMethods {
	GameObject(*CreateLine)(Font, char* buffer, size_t bufferLength);
	void(*Draw)(Font, unsigned short character, Camera camera);
	void(*SetMaterial)(Font, Material);
	Font(*Create)(Model);
	Font(*Import)(char* path, FileFormat format);
	void (*Dispose)(Font);
};

extern const struct _fontMethods Fonts;

struct _fontCharacterMethods {
	FontCharacter(*Create)(Mesh);
	void (*Dispose)(FontCharacter);
};

extern const struct _fontCharacterMethods FontCharacters;