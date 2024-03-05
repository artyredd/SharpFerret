#include "core/csharp.h"
#include "stdlib.h"
#include "core/file.h"
#include "engine/modeling/importer.h"
#include "core/guards.h"
#include <ctype.h>
#include <string.h>
#include "engine/modeling/model.h"
#include "core/memory.h"
#include "core/math/vectors.h"
#include "core/parsing.h"
#include "core/strings.h"

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
	ulong ObjectSize;
	Sequence Vertex;
	ulong VertexSize;
	Sequence TextureVertex;
	ulong TextureSize;
	Sequence NormalVertex;
	ulong NormalSize;
	Sequence Smoothing;
	ulong SmoothingSize;
	Sequence Face;
	ulong FaceSize;
	Sequence Material;
	ulong MaterialSize;
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
	ulong StreamLength;
	float* VertexBuffer;
	ulong VerticesCount;
	ulong VerticesLength;
	float* TextureBuffer;
	ulong TexturesCount;
	ulong TexturesLength;
	float* NormalBuffer;
	ulong NormalsCount;
	ulong NormalsLength;
	void(*Dispose)(FileBuffer);
};

#define _TO_STRING(s) #s
#define ToString(s) TO_STRING(s)

#define BUFFER_SIZE 1024
#define MAX_OBJECT_NAME_LENGTH 128
#define MAX_FACES SHRT_MAX

static bool TryImportModel(string path, FileFormat format, Model* out_model);
static Model ImportModel(string path, FileFormat format);
const struct _modelImporterMethods Importers = {
	.TryImport = &TryImportModel,
	.Import = &ImportModel
};

static void DisposeFileBuffer(FileBuffer buffer)
{
	// the character buffer is not alloced by CreateFileBuffer do not free it
	Memory.Free(buffer->NormalBuffer, Memory.GenericMemoryBlock);
	Memory.Free(buffer->TextureBuffer, Memory.GenericMemoryBlock);
	Memory.Free(buffer->VertexBuffer, Memory.GenericMemoryBlock);
	Memory.Free(buffer, Memory.GenericMemoryBlock);
}

static bool VerifyFormat(FileFormat format)
{
	for (ulong i = 0; i < sizeof(SupportedFormats) / sizeof(FileFormat); i++)
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
	ulong VertexCount;
	/// <summary>
	/// the number of normals in the entire file
	/// </summary>
	ulong NormalCount;
	/// <summary>
	/// the number of texture in the entire file
	/// </summary>
	ulong TextureCount;
	/// <summary>
	/// The number of objects within the file
	/// </summary>
	ulong ObjectCount;
	/// <summary>
	/// The number of materials in the entire file
	/// </summary>
	ulong MaterialCount;
	/// <summary>
	/// the number of faces in the entire file
	/// </summary>
	ulong FaceCount;
	/// <summary>
	/// An array of the number of faces for each object encountered, the number of faces for the nth object is FaceCounts[n]
	/// </summary>
	ulong* FaceCounts;
};

struct _bufferCollection {
	/// <summary>
	/// The array of meshes for the entire model
	/// </summary>
	Mesh* Meshes;
	/// <summary>
	/// The index of the next empty mesh within the Meshes array
	/// </summary>
	ulong MeshIndex;
	/// <summary>
	/// The vertex buffer that is shared for the entire model
	/// </summary>
	vector3* Vertices;
	/// <summary>
	/// The current index that  is empty within the vertices buffer
	/// </summary>
	ulong VertexIndex;
	/// <summary>
	/// The texture buffer that is shared for the entire model
	/// </summary>
	vector2* Textures;
	/// <summary>
	/// The current index that  is empty within the texture buffer
	/// </summary>
	ulong TextureIndex;
	/// <summary>
	/// The normals buffer that is shared for the entire model
	/// </summary>
	vector3* Normals;
	/// <summary>
	/// The current index that  is empty within the normals buffer
	/// </summary>
	ulong NormalIndex;
};

static void DisposeBufferCollection(struct _bufferCollection* buffers)
{
	Memory.Free(buffers->Meshes, Memory.GenericMemoryBlock);
	Memory.Free(buffers->Vertices, Memory.GenericMemoryBlock);
	Memory.Free(buffers->Textures, Memory.GenericMemoryBlock);
	Memory.Free(buffers->Normals, Memory.GenericMemoryBlock);
}

