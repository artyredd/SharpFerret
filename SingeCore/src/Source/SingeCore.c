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

#include "graphics/shaders.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"

#include "input.h"
#include "modeling/importer.h"
#include "modeling/model.h"
#include "singine/parsing.h"
#include "graphics/renderModel.h"

Window window;

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

	Model cube = ImportModel("doublecube.obj", FileFormats.Obj);

	Image icon = LoadImage("icon.png");

	using(icon, sWindow.SetIcon(window, icon));

	Image uv = LoadImage("cubeuv.png");

	Shader shader = CompileShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	GLuint uvID;
	glGenTextures(1, &uvID);
	glBindTexture(GL_TEXTURE_2D, uvID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, uv->Width, uv->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, uv->Pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uv->Dispose(uv);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	mat4 projection;

	glm_perspective(glm_rad(70.0f), 16.0f / 9.0f, 0.1f, 100.0f, projection);

	mat4 view;

	vec3 target;

	glm_vec3_zero(target);

	vec3 cameraPosition = { 3,2,3 };
	vec3 up = { 0,1.0f,0 };

	glm_lookat(
		cameraPosition,
		target,
		up,
		view
	);

	mat4 MVP;

	glm_mat4_mul(projection, view, MVP);

	unsigned int mvpId = glGetUniformLocation(shader->Handle, "MVP");
	unsigned int textureID = glGetUniformLocation(shader->Handle, "myTextureSampler");

	DefaultShader = shader;

	size_t numberOfMeshes = cube->Count;

	// alloc array of points to rendermesh
	RenderMesh* meshes = SafeAlloc(numberOfMeshes * sizeof(RenderMesh));

	size_t i = 0;
	Mesh head = cube->Head;
	while (head isnt null)
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

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader->Handle);

		mat4 model;

		glm_mat4_identity(model);

		mat4 modelMVP;
		glm_mat4_mul(MVP, model, modelMVP);

		glUniformMatrix4fv(mvpId, 1, false, &modelMVP[0][0]);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, uvID);
		//glUniform1i(textureID, 0);

		for (size_t i = 0; i < numberOfMeshes; i++)
		{
			meshes[i]->Draw(meshes[i], modelMVP);
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