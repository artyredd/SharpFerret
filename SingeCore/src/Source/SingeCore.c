// SingeCore.c
#include <stdio.h>
#include <stdlib.h>

#include "csharp.h"
#include "singine/memory.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "graphics/window.h"

#include "graphics/window.h"
#include "graphics/imaging.h"

#include "graphics/shadercompiler.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"

#include "input.h"
#include "modeling/importer.h"
#include "modeling/model.h"
#include "singine/parsing.h"
#include "graphics/renderMesh.h"
#include "graphics/camera.h"
#include "cglm/affine.h"
#include "math/quaternions.h"
#include "cglm/quat.h"
#include "singine/time.h"
#include "helpers/quickmask.h"
#include "singine/gameobjectHelpers.h"
#include "graphics/texture.h"
#include "graphics/colors.h"
#include "graphics/font.h"
#include <string.h>
#include "graphics/text.h"
#include "graphics/recttransform.h"
#include "singine/strings.h"
#include "graphics/graphicsDevice.h"
#include "tests.h"
#include "graphics/scene.h"
#include "physics/physics.h"

#include "graphics/renderbuffers.h"
#include "graphics/framebuffers.h"
#include "singine/defaults.h"
#include "graphics/drawing.h"

// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"
#include <physics/Collider.h>

#include "cglm/mat4.h"
#include "cglm/vec3.h"

#include "math/triangles.h"

Window window;

void DebugCameraPosition(Camera camera);
void ToggleNormalShaders(GameObject* gameobjects, size_t size, bool enabled);

