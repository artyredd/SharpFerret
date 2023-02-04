#include "graphics/drawing.h"
#include "graphics/scene.h"
#include "math/triangles.h"
#include "string.h"
#include "singine/time.h"
#include "data/objectPool.h"

static void DrawTriangle(const float* triangle, Material material);
static void SetScene(Scene scene);
static void ClearLastFrame();

const struct _drawing Drawing = 
{
	.SetScene = SetScene,
	.DrawTriangle = &DrawTriangle,
	.ClearLastFrame = ClearLastFrame
};

Scene Global_CurrentDrawingScene;

static float triangleNormal[3];

static float triangleVertices[9];

struct _mesh Global_TriangleMesh = {
	.Name = "Triangle",
	.SmoothingEnabled = false,
	.TextureVertexData = null,
	.TextureCount = 0,
	.NormalCount = 3,
	.NormalVertexData = triangleNormal,
	.VertexCount = 9,
	.VertexData = triangleVertices,
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
//
//static struct _objectPool RenderMeshPool = {
//	.AutoResize = true,
//	.Count = 0,
//	.ObjectProvider = RenderMeshProvider,
//	.ObjectRemover = RenderMeshRemover,
//	.ProviderState = &TriangleMesh,
//	.State = {
//		.Capacity = 0,
//		.Objects = null,
//		.FirstAvailableIndex  = 0
//	}
//};

RenderMesh Global_DrawTriangleRenderMesh = null;

static void DrawTriangle(const float* triangle, Material material)
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
	memcpy(Global_TriangleMesh.VertexData, triangle, 9 * sizeof(float));

	// calculate the normal for the triangle
	Triangles.CalculateNormal((vec3*)triangle, Global_TriangleMesh.NormalVertexData);
	
	//RenderMesh renderMesh = ObjectPools.Get(&RenderMeshPool);

	Materials.Draw(material, Global_DrawTriangleRenderMesh, Global_CurrentDrawingScene);
}

static void SetScene(Scene scene)
{
	Global_CurrentDrawingScene = scene;
}

static void ClearLastFrame()
{
	// dispose all the rendermeshes from last frame
	//ObjectPools.ReleaseAll(&RenderMeshPool);
}