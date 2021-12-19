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

	Model cube = ImportModel("cube.obj", FileFormats.Obj);

	Image icon = LoadImage("icon.png");

	using(icon, sWindow.SetIcon(window, icon));

	Shader shader = CompileShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	glBufferData(GL_ARRAY_BUFFER, cube->Meshes->VertexCount * sizeof(float), cube->Meshes->Vertices, GL_STATIC_DRAW);

	GLuint uvBuffer;
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, cube->Meshes->TextureVertexCount * sizeof(float), cube->Meshes->TextureVertices, GL_STATIC_DRAW);

	mat4 projection;

	glm_perspective(glm_rad(70.0f), 16.0f / 9.0f, 0.1f, 100.0f, projection);

	mat4 view;

	vec3 target;

	glm_vec3_zero(target);

	vec3 cameraPosition = { 3,2,-3 };
	vec3 up = { 0,1.0f,0 };

	glm_lookat(
		cameraPosition,
		target,
		up,
		view
	);

	mat4 model;

	glm_mat4_identity(model);

	mat4 MVP;

	glm_mat4_mul(projection, view, MVP);

	glm_mat4_mul(MVP, model, MVP);

	unsigned int mvpId = glGetUniformLocation(shader->Handle, "MVP");

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader->Handle);

		glUniformMatrix4fv(mvpId, 1, false, &MVP[0][0]);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			false,
			0,
			null
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			null                    // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, cube->Meshes->VertexCount / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// swap the back buffer with the front one
		glfwSwapBuffers(window->Handle);

		PollInput();
	} while (GetKey(window, KeyCodes.Escape) != true && ShouldClose(window) != true);

	cube->Dispose(cube);

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