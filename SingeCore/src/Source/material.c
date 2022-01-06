#include "graphics/material.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "singine/guards.h"
#include "helpers/macros.h"
#include "helpers/quickmask.h"
#include "cglm/mat4.h"
#include "math/vectors.h"

static void Dispose(Material material);
static Material Create(const Shader shader, const Texture texture);
static void Draw(Material, RenderMesh, Camera);
static Material CreateMaterial(void);
static Material InstanceMaterial(const Material);
static void SetMainTexture(Material, Texture);
static void SetShader(Material, const Shader);
static void DisableSetting(Material, const MaterialSetting);
static void EnableSetting(Material, const MaterialSetting);
static void SetSetting(Material, const MaterialSetting, const bool enabled);
static bool HasSetting(Material, const MaterialSetting);
static void SetColor(Material, const Color);
static void SetColors(Material, const float r, const float g, const float b, const float a);

const struct _materialMethods Materials = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Draw = &Draw,
	.Instance = &InstanceMaterial,
	.SetMainTexture = &SetMainTexture,
	.SetShader = &SetShader,
	.HasSetting = &HasSetting,
	.SetSetting = &SetSetting,
	.EnableSetting = &EnableSetting,
	.DisableSetting = &DisableSetting,
	.SetColor = &SetColor,
	.SetColors = &SetColors
};


#define UseCameraPerspectiveFlag FLAG_0
#define UseCullingFlag FLAG_1
#define UseTransparencyFlag FLAG_2

const struct _materialSettings MaterialSettings = {
	.UseCameraPerspective = UseCameraPerspectiveFlag,
	.BackfaceCulling = UseCullingFlag,
	.Transparency = UseTransparencyFlag
};

#define DEFAULT_MATERIAL_SETTINGS (UseCameraPerspectiveFlag | UseCullingFlag)

static void Dispose(Material material)
{
	if (material is null)
	{
		return;
	}

	Shaders.Dispose(material->Shader);

	Textures.Dispose(material->MainTexture);

	SafeFree(material);
}

static Material Create(Shader shader, Texture texture)
{
	Material material = SafeAlloc(sizeof(struct _material));

	material->MainTexture = Textures.Instance(texture);
	material->Shader = Shaders.Instance(shader);
	material->State.Settings = DEFAULT_MATERIAL_SETTINGS;

	SetVector4(material->Color, 1, 1, 1, 1);

	return material;
}

static Material CreateMaterial()
{
	return Create(null, null);
}

static Material InstanceMaterial(Material material)
{
	if (material is null)
	{
		return null;
	}

	Shader shader = Shaders.Instance(material->Shader);
	Texture mainTexture = Textures.Instance(material->MainTexture);

	Material newMaterial = Create(null, null);

	newMaterial->Shader = shader;
	newMaterial->MainTexture = mainTexture;

	Vectors3CopyTo(material->Color, newMaterial->Color);
	CopyMember(material, newMaterial, State.Settings);

	return newMaterial;
}

static void PerformDraw(Material material, RenderMesh mesh, mat4 MVPMatrix)
{
	if (material->Shader isnt null)
	{
		Shader shader = material->Shader;

		// perform any shader setup if we need to
		if (shader->BeforeDraw isnt null)
		{
			shader->BeforeDraw(shader, MVPMatrix);
		}

		int colorHandle;
		if (Shaders.TryGetUniform(material->Shader, Uniforms.Color, &colorHandle))
		{
			glUniform4fv(colorHandle, 1, material->Color);
		}

		// check to see if we need to load a texture into the maintexture
		if (material->MainTexture isnt null)
		{
			int textureHandle;
			if (Shaders.TryGetUniform(material->Shader, Uniforms.Texture0, &textureHandle))
			{
				glEnable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, material->MainTexture->Handle->Handle);
				glUniform1i(textureHandle, 0);
				glDisable(GL_TEXTURE_2D);
			}
		}

		// draw the triangles
		RenderMeshes.Draw(mesh);

		// perform shader cleanup if needed
		if (shader->AfterDraw isnt null)
		{
			shader->AfterDraw(shader);
		}
	}
}

static void Draw(Material material, RenderMesh mesh, Camera camera)
{
	unsigned int settings = material->State.Settings;

	// check if we should use the camera's perspective
	vec4* MVP;
	if (HasFlag(settings, MaterialSettings.UseCameraPerspective))
	{
		vec4* modelMatrix = Transforms.Refresh(mesh->Transform);

		vec4* cameraVP = Cameras.Refresh(camera);

		mat4 mvp;
		glm_mat4_mul(cameraVP, modelMatrix, mvp);

		MVP = mvp;
	}
	else
	{
		MVP = Transforms.Refresh(mesh->Transform);
	}

	// check to see if we need to DISABLE back face culling(by default it's on)
	bool shouldDisableCulling = HasFlag(settings, MaterialSettings.BackfaceCulling) is false;

	if (shouldDisableCulling)
	{
		glDisable(GL_CULL_FACE);
	}

	// check to see if we should enable transparency
	bool shouldEnableTransparency = HasFlag(settings, MaterialSettings.Transparency);

	if (shouldEnableTransparency)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// actually draw the mesh
	PerformDraw(material, mesh, MVP);

	// re-enable culling
	if (shouldDisableCulling)
	{
		glEnable(GL_CULL_FACE);
	}

	if (shouldEnableTransparency)
	{
		glDisable(GL_BLEND);
	}
}


static void SetMainTexture(Material material, Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->MainTexture);
	material->MainTexture = Textures.Instance(texture);
}

static void SetShader(Material material, Shader shader)
{
	GuardNotNull(material);
	Shaders.Dispose(material->Shader);
	material->Shader = Shaders.Instance(shader);
}

static void DisableSetting(Material material, MaterialSetting setting)
{
	GuardNotNull(material);

	ClearFlag(material->State.Settings, setting);
}

static void EnableSetting(Material material, MaterialSetting setting)
{
	GuardNotNull(material);

	SetFlag(material->State.Settings, setting);
}

static void SetSetting(Material material, MaterialSetting setting, bool enabled)
{
	GuardNotNull(material);

	AssignFlag(material->State.Settings, setting, enabled);
}

static bool HasSetting(Material material, MaterialSetting setting)
{
	GuardNotNull(material);

	return HasFlag(material->State.Settings, setting);
}

static void SetColor(Material material, const Color color)
{
	GuardNotNull(material);
	Vectors4CopyTo(color, material->Color);
}

static void SetColors(Material material, const float r, const float g, const float b, const float a)
{
	GuardNotNull(material);
	SetVector4(material->Color, r, g, b, a);
}