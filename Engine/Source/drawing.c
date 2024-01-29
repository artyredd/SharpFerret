#include "engine/graphics/drawing.h"
#include "engine/graphics/scene.h"
#include "core/math/triangles.h"
#include "string.h"
#include "engine/time.h"
#include "engine/data/objectPool.h"

private void DrawTriangle(triangle triangle, Material material);
private void SetScene(Scene scene);
private void SetDefaultMaterial(Material);
private void DrawDefaultTriangle(triangle);
private void DrawCubeFromPoints(vector3, vector3);

const struct _drawing Drawing = 
{
	.SetScene = SetScene,
	.SetDefaultMaterial = SetDefaultMaterial,
	.DrawDefaultTriangle = DrawDefaultTriangle,
	.DrawTriangle = &DrawTriangle,
	.DrawCubeFromPoints = DrawCubeFromPoints
};

Scene Global_CurrentDrawingScene;
Material Global_DefaultDrawingMaterial;

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

private void SetDefaultMaterial(Material material)
{
	Global_DefaultDrawingMaterial = Materials.Instance(material);
}

private void DrawDefaultTriangle(triangle triangle)
{
	DrawTriangle(triangle, Global_DefaultDrawingMaterial);
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

private void DrawRectangle(vector3 point1, vector3 point2, vector3 point3, vector3 point4, Material material)
{
	DrawTriangle((triangle) { point1, point2, point3}, material);
	DrawTriangle((triangle) { point1, point4, point3 }, material);
}

private void DrawBottomPlane(vector3 lowerCorner, vector3 upperCorner, Material material)
{
	upperCorner.y = lowerCorner.y;

	vector3 point1 = lowerCorner;
	vector3 point2 = lowerCorner;
	point2.x += upperCorner.x;
	point2.z += upperCorner.z;

	vector3 point3 = upperCorner;
	vector3 point4 = lowerCorner;
	point4.x -= upperCorner.x;
	point4.z -= upperCorner.z;

	DrawRectangle(point1, point2, point3, point4,material);
}

private void DrawCubeFromPoints(vector3 lowerCorner, vector3 upperCorner)
{
	DrawBottomPlane(lowerCorner,upperCorner, Global_DefaultDrawingMaterial);
}