#include "csharp.h"
#include "graphics/shaders.h"
#include "singine/memory.h"
#include "GL/glew.h"

static void Dispose(Shader shader)
{
	glDeleteProgram(shader->Handle);
	SafeFree(shader);
}

Shader CreateShader()
{
	Shader newShader = SafeAlloc(sizeof(struct _shader));

	newShader->Handle = 0;
	newShader->MVPHandle = -1;
	newShader->Dispose = &Dispose;
	newShader->AfterDraw = null;
	newShader->BeforeDraw = null;
	newShader->DrawMesh = null;

	return newShader;
}