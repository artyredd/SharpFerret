#include "engine/graphics/renderMesh.h"
#include "core/memory.h"
#include "GL/glew.h"
#include "core/macros.h"
#include "core/strings.h"

private RenderMesh InstanceMesh(RenderMesh mesh);
private void Draw(RenderMesh model);
private void Dispose(RenderMesh mesh);
private bool TryBindMesh(const Mesh mesh, RenderMesh* out_model);
private RenderMesh Duplicate(RenderMesh mesh);
private RenderMesh CreateRenderMesh(void);
private bool TryBindModel(Model model, RenderMesh** out_meshArray);
private void Save(File, RenderMesh mesh);

const struct _renderMeshMethods RenderMeshes = {
	.Dispose = &Dispose,
	.Draw = &Draw,
	.TryBindMesh = &TryBindMesh,
	.TryBindModel = &TryBindModel,
	.Instance = &InstanceMesh,
	.Duplicate = &Duplicate,
	.Create = &CreateRenderMesh,
	.Save = &Save
};

private void OnBufferDispose(SharedHandle handle)
{
	GraphicsDevice.DeleteBuffer(handle->Handle);
}

private void OnNameDispose(Pointer(byte) resource, void* state)
{
	if (state is null)
	{
		Memory.Free(resource->Resource, Memory.String);
	}
}

DEFINE_TYPE_ID(RenderMesh);

private void Dispose(RenderMesh mesh)
{
	if (mesh is null)
	{
		return;
	}

	// since the name is shared between all rendermeshes that come from the same model we must not dispose the name till the last render
	// mesh instance is being disposed
	Pointers(byte).Dispose(mesh->Name, null, &OnNameDispose);

	// since these handles are shared among possibly many instances we only want to actually
	// clear the buffer when the final instance has been disposed
	SharedHandles.Dispose(mesh->VertexBuffer, mesh->VertexBuffer, &OnBufferDispose);
	SharedHandles.Dispose(mesh->UVBuffer, mesh->UVBuffer, &OnBufferDispose);
	SharedHandles.Dispose(mesh->NormalBuffer, mesh->NormalBuffer, &OnBufferDispose);

	Transforms.Dispose(mesh->Transform);

	Memory.Free(mesh, RenderMeshTypeId);
}

private void LoadAttributeBuffer(unsigned int Position, unsigned int Handle, unsigned int dimensions)
{
	glEnableVertexAttribArray(Position);

	glBindBuffer(GL_ARRAY_BUFFER, Handle);

	glVertexAttribPointer(
		Position,
		dimensions,
		GL_FLOAT,
		false,
		0,
		null
	);
}

private void Draw(RenderMesh mesh)
{
	if (mesh->VertexBuffer isnt null)
	{

		LoadAttributeBuffer(VertexShaderPosition, mesh->VertexBuffer->Handle, 3);

		if (mesh->CopyBuffersOnDraw)
		{
			glBufferSubData(GL_ARRAY_BUFFER,
				0,
				mesh->NumberOfTriangles * 3 * sizeof(float),
				((Mesh)mesh->Mesh->Resource)->Vertices
			);
		}
	}

	if (mesh->UVBuffer isnt null)
	{
		LoadAttributeBuffer(UVShaderPosition, mesh->UVBuffer->Handle, 2);
	}

	if (mesh->NormalBuffer isnt null)
	{
		LoadAttributeBuffer(NormalShaderPosition, mesh->NormalBuffer->Handle, 3);
	}

	if (mesh->ShadeSmooth)
	{
		glShadeModel(GL_SMOOTH);
	}
	else
	{
		glShadeModel(GL_FLAT);
	}


	// the entire mesh pipling ive written handles up to ulong
	// its casted down to int here for DrawArrays
	// this may cause issues at this line for models with > 32767 triangles
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mesh->NumberOfTriangles);

	glDisableVertexAttribArray(VertexShaderPosition);
	glDisableVertexAttribArray(UVShaderPosition);
	glDisableVertexAttribArray(NormalShaderPosition);
}

private RenderMesh CreateRenderMesh()
{
	Memory.RegisterTypeName(nameof(RenderMesh), &RenderMeshTypeId);

	RenderMesh mesh = Memory.Alloc(sizeof(struct _renderMesh), RenderMeshTypeId);

	mesh->Transform = Transforms.Create();

	mesh->CopyBuffersOnDraw = false;

	return mesh;
}

