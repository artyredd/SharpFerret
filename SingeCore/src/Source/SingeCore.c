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
#include "input.h"
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

#include "tests.h"

// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"

Window window;

void DebugCameraPosition(Camera camera);

int main()
{
	Tests.RunAll();

	Windows.StartRuntime();

	Windows.SetHint(WindowHints.MSAASamples, 4);

	// use opengl 3.3
	Windows.SetHint(ContextHints.VersionMajor, 3);
	Windows.SetHint(ContextHints.VersionMinor, 3);

	Windows.SetHint(OpenGLHints.ForwardCompatibility, true); // To make MacOS happy; should not be needed
	Windows.SetHint(OpenGLHints.Profile, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able
	Windows.SetHint(WindowHints.Resizable, true);
	Windows.SetHint(WindowHints.Decorated, true);

	window = Windows.Create(1920, 1080, "Singine");

	SetInputWindow(window);

	//sWindow.SetMode(window, WindowModes.FullScreen);

	Windows.SetClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// bind graphics card with GDI
	Windows.SetCurrent(window);

	// initiliaze GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		throw(IndexOutOfRangeException);
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glEnable(GL_STENCIL_TEST);


	SetCursorMode(CursorModes.Disabled);

	Image icon = Images.LoadImage("assets/textures/icon.png");

	Windows.SetIcon(window, icon);

	Windows.SetMode(window, WindowModes.Windowed);

	glfwSwapInterval(0);

	Images.Dispose(icon);

	// load the default material so we can render gameobjects that have no set material
	Material defaultMaterial = Materials.Load("assets/materials/default.material");
	GameObjects.SetDefaultMaterial(defaultMaterial);

	Material textMaterial = Materials.Load("assets/materials/textMaterial.material");

	Material textureMaterial = Materials.Load("assets/materials/debugOrientationGUI.material");

	Camera camera = Cameras.CreateCamera();

	// bind a vertex array for OpenGL this is required to render objects
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	Font font = Fonts.Import("assets/fonts/ComicMono.obj", FileFormats.Obj);
	Fonts.SetMaterial(font, textMaterial);

	GameObject ball = GameObjects.Load("assets/prefabs/ball.gameobject");

	GameObject cube = GameObjects.Load("assets/prefabs/cube.gameobject");

	GameObject otherBall = GameObjects.Duplicate(ball);
	GameObject car = GameObjects.Load("assets/prefabs/car.gameobject");
	GameObject room = GameObjects.Load("assets/prefabs/room.gameobject");

	float speed = 10.0f;

	float rotateAmount = 0.0f;

	vec3 position;

	vec3 positionModifier;

	Transforms.SetParent(otherBall->Transform, ball->Transform);
	Transforms.SetScales(otherBall->Transform, 0.5, 0.5, 0.5);
	Transforms.SetPositions(otherBall->Transform, 0, 0, 3);
	Transforms.SetPositions(car->Transform, -7, 0, -7);
	Transforms.SetPositions(cube->Transform, 3, 3, 3);
	Transforms.SetPositions(ball->Transform, 5, 1, 5);

	Mesh squareMesh = Meshes.Create();

	float verts[18] = {
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
		-1, 1, 0,
		1, -1, 0
	};

	float textures[12] = {
		0,0,
		0,1,
		1,1,
		1,0,
		0,0,
		1,1,
	};

	squareMesh->VertexCount = 3 * 2 * 3;
	squareMesh->Vertices = verts;

	squareMesh->TextureCount = 12;
	squareMesh->TextureVertices = textures;

	GameObject square = CreateGameObjectFromMesh(squareMesh);

	Transforms.ScaleAll(square->Transform, 0.5f);

	SafeFree(squareMesh);

	GameObjects.SetMaterial(square, font->Material);

	GameObject subSquare = GameObjects.Duplicate(square);

	GameObjects.SetMaterial(subSquare, textureMaterial);

	Materials.SetColors(subSquare->Material, 1, 0, 0, 1);

	Transforms.SetParent(subSquare->Transform, square->Transform);

	RectTransforms.SetTransform(subSquare->Transform, Anchors.LowerLeft, Pivots.UpperLeft, 0, 0, 0.5f, 0.5f);


	// orient the camera so we see some geometry without moving the camera
	//Transforms.SetPositions(camera->Transform, 2.11f, 1.69f, 8.39f);
	Transforms.SetPositions(camera->Transform, 0, 0, 0);

	Text text = Texts.CreateEmpty(font, 512);

	float fontSize = 0.06125f;

	RectTransforms.SetTransform(text->GameObject->Transform, Anchors.UpperLeft, Pivots.UpperLeft, 0, 0, fontSize, fontSize);

	float amount = 0;

	Materials.SetColor(cube->Material, Colors.Red);
	Materials.SetColor(car->Material, Colors.Green);

	GameObjects.Save(cube, "assets/prefabs/cube.gameobject");
	GameObjects.Save(ball, "assets/prefabs/ball.gameobject");
	GameObjects.Save(car, "assets/prefabs/car.gameobject");
	GameObjects.Save(room, "assets/prefabs/room.gameobject");


	GameObject otherCube = GameObjects.Duplicate(cube);

	//Transforms.SetParent(otherCube->Transform, cube->Transform);
	//Transforms.SetPositions(otherCube->Transform, 0, 0, 0);
	Transforms.ScaleAll(otherCube->Transform, 1.2f);
	Materials.SetColors(otherCube->Material, 1, 1, 1, 1);


	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	// we update time once before the start of the program becuase if startup takes a long time delta time may be large for the first call
	UpdateTime();
	do {
		UpdateTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		Vectors3CopyTo(camera->Transform->Position, position);

		float modifier = speed * (float)DeltaTime();

		rotateAmount += modifier;

		vec3 ballPosition = { 5,2 + (float)sin(rotateAmount), 5 };

		Transforms.SetPosition(ball->Transform, ballPosition);

		Transforms.SetRotationOnAxis(ball->Transform, rotateAmount / (float)GLM_PI, Vector3.Up);

		Transforms.SetRotationOnAxis(otherBall->Transform, -rotateAmount / (float)GLM_PI, Vector3.Up);

		// drive car
		vec3 carDirection;
		Transforms.GetDirection(car->Transform, Directions.Forward, carDirection);

		ScaleVector3(carDirection, modifier);

		Transforms.AddPosition(car->Transform, carDirection);

		Transforms.RotateOnAxis(car->Transform, ((float)GLM_PI / 8.0f) * modifier, Vector3.Up);

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
		if (GetAxis(Axes.Horizontal) < 0)
		{
			Transforms.TranslateX(text->GameObject->Transform, -modifier);
		}
		else if (GetAxis(Axes.Horizontal) > 0)
		{
			Transforms.TranslateX(text->GameObject->Transform, modifier);
		}
		if (GetAxis(Axes.Vertical) < 0)
		{
			++amount;
		}
		else if (GetAxis(Axes.Vertical) > 0)
		{
			--amount;
		}

		Transforms.SetPositions(otherCube->Transform, (float)(3 * cos(Time())), 3, 3);
		Transforms.SetPositions(cube->Transform, (float)(3 * cos(Time())), 3, 3);

		int count = sprintf_s(text->Text, text->Length, "%2.4lf ms (high:%2.4lf ms avg:%2.4lf)\n%4.1lf FPS", FrameTime(), HighestFrameTime(), AverageFrameTime(), 1.0 / FrameTime());//, );
		Texts.SetText(text, text->Text, count);

		FPSCamera.Update(camera);

		Transforms.SetPosition(camera->Transform, position);

		GameObjects.Draw(otherCube, camera);
		GameObjects.Draw(cube, camera);

		GameObjects.Draw(car, camera);
		GameObjects.Draw(ball, camera);
		GameObjects.Draw(otherBall, camera);
		GameObjects.Draw(room, camera);

		//GameObjects.Draw(subSquare, camera);
		//GameObjects.Draw(square, camera);
		Texts.Draw(text, camera);

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();

		//fprintf(stdout,"Total: %0.4fs	FrameTime: %0.4fms"NEWLINE, Time(), FrameTime() * 1000.0);
	} while (GetKey(KeyCodes.Escape) != true && Windows.ShouldClose(window) != true);

	Texts.Dispose(text);

	Fonts.Dispose(font);

	GameObjects.Destroy(ball);
	GameObjects.Destroy(otherBall);
	GameObjects.Destroy(car);
	GameObjects.Destroy(room);
	GameObjects.Destroy(cube);
	GameObjects.Destroy(square);
	GameObjects.Destroy(otherCube);
	GameObjects.Destroy(subSquare);

	Materials.Dispose(textMaterial);
	Materials.Dispose(defaultMaterial);
	Materials.Dispose(textureMaterial);

	Cameras.Dispose(camera);

	Windows.Dispose(window);

	Windows.StopRuntime();

	// ensure leak free
	PrintAlloc(stdout);
	PrintFree(stdout);

	if (AllocCount() > FreeCount())
	{
		throw(MemoryLeakException);
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