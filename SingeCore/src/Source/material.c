#include "graphics/material.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "singine/guards.h"

static void Dispose(Material material);
static Material Create(Shader shader, Texture texture);
static void Draw(Material material);
static Material CreateMaterial(void);
static Material InstanceMaterial(Material);
static void SetMainTexture(Material, Texture);
static void SetShader(Material, Shader);

const struct _materialMethods Materials = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Draw = &Draw,
	.Instance = &InstanceMaterial,
	.SetMainTexture = &SetMainTexture,
	.SetShader = &SetShader
};

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

	SetVector3(material->Color, 0, 0, 0);

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

	return newMaterial;
}

static void Draw(Material material)
{
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