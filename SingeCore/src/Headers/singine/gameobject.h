#pragma once
#include "graphics/transform.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/material.h"

#define MAX_GAMEOBJECT_NAME_LENGTH 512 // in ANSI characters(bytes)

typedef struct _gameObject* GameObject;

struct _gameObject {
	/// <summary>
	/// Unique Identifier for this object
	/// </summary>
	size_t Id;
	/// <summary>
	/// A pointer to the name of this object
	/// </summary>
	char* Name;
	size_t NameLength;
	/// <summary>
	/// The transform for this object
	/// </summary>
	Transform Transform;
	/// <summary>
	/// The array of render meshes for this object, if it has any
	/// </summary>
	RenderMesh* Meshes;
	/// <summary>
	/// The number of render meshes this object controls
	/// </summary>
	size_t Count;
	Material Material;
};

struct _gameObjectMethods {
	GameObject(*Create)();
	void (*Draw)(GameObject, Camera);
	GameObject(*Duplicate)(GameObject);
	void (*SetName)(GameObject, char* name);
	/// <summary>
	/// Creates a new instance of the provided material, disposes the old assigned material, then sets the material, provided material can be null
	/// </summary>
	void(*SetMaterial)(GameObject, Material);
	void (*Destroy)(GameObject);
};

const extern struct _gameObjectMethods GameObjects;