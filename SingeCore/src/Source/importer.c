#include "csharp.h"
#include "stdlib.h"
#include "singine/file.h"
#include "modeling/importer.h"
#include "singine/guards.h"
#include <ctype.h>
#include <string.h>
#include "modeling/model.h"
#include "singine/memory.h"
#include "cunit.h"
#include "math/vectors.h"
#include "singine/parsing.h"

typedef int Token;
typedef const char* Sequence;

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

static struct _sequences {
	Sequence Object;
	Sequence Vertex;
	Sequence TextureVertex;
	Sequence NormalVertex;
	Sequence Smoothing;
	Sequence Face;
} Sequences = {
	.Object = "o ",
	.Vertex = "v ",
	.TextureVertex = "vt ",
	.NormalVertex = "vn ",
	.Smoothing = "s ",
	.Face = "f "
};

#define _TO_STRING(s) #s
#define ToString(s) TO_STRING(s)

#define BUFFER_SIZE 1024
#define MAX_OBJECT_NAME_LENGTH 61

static void DisposeModel(Model model)
{
	if (model->Meshes != null)
	{
		SafeFree(model->Meshes->Vertices);
		SafeFree(model->Meshes->TextureVertices);
		SafeFree(model->Meshes->Normals);
		SafeFree(model->Meshes->Name);
		SafeFree(model->Meshes);
	}

	SafeFree(model);
}

static Mesh CreateMesh()
{
	Mesh mesh = SafeAlloc(sizeof(struct _mesh));

	mesh->Name = null;

	mesh->Normals = null;
	mesh->Vertices = null;
	mesh->TextureVertices = null;

	mesh->NormalCount = 0;
	mesh->VertexCount = 0;
	mesh->TextureVertexCount = 0;

	return mesh;
}

static Model CreateModel()
{
	Model model = SafeAlloc(sizeof(struct _model));

	model->Meshes = null;
	model->Name = null;
	model->Dispose = &DisposeModel;

	return model;
}

// forgive me
static bool TryGetVectorPattern(File stream,
	char* buffer,
	size_t bufferLength,
	const char* pattern,
	const char* abortPattern,
	size_t floatsPerRow,
	bool(*Parser)(char* buffer, float* out_values),
	float** out_vertices,
	size_t* out_count)
{
	// count the number of vertices that we should alloc
	size_t count;
	if (TryGetSequenceCount(stream,
		pattern, // target
		strlen(pattern), // target length
		abortPattern, // abort
		strlen(abortPattern), // abort length
		&count) // out number of occurences
		is false)
	{
		return false;
	}

	size_t floatCount = floatsPerRow * count;

	// alloc the array
	float* result = SafeAlloc(sizeof(float) * floatCount);

	size_t offset = strlen(pattern);

	for (size_t i = 0; i < count; i++)
	{
		size_t length;
		if (TryReadLine(stream, buffer, 0, bufferLength, &length) is false)
		{
			return false;
		}

		float* subarray = result + (i * floatsPerRow);

		if (Parser(buffer + offset, subarray) is false)
		{
			return false;
		}
	}

	*out_vertices = result;
	*out_count = count;

	return true;
}

static bool TryParseFace(const char* buffer, size_t* out_attributes)
{
	int count = sscanf_s(buffer, "%lli/%lli/%lli %lli/%lli/%lli %lli/%lli/%lli",
		&out_attributes[0],
		&out_attributes[1],
		&out_attributes[2],
		&out_attributes[3],
		&out_attributes[4],
		&out_attributes[5],
		&out_attributes[6],
		&out_attributes[7],
		&out_attributes[8]);

	if (count != 9)
	{
		return false;
	}

	return true;
}

