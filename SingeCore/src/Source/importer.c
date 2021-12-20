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

typedef struct _fileBuffer* FileBuffer;

struct _fileBuffer {
	char* StreamBuffer;
	size_t StreamLength;
	float* VertexBuffer;
	size_t VerticesCount;
	size_t VerticesLength;
	float* TextureBuffer;
	size_t TexturesCount;
	size_t TexturesLength;
	float* NormalBuffer;
	size_t NormalsCount;
	size_t NormalsLength;
	void(*Dispose)(FileBuffer);
};

#define _TO_STRING(s) #s
#define ToString(s) TO_STRING(s)

#define BUFFER_SIZE 1024
#define MAX_OBJECT_NAME_LENGTH 61

static void DisposeModel(Model model)
{
	if (model->Head != null)
	{
		Mesh head = model->Head;
		while (head != null)
		{
			Mesh tmp = head;

			head = head->Next;

			SafeFree(tmp->Vertices);
			SafeFree(tmp->TextureVertices);
			SafeFree(tmp->Normals);
			SafeFree(tmp->Name);

			SafeFree(tmp);
		}
	}

	SafeFree(model);
}

static void DisposeFileBuffer(FileBuffer buffer)
{
	// the character buffer is not alloced by CreateFileBuffer do not free it
	SafeFree(buffer->NormalBuffer);
	SafeFree(buffer->TextureBuffer);
	SafeFree(buffer->VertexBuffer);
	SafeFree(buffer);
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
	mesh->TextureCount = 0;

	mesh->Next = null;

	return mesh;
}

static Model CreateModel()
{
	Model model = SafeAlloc(sizeof(struct _model));

	model->Head = model->Tail = null;
	model->Name = null;
	model->Dispose = &DisposeModel;

	return model;
}

static FileBuffer CreateFileBuffer(char* streamBuffer, size_t characterBufferSize, size_t vertices, size_t textures, size_t normals)
{
	FileBuffer buffer = SafeAlloc(sizeof(struct _fileBuffer));

	buffer->Dispose = &DisposeFileBuffer;

	buffer->StreamBuffer = streamBuffer;
	buffer->StreamLength = characterBufferSize;

	buffer->NormalBuffer = SafeAlloc(sizeof(float) * 3 * normals);
	buffer->NormalsCount = 0;
	buffer->NormalsLength = normals;

	buffer->TextureBuffer = SafeAlloc(sizeof(float) * 2 * textures);
	buffer->TexturesCount = 0;
	buffer->TexturesLength = textures;

	buffer->VertexBuffer = SafeAlloc(sizeof(float) * 3 * vertices);
	buffer->VerticesCount = 0;
	buffer->VerticesLength = vertices;

	return buffer;
}

static bool CompareStrings(const char* left, size_t leftLength, const char* right, size_t rightLength)
{
	GuardNotNull(left);
	GuardNotNull(right);
	GuardNotZero(leftLength);
	GuardNotZero(rightLength);

	for (size_t i = 0; i < min(leftLength, rightLength); i++)
	{
		if (left[i] isnt right[i])
		{
			return false;
		}
	}

	return true;
}

