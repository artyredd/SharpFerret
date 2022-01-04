// SingeCore.c
#include <stdio.h>
#include <stdlib.h>

#include "csharp.h"
#include "singine/memory.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "graphics/window.h"
#include "singine/enumerable.h"

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


// scripts (not intrinsically part of the engine)
#include "scripts/fpsCamera.h"

Window window;

Shader LoadShader(const char*, const char*);

int main()
{
	StartRuntime();

	SetHint(WindowHints.MSAASamples, 4);

	// use opengl 3.3
	SetHint(ContextHints.VersionMajor, 3);
	SetHint(ContextHints.VersionMinor, 3);

	SetHint(OpenGLHints.ForwardCompatibility, true); // To make MacOS happy; should not be needed
	SetHint(OpenGLHints.Profile, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able
	SetHint(WindowHints.Resizable, true);
	SetHint(WindowHints.Decorated, true);

	window = CreateWindow(1920, 1080, "Singine");

	SetInputWindow(window);

	//sWindow.SetMode(window, WindowModes.FullScreen);

	SetClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// bind graphics card with GDI
	sWindow.SetCurrent(window);

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

	Image icon = LoadImage("icon.png");

	using(icon, sWindow.SetIcon(window, icon));

	Image uv = LoadImage("cubeuv.png");

	Shader shader = LoadShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	GLuint uvID;
	glGenTextures(1, &uvID);
	glBindTexture(GL_TEXTURE_2D, uvID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uv->Width, uv->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, uv->Pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SetCursorMode(CursorModes.Disabled);

	uv->Dispose(uv);

	Camera camera = CreateCamera();

	vec3 startingPosition = { 3,2,3 };

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	unsigned int textureID = glGetUniformLocation(shader->Handle, "myTextureSampler");

	DefaultShader = shader;



	GameObject ball = LoadGameObjectFromModel("ball.obj", FileFormats.Obj);
	GameObject otherBall = GameObjects.Duplicate(ball);
	GameObject room = LoadGameObjectFromModel("room.obj", FileFormats.Obj);

	//test memory leak
	for (size_t i = 0; i < 5; i++)
	{
		GameObject tmp = GameObjects.Duplicate(ball);

		GameObjects.Destroy(tmp);
	}

	SetParent(otherBall->Transform, ball->Transform);

	float speed = 10.0f;

	vec4 scaler = { 1,1,1,1 };

	float scaleAmount = 1.0f;
	float rotateAmount = 0.0f;

	vec3 position = { 3.25f, 0.5f, -3.18f };

	SetPosition(camera->Transform, position);

	Quaternion rotation;

	glm_quat(rotation, (float)GLM_PI, 0, 0, 1);

	vec3 positionModifier;

	SetScales(otherBall->Transform, 0.5, 0.5, 0.5);
	SetPositions(otherBall->Transform, 0, 0, 3);

	do {
		UpdateTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Vectors3CopyTo(camera->Transform->Position, position);

		float modifier = speed * (float)DeltaTime();

		//GetDirection(ballMesh->Transform, Directions.Forward, ballDirection);
		//ScaleVector3(ballDirection, modifier);
		////AddPosition(ballMesh->Transform, ballDirection);

		rotateAmount += modifier;

		Quaternion ballRotation;
		glm_quat(ballRotation, rotateAmount / (float)GLM_PI, 0, 1, 0);
		SetRotation(ball->Transform, ballRotation);

		glm_quat(ballRotation, -(rotateAmount / (float)GLM_PI), 0, 1, 0);
		SetRotation(otherBall->Transform, ballRotation);

		if (GetKey(KeyCodes.A))
		{
			GetDirection(camera->Transform, Directions.Left, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.D))
		{
			GetDirection(camera->Transform, Directions.Right, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.W))
		{
			GetDirection(camera->Transform, Directions.Forward, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.S))
		{
			GetDirection(camera->Transform, Directions.Back, positionModifier);
			ScaleVector3(positionModifier, modifier);
			SubtractVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.Space))
		{
			GetDirection(camera->Transform, Directions.Up, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}
		if (GetKey(KeyCodes.LeftShift))
		{
			GetDirection(camera->Transform, Directions.Down, positionModifier);
			ScaleVector3(positionModifier, modifier);
			AddVectors3(position, positionModifier);
		}

		////glActiveTexture(GL_TEXTURE0);
		////glBindTexture(GL_TEXTURE_2D, uvID);
		////glUniform1i(textureID, 0); 

		fprintf(stdout, "Position: ");
		PrintVector3(position, stdout);
		fprintf(stdout, " Rotation: [ x: %0.2fpi, y: %0.2fpi ] Forward: ", FPSCamera.State.HorizontalAngle / GLM_PI, FPSCamera.State.VerticalAngle / GLM_PI);
		vec3 forwardVector;
		GetDirection(camera->Transform, Directions.Forward, forwardVector);
		PrintVector3(forwardVector, stdout);
		fprintf(stdout, NEWLINE);

		FPSCamera.Update(camera);

		SetPosition(camera->Transform, position);

		GameObjects.Draw(room, camera);
		GameObjects.Draw(ball, camera);
		GameObjects.Draw(otherBall, camera);


		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();

		//fprintf(stdout,"Total: %0.4fs	FrameTime: %0.4fms"NEWLINE, Time(), FrameTime() * 1000.0);
	} while (GetKey(KeyCodes.Escape) != true && ShouldClose(window) != true);

	GameObjects.Destroy(ball);
	GameObjects.Destroy(otherBall);
	GameObjects.Destroy(room);

	camera->Dispose(camera);

	shader->Dispose(shader);

	window->Dispose(window);

	StopRuntime();

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
	glUseProgram(shader->Handle);

	if (shader->MVPHandle is - 1)
	{
		if (TryGetUniform(shader, "MVP", &shader->MVPHandle) is false)
		{
			throw(FailedToLocationMVPUniformException);
		}
	}

	glUniformMatrix4fv(shader->MVPHandle, 1, false, &mvp[0][0]);
}

void Draw(Shader shader, void* renderMesh)
{
	RenderMesh mesh = renderMesh;

	mesh->Draw(mesh, null);
}

void AfterDraw(Shader shader)
{

}

Shader LoadShader(const char* vertexPath, const char* fragmentPath)
{
	Shader shader = CompileShader(vertexPath, fragmentPath);

	if (shader is null)
	{
		throw(FailedToCompileShaderException);
	}

	shader->BeforeDraw = &BeforeDraw;
	shader->DrawMesh = &Draw;
	shader->AfterDraw = &AfterDraw;

	return shader;
}