static bool TryGetIntegerPattern(File stream,
	char* buffer,
	size_t bufferLength,
	const char* pattern,
	const char* abortPattern,
	size_t integersPerRow,
	bool(*Parser)(char* buffer, size_t* out_values),
	size_t** out_integers,
	size_t* out_count)
{
	// count the number of vertices that we should alloc
	size_t count;
	if (TryGetSequenceCount(stream,
		pattern, // target
		strlen(pattern), // target length
		abortPattern, // abort
		abortPattern is null ? 0 : strlen(abortPattern), // abort length
		&count) // out number of occurences
		is false)
	{
		return false;
	}

	size_t floatCount = integersPerRow * count;

	// alloc the array
	size_t* result = SafeAlloc(sizeof(size_t) * floatCount);

	size_t offset = strlen(pattern);

	for (size_t i = 0; i < count; i++)
	{
		size_t length;
		if (TryReadLine(stream, buffer, 0, bufferLength, &length) is false)
		{
			return false;
		}

		size_t* subarray = result + (i * integersPerRow);

		if (Parser(buffer + offset, subarray) is false)
		{
			return false;
		}
	}

	*out_integers = result;
	*out_count = count;

	return true;
}

static bool TryGetVertices(File stream, char* buffer, size_t bufferLength, float** out_vertices, size_t* out_count)
{
	return TryGetVectorPattern(stream, buffer, bufferLength, Sequences.Vertex, Sequences.TextureVertex, 3, &TryParseVector3, out_vertices, out_count);
}

static bool TryGetTextureVertices(File stream, char* buffer, size_t bufferLength, float** out_vertices, size_t* out_count)
{
	return TryGetVectorPattern(stream, buffer, bufferLength, Sequences.TextureVertex, Sequences.NormalVertex, 2, &TryParseVector2, out_vertices, out_count);
}

static bool TryGetNormalVertices(File stream, char* buffer, size_t bufferLength, float** out_vertices, size_t* out_count)
{
	return TryGetVectorPattern(stream, buffer, bufferLength, Sequences.NormalVertex, Sequences.Smoothing, 3, &TryParseVector3, out_vertices, out_count);
}

static bool TryGetFaceAttributes(File stream, char* buffer, size_t bufferLength, size_t** out_attributes, size_t* out_count)
{
	return TryGetIntegerPattern(stream, buffer, bufferLength, Sequences.Face, NULL, 9, &TryParseFace, out_attributes, out_count);
}

static Mesh ComposeFaces(const float* vertices, const  size_t vertexCount, const  float* textures, const size_t textureCount, const float* normals, const  size_t normalCount, const  size_t* faces, const  size_t faceCount)
{
	Mesh mesh = CreateMesh();

	// 3 vec3 per face, 3 float per vec3
	mesh->VertexCount = faceCount * 3 * 3;
	mesh->Vertices = SafeAlloc(mesh->VertexCount * sizeof(float));

	// 3 vec2 per face, 2 float per vec2
	mesh->TextureVertexCount = faceCount * 3 * 2;
	mesh->TextureVertices = SafeAlloc(mesh->TextureVertexCount * sizeof(float));

	// 3 vec3 per face, 3 float per vec3
	mesh->NormalCount = faceCount * 3 * 3;
	mesh->Normals = SafeAlloc(mesh->NormalCount * sizeof(float));

	// go through the faces and append the attributes to the final arrays
	// each face has 9 integers representing 3 indices of the vertexs, uvs, and normals that represent the face
	size_t numberOfTriplets = faceCount * 3;

	for (size_t i = 0; i < numberOfTriplets; i++)
	{
		const size_t* buffer = faces + (i * 3);

		// .obj files are 1 indexed subtract one to get 0 indexed
		size_t vertexIndex = buffer[0] - 1;
		size_t uvIndex = buffer[1] - 1;
		size_t normalIndex = buffer[2] - 1;

		// there are 3 floats per vertex so the address of the nth vec3 is index * 3
		const float* subVertices = vertices + (vertexIndex * 3);

		// there are 2 floats per uv
		const float* subUVs = textures + (uvIndex * 2);

		// there are 3 floats per normal
		const float* subNormals = normals + (normalIndex * 3);

		// copy the floats over to their final arrays
		Vector3CopyTo(subVertices, mesh->Vertices + (i * 3));
		Vector2CopyTo(subUVs, mesh->TextureVertices + (i * 2));
		Vector3CopyTo(subNormals, mesh->Normals + (i * 3));
	}

	return mesh;
}

