#include "csharp.h"
#include "stdlib.h"
#include "singine/file.h"
#include "modeling/importer.h"
#include "singine/guards.h"
#include <ctype.h>
#include <string.h>
#include "modeling/model.h"
#include "singine/memory.h"

typedef int Token;

static struct _tokens {
	const Token Object;
	const Token Comment;
	const Token Vertex;
	const Token Face;
	const Token Smoothing;
} Tokens = {
	'o', // Object
	'#', // Comment
	'v', // Vertex
	'f', // Face
	's' // smoothing
};

#define TO_STRING(s) #s

#define BUFFER_SIZE 1024

// the largest length a name of an object in blender can be +1 for null temrination
#define MAX_OBJECT_NAME_LENGTH 62
#define OBJECT_NAME_SCAN_FORMAT(length) "%" TO_STRING(length) "s"

static Mesh CreateMesh()
{
	Mesh mesh = SafeAlloc(sizeof(struct _mesh));
	
	mesh->Name = null;

	mesh->Faces = null;

	mesh->Normals = null;
	mesh->Vertices = null;
	mesh->TextureVertices = null;

	mesh->NormalCount = 0;
	mesh->VertexCount = 0;
	mesh->TextureVerticesCount = 0;

	mesh->FaceCount = 0;

	return mesh;
}

static bool TryReadObjectName(File stream, char** out_name)
{
	
}

static bool TryGetVertices(File stream, vec3* out_vertices, size_t* out_count)
{

}

static bool TryCreateMesh(File stream, char* buffer, size_t bufferLength, Mesh* out_mesh)
{
	*out_mesh = null;

	// read the name of the object
	char* name;
	if (TryReadObjectName(stream, &name) is false)
	{
		return false;
	}



	return true;
}

static bool VerifyFormat(FileFormat format)
{
	for (size_t i = 0; i < sizeof(SupportedFormats) / sizeof(FileFormat); i++)
	{
		if (strcmp(format, SupportedFormats[i]))
		{
			return true;
		}
	}
	return false;
}

static Model CreateModel()
{
	Model model = SafeAlloc(sizeof(struct _model));

	model->Name = null;
	model->Meshes = null;

	return model;
}

static bool TryImportModel(char* path, FileFormat format)
{
	// make sure the format is supported
	if (VerifyFormat(format) is false)
	{
		return false;
	}

	// open the path
	File stream;
	if (TryOpen(path, FileModes.Read, &stream) is false)
	{
		return false;
	}

	//Model model = CreateModel();

	char buffer[BUFFER_SIZE];

	// read the file until we find an object to start creating a mesh for
	size_t lineLength;
	while (TryReadLine(stream, buffer, 0, BUFFER_SIZE, &lineLength))
	{
		int token = buffer[0];

		// ignore comments
		if (token is Tokens.Comment)
		{
			continue;
		}
		if (token is Tokens.Object)
		{
			
		}
	}

	return true;
}

Mesh ImportModel(char* path, FileFormat format)
{
	return null;
}