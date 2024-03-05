#pragma once
#include "engine/graphics/light.h"
#include "engine/graphics/camera.h"

typedef struct _scene* Scene;

struct _scene {
	/// <summary>
	/// The camera that should be used to render the scene
	/// </summary>
	Camera MainCamera;
	/// <summary>
	/// The array of point, directional, and spot lights within the scene
	/// </summary>
	Light* Lights;
	/// <summary>
	/// The number of lights within the lights array
	/// </summary>
	ulong LightCount;
};

struct _sceneMethods {
	Scene(*Create)(void);
	void(*Dispose)(Scene);
	void (*AddLight)(Scene, Light);
};

extern const struct _sceneMethods Scenes;