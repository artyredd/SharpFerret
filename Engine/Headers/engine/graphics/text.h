#pragma once

#include "engine/gameobject.h"
#include "engine/graphics/font.h"

struct _textState {
	/// <summary>
	/// A flag bitmask containing information on what portions of a text object have changed and should be updated on the next draw call
	/// </summary>
	unsigned int Modified;
};

typedef struct _text* Text;

struct _text {
	/// <summary>
	/// The underlying text for this gui text
	/// </summary>
	char* Text;
	/// <summary>
	/// The length of the text array
	/// </summary>
	ulong Length;
	/// <summary>
	/// The number of characters in the string, this should always be less than or equal to the length of the text array
	/// </summary>
	ulong Count;
	/// <summary>
	/// The font that should be used to render this text
	/// </summary>
	Font Font;
	/// <summary>
	/// Whether or not text should be rendered top-to-botton(true) or bottom-to-top(false)
	/// </summary>
	bool AlignTop;
	/// <summary>
	/// The underlying gameobject that should be drawn
	/// </summary>
	GameObject GameObject;
	struct _textState State;
};

struct _textMethods {
	Text(*Create)(void);
	Text(*CreateText)(Font font, char* string, ulong size);
	Text(*CreateEmpty)(Font font, ulong size);
	void (*Dispose)(Text);
	void (*Draw)(Text, Scene);
	void(*SetDefaultFont)(Font);
	Font(*GetDefaultFont)(void);
	void (*SetFont)(Text, Font font);
	void(*SetCharacter)(Text, ulong index, unsigned int newCharacter);
	void (*SetText)(Text, char* string, ulong size);
};

extern const struct _textMethods Texts;