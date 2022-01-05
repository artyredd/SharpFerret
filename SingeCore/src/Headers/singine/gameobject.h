#pragma once
#include "graphics/transform.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/material.h"

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
	void (*Draw)(GameObject, Camera);
	GameObject(*Duplicate)(GameObject);
	void (*SetName)(GameObject, char* name);
	void (*Destroy)(GameObject);
};

const extern struct _gameObjectMethods GameObjects;

GameObject CreateGameObject();