#include "graphics/drawing.h"
#include "graphics/scene.h"
#include "math/triangles.h"
#include "string.h"
#include "singine/time.h"
#include "data/objectPool.h"

static void DrawTriangle(triangle triangle, Material material);
static void SetScene(Scene scene);

const struct _drawing Drawing = 
{
	.SetScene = SetScene,
	.DrawTriangle = &DrawTriangle
};

Scene Global_CurrentDrawingScene;

static vector3 triangleNormal;

static triangle triangleVertices;

struct _mesh Global_TriangleMesh = {
	.Name = "Triangle",
	.SmoothingEnabled = false,
	.TextureVertices = null,
	.TextureCount = 0,
	.NormalCount = 3,
	.NormalVertices = &triangleNormal,
	.VertexCount = 9,
	.Vertices = (vector3*) & triangleVertices,
};

static RenderMesh RenderMeshProvider(Mesh mesh)
{
	RenderMesh result;
	if (RenderMeshes.TryBindMesh(mesh, &result) is false)
	{
		throw(FailedToBindMeshException);
	}

	return result;
}

static void RenderMeshRemover(RenderMesh mesh)
{
	RenderMeshes.Dispose(mesh);
}

RenderMesh Global_DrawTriangleRenderMesh = null;

static void DrawTriangle(triangle triangle, Material material)
{
	if (Global_CurrentDrawingScene is null)
	{
		throw(FailedToSetSceneException);
	}

	if (Global_DrawTriangleRenderMesh is null)
	{
		if (RenderMeshes.TryBindMesh(&Global_TriangleMesh, &Global_DrawTriangleRenderMesh) is false)
		{
			throw(FailedToBindMeshException);
		}

		Global_DrawTriangleRenderMesh->CopyBuffersOnDraw = true;
	}

	// copy over the vertices
	memcpy(Global_TriangleMesh.Vertices, &triangle, 9 * sizeof(float));

	// calculate the normal for the triangle
	*(struct vector3*)Global_TriangleMesh.NormalVertices = Triangles.CalculateNormal(triangle);
	
	Materials.Draw(material, Global_DrawTriangleRenderMesh, Global_CurrentDrawingScene);
}

static void SetScene(Scene scene)
{
	Global_CurrentDrawingScene = scene;
}