static bool TryCountElements(File stream, array(char) buffer, struct _elementCounts* out_counts)
{
	if (out_counts is null || stream is null)
	{
		return false;
	}

	rewind(stream);

	// iterate through the file and count various element types so we can alloc the correct amount of space
	// for when we actually process the file
	ulong lineLength;

	while (Files.TryReadLine(stream, buffer, 0, &lineLength))
	{
		// if for some reason the line is blank ignore it
		if (lineLength < 1)
		{
			continue;
		}

		// get the first char so we don't compare entire strings
		char c = buffer->Values[0];

		// !! these are ordered by frequency count, not in order of occurence !!

		if (c is Tokens.Vertex)
		{
			// because multiple sequences share the same starting char do more here
			c = buffer->Values[1];

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

static bool TryParseFace(const char* buffer, ulong* out_attributes)
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
static void VoidMutateVector2(vector2* vector) { /* no action */ }
static void VoidMutateVector3(vector3* vector) { /* no action */ }
#pragma warning(default: 4100)

// this is a monolith, i apologize, but the alternative with additional stack frames was significantly slower and was a bottle neck
static bool TryParseObjects(File stream,
	string buffer,
	struct _elementCounts* counts,
	struct _bufferCollection* buffers,
	void (*MutateVertex)(vector3* vertex),
	void (*MutateTexture)(vector2* texture),
	void (*MutateNormal)(vector3* normal)
)
{
	Mesh currentMesh = null;

	char* offset;
	ulong size;

	// create tmp variables to store the current meshes counts so when we start adding faces
	// we can alloc the space for the final objects
	ulong vertexCount = 0;
	ulong textureCount = 0;
	ulong normalCount = 0;

	// create some variables to ensure we read the file correctly and encounter no errors
	ulong materialCount = 0;
	ulong objectCount = 0;

	// iterate the file line by line
	ulong lineLength;
	while (Files.TryReadLine(stream, buffer, 0, &lineLength))
	{
		char token = buffer->Values[0];

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
			offset = buffer->Values + Sequences.ObjectSize;
			size = min(lineLength - Sequences.ObjectSize, lineLength);

			char* name;
			if (Parsing.TryGetLine(offset, size, MAX_OBJECT_NAME_LENGTH, &name) is false)
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
			token = buffer->Values[1];

			// texture vertex
			if (token is 't')
			{
				// get a pointer to the current position of the texture buffer we are at
				// texture vertice are vector 2s
				vector2* vector = buffers->Textures + buffers->TextureIndex;
				offset = buffer->Values + Sequences.TextureSize;
				size = min(lineLength - Sequences.TextureSize, lineLength);

				if (Vector2s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateTexture(vector);

				// move the index over by the number of floats we wrote
				buffers->TextureIndex += 1;
				textureCount += 1;
			}
			// normals
			else if (token is 'n')
			{
				// get a pointer to the current position of the normal buffer we are at
				// normal vertices are vector 3s
				vector3* vector = buffers->Normals + buffers->NormalIndex;
				offset = buffer->Values + Sequences.NormalSize;
				size = min(lineLength - Sequences.NormalSize, lineLength);

				if (Vector3s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateNormal(vector);

				// move the index over by the number of floats we wrote
				buffers->NormalIndex += 1;
				normalCount += 1;
			}
			// regular vertex
			else
			{
				// get a pointer to the current position of the vertex buffer we are at
				// vertices are vector 3s
				vector3* vector = buffers->Vertices + buffers->VertexIndex;
				offset = buffer->Values + Sequences.VertexSize;
				size = min(lineLength - Sequences.VertexSize, lineLength);

				if (Vector3s.TryDeserialize(offset, size, vector) is false)
				{
					Meshes.Dispose(currentMesh);
					return false;
				}

				MutateVertex(vector);

				// move the index over by the number of floats we wrote
				buffers->VertexIndex += 1;
				vertexCount += 1;
			}

			continue;
		}

		// when we find a face we have to use the already parsed vertices to add to the meshes actual buffers
		if (token is Tokens.Face && currentMesh isnt null)
		{
			// a line with a face definition holds UP TO 9 entries
			// 3 vertices(Garunteed), 3 textures (optional), 3 normals (optional)
			// where each entry is the INDEX of the actual thing within their respective global array
			ulong indices[9];

			// zero the array
			Memory.ZeroArray(indices, sizeof(ulong) * 9);

			offset = buffer->Values + Sequences.FaceSize;

			// read the indices
			if (TryParseFace(offset, indices) is false)
			{
				Meshes.Dispose(currentMesh);
				return false;
			}

			ulong faceCount = counts->FaceCounts[objectCount - 1] * 3;

			// make sure we have arrays to write values to
			if (currentMesh->Vertices is null && vertexCount > 0)
			{
				currentMesh->Vertices = Memory.Alloc(sizeof(vector3) * faceCount, Memory.GenericMemoryBlock);
			}
			if (currentMesh->TextureVertices is null && textureCount > 0)
			{
				currentMesh->TextureVertices = Memory.Alloc(sizeof(vector2) * faceCount, Memory.GenericMemoryBlock);
			}
			if (currentMesh->NormalVertices is null && normalCount > 0)
			{
				currentMesh->NormalVertices = Memory.Alloc(sizeof(vector3) * faceCount, Memory.GenericMemoryBlock);
			}

			// since there are 3 triplets loop
			for (ulong i = 0; i < 3; i++)
			{
				const ulong* face = indices + (i * 3);

				if (vertexCount > 0)
				{
					// .obj files are 1 indexed subtract one to get 0 indexed
					ulong vertexIndex = safe_subtract(face[0], 1);

					// there are 3 floats per vertex so the address of the nth vector3 is index * 3
					const vector3 subVertices = buffers->Vertices[vertexIndex];

					// copy the floats over to their final arrays
					currentMesh->Vertices[currentMesh->VertexCount] = subVertices;
					currentMesh->VertexCount++;
				}

				// if we didnt count any textures we shouldn't try to write them to an array
				if (textureCount > 0)
				{
					// .obj files are 1 indexed subtract one to get 0 indexed
					ulong uvIndex = safe_subtract(face[1], 1);

					// there are 2 floats per uv
					const vector2 subUVs = buffers->Textures[uvIndex];

					currentMesh->TextureVertices[currentMesh->TextureCount] = subUVs;
					currentMesh->TextureCount++;
				}

				// if we didnt count any normals we shouldn't try to write them to an array
				if (normalCount > 0)
				{
					// .obj files are 1 indexed subtract one to get 0 indexed
					ulong normalIndex = safe_subtract(face[2], 1);

					// there are 3 floats per normal
					const vector3 subNormals = buffers->Normals[normalIndex];

					currentMesh->NormalVertices[currentMesh->NormalCount] = subNormals;
					currentMesh->NormalCount++;
				}
			}
			continue;
		}

		if (token is Tokens.Smoothing && currentMesh isnt null)
		{
			offset = buffer->Values + Sequences.SmoothingSize;
			size = min(lineLength - Sequences.SmoothingSize, lineLength);

			bool smoothing;
			if (Parsing.TryGetBool(offset, size, &smoothing) is false)
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
			offset = buffer->Values + Sequences.MaterialSize;

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
ulong FaceCounts[MAX_FACES];

DEFINE_TYPE_ID(Mesh);

static bool TryImportModelStream(File stream,
	Model* out_model,
	void (*MutateVertex)(vector3* vertex),
	void (*MutateTexture)(vector2* texture),
	void (*MutateNormal)(vector3* normal)
)
{

	array(char) streamBuffer = empty_stack_array(char, BUFFER_SIZE);

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

	Memory.ZeroArray(FaceCounts, MAX_FACES * sizeof(ulong));

	// count the occurences of the elements within the file
	if (TryCountElements(stream,
		streamBuffer,
		&elementCounts) is false)
	{
		Files.TryClose(stream);
		// if we were not able to count all the elements we should return false, something weird happened
		return false;
	}

	REGISTER_TYPE(Mesh);

	Mesh* meshes = Memory.Alloc(sizeof(Mesh) * elementCounts.ObjectCount, MeshTypeId);

	// create a place to store all the vertices temporarily
	struct _bufferCollection buffers = {
		.Meshes = meshes,
		.MeshIndex = 0,
		.Vertices = Memory.Alloc(sizeof(vector3) * elementCounts.VertexCount, Memory.GenericMemoryBlock),
		.VertexIndex = 0,
		.Normals = Memory.Alloc(sizeof(vector3) * elementCounts.NormalCount, Memory.GenericMemoryBlock),
		.NormalIndex = 0,
		.Textures = Memory.Alloc(sizeof(vector2) * elementCounts.TextureCount, Memory.GenericMemoryBlock),
		.TextureIndex = 0
	};

	if (TryParseObjects(stream, streamBuffer, &elementCounts, &buffers,
		MutateVertex is null ? &VoidMutateVector3 : MutateVertex,
		MutateTexture is null ? &VoidMutateVector2 : MutateTexture,
		MutateNormal is null ? &VoidMutateVector3 : MutateNormal
	) is false)
	{
		Memory.Free(meshes, MeshTypeId);
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

static bool TryImportModel(string path, FileFormat format, Model* out_model)
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
		Files.Close(stream);
		return false;
	}

	// set the name of the model as the path that was used to load it
	model->Name = Strings.Duplicate(path->Values, path->Count);

	*out_model = model;

	if (Files.TryClose(stream) is false)
	{
		Models.Dispose(model);

		return false;
	}

	return true;
}

static Model ImportModel(string path, FileFormat format)
{
	Model model;
	if (TryImportModel(path, format, &model) is false)
	{
		return null;
	}

	return model;
}