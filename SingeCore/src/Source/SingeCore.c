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

Window window;

Shader LoadShader(const char*, const char*);

int main()
{
	StartRuntime();

	SetHint(WindowHints.MSAASamples, 4);

	// use opengl 3.3
	SetHint(ContextHints.VersionMajor, 3);
	SetHint(ContextHints.VersionMinor, 3);

	SetHint(OpenGLHints.ForwardCompatibility, GL_TRUE); // To make MacOS happy; should not be needed
	SetHint(OpenGLHints.Profile, GLFW_OPENGL_CORE_PROFILE);

	// allow it to be windowed and resize-able
	SetHint(WindowHints.Resizable, GLFW_TRUE);
	SetHint(WindowHints.Decorated, GLFW_TRUE);

	window = CreateWindow(1920, 1080, "Singine");

	glClearColor(1.0f, 1.0f, 0.4f, 0.0f);

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

	Model cube;
	if (TryImportModel("doublecube.obj", FileFormats.Obj, &cube) is false)
	{
		throw(FailedToReadFileException);
	}

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

	uv->Dispose(uv);

	Camera camera = CreateCamera();

	vec3 startingPosition = { 3,2,3 };

	camera->SetPositionVector(camera, startingPosition);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	unsigned int textureID = glGetUniformLocation(shader->Handle, "myTextureSampler");

	DefaultShader = shader;

	size_t numberOfMeshes = cube->Count;

	// alloc array of points to rendermesh
	RenderMesh* meshes = SafeAlloc(numberOfMeshes * sizeof(RenderMesh));

	size_t i = 0;
	Mesh head = cube->Head;
	while (head)
	{
		RenderMesh renderMesh;
		if (TryBindMesh(head, &renderMesh) is false)
		{
			throw(NotImplementedException);
		}

		meshes[i++] = renderMesh;

		head = head->Next;
	}

	cube->Dispose(cube);

	float speed = 0.01f;

	vec4 scaler = { 2,2,2,1 };

	float scaleAmount = 1.0f;
	float rotateAmount = 0.0f;


	Quaternion rotation;

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (GetKey(window, KeyCodes.A))
		{
			camera->AddPositionX(camera, -speed);
		}
		if (GetKey(window, KeyCodes.D))
		{
			camera->AddPositionX(camera, speed);
		}
		if (GetKey(window, KeyCodes.W))
		{
			camera->AddPositionZ(camera, -speed);
		}
		if (GetKey(window, KeyCodes.S))
		{
			camera->AddPositionZ(camera, speed);
		}
		if (GetKey(window, KeyCodes.Space))
		{
			camera->AddPositionY(camera, speed);
		}
		if (GetKey(window, KeyCodes.LeftShift))
		{
			camera->AddPositionY(camera, -speed);
		}

		////glActiveTexture(GL_TEXTURE0);
		////glBindTexture(GL_TEXTURE_2D, uvID);
		////glUniform1i(textureID, 0);

		if (GetKey(window, KeyCodes.Up))
		{
			scaleAmount += speed;
		}

		if (GetKey(window, KeyCodes.Down))
		{
			scaleAmount -= speed;
		}

		if (GetKey(window, KeyCodes.Left))
		{
			rotateAmount += speed;
		}

		if (GetKey(window, KeyCodes.Right))
		{
			rotateAmount -= speed;
		}

		SetVector3(scaler, scaleAmount, scaleAmount, scaleAmount);

		glm_quat(rotation, rotateAmount, 1, 0, 0);

		SetRotation(meshes[0]->Transform, rotation);

		SetScale(meshes[0]->Transform, scaler);

		for (size_t i = 0; i < numberOfMeshes; i++)
		{
			camera->DrawMesh(camera, meshes[i], shader);
		}

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();
	} while (GetKey(window, KeyCodes.Escape) != true && ShouldClose(window) != true);

	for (size_t i = 0; i < numberOfMeshes; i++)
	{
		meshes[i]->Dispose(meshes[i]);
	}

	SafeFree(meshes);

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