int main()
{
	// UNIT TESTING
	//Tests.RunAll();

	// create a window to bind to GDI
	Windows.StartRuntime();

	Windows.SetHint(WindowHints.MSAASamples, 4);

	// use opengl 3.3
	Windows.SetHint(ContextHints.VersionMajor, 4);
	Windows.SetHint(ContextHints.VersionMinor, 4);

	Windows.SetHint(OpenGLHints.ForwardCompatibility, true); // To make MacOS happy; should not be needed
	Windows.SetHint(OpenGLHints.Profile, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able
	Windows.SetHint(WindowHints.Resizable, true);
	Windows.SetHint(WindowHints.Decorated, true);

	window = Windows.Create(DEFAULT_VIEWPORT_RESOLUTION_X, DEFAULT_VIEWPORT_RESOLUTION_Y, "Singine");

	// load window icon
	Image icon = Images.LoadImage("assets/textures/icon.png");
	Windows.SetIcon(window, icon);
	Images.Dispose(icon);

	SetInputWindow(window);

	Windows.SetClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// bind graphics card with GDI
	Windows.SetCurrent(window);

	Windows.SetMode(window, WindowModes.Windowed);

	SetCursorMode(CursorModes.Disabled);

	// initiliaze GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	// set up the basic GDI settings
	GraphicsDevice.EnableDepthTesting();
	GraphicsDevice.SetDepthTest(Comparisons.LessThan);

	GraphicsDevice.EnableCulling(CullingTypes.Back);

	GraphicsDevice.EnableStencil();
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	GraphicsDevice.SetStencilFull(Comparisons.Always, 1, 0xFF);
	GraphicsDevice.SetStencilMask(0xFF);

	// set frame cap to infinite for testing purposes
	glfwSwapInterval(0);

	// bind a vertex array for OpenGL this is required to render objects
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	// load the default material so we can render gameobjects that have no set material
	Material defaultMaterial = Materials.Load("assets/materials/default.material");
	GameObjects.SetDefaultMaterial(defaultMaterial);


	// load all the gameobjects we use in the main game loop
	GameObject skybox = GameObjects.Load("assets/prefabs/skybox.gameobject");
	GameObject fox = GameObjects.Load("assets/prefabs/fox.gameobject");
	GameObject cube = GameObjects.Load("assets/prefabs/cube.gameobject");
	
	GameObject otherCube = GameObjects.Duplicate(cube);
	GameObjects.SetMaterial(otherCube, defaultMaterial);
	Transforms.ScaleAll(otherCube->Transform, 1.2f);
	Materials.SetColors(otherCube->Material, 1, 1, 1, 1);

	GameObject ball = GameObjects.Load("assets/prefabs/ball.gameobject");
	GameObject otherBall = GameObjects.Load("assets/prefabs/ball.gameobject");

	GameObject car = GameObjects.Load("assets/prefabs/proto.gameobject");
	Materials.SetColor(car->Material, Colors.Green);

	GameObject statue = GameObjects.Load("assets/prefabs/statue.gameobject");
	GameObject otherStatue = GameObjects.Load("assets/prefabs/mirrorStatue.gameobject");
	Transforms.RotateOnAxis(otherStatue->Transform, (float)GLM_PI/2, Vector3.Up);

	GameObject reflectiveSphere = GameObjects.Load("assets/prefabs/reflectiveSphere.gameobject");
	GameObject sphere = GameObjects.Load("assets/prefabs/sphere.gameobject");
	GameObject lightMarker = GameObjects.Load("assets/prefabs/cube.gameobject");
	 
	// assign the area textures so reflective materials will reflect the skybox
	Materials.SetAreaTexture(cube->Material, skybox->Material->MainTexture);
	Materials.SetReflectionTexture(cube->Material, cube->Material->SpecularTexture);

	Materials.SetAreaTexture(reflectiveSphere->Material, skybox->Material->MainTexture);
	Materials.SetAreaTexture(statue->Material, skybox->Material->MainTexture);
	Materials.SetAreaTexture(otherStatue->Material, skybox->Material->MainTexture);

	// load the font we use in-game
	Font font = Fonts.Import("assets/fonts/ComicMono.obj", FileFormats.Obj);

	// since text-mesh fonts are actual gameobjects like all others, set the material so we can see it
	Material textMaterial = Materials.Load("assets/materials/textMaterial.material");

	Fonts.SetMaterial(font, textMaterial);

	Materials.Dispose(textMaterial);

	// create a text object so we can display frame times
	Text text = Texts.CreateEmpty(font, 512);

	float fontSize = 0.06125f;

	RectTransforms.SetTransform(text->GameObject->Transform, Anchors.UpperLeft, Pivots.UpperLeft, 0, -fontSize, fontSize, fontSize);

	// create a test light for manual testing
	Light light = Lights.Create(LightTypes.Directional);

	// light body
	Transforms.SetPositions(light->Transform, -1, 20, -20);

	Transforms.SetPositions(lightMarker->Transform, 0, 0, 0);

	Material outlineMaterial = Materials.Load("assets/materials/outline.material");

	GameObjects.SetMaterial(lightMarker, outlineMaterial);

	Materials.Dispose(outlineMaterial);

	Transforms.ScaleAll(lightMarker->Transform, 0.25f);

	Transforms.SetParent(lightMarker->Transform, light->Transform);

	// point directional light at center
	Transforms.LookAt(light->Transform, Vector3.Zero);

	light->Enabled = true;
	light->Radius = 100.0f;
	light->Range = 100.0f;
	light->Intensity = 0.5f;
	light->Orthographic = true;

	// create a scene to render
	Camera camera = Cameras.Create();
	Cameras.SetFarClippingDistance(camera, 500.0f);
	camera->Orthographic = false;

	// don't spawn camera at origin, it's disorienting
	Transforms.SetPositions(camera->Transform, -3, 3, 3);

	Scene scene = Scenes.Create();

	scene->MainCamera = camera;

	// add the light to the scene
	Scenes.AddLight(scene, light);

	// collider testing
	GameObject colliderObject1 = GameObjects.Load("assets/prefabs/cube.gameobject");
	GameObject colliderObject2 = GameObjects.Load("assets/prefabs/cube.gameobject");

	Materials.SetColors(colliderObject1->Material, 1.0, 0.0, 0.0, 0.0);
	Materials.SetColors(colliderObject2->Material, 0.0, 1.0, 0.0, 0.0);

	Collider collider1 = Colliders.Load("assets/colliders/cube.collider");
	Collider collider2 = Colliders.Load("assets/colliders/cube.collider");

	collider1->Transform = colliderObject1->Transform;
	collider2->Transform = colliderObject2->Transform;

	Transforms.SetPositions(colliderObject1->Transform, 10, 10, 10);
	Transforms.SetPositions(colliderObject2->Transform, 10, 10, 10);

	// group up the gameobjects so we can call GameObjects.DrawMany instead of .Draw for each object
	GameObject gameobjects[] = {
		lightMarker,
		cube,
		sphere,
		car,
		ball,
		otherBall,
		fox,
		statue,
		otherStatue,
		reflectiveSphere,
		colliderObject1,
		colliderObject2
	};

	const size_t gameobjectCount = sizeof(gameobjects) / sizeof(GameObject);

	// load the shadow material used to render shadowmaps
	// and create a camera that should be used to render to the framebuffer for shadows
	Camera shadowCamera = Cameras.Create();

	Material shadowMapMaterial = Materials.Load("assets/materials/shadow.material");

	// main game loop
	bool showNormals = false;

	ToggleNormalShaders(gameobjects, gameobjectCount, showNormals);

	// set some parents and positions so all the objects aren't all sitting at world origin
	Transforms.SetParent(otherBall->Transform, ball->Transform);
	Transforms.SetScales(otherBall->Transform, 0.5, 0.5, 0.5);
	Transforms.SetPositions(otherBall->Transform, 0, 0, 3);
	Transforms.SetPositions(car->Transform, -7, 0, -7);
	Transforms.SetPositions(ball->Transform, 5, 1, 5);
	Transforms.SetRotationOnAxis(statue->Transform, (float)GLM_PI/2, Vector3.Up);


	float speed = 10.0f;

	float rotateAmount = 0.0f;

	vec3 position;

	vec3 positionModifier; 
	vec3 lightOffset = { -20, 20, 0 };

	double colliderPosition = 10.0;

	bool intersects = false;

	Drawing.SetScene(scene);

	Material unlit = Materials.Load("assets/materials/unlit.material");

	// we update time once before the start of the program becuase if startup takes a long time delta time may be large for the first call                                                                                                                                                       
	double timer = 0;
	Time.Update();
	do {
		// ensure deltaTime is updated
		Time.Update();

		float modifier = speed * (float)Time.DeltaTime();

		rotateAmount += modifier;

		// move the soccer balls to test transform inheritance
		vec3 ballPosition = { 5, (2 + (float)sin(rotateAmount)), 5 };

		Transforms.SetPosition(ball->Transform, ballPosition);

		Transforms.SetRotationOnAxis(ball->Transform, rotateAmount / (float)GLM_PI, Vector3.Up);

		Transforms.SetRotationOnAxis(otherBall->Transform, -rotateAmount / (float)GLM_PI, Vector3.Up);

		Transforms.RotateOnAxis(collider1->Transform, Time.DeltaTime(), Vector3.Right);
		Transforms.RotateOnAxis(collider1->Transform, Time.DeltaTime(), Vector3.Up);

		// drive car
		vec3 carDirection;
		Transforms.GetDirection(car->Transform, Directions.Back, carDirection);

		ScaleVector3(carDirection, modifier);

		Transforms.AddPosition(car->Transform, carDirection);

		Transforms.RotateOnAxis(car->Transform, ((float)GLM_PI / 8.0f) * modifier, Vector3.Up);

		// rotate sun
		vec3 newLightPos;

		Quaternion lightRotation;
		glm_quatv(lightRotation, (rotateAmount / (float)GLM_PI)/16.0f, Vector3.Up);
		glm_quat_rotatev(lightRotation, lightOffset, newLightPos);

		Transforms.SetPosition(light->Transform, lightOffset);
		Transforms.LookAt(light->Transform, Vector3.Zero);
		//Transforms.RotateOnAxis(lightPivot, , Vector3.Up);

		// make a copy of camera's position
		Vectors3CopyTo(camera->Transform->Position, position);

		if (GetKey(KeyCodes.A))
		{
			Transforms.GetDirection(camera->Transform, Directions.Left, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.D))
		{
			Transforms.GetDirection(camera->Transform, Directions.Right, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.W))
		{
			Transforms.GetDirection(camera->Transform, Directions.Forward, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.S))
		{
			Transforms.GetDirection(camera->Transform, Directions.Back, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.Space))
		{
			Transforms.GetDirection(camera->Transform, Directions.Up, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.LeftShift))
		{
			Transforms.GetDirection(camera->Transform, Directions.Down, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.Left))
		{
			colliderPosition -= Time.DeltaTime();
		}
		if (GetKey(KeyCodes.Right))
		{
			colliderPosition += Time.DeltaTime();
		}

		vec3 newColliderPos;

		Vectors3CopyTo(collider2->Transform->Position, newColliderPos);
		
		newColliderPos[1] = (float)colliderPosition;

		Transforms.SetPosition(collider2->Transform, newColliderPos);

		// toggle debug normals
		if (GetKey(KeyCodes.N))
		{
			ToggleNormalShaders(gameobjects, gameobjectCount, !showNormals);

			showNormals = !showNormals;
		}

		Transforms.SetPositions(otherCube->Transform, (float)(3 * cos(Time.Time())), 3, 3);

		Transforms.SetRotationOnAxis(cube->Transform, (float)(3 * cos(Time.Time())), Vector3.Up);

		int count = sprintf_s(text->Text, text->Length, 
			"%2.4lf ms (high:%2.4lf ms avg:%2.4lf)\n%4.1lf FPS\nIntersecting:%s",
			Time.Statistics.FrameTime(),
			Time.Statistics.HighestFrameTime(),
			Time.Statistics.AverageFrameTime(),
			1.0 / Time.Statistics.FrameTime(), intersects ? "true" : "false");

		Texts.SetText(text, text->Text, count);

		// update the FPS camera
		Transforms.SetPosition(camera->Transform, position);
		FPSCamera.Update(camera);

		// before we draw anything we should update the physics system
		Physics.Update( Time.DeltaTime() );

		GameObjects.GenerateShadowMaps(gameobjects, sizeof(gameobjects) / sizeof(GameObject), scene, shadowMapMaterial, shadowCamera);

		// draw scene
		FrameBuffers.ClearAndUse(FrameBuffers.Default);

		scene->MainCamera = camera;

		GameObjects.DrawMany(gameobjects, sizeof(gameobjects) / sizeof(GameObject), scene, null);
		
		// 0.01
		timer += Time.DeltaTime();
		if (timer >= 0.01)
		{
			timer = 0;

			intersects = Colliders.Intersects(collider1, collider2);

			if (intersects)
			{
				Materials.SetColors(colliderObject1->Material, 0.0, 1.0, 1.0, 0.0);
				Materials.SetColors(colliderObject2->Material, 1.0, 1.0, 0.0, 0.0);
			}
			else
			{
				Materials.SetColors(colliderObject1->Material, 1.0, 0.0, 0.0, 0.0);
				Materials.SetColors(colliderObject2->Material, 0.0, 1.0, 0.0, 0.0);
			}
		}
		

		GameObjects.Draw(skybox, scene);

		Texts.Draw(text, scene);

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();

	} while (GetKey(KeyCodes.Escape) != true && Windows.ShouldClose(window) != true);

	// destroy all the game loop objects
	Lights.Dispose(light);

	Texts.Dispose(text);

	Fonts.Dispose(font);

	GameObjects.Destroy(ball);
	//GameObjects.Destroy(otherBall);
	GameObjects.Destroy(car);
	GameObjects.Destroy(fox);
	GameObjects.Destroy(cube);
	//GameObjects.Destroy(otherCube);
	GameObjects.Destroy(lightMarker);
	GameObjects.Destroy(reflectiveSphere);
	GameObjects.Destroy(skybox);
	GameObjects.Destroy(sphere);
	GameObjects.Destroy(statue);
	GameObjects.Destroy(otherStatue);

	Materials.Dispose(defaultMaterial);
	Materials.Dispose(shadowMapMaterial);

	Cameras.Dispose(camera);
	Cameras.Dispose(shadowCamera);

	Scenes.Dispose(scene);

	Windows.Dispose(window);

	Windows.StopRuntime();

	PrintAlloc(stdout);
	PrintFree(stdout);

	// make sure we didn't orphan any textures, buffers, etc.. on the GDI
	if (GraphicsDevice.TryVerifyCleanup() is false)
	{
		// theres a known memory leak somewhere in the transform/gameobject dept,
		// but ill fix that eventually
		//throw(MemoryLeakException);
	}
}

void DebugCameraPosition(Camera camera)
{
	fprintf(stdout, "Position: ");
	PrintVector3(camera->Transform->Position, stdout);
	fprintf(stdout, " Rotation: [ x: %0.2fpi, y: %0.2fpi ] Forward: ", FPSCamera.State.HorizontalAngle / GLM_PI, FPSCamera.State.VerticalAngle / GLM_PI);
	vec3 forwardVector;
	Transforms.GetDirection(camera->Transform, Directions.Forward, forwardVector);
	PrintVector3(forwardVector, stdout);
	fprintf(stdout, NEWLINE);
}

void ToggleNormalShaders(GameObject* gameobjects, size_t size, bool enabled)
{
	for (size_t i = 0; i < size; i++)
	{
		// get the material
		Material material = gameobjects[i]->Material;

		// iterate the shaders and disable any normal shaders by name
		for (size_t ithShader = 0; ithShader < material->Count; ithShader++)
		{
			static const char* path = "assets/shaders/debug_normal.shader";
			static const size_t length = sizeof("assets/shaders/debug_normal.shader") - 1;

			Shader shader = material->Shaders[ithShader];

			if (Strings.Equals(path, length, shader->Name, strlen(shader->Name)))
			{
				shader->Enabled = enabled;
			}
		}
	}
}