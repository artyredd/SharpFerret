#pragma once
#include "graphics/transform.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "graphics/material.h"
#include "graphics/scene.h"

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
	/// <summary>
	/// Retrieves a new instance of the default material
	/// </summary>
	Material(*GetDefaultMaterial)(void);
	/// <summary>
	/// Sets the default material all gameobjects start with, unless a material is provided on their creation, this only stores a pointer to the provided material
	/// the material is not instanced and will not be Disposed any of these methods, it's the callers responsibility to dispose of the provided material
	/// </summary>
	void(*SetDefaultMaterial)(Material);
	GameObject(*CreateWithMaterial)(Material);
	GameObject(*Create)();
	/// <summary>
	/// Creates an empty gameobject with no material with the given size of rendermeshes
	/// </summary>
	GameObject(*CreateEmpty)(size_t count);
	/// <summary>
	/// Clears the provided gameobject's rendermesh array
	/// </summary>
	void (*Clear)(GameObject);
	/// <summary>
	/// Clears and changes the length of the rendermesh array to the provided count
	/// </summary>
	void (*Resize)(GameObject, size_t count);
	void (*DrawMany)(GameObject* array, size_t count, Scene, Material override);
	void (*Draw)(GameObject, Scene);
	GameObject(*Duplicate)(GameObject);
	void (*SetName)(GameObject, char* name);
	/// <summary>
	/// Creates a new instance of the provided material, disposes the old assigned material, then sets the material, provided material can be null
	/// </summary>
	void(*SetMaterial)(GameObject, Material);
	void (*DestroyMany)(GameObject* array, size_t count);
	void (*Destroy)(GameObject);
	GameObject(*Load)(const char* path);
	bool (*Save)(GameObject, const char* path);
	void (*GenerateShadowMaps)(GameObject* array, size_t count, Scene scene, Material shadowMaterial, Material cubemapShadowMaterial, Camera shadowCamera);
};

const extern struct _gameObjectMethods GameObjects;