#include "csharp.h"
#include "stdlib.h"
#include "singine/file.h"
#include "modeling/importer.h"
#include "singine/guards.h"
#include <ctype.h>
#include <string.h>
#include "modeling/model.h"
#include "singine/memory.h"
#include "math/vectors.h"
#include "singine/parsing.h"
#include "singine/strings.h"
#include "cglm/vec3.h"

typedef int Token;
typedef const char* Sequence;

static struct _tokens {
	const Token Object;
	const Token Comment;
	const Token Vertex;
	const Token Face;
	const Token Smoothing;
	const Token Material;
} Tokens = {
	'o', // Object
	'#', // Comment
	'v', // Vertex
	'f', // Face
	's', // smoothing
	.Material = 'u'
};

static struct _sequences {
	Sequence Object;
	size_t ObjectSize;
	Sequence Vertex;
	size_t VertexSize;
	Sequence TextureVertex;
	size_t TextureSize;
	Sequence NormalVertex;
	size_t NormalSize;
	Sequence Smoothing;
	size_t SmoothingSize;
	Sequence Face;
	size_t FaceSize;
	Sequence Material;
	size_t MaterialSize;
} Sequences = {
	.Object = "o ",
	.ObjectSize = sizeof("o ") - 1,
	.Vertex = "v ",
	.VertexSize = sizeof("v ") - 1,
	.TextureVertex = "vt ",
	.TextureSize = sizeof("vt ") - 1,
	.NormalVertex = "vn ",
	.NormalSize = sizeof("vn ") - 1,
	.Smoothing = "s ",
	.SmoothingSize = sizeof("s ") - 1,
	.Face = "f ",
	.FaceSize = sizeof("f ") - 1,
	.Material = "usemtl ",
	.MaterialSize = sizeof("usemtl ") - 1,
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
#define MAX_OBJECT_NAME_LENGTH 128
#define MAX_FACES SHRT_MAX

static bool TryImportModel(char* path, FileFormat format, Model* out_model);
static Model ImportModel(char* path, FileFormat format);
const struct _modelImporterMethods Importers = {
	.TryImport = &TryImportModel,
	.Import = &ImportModel
};

static void DisposeFileBuffer(FileBuffer buffer)
{
	// the character buffer is not alloced by CreateFileBuffer do not free it
	SafeFree(buffer->NormalBuffer);
	SafeFree(buffer->TextureBuffer);
	SafeFree(buffer->VertexBuffer);
	SafeFree(buffer);
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

/// <summary>
/// contains count information for a object file
/// </summary>
struct _elementCounts {
	/// <summary>
	/// The number of vertices in the entire file
	/// </summary>
	size_t VertexCount;
	/// <summary>
	/// the number of normals in the entire file
	/// </summary>
	size_t NormalCount;
	/// <summary>
	/// the number of texture in the entire file
	/// </summary>
	size_t TextureCount;
	/// <summary>
	/// The number of objects within the file
	/// </summary>
	size_t ObjectCount;
	/// <summary>
	/// The number of materials in the entire file
	/// </summary>
	size_t MaterialCount;
	/// <summary>
	/// the number of faces in the entire file
	/// </summary>
	size_t FaceCount;
	/// <summary>
	/// An array of the number of faces for each object encountered, the number of faces for the nth object is FaceCounts[n]
	/// </summary>
	size_t* FaceCounts;
};

struct _bufferCollection {
	/// <summary>
	/// The array of meshes for the entire model
	/// </summary>
	Mesh* Meshes;
	/// <summary>
	/// The index of the next empty mesh within the Meshes array
	/// </summary>
	size_t MeshIndex;
	/// <summary>
	/// The vertex buffer that is shared for the entire model
	/// </summary>
	float* Vertices;
	/// <summary>
	/// The current index that  is empty within the vertices buffer
	/// </summary>
	size_t VertexIndex;
	/// <summary>
	/// The texture buffer that is shared for the entire model
	/// </summary>
	float* Textures;
	/// <summary>
	/// The current index that  is empty within the texture buffer
	/// </summary>
	size_t TextureIndex;
	/// <summary>
	/// The normals buffer that is shared for the entire model
	/// </summary>
	float* Normals;
	/// <summary>
	/// The current index that  is empty within the normals buffer
	/// </summary>
	size_t NormalIndex;
};

static void DisposeBufferCollection(struct _bufferCollection* buffers)
{
	SafeFree(buffers->Meshes);
	SafeFree(buffers->Vertices);
	SafeFree(buffers->Textures);
	SafeFree(buffers->Normals);
}

static bool TryCountElements(File stream, char* buffer, const size_t bufferLength, struct _elementCounts* out_counts)
{
	if (out_counts is null || stream is null)
	{
		return false;
	}

	rewind(stream);

	// iterate through the file and count various element types so we can alloc the correct amount of space
	// for when we actually process the file
	size_t lineLength;

	while (Files.TryReadLine(stream, buffer, 0, bufferLength, &lineLength))
	{
		// if for some reason the line is blank ignore it
		if (lineLength < 1)
		{
			continue;
		}

		// get the first char so we don't compare entire strings
		char c = buffer[0];

		// !! these are ordered by frequency count, not in order of occurence !!

		if (c is Tokens.Vertex)
		{
			// because multiple sequences share the same starting char do more here
			c = buffer[1];

			if (c is 't')
			{
				++(out_counts->TextureCount);
				continue;
			}

			if (c is 'n')
			{
				++(out_counts->NormalCount);
				continue;
			}

			++(out_counts->VertexCount);
			continue;
		}

		if (c is Tokens.Face)
		{
			++(out_counts->FaceCount);

			// we should keep track of the number of faces in each object for our second pass, so we dont have to do a third pass
			out_counts->FaceCounts[out_counts->ObjectCount - 1]++;

			continue;
		}

		if (c is Tokens.Object)
		{
			++(out_counts->ObjectCount);
			continue;
		}

		if (c is Tokens.Smoothing)
		{
			continue;
		}

		if (c is Tokens.Material)
		{
			++(out_counts->MaterialCount);
			continue;
		}
	}

	rewind(stream);

	return ferror(stream) is 0;
}

static bool TryParseFace(const char* buffer, size_t* out_attributes)
{
	// base case assume the file includes normals, uv and vertices
	int count = sscanf_s(buffer, "%lli/%lli/%lli %lli/%lli/%lli %lli/%lli/%lli",
		&out_attributes[0], // vertex
		&out_attributes[1], // uv
		&out_attributes[2], // normal
		&out_attributes[3], // vertex
		&out_attributes[4], // uv
		&out_attributes[5], // normal
		&out_attributes[6], // vertex
		&out_attributes[7], // uv
		&out_attributes[8]); // normal

	if (count is 9)
	{
		return true;
	}

	// if there are no normals OR UV's then our count will be 1(the first number)
	// format: "f 57 56 58"
	if (count is 1)
	{
		count = sscanf_s(buffer, "%lli %lli %lli",
			&out_attributes[0],
			&out_attributes[3],
			&out_attributes[6]);

		if (count is 3)
		{
			return true;
		}
	}

	// try to import it with vertices and normals but no uv
	count = sscanf_s(buffer, "%lli//%lli %lli//%lli %lli//%lli",
		&out_attributes[0], // vertex
		&out_attributes[2], // normal
		&out_attributes[3], // vertex
		&out_attributes[5], // normal
		&out_attributes[6], // vertex
		&out_attributes[8]); // normal

	if (count is 6)
	{
		return true;
	}

	// try to import with uv but no normals
	count = sscanf_s(buffer, "%lli/%lli/ %lli/%lli/ %lli/%lli/",
		&out_attributes[0], // vertex
		&out_attributes[1], // uv
		&out_attributes[3], // vertex
		&out_attributes[4], // uv
		&out_attributes[6], // vertex
		&out_attributes[7]); // uv

	return count == 6;
}

// does not mutate the provided vector
// ignore unreferenced param
#pragma warning(disable: 4100)
static void VoidMutate(float* vector) { /* no action */ }
#pragma warning(default: 4100)

// this is a monolith, i apologize, but the alternative with additional stack frames was significantly slower and was a bottle neck
static bool TryParseObjects(File stream,
	char* buffer,
	const size_t bufferLength,
	struct _elementCounts* counts,
	struct _bufferCollection* buffers,
	void (*MutateVertex)(float* vertex),
	void (*MutateTexture)(float* texture),
	void (*MutateNormal)(float* normal)
)
{
	Mesh currentMesh = null;

	char* offset;
	size_t size;

	// create tmp variables to store the current meshes counts so when we start adding faces
	// we can alloc the space for the final objects
	size_t vertexCount = 0;
	size_t textureCount = 0;
	size_t normalCount = 0;

	// create some variables to ensure we read the file correctly and encounter no errors
	size_t materialCount = 0;
	size_t objectCount = 0;

	// iterate the file line by line
	size_t lineLength;
	while (Files.TryReadLine(stream, buffer, 0, bufferLength, &lineLength))
	{
		char token = buffer[0];

		// ignore comments
		if (token is Tokens.Comment)
		{
			continue;
		}

		// if we found an object, finish the last one and start over
		if (token is Tokens.Object)
		{
			Mesh newMesh = Meshes.Create();

			// set the name of the current mesh with the name after the token
			offset = buffer + Sequences.ObjectSize;
			size = min(lineLength - Sequences.ObjectSize, lineLength);

			char* name;
			if (TryParseLine(offset, size, MAX_OBJECT_NAME_LENGTH, &name) is false)
			{
				Meshes.Dispose(currentMesh);

				return false;
			}

			newMesh->Name = name;

			// increment the number of objects
			buffers->Meshes[objectCount] = newMesh;

			currentMesh = newMesh;

			++(objectCount);

			// make sure we didn't find more objects than we expected
			if (objectCount > counts->ObjectCount)
			{
				Meshes.Dispose(currentMesh);
				return false;
			}

			// reset counts so we can use them for the next object
			textureCount = 0;
			vertexCount = 0;
			textureCount = 0;

			continue;
		}

		// if we found a vertex write it to the global array
		// since every face references the same global array when we build the faces for any specific object
		if (token is Tokens.Vertex)
		{
			token = buffer[1];

			// texture vertex
			if (token is 't')
			{
				// get a pointer to the current position of the texture buffer we are at
				// texture vertice are vector 2s
				float* vector = buffers->Textures + buffers->TextureIndex;
				offset = buffer + Sequences.TextureSize;
				size = min(lineLength - Sequences.TextureSize, lineLength);

				if (Vector2s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateTexture(vector);

				// move the index over by the number of floats we wrote
				buffers->TextureIndex += 2;
				textureCount += 2;
			}
			// normals
			else if (token is 'n')
			{
				// get a pointer to the current position of the normal buffer we are at
				// normal vertices are vector 3s
				float* vector = buffers->Normals + buffers->NormalIndex;
				offset = buffer + Sequences.NormalSize;
				size = min(lineLength - Sequences.NormalSize, lineLength);

				if (Vector3s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateNormal(vector);

				// move the index over by the number of floats we wrote
				buffers->NormalIndex += 3;
				normalCount += 3;
			}
			// regular vertex
			else
			{
				// get a pointer to the current position of the vertex buffer we are at
				// vertices are vector 3s
				float* vector = buffers->Vertices + buffers->VertexIndex;
				offset = buffer + Sequences.VertexSize;
				size = min(lineLength - Sequences.VertexSize, lineLength);

				if (Vector3s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateVertex(vector);

				// move the index over by the number of floats we wrote
				buffers->VertexIndex += 3;
				vertexCount += 3;
			}

			continue;
		}

		// when we find a face we have to use the already parsed vertices to add to the meshes actual buffers
		if (token is Tokens.Face && currentMesh isnt null)
		{
			// a line with a face definition holds UP TO 9 entries
			// 3 vertices(Garunteed), 3 textures (optional), 3 normals (optional)
			// where each entry is the INDEX of the actual thing within their respective global array
			size_t indices[9];

			// zero the array
			ZeroArray(indices, sizeof(size_t) * 9);

			offset = buffer + Sequences.FaceSize;

			// read the indices
			if (TryParseFace(offset, indices) is false)
			{
				Meshes.Dispose(currentMesh);
				return false;
			}

			size_t faceCount = counts->FaceCounts[objectCount - 1] * 3;

			// make sure we have arrays to write values to
			if (currentMesh->Vertices is null && vertexCount > 0)
			{
				currentMesh->Vertices = SafeAlloc(sizeof(float) * 3 * faceCount);
			}
			if (currentMesh->TextureVertices is null && textureCount > 0)
			{
				currentMesh->TextureVertices = SafeAlloc(sizeof(float) * 2 * faceCount);
			}
			if (currentMesh->Normals is null && normalCount > 0)
			{
				currentMesh->Normals = SafeAlloc(sizeof(float) * 3 * faceCount);
			}

			// since there are 3 triplets loop
			for (size_t i = 0; i < 3; i++)
			{
				const size_t* face = indices + (i * 3);

				// .obj files are 1 indexed subtract one to get 0 indexed
				size_t vertexIndex = face[0] - 1;
				size_t uvIndex = face[1] - 1;
				size_t normalIndex = face[2] - 1;

				// there are 3 floats per vertex so the address of the nth vec3 is index * 3
				const float* subVertices = buffers->Vertices + (vertexIndex * 3);

				// there are 2 floats per uv
				const float* subUVs = buffers->Textures + (uvIndex * 2);

				// there are 3 floats per normal
				const float* subNormals = buffers->Normals + (normalIndex * 3);

				// copy the floats over to their final arrays
				Vectors3CopyTo(subVertices, currentMesh->Vertices + currentMesh->VertexCount);
				currentMesh->VertexCount += 3;

				// if we didnt count any textures we shouldn't try to write them to an array
				if (textureCount > 0)
				{
					Vectors2CopyTo(subUVs, currentMesh->TextureVertices + currentMesh->TextureCount);
					currentMesh->TextureCount += 2;
				}

				// if we didnt count any normals we shouldn't try to write them to an array
				if (normalCount > 0)
				{
					Vectors3CopyTo(subNormals, currentMesh->Normals + currentMesh->NormalCount);
					currentMesh->NormalCount += 3;
				}
			}
			continue;
		}

		if (token is Tokens.Smoothing && currentMesh isnt null)
		{
			offset = buffer + Sequences.SmoothingSize;
			size = min(lineLength - Sequences.SmoothingSize, lineLength);

			bool smoothing;
			if (TryParseBoolean(offset, size, &smoothing) is false)
			{
				Meshes.Dispose(currentMesh);
				return false;
			}
			currentMesh->SmoothingEnabled = smoothing;
			continue;
		}

		if (token is Tokens.Material && currentMesh isnt null)
		{
			// copy the name of the material over to the current mesh
			offset = buffer + Sequences.MaterialSize;

			// becuase we're not relying on nul terminated we should add 1 to inclde the nul terminator character
			size = min(lineLength - Sequences.MaterialSize, lineLength) + 1;

			char* material = Strings.Duplicate(offset, size);
			currentMesh->MaterialName = material;

			++(materialCount);

			// make sure we didn't find more materials than we expected
			if (materialCount > counts->MaterialCount)
			{
				Meshes.Dispose(currentMesh);
				return false;
			}
			continue;
		}
	}

	return true;
}

// buffer to keep the face counts
size_t FaceCounts[MAX_FACES];

static bool TryImportModelStream(File stream,
	Model* out_model,
	void (*MutateVertex)(float* vertex),
	void (*MutateTexture)(float* texture),
	void (*MutateNormal)(float* normal)
)
{

	char streamBuffer[BUFFER_SIZE];

	// create a place to put the counts of the file
	struct _elementCounts elementCounts = {
		.FaceCount = 0,
		.MaterialCount = 0,
		.NormalCount = 0,
		.ObjectCount = 0,
		.TextureCount = 0,
		.VertexCount = 0,
		.FaceCounts = FaceCounts
	};

	ZeroArray(FaceCounts, MAX_FACES * sizeof(size_t));

	// count the occurences of the elements within the file
	if (TryCountElements(stream,
		streamBuffer,
		BUFFER_SIZE,
		&elementCounts) is false)
	{
		Files.TryClose(stream);
		// if we were not able to count all the elements we should return false, something weird happened
		return false;
	}

	Mesh* meshes = SafeAlloc(sizeof(Mesh) * elementCounts.ObjectCount);

	// create a place to store all the vertices temporarily
	struct _bufferCollection buffers = {
		.Meshes = meshes,
		.MeshIndex = 0,
		.Vertices = SafeAlloc(sizeof(float) * elementCounts.VertexCount * 3),
		.VertexIndex = 0,
		.Normals = SafeAlloc(sizeof(float) * elementCounts.NormalCount * 3),
		.NormalIndex = 0,
		.Textures = SafeAlloc(sizeof(float) * elementCounts.TextureCount * 2),
		.TextureIndex = 0
	};

	if (TryParseObjects(stream, streamBuffer, BUFFER_SIZE, &elementCounts, &buffers,
		MutateVertex is null ? &VoidMutate : MutateVertex,
		MutateTexture is null ? &VoidMutate : MutateTexture,
		MutateNormal is null ? &VoidMutate : MutateNormal
	) is false)
	{
		SafeFree(meshes);
		DisposeBufferCollection(&buffers);
		Files.TryClose(stream);
		return false;
	}

	Model model = Models.Create();

	model->Count = elementCounts.ObjectCount;

	model->Meshes = meshes;

	buffers.Meshes = null;

	// make sure to dispose the buffer collection when we are done
	DisposeBufferCollection(&buffers);

	*out_model = model;

	return true;
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
	if (Files.TryOpen(path, FileModes.ReadBinary, &stream) is false)
	{
		return false;
	}

	Model model;
	if (TryImportModelStream(stream, &model, null, null, null) is false)
	{
		return false;
	}

	// set the name of the model as the path that was used to load it
	model->Name = Strings.DuplicateTerminated(path);

	*out_model = model;

	if (Files.TryClose(stream) is false)
	{
		Models.Dispose(model);

		return false;
	}

	return true;
}

static Model ImportModel(char* path, FileFormat format)
{
	Model model;
	if (TryImportModel(path, format, &model) is false)
	{
		return null;
	}

	return model;
}