static bool TryCreateMesh(File stream, char* buffer, size_t bufferLength, Mesh* out_mesh)
{
	*out_mesh = null;

	// read the name of the object
	char* name;
	// offset the buffer by the sequence so we dont use the object sequence as the name lmao
	if (TryParseString(buffer + strlen(Sequences.Object), bufferLength, MAX_OBJECT_NAME_LENGTH + 1, &name) is false)
	{
		return false;
	}

	float* vertices;
	size_t vertexCount;
	if (TryGetVertices(stream, buffer, bufferLength, &vertices, &vertexCount) is false)
	{
		SafeFree(name);

		return false;
	}

	float* textureVertices;
	size_t textureVertexCount;
	if (TryGetTextureVertices(stream, buffer, bufferLength, &textureVertices, &textureVertexCount) is false)
	{
		SafeFree(name);
		SafeFree(vertices);

		return false;
	}

	float* normalVertices;
	size_t normalVertexCount;
	if (TryGetNormalVertices(stream, buffer, bufferLength, &normalVertices, &normalVertexCount) is false)
	{
		SafeFree(name);
		SafeFree(vertices);
		SafeFree(textureVertices);

		return false;
	}

	// consume smoothing flag
	size_t lineLength;
	if (TryReadLine(stream, buffer, 0, bufferLength, &lineLength) is false)
	{
		SafeFree(name);
		SafeFree(vertices);
		SafeFree(textureVertices);
		SafeFree(normalVertices);

		return false;
	}

	bool smoothingEnabled;
	if (TryParseBoolean(buffer + strlen(Sequences.Smoothing), 3, &smoothingEnabled) is false)
	{
		SafeFree(name);
		SafeFree(vertices);
		SafeFree(textureVertices);
		SafeFree(normalVertices);

		return false;
	}

	size_t* faceAttributes;
	size_t faces;
	if (TryGetFaceAttributes(stream, buffer, bufferLength, &faceAttributes, &faces) is false)
	{
		SafeFree(name);
		SafeFree(vertices);
		SafeFree(textureVertices);
		SafeFree(normalVertices);

		return false;
	}

	*out_mesh = ComposeFaces(vertices, vertexCount, textureVertices, textureVertexCount, normalVertices, normalVertexCount, faceAttributes, faces);

	(*out_mesh)->Name = name;
	(*out_mesh)->SmoothingEnabled = smoothingEnabled;

	SafeFree(vertices);
	SafeFree(textureVertices);
	SafeFree(normalVertices);
	SafeFree(faceAttributes);

	return true;
}

static bool VerifyFormat(FileFormat format)
{
	for (size_t i = 0; i < sizeof(SupportedFormats) / sizeof(FileFormat); i++)
	{
		if (strcmp(format, SupportedFormats[i]) is 0)
		{
			return true;
		}
	}
	return false;
}

static bool TryImportModel(char* path, FileFormat format, Model* out_model)
{
	// make sure the format is supported
	if (VerifyFormat(format) is false)
	{
		return false;
	}

	// open the path
	File stream;
	if (TryOpen(path, FileModes.ReadBinary, &stream) is false)
	{
		return false;
	}

	Model model = CreateModel();

	char buffer[BUFFER_SIZE];

	// read the file until we find an object to start creating a mesh for
	size_t lineLength;
	while (TryReadLine(stream, buffer, 0, BUFFER_SIZE, &lineLength))
	{
		int token = buffer[0];

		if (token is Tokens.Object)
		{
			Mesh mesh;
			if (TryCreateMesh(stream, buffer, BUFFER_SIZE, &mesh) is false)
			{
				model->Dispose(model);

				return false;
			}

			model->Meshes = mesh;
		}
	}

	if (TryClose(stream) is false)
	{
		model->Dispose(model);
		return false;
	}

	*out_model = model;

	return true;
}

Model ImportModel(char* path, FileFormat format)
{
	Model model;
	if (TryImportModel(path, format, &model) is false)
	{
		return null;
	}
	return model;
}