// forgive me
static bool TryGetVectorPattern(File stream,
	char* buffer,
	size_t bufferLength,
	const char* pattern,
	const char* abortPattern,
	size_t floatsPerRow,
	float* resultBuffer,
	bool(*Parser)(char* buffer, float* out_values),
	size_t* out_count)
{
	// count the number of vertices that we should alloc
	size_t count = 0;

	size_t offset = strlen(pattern);
	size_t abortLength = strlen(abortPattern);

	size_t length;

	// check to see if there is already a parsable string in the buffer
	if (Parser(buffer + offset, resultBuffer))
	{
		++count;
	}

	while (TryReadLine(stream, buffer, 0, bufferLength, &length))
	{
		// if we encounter the abort pattern we should stop parsing
		// this is needed becuase we may want to parse for vector 2's but a vec2 parser wont fail to parse a string
		// that contains a vector3 as a vector2
		if (CompareStrings(abortPattern, abortLength, buffer, min(abortLength, length)))
		{
			break;
		}

		float* subarray = resultBuffer + (count * floatsPerRow);

		if (Parser(buffer + offset, subarray) is false)
		{
			break;
		}

		++count;
	}

	// let the caller know how many values we appended to the end of the resultBuffer so the next time this is called we start at the right buffer
	// offset
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

static bool TryGetFaceAttributes(File stream, char* buffer, size_t bufferLength, size_t** out_attributes, size_t* out_count)
{
	return TryGetIntegerPattern(stream, buffer, bufferLength, Sequences.Face, Sequences.Object, 9, &TryParseFace, out_attributes, out_count);
}

static bool TryCountElements(File stream, char* buffer, size_t bufferLength, size_t* out_vertices, size_t* out_textures, size_t* out_normals)
{
	*out_vertices = 0;
	*out_textures = 0;
	*out_normals = 0;

	rewind(stream);

	size_t vertices = 0;
	size_t textures = 0;
	size_t normals = 0;

	size_t vertexSequenceLength = strlen(Sequences.Vertex);
	size_t texturesSequenceLength = strlen(Sequences.TextureVertex);
	size_t normalsSequenceLength = strlen(Sequences.NormalVertex);

	size_t lineLength;
	while (TryReadLine(stream, buffer, 0, bufferLength, &lineLength))
	{
		// dont waste time trying to compare a comment with our sequence strings
		if (buffer[0] is Tokens.Comment)
		{
			continue;
		}

		if (CompareStrings(Sequences.Vertex, vertexSequenceLength, buffer, min(vertexSequenceLength, lineLength)))
		{
			++vertices;
			continue;
		}
		if (CompareStrings(Sequences.TextureVertex, texturesSequenceLength, buffer, min(texturesSequenceLength, lineLength)))
		{
			++textures;
			continue;
		}
		if (CompareStrings(Sequences.NormalVertex, normalsSequenceLength, buffer, min(normalsSequenceLength, lineLength)))
		{
			++normals;
			continue;
		}
	}

	// if we broke out of the loop we either finished our work or the stream failed, check it
	if (ferror(stream))
	{
		return false;
	}

	*out_vertices = vertices;
	*out_textures = textures;
	*out_normals = normals;

	// restart the stream from the beginning
	rewind(stream);

	return true;
}

static Mesh ComposeFaces(FileBuffer buffer, const  size_t* faces, const  size_t faceCount)
{
	Mesh mesh = CreateMesh();

	// 3 vec3 per face, 3 float per vec3
	mesh->VertexCount = faceCount * 3 * 3;
	mesh->Vertices = SafeAlloc(mesh->VertexCount * sizeof(float));

	// 3 vec2 per face, 2 float per vec2
	mesh->TextureCount = faceCount * 3 * 2;
	mesh->TextureVertices = SafeAlloc(mesh->TextureCount * sizeof(float));

	// 3 vec3 per face, 3 float per vec3
	mesh->NormalCount = faceCount * 3 * 3;
	mesh->Normals = SafeAlloc(mesh->NormalCount * sizeof(float));

	// go through the faces and append the attributes to the final arrays
	// each face has 9 integers representing 3 indices of the vertexs, uvs, and normals that represent the face
	size_t numberOfTriplets = faceCount * 3;

	for (size_t i = 0; i < numberOfTriplets; i++)
	{
		const size_t* face = faces + (i * 3);

		// .obj files are 1 indexed subtract one to get 0 indexed
		size_t vertexIndex = face[0] - 1;
		size_t uvIndex = face[1] - 1;
		size_t normalIndex = face[2] - 1;

		// there are 3 floats per vertex so the address of the nth vec3 is index * 3
		const float* subVertices = buffer->VertexBuffer + (vertexIndex * 3);

		// there are 2 floats per uv
		const float* subUVs = buffer->TextureBuffer + (uvIndex * 2);

		// there are 3 floats per normal
		const float* subNormals = buffer->NormalBuffer + (normalIndex * 3);

		// copy the floats over to their final arrays
		Vector3CopyTo(subVertices, mesh->Vertices + (i * 3));
		Vector2CopyTo(subUVs, mesh->TextureVertices + (i * 2));
		Vector3CopyTo(subNormals, mesh->Normals + (i * 3));
	}

	return mesh;
}

static bool TryCreateMesh(File stream, FileBuffer buffer, Mesh* out_mesh)
{
	*out_mesh = null;

	// read the name of the object
	char* name;

	// offset the buffer by the sequence so we dont use the object sequence as the name lmao
	if (TryParseString(buffer->StreamBuffer + strlen(Sequences.Object), buffer->StreamLength, MAX_OBJECT_NAME_LENGTH + 1, &name) is false)
	{
		return false;
	}


	size_t offset = buffer->VerticesCount * 3;

	size_t vertexCount;
	if (TryGetVectorPattern(stream, buffer->StreamBuffer, buffer->StreamLength, Sequences.Vertex, Sequences.TextureVertex, 3, buffer->VertexBuffer + offset, TryParseVector3, &vertexCount) is false)
	{
		SafeFree(name);

		return false;
	}

	// increment the count for the vertices so the next call to this method appends vertices after ours instead of overwriting
	buffer->VerticesCount += vertexCount;

	offset = buffer->TexturesCount * 2;

	size_t textureVertexCount;
	if (TryGetVectorPattern(stream, buffer->StreamBuffer, buffer->StreamLength, Sequences.TextureVertex, Sequences.NormalVertex, 2, buffer->TextureBuffer + offset, TryParseVector2, &textureVertexCount) is false)
	{
		SafeFree(name);

		return false;
	}

	buffer->TexturesCount += textureVertexCount;

	offset = buffer->NormalsCount * 3;

	size_t normalVertexCount;
	if (TryGetVectorPattern(stream, buffer->StreamBuffer, buffer->StreamLength, Sequences.NormalVertex, Sequences.Smoothing, 3, buffer->NormalBuffer + offset, TryParseVector3, &normalVertexCount) is false)
	{
		SafeFree(name);
		return false;
	}

	buffer->NormalsCount += normalVertexCount;

	// check to see if there is smoothing enabled, this is normally preceded by the laast normal vector and should be in the buffer
	// so we should attemp to parse it
	bool smoothingEnabled;
	if (TryParseBoolean(buffer->StreamBuffer + strlen(Sequences.Smoothing), 3, &smoothingEnabled) is false)
	{
		SafeFree(name);

		return false;
	}

	// at this point the smoothing is still in the buffer, but GetFaceAttributes starts with reading a line so this doesnt matter
	size_t* faceAttributes;
	size_t faces;
	if (TryGetFaceAttributes(stream, buffer->StreamBuffer, buffer->StreamLength, &faceAttributes, &faces) is false)
	{
		SafeFree(name);
		return false;
	}

	*out_mesh = ComposeFaces(buffer, faceAttributes, faces);

	(*out_mesh)->Name = name;
	(*out_mesh)->SmoothingEnabled = smoothingEnabled;

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

	char streamBuffer[BUFFER_SIZE];

	// determine how much we should alloc for the components we use to build the final face arrays for objects
	// this will tell us how many total counts in the entire .obj file there are
	size_t vertexCount;
	size_t textureCount;
	size_t normalCount;

	if (TryCountElements(stream, streamBuffer, BUFFER_SIZE, &vertexCount, &textureCount, &normalCount) is false)
	{
		if (TryClose(stream) is false)
		{
			throw(FailedToCloseFileException);
		}
		return false;
	}

	// all objects reference the same group of vertices, textures, and normals per .pbj spec
	// for example if two cubes exist within the .obj, the first vertex of the second cube would reference the 9th vertex in the array

	// create a portable way to pass around the global file buffers
	FileBuffer buffer = CreateFileBuffer(streamBuffer, BUFFER_SIZE, vertexCount, textureCount, normalCount);

	Model model = CreateModel();

	// read the file until we find an object to start creating a mesh for
	size_t lineLength;
	while (TryReadLine(stream, streamBuffer, 0, BUFFER_SIZE, &lineLength))
	{
		int token = streamBuffer[0];

		if (token is Tokens.Object)
		{
			Mesh mesh;
			if (TryCreateMesh(stream, buffer, &mesh) is false)
			{
				model->Dispose(model);
				buffer->Dispose(buffer);

				return false;
			}

			// append the new mesh to end the of then linked list of meshes on the model
			if (model->Tail != null)
			{
				model->Tail->Next = mesh;
				model->Tail = mesh;
			}
			else
			{
				model->Head = model->Tail = mesh;
			}
		}
	}

	buffer->Dispose(buffer);

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