private bool TryBindBuffer(float* buffer, ulong sizeInBytes, SharedHandle destinationBuffer)
{
	destinationBuffer->Handle = 0;

	GLuint indexBuffer = GraphicsDevice.GenerateBuffer();
	glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeInBytes, buffer, GL_STREAM_DRAW);

	GLint size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	if (sizeInBytes != size)
	{
		GraphicsDevice.DeleteBuffer(indexBuffer);

		fprintf(stderr, "Failed to bind a buffer for a model, attempted to bind %lli bytes but only bound %i bytes", sizeInBytes, size);

		return false;
	}

	destinationBuffer->Handle = indexBuffer;

	return true;
}

private bool TryBindMesh(const Mesh mesh, RenderMesh* out_renderMesh)
{
	*out_renderMesh = null;

	RenderMesh model = CreateRenderMesh();

	// since this is a new mesh we should create new buffers from scratch

	model->VertexBuffer = SharedHandles.Create();

	if (TryBindBuffer((float*)mesh->Vertices, mesh->VertexCount * sizeof(vector3), model->VertexBuffer) is false)
	{
		RenderMeshes.Dispose(model);
		return false;
	}

	if (mesh->TextureCount isnt 0)
	{
		model->UVBuffer = SharedHandles.Create();

		if (TryBindBuffer((float*)mesh->TextureVertices, mesh->TextureCount * sizeof(vector2), model->UVBuffer) is false)
		{
			RenderMeshes.Dispose(model);
			return false;
		}
	}


	if (mesh->NormalCount isnt 0)
	{
		model->NormalBuffer = SharedHandles.Create();

		if (TryBindBuffer((float*)mesh->NormalVertices, mesh->NormalCount * sizeof(vector3), model->NormalBuffer) is false)
		{
			RenderMeshes.Dispose(model);
			return false;
		}
	}

	model->NumberOfTriangles = mesh->VertexCount;

	model->ShadeSmooth = mesh->SmoothingEnabled;

	model->Mesh = Pointers(mesh).Create(mesh);

	*out_renderMesh = model;

	return true;
}

private void RenderMeshCopyTo(RenderMesh source, RenderMesh destination)
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

	if (source->Name isnt null)
	{
		destination->Name = Pointers(byte).Instance(source->Name);
	}

	if (source->Mesh isnt null)
	{
		destination->Mesh = Pointers(mesh).Instance(source->Mesh);
	}
}

private RenderMesh InstanceMesh(RenderMesh mesh)
{
	RenderMesh result = CreateRenderMesh();

	RenderMeshCopyTo(mesh, result);

	return result;
}

private RenderMesh Duplicate(RenderMesh mesh)
{
	RenderMesh result = InstanceMesh(mesh);

	Transforms.CopyTo(mesh->Transform, result->Transform);

	CopyMember(mesh, result, Id);

	return result;
}

private bool TryBindModel(Model model, RenderMesh** out_meshArray)
{
	RenderMesh* meshesArray = Memory.Alloc(sizeof(RenderMesh) * model->Count, RenderMeshTypeId);

	// all sub-meshes within a model share the same name
	char* sharedName = Strings.DuplicateTerminated(model->Name);

	Pointer(byte) name = Pointers(byte).Create(sharedName);

	for (ulong i = 0; i < model->Count; i++)
	{
		Mesh mesh = model->Meshes[i];

		RenderMesh newMesh;
		if (RenderMeshes.TryBindMesh(mesh, &newMesh) is false)
		{
			// dispose of the extra instance we made for convenience
			Pointers(byte).Dispose(name, null, null);

			// dispose of any children before this index where we failed
			for (int childIndex = 0; childIndex < i; ++childIndex)
			{
				RenderMeshes.Dispose(meshesArray[childIndex]);
			}

			Memory.Free(meshesArray, RenderMeshTypeId);

			return false;
		}

		newMesh->Name = Pointers(byte).Instance(name);

		newMesh->Mesh = Pointers(mesh).Create(mesh);

		meshesArray[i] = newMesh;
	}

	// dispose of the extra instance we made for convenience
	Pointers(byte).Dispose(name, null, null);

	*out_meshArray = meshesArray;

	return true;
}

private void Save(File stream, RenderMesh mesh)
{
	if (mesh isnt null)
	{
		fprintf(stream, "%s", (char*)mesh->Name->Resource);
	}
}