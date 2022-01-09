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

// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"

Window window;

Shader LoadShader(const char*, const char*);
void DebugCameraPosition(Camera camera);

int main()
{
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

	SetCursorMode(CursorModes.Disabled);

	Image icon = Images.LoadImage("icon.png");

	Windows.SetIcon(window, icon);

	Windows.SetMode(window, WindowModes.FullScreen);

	glfwSwapInterval(0);

	Windows.SetHint(WindowHints.RefreshRate, 30);

	Images.Dispose(icon);

	Shader texturedShader = LoadShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	Shader uvShader = LoadShader("SimpleVertexShader.vertexshader", "ColorUV.fragmentshader");

	Shader guiShader = LoadShader("GUIShader.vertexshader", "SimpleFragmentShader.fragmentshader");
	Shader glowShader = LoadShader("DistanceGlow.vertexshader", "DistanceGlow.fragmentshader");

	// check shader instancing and disposing
	for (size_t i = 0; i < 5; i++)
	{
		Shader instance = Shaders.Instance(texturedShader);

		Shaders.Dispose(instance);
	}

	if (texturedShader->Handle->ActiveInstances != 1)
	{
		throw(InvalidArgumentException);
	}

	Image uv = Images.LoadImage("cubeuv.png");

	Texture cubeTexture;
	if (Textures.TryCreateTexture(uv, &cubeTexture) is false)
	{
		throw(FailedToOpenFileException);
	}

	Images.Dispose(uv);

	// test texture disposing
	for (size_t i = 0; i < 5; i++)
	{
		Texture newInstance = Textures.Instance(cubeTexture);

		Textures.Dispose(newInstance);
	}

	if (cubeTexture->Handle->ActiveInstances != 1)
	{
		throw(InvalidArgumentException);
	}

	Material texturedMaterial = Materials.Create(texturedShader, cubeTexture);

	Material uvMaterial = Materials.Create(uvShader, null);

	Material guiMaterial = Materials.Create(guiShader, null);

	Material glowMaterial = Materials.Create(guiShader, null);

	Materials.EnableSetting(guiMaterial, MaterialSettings.Transparency);
	Materials.DisableSetting(guiMaterial, MaterialSettings.UseCameraPerspective);
	Materials.DisableSetting(guiMaterial, MaterialSettings.BackfaceCulling);

	Materials.DisableSetting(glowMaterial, MaterialSettings.UseCameraPerspective);
	Materials.DisableSetting(glowMaterial, MaterialSettings.BackfaceCulling);

	GameObjects.SetDefaultMaterial(uvMaterial);

	Camera camera = Cameras.CreateCamera();

	// bind a vertex array for OpenGL this is required to render objects
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	GameObject cube = LoadGameObjectFromModel("cube.obj", FileFormats.Obj);

	Font font = Fonts.Import("ComicMono.obj", FileFormats.Obj);
	Fonts.SetMaterial(font, glowMaterial);
	Materials.SetColor(font->Material, Colors.Red);

	GameObject ball = LoadGameObjectFromModel("ball.obj", FileFormats.Obj);
	GameObjects.SetMaterial(ball, uvMaterial);

	GameObject otherBall = GameObjects.Duplicate(ball);
	GameObject car = LoadGameObjectFromModel("car.obj", FileFormats.Obj);
	GameObject room = LoadGameObjectFromModel("room.obj", FileFormats.Obj); //GameObjects.Duplicate(ball);// //


	GameObjects.SetMaterial(car, texturedMaterial);
	Materials.SetColor(car->Material, Colors.Green);

	GameObjects.SetMaterial(cube, texturedMaterial);

	size_t previosInstanceCount = ball->Material->Shader->Handle->ActiveInstances;

	//test memory leak
	for (size_t i = 0; i < 5; i++)
	{
		GameObject tmp = GameObjects.Duplicate(ball);

		GameObjects.Destroy(tmp);
	}

	// check memory leak for GameObjects
	if (ball->Material->Shader->Handle->ActiveInstances != previosInstanceCount)
	{
		throw(InvalidArgumentException);
	}

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

	GameObject guiTexture = CreateGameObjectFromMesh(squareMesh);

	SafeFree(squareMesh);

	GameObjects.SetMaterial(guiTexture, guiMaterial);

	Image debugUv = Images.LoadImage("uv_debug.png");

	Texture debugUvTexture;
	if (Textures.TryCreateTexture(debugUv, &debugUvTexture) is false)
	{
		throw(InvalidArgumentException);
	}

	Images.Dispose(debugUv);

	Materials.SetMainTexture(guiTexture->Material, debugUvTexture);

	Textures.Dispose(debugUvTexture);

	// orient the camera so we see some geometry without moving the camera
	Transforms.SetPositions(camera->Transform, 2.11f, 1.69f, 8.39f);

	Materials.SetColor(cube->Material, Colors.Red);

	/*float width = 1;
	float height = 0.5;

	Transforms.SetPositions(guiTexture->Transform, (1.0 / width) - 1.0, (1 / height) - 1.0, 0);
	Transforms.SetScales(guiTexture->Transform, width, height, 1);*/


	GameObject otherGuiTexture = GameObjects.Duplicate(guiTexture);

	Transforms.SetParent(guiTexture->Transform, otherGuiTexture->Transform);
	Transforms.SetScales(otherGuiTexture->Transform, 1, 1, 1);
	Transforms.SetPositions(otherGuiTexture->Transform, -0.5, -0.5, 0);

	Materials.SetColor(guiTexture->Material, Colors.Red);
	Materials.SetColor(otherGuiTexture->Material, Colors.Green);

	/*char* word = "The quick brown fox jumped over the fence!";

	Text text = Texts.CreateText(font, word, strlen(word));

	char* largerWord = "The quick orange fox jumped over the fence!";
	Texts.SetText(text, largerWord, strlen(largerWord));

	Texts.SetText(text, word, strlen(word));

	int count = sprintf_s(text->Text, text->Length, "%0.4f", 69.884848484f);
	Texts.SetText(text, text->Text, count);*/

	Text text = Texts.CreateEmpty(font, 512);

	float fontSize = 0.06125f;

	Transforms.ScaleAll(text->GameObject->Transform, fontSize);

	Transforms.SetPositions(text->GameObject->Transform, -(1 / fontSize), (1 / fontSize) - text->Font->LineHeight, 0);

	float amount = 0;

	// we update time once before the start of the program becuase if startup takes a long time delta time may be large for the first call
	UpdateTime();
	do {
		UpdateTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		int count = sprintf_s(text->Text, text->Length, "%2.4lf ms (high:%2.4lf ms avg:%2.4lf)\n%4.1lf FPS", FrameTime(), HighestFrameTime(), AverageFrameTime(), 1.0 / FrameTime());//, );
		Texts.SetText(text, text->Text, count);

		FPSCamera.Update(camera);

		Transforms.SetPosition(camera->Transform, position);

		GameObjects.Draw(cube, camera);

		GameObjects.Draw(car, camera);
		GameObjects.Draw(ball, camera);
		GameObjects.Draw(otherBall, camera);
		GameObjects.Draw(room, camera);

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
	GameObjects.Destroy(guiTexture);
	GameObjects.Destroy(otherGuiTexture);

	Textures.Dispose(cubeTexture);

	Materials.Dispose(texturedMaterial);
	Materials.Dispose(uvMaterial);
	Materials.Dispose(guiMaterial);
	Materials.Dispose(glowMaterial);

	Cameras.Dispose(camera);

	Shaders.Dispose(texturedShader);

	Shaders.Dispose(uvShader);

	Shaders.Dispose(glowShader);

	Shaders.Dispose(guiShader);

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

void BeforeDraw(Shader shader, mat4 mvp)
{
	glUseProgram(shader->Handle->Handle);

	int handle;
	if (Shaders.TryGetUniform(shader, Uniforms.MVP, &handle) is false)
	{
		return; //throw(FailedToLocationMVPUniformException);
	}

	glUniformMatrix4fv(handle, 1, false, &mvp[0][0]);
}

Shader LoadShader(const char* vertexPath, const char* fragmentPath)
{
	Shader shader = ShaderCompilers.CompileShader(vertexPath, fragmentPath);

	if (shader is null)
	{
		throw(FailedToCompileShaderException);
	}

	shader->BeforeDraw = &BeforeDraw;

	return shader;
}