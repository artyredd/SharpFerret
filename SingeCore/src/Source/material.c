#include "graphics/material.h"
#include "singine/memory.h"
#include "GL/glew.h"
#include "singine/guards.h"
#include "helpers/macros.h"
#include "helpers/quickmask.h"
#include "cglm/mat4.h"
#include "math/vectors.h"
#include "singine/file.h"
#include "string.h"

static void Dispose(Material material);
static Material Create(const Shader shader, const Texture texture);
static void Draw(Material, RenderMesh, Camera);
static Material CreateMaterial(void);
static Material InstanceMaterial(const Material);
static void SetMainTexture(Material, Texture);
static void SetShader(Material, const Shader, size_t index);
static void DisableSetting(Material, const ShaderSetting);
static void EnableSetting(Material, const ShaderSetting);
static void SetSetting(Material, const ShaderSetting, const bool enabled);
static bool HasSetting(Material, const ShaderSetting);
static void SetColor(Material, const Color);
static void SetColors(Material, const float r, const float g, const float b, const float a);
static Material Load(const char* path);

const struct _materialMethods Materials = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Load = &Load,
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

#define DEFAULT_MATERIAL_SETTINGS (ShaderSettings.UseCameraPerspective | ShaderSettings.BackfaceCulling)

#define MATERIAL_LOADER_BUFFER_SIZE 1024

typedef const char* Token;

static const struct _materialTokens {
	Token Shaders;
	Token Color;
	Token UseBackfaceCulling;
	Token UseCameraPerspective;
	Token EnableTransparency;
	int Comment;
} Tokens = {
	.Shaders = "shaders",
	.Color = "color",
	.UseBackfaceCulling = "useBackfaceCulling",
	.UseCameraPerspective = "useCameraPerspective",
	.EnableTransparency = "enableTransparency",
	.Comment = '#'
};

static void Dispose(Material material)
{
	if (material is null)
	{
		return;
	}

	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];
		Shaders.Dispose(shader);
	}

	SafeFree(material->Shaders);

	Textures.Dispose(material->MainTexture);

	SafeFree(material);
}

static Material Create(Shader shader, Texture texture)
{
	Material material = SafeAlloc(sizeof(struct _material));

	material->MainTexture = Textures.Instance(texture);
	material->Shaders = SafeAlloc(sizeof(Shader));
	material->Shaders[0] = Shaders.Instance(shader);
	material->Count = 1;
	material->State.Settings = DEFAULT_MATERIAL_SETTINGS;

	SetVector4(material->Color, 1, 1, 1, 1);

	return material;
}

static Material CreateMaterial()
{
	return Create(null, null);
}

static void CopyShadersTo(Shader* source, size_t sourceSize, Shader* destination, size_t destinationSize)
{
	for (size_t i = 0; i < min(sourceSize, destinationSize); i++)
	{
		destination[i] = source[i];
	}
}

static void ResizeShaders(Material material, size_t desiredCount)
{
	if (material->Count < desiredCount)
	{
		size_t newSize = desiredCount * sizeof(Shader);

		if (TryRealloc(material->Shaders, newSize, (void**)&material->Shaders) is false)
		{
			Shader* newArray = SafeAlloc(newSize);

			CopyShadersTo(material->Shaders, material->Count, newArray, desiredCount);

			SafeFree(material->Shaders);
			material->Shaders = newArray;
		}
		else
		{
			// if we realloced non-zero bytes are at the end of the array
			// set them to 0
			for (size_t i = material->Count; i < desiredCount; i++)
			{
				material->Shaders[i] = null;
			}
		}

		material->Count = desiredCount;
	}
}

static void InstanceShadersTo(Material source, Material destination)
{
	// make sure to dispose of any old ones if they exist
	if (destination->Shaders isnt null)
	{
		for (size_t i = 0; i < destination->Count; i++)
		{
			Shader shader = destination->Shaders[i];
			Shaders.Dispose(shader);
		}
	}

	// check to see if the array is big enough
	ResizeShaders(destination, source->Count);

	if (destination->Shaders is null)
	{
		throw(UnexpectedOutcomeException);
	}

	// instance the shaders to the destination array
	for (size_t i = 0; i < source->Count; i++)
	{
		destination->Shaders[i] = Shaders.Instance(source->Shaders[i]);
	}
}

static Material InstanceMaterial(Material material)
{
	if (material is null)
	{
		return null;
	}

	Texture mainTexture = Textures.Instance(material->MainTexture);

	Material newMaterial = Create(null, null);

	InstanceShadersTo(material, newMaterial);

	newMaterial->MainTexture = mainTexture;

	Vectors3CopyTo(material->Color, newMaterial->Color);
	CopyMember(material, newMaterial, State.Settings);

	return newMaterial;
}

static void PrepareSettings(unsigned int settings)
{
	// check to see if we need to DISABLE back face culling(by default it's on)
	if (HasFlag(settings, ShaderSettings.BackfaceCulling) is false)
	{
		glDisable(GL_CULL_FACE);
	}

	// check to see if we should enable transparency
	if (HasFlag(settings, ShaderSettings.Transparency))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

static void CleanupSettings(unsigned int settings)
{
	// re-enable culling
	if (HasFlag(settings, ShaderSettings.BackfaceCulling) is false)
	{
		glEnable(GL_CULL_FACE);
	}

	if (HasFlag(settings, ShaderSettings.Transparency))
	{
		glDisable(GL_BLEND);
	}
}

static void PerformDraw(Material material, RenderMesh mesh, mat4 MVPMatrix)
{
	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];

		if (shader isnt null)
		{
			PrepareSettings(shader->Settings);

			// perform any shader setup if we need to
			if (shader->BeforeDraw isnt null)
			{
				shader->BeforeDraw(shader, MVPMatrix);
			}

			int colorHandle;
			if (Shaders.TryGetUniform(shader, Uniforms.Color, &colorHandle))
			{
				glUniform4fv(colorHandle, 1, material->Color);
			}

			// check to see if we need to load a texture into the maintexture
			if (material->MainTexture isnt null)
			{
				int textureHandle;
				if (Shaders.TryGetUniform(shader, Uniforms.Texture0, &textureHandle))
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

			CleanupSettings(shader->Settings);
		}
	}
}

static void Draw(Material material, RenderMesh mesh, Camera camera)
{
	unsigned int settings = material->State.Settings;

	// check if we should use the camera's perspective
	vec4* MVP;
	if (HasFlag(settings, ShaderSettings.UseCameraPerspective))
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

	PrepareSettings(settings);

	// actually draw the mesh
	PerformDraw(material, mesh, MVP);

	CleanupSettings(settings);
}

static void SetMainTexture(Material material, Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->MainTexture);
	material->MainTexture = Textures.Instance(texture);
}

static void SetShader(Material material, Shader shader, size_t index)
{
	GuardNotNull(material);

	ResizeShaders(material, index + 1);

	Shaders.Dispose(material->Shaders[index]);

	material->Shaders[index] = Shaders.Instance(shader);
}

static void DisableSetting(Material material, ShaderSetting setting)
{
	GuardNotNull(material);

	ClearFlag(material->State.Settings, setting);
}

static void EnableSetting(Material material, ShaderSetting setting)
{
	GuardNotNull(material);

	SetFlag(material->State.Settings, setting);
}

static void SetSetting(Material material, ShaderSetting setting, bool enabled)
{
	GuardNotNull(material);

	AssignFlag(material->State.Settings, setting, enabled);
}

static bool HasSetting(Material material, ShaderSetting setting)
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

static Material Load(const char* path)
{
	File file;
	if (Files.TryOpen(path, FileModes.ReadBinary, &file) is false)
	{
		fprintf(stderr, "Failed to load material definitin at path: %s", path);
		return null;
	}

	// create an empty material
	Material material = CreateMaterial();

	char buffer[MATERIAL_LOADER_BUFFER_SIZE];
	
	size_t bufferLength = MATERIAL_LOADER_BUFFER_SIZE;

	size_t lineLength;
	while (Files.TryReadLine(file, buffer, 0, bufferLength, &lineLength))
	{
		// ignore comments
		if (buffer[0] is Tokens.Comment)
		{
			continue;
		}

		// load any shaders
		if (memcmp(buffer, Tokens.Shaders, min(strlen(Tokens.Shaders), bufferLength)) is 0)
		{
			// load the shaders into the material
		}

		// try to load the color
		if (memcmp(buffer, Tokens.Color, min(strlen(Tokens.Color), bufferLength)) is 0)
		{
			const char* offset = buffer + strlen(Tokens.Color) + 1;

			if (Vector4s.TryDeserialize(offset, bufferLength, material->Color) is false)
			{
				fprintf(stderr, "Failed to deserialize color for material: %s", path);
			}
		}
	}

	return material;
}