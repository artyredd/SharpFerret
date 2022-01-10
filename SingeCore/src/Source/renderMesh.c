#include "graphics/renderMesh.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "cglm/mat4.h"
#include "helpers/macros.h"

static RenderMesh InstanceMesh(RenderMesh mesh);
static void Draw(RenderMesh model);
static void Dispose(RenderMesh mesh);
static bool TryBindMesh(const Mesh mesh, RenderMesh* out_model);
static RenderMesh Duplicate(RenderMesh mesh);
static RenderMesh CreateRenderMesh(void);

const struct _renderMeshMethods RenderMeshes = {
	.Dispose = &Dispose,
	.Draw = &Draw,
	.TryBindMesh = &TryBindMesh,
	.Instance = &InstanceMesh,
	.Duplicate = &Duplicate,
	.Create = &CreateRenderMesh
};

static void OnBufferDispose(unsigned int handle)
{
	glDeleteBuffers(1, &handle);
}

static void Dispose(RenderMesh mesh)
{
	if (mesh is null)
	{
		return;
	}

	// since these handles are shared among possibly many instances we only want to actually
	// clear the buffer when the final instance has been disposed
	SharedHandles.Dispose(mesh->VertexBuffer, &OnBufferDispose);
	SharedHandles.Dispose(mesh->UVBuffer, &OnBufferDispose);
	SharedHandles.Dispose(mesh->NormalBuffer, &OnBufferDispose);

	Transforms.Dispose(mesh->Transform);

	SafeFree(mesh);
}

static void Draw(RenderMesh model)
{
	if (model->VertexBuffer isnt null)
	{
		glEnableVertexAttribArray(VertexShaderPosition);

		glBindBuffer(GL_ARRAY_BUFFER, model->VertexBuffer->Handle);

		glVertexAttribPointer(
			VertexShaderPosition,
			3,
			GL_FLOAT,
			false,
			0,
			null
		);
	}

	if (model->UVBuffer isnt null)
	{
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(UVShaderPosition);
		glBindBuffer(GL_ARRAY_BUFFER, model->UVBuffer->Handle);
		glVertexAttribPointer(
			UVShaderPosition,                 // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			null                    // array buffer offset
		);
	}

	// the entire mesh pipling ive written handles up to size_t
	// its casted down to int here for DrawArrays
	// this may cause issues at this line for models with > 65565 triangles
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)model->NumberOfTriangles);

	glDisableVertexAttribArray(VertexShaderPosition);
	glDisableVertexAttribArray(UVShaderPosition);
}

static RenderMesh CreateRenderMesh()
{
	RenderMesh mesh = SafeAlloc(sizeof(struct _renderMesh));

	mesh->Id = 0;

	mesh->UVBuffer = null;
	mesh->VertexBuffer = null;
	mesh->NormalBuffer = null;

	mesh->NumberOfTriangles = 0;

	mesh->Transform = Transforms.Create();

	return mesh;
}

static bool TryBindBuffer(float* buffer, size_t sizeInBytes, SharedHandle destinationBuffer)
{
	destinationBuffer->Handle = 0;

	GLuint index_buffer; // Save this for later rendering
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, buffer, GL_STATIC_DRAW);

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	if (sizeInBytes != size)
	{
		glDeleteBuffers(1, &index_buffer);

		fprintf(stderr, "Failed to bind a buffer for a model, attempted to bind %lli bytes but only bound %i bytes", sizeInBytes, size);

		return false;
	}

	destinationBuffer->Handle = index_buffer;

	return true;
}

static bool TryBindMesh(const Mesh mesh, RenderMesh* out_model)
{
	*out_model = null;

	RenderMesh model = CreateRenderMesh();

	// since this is a new mesh we should create new buffers from scratch

	model->VertexBuffer = SharedHandles.Create();

	if (TryBindBuffer(mesh->Vertices, mesh->VertexCount * sizeof(float), model->VertexBuffer) is false)
	{
		RenderMeshes.Dispose(model);
		return false;
	}

	if (mesh->TextureCount isnt 0)
	{
		model->UVBuffer = SharedHandles.Create();

		if (TryBindBuffer(mesh->TextureVertices, mesh->TextureCount * sizeof(float), model->UVBuffer) is false)
		{
			RenderMeshes.Dispose(model);
			return false;
		}
	}


	if (mesh->NormalCount isnt 0)
	{
		model->NormalBuffer = SharedHandles.Create();

		if (TryBindBuffer(mesh->Normals, mesh->NormalCount * sizeof(float), model->NormalBuffer) is false)
		{
			RenderMeshes.Dispose(model);
			return false;
		}
	}

	model->NumberOfTriangles = mesh->VertexCount / 3;

	*out_model = model;

	return true;
}

static void RenderMeshCopyTo(RenderMesh source, RenderMesh destination)
{
	CopyMember(source, destination, VertexBuffer);

	if (source->VertexBuffer isnt null)
	{
		++source->VertexBuffer->ActiveInstances;
	}

	CopyMember(source, destination, UVBuffer);

	if (source->UVBuffer isnt null)
	{
		++source->UVBuffer->ActiveInstances;
	}

	CopyMember(source, destination, NormalBuffer);

	if (source->NormalBuffer isnt null)
	{
		++source->NormalBuffer->ActiveInstances;
	}

	CopyMember(source, destination, NumberOfTriangles);
}

static RenderMesh InstanceMesh(RenderMesh mesh)
{
	RenderMesh result = CreateRenderMesh();

	RenderMeshCopyTo(mesh, result);

	return result;
}

static RenderMesh Duplicate(RenderMesh mesh)
{
	RenderMesh result = InstanceMesh(mesh);

	Transforms.CopyTo(mesh->Transform, result->Transform);

	CopyMember(mesh, result, Id);

	return result;
}