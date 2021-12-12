#include "csharp.h"
#include "graphics/shaders.h"

#include "io/file.h"
#include "singine/memory.h"

static void CompileFragmentShader(const char* data)
{

}

static void CompileVertexShader(const char* data)
{

}

Shader Compile(const char* vertexPath, const char* fragmentPath)
{
	char* vertexData = ReadAll(vertexPath);
	char* fragmentData = ReadAll(fragmentPath);



	SafeFree(vertexData);
	SafeFree(fragmentData);
}