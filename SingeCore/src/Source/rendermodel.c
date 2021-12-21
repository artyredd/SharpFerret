#include "graphics/renderModel.h"
#include "singine/memory.h"
#include "GL/glew.h"

static void Dispose(RenderMesh model)
{
	// never dispose of the shader in the model
	glDeleteBuffers(1, &model->VertexBuffer);
	glDeleteBuffers(1, &model->UVBuffer);
	glDeleteBuffers(1, &model->NormalBuffer);

	SafeFree(model);
}

static void Draw(RenderMesh model, mat4 position)
{
	glUseProgram(model->Shader->Handle);

	glEnableVertexAttribArray(VertexShaderPosition);

	glBindBuffer(GL_ARRAY_BUFFER, model->VertexBuffer);

	glVertexAttribPointer(
		VertexShaderPosition,
		3,
		GL_FLOAT,
		false,
		0,
		null
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(UVShaderPosition);
	glBindBuffer(GL_ARRAY_BUFFER, model->UVBuffer);
	glVertexAttribPointer(
		UVShaderPosition,                 // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		null                    // array buffer offset
	);

	// the entire mesh pipling ive written handles up to size_t
	// its casted down to int here for DrawArrays
	// this may cause issues at this line for models with > 65565 triangles
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)model->NumberOfTriangles);

	glDisableVertexAttribArray(VertexShaderPosition);
	glDisableVertexAttribArray(UVShaderPosition);
}

static RenderMesh CreateRenderModel()
{
	RenderMesh mesh = SafeAlloc(sizeof(struct _renderMesh));

	mesh->Dispose = &Dispose;
	mesh->Draw = &Draw;

	mesh->Shader = DefaultShader;
	mesh->UVBuffer = 0;
	mesh->VertexBuffer = 0;
	mesh->NormalBuffer = 0;

	mesh->NumberOfTriangles = 0;

	return mesh;
}

static bool TryBindBuffer(float* buffer, size_t sizeInBytes, unsigned int* out_handle)
{
	*out_handle = 0;

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

	*out_handle = index_buffer;

	return true;
}

bool TryBindMesh(const Mesh mesh, RenderMesh* out_model)
{
	*out_model = null;

	RenderMesh model = CreateRenderModel();

	if (TryBindBuffer(mesh->Vertices, mesh->VertexCount * sizeof(float), &model->VertexBuffer) is false)
	{
		model->Dispose(model);
		return false;
	}

	if (TryBindBuffer(mesh->TextureVertices, mesh->TextureCount * sizeof(float), &model->UVBuffer) is false)
	{
		model->Dispose(model);
		return false;
	}

	if (TryBindBuffer(mesh->Normals, mesh->NormalCount * sizeof(float), &model->NormalBuffer) is false)
	{
		model->Dispose(model);
		return false;
	}

	model->NumberOfTriangles = mesh->VertexCount / 3;

	*out_model = model;

	return true;
}