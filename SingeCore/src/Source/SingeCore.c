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


// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"

Window window;

Shader LoadShader(const char*, const char*);

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SetCursorMode(CursorModes.Disabled);

	Image icon = Images.LoadImage("icon.png");

	Windows.SetIcon(window, icon);

	Images.Dispose(icon);

	Shader texturedShader = LoadShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	Shader uvShader = LoadShader("SimpleVertexShader.vertexshader", "ColorUV.fragmentshader");

	Shader guiShader = LoadShader("GUIShader.vertexshader", "SimpleFragmentShader.fragmentshader");

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
	guiMaterial->UseCameraPerspective = false;

	Camera camera = Cameras.CreateCamera();

	// bind a vertex array for OpenGL this is required to render objects
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GameObject ball = LoadGameObjectFromModel("ball.obj", FileFormats.Obj);
	GameObjects.SetMaterial(ball, uvMaterial);

	GameObject otherBall = GameObjects.Duplicate(ball);
	GameObject car = LoadGameObjectFromModel("ball.obj", FileFormats.Obj);
	GameObject room = GameObjects.Duplicate(ball);//LoadGameObjectFromModel("room.obj", FileFormats.Obj);
	GameObject cube = LoadGameObjectFromModel("cube.obj", FileFormats.Obj);

	GameObjects.SetMaterial(car, uvMaterial);
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

	GameObject square = CreateGameObjectFromMesh(squareMesh);

	SafeFree(squareMesh);

	GameObjects.SetMaterial(square, guiMaterial);

	Image debugUv = Images.LoadImage("uv_debug.png");

	Texture debugUvTexture;
	if (Textures.TryCreateTexture(debugUv, &debugUvTexture) is false)
	{
		throw(InvalidArgumentException);
	}

	Images.Dispose(debugUv);

	Materials.SetMainTexture(square->Material, debugUvTexture);

	Textures.Dispose(debugUvTexture);

	// orient the camera so we see some geometry without moving the camera
	Transforms.SetPositions(camera->Transform, 2.11f, 1.69f, 8.39f);

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

		Quaternion ballRotation;
		glm_quat(ballRotation, rotateAmount / (float)GLM_PI, 0, 1, 0);
		Transforms.SetRotation(ball->Transform, ballRotation);

		glm_quat(ballRotation, -(rotateAmount / (float)GLM_PI), 0, 1, 0);
		Transforms.SetRotation(otherBall->Transform, ballRotation);

		// drive car
		vec3 carDirection;
		Transforms.GetDirection(car->Transform, Directions.Forward, carDirection);

		ScaleVector3(carDirection, modifier);

		Transforms.AddPosition(car->Transform, carDirection);

		Quaternion carRotation;
		glm_quat(carRotation, ((float)GLM_PI / 8.0f) * modifier, 0, 1, 0);

		Transforms.Rotate(car->Transform, carRotation);

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

		fprintf(stdout, "Position: ");
		PrintVector3(position, stdout);
		fprintf(stdout, " Rotation: [ x: %0.2fpi, y: %0.2fpi ] Forward: ", FPSCamera.State.HorizontalAngle / GLM_PI, FPSCamera.State.VerticalAngle / GLM_PI);
		vec3 forwardVector;
		Transforms.GetDirection(camera->Transform, Directions.Forward, forwardVector);
		PrintVector3(forwardVector, stdout);
		fprintf(stdout, NEWLINE);

		FPSCamera.Update(camera);

		Transforms.SetPosition(camera->Transform, position);



		GameObjects.Draw(cube, camera);

		GameObjects.Draw(car, camera);
		GameObjects.Draw(ball, camera);
		GameObjects.Draw(otherBall, camera);
		GameObjects.Draw(room, camera);

		GameObjects.Draw(square, camera);

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();

		//fprintf(stdout,"Total: %0.4fs	FrameTime: %0.4fms"NEWLINE, Time(), FrameTime() * 1000.0);
	} while (GetKey(KeyCodes.Escape) != true && Windows.ShouldClose(window) != true);

	GameObjects.Destroy(ball);
	GameObjects.Destroy(otherBall);
	GameObjects.Destroy(car);
	GameObjects.Destroy(room);
	GameObjects.Destroy(cube);
	GameObjects.Destroy(square);

	Textures.Dispose(cubeTexture);

	Materials.Dispose(texturedMaterial);
	Materials.Dispose(uvMaterial);
	Materials.Dispose(guiMaterial);

	Cameras.Dispose(camera);

	Shaders.Dispose(texturedShader);

	Shaders.Dispose(uvShader);

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