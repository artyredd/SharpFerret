#pragma once

#include "engine/gameobject.h"
#include "engine/graphics/material.h"
#include <engine/modeling/importer.h>
#include "engine/graphics/scene.h"

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
	/// <summary>
	/// The lowest part of the character
	/// </summary>
	float MinY;
	/// <summary>
	/// The largest part of of the character
	/// </summary>
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
	/// The lowest point in the font
	/// </summary>
	float MinY;
	/// <summary>
	/// The tallest point in the font
	/// </summary>
	float MaxY;
	/// <summary>
	/// This is distance between the lowest and highest mesh point in the font
	/// </summary>
	float LineHeight;
	/// <summary>
	/// The fonts base EM size which is 70% the height of the largest capital letter
	/// </summary>
	float EmSize;
	/// <summary>
	/// The width of a white space character, tab is 4 of these
	/// </summary>
	float SpaceWidth;
	/// <summary>
	/// The font material that should be used to render this font
	/// </summary>
	Material Material;
	/// <summary>
	/// The number of instances of this font, when this reaches 1 or less any following calls to dispose actually disposes the object itself
	/// </summary>
	size_t ActiveInstances;
};

struct _fontMethods {
	void(*Draw)(Font, unsigned short character, Scene scene);
	void(*SetMaterial)(Font, Material);
	Font(*Create)(Model);
	Font(*Import)(char* path, FileFormat format);
	void (*Dispose)(Font);
	Font(*Instance)(Font);
	FontCharacter(*GetCharacter)(Font, unsigned int character);
};

extern const struct _fontMethods Fonts;

struct _fontCharacterMethods {
	FontCharacter(*Create)(Mesh);
	void (*Dispose)(FontCharacter);
};

extern const struct _fontCharacterMethods FontCharacters;