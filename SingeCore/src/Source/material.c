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
#include "singine/config.h"
#include "graphics/graphicsDevice.h"
#include "singine/parsing.h"
#include "graphics/shadercompiler.h"
#include "singine/strings.h"

static void Dispose(Material material);
static Material Create(const Shader shader, const Texture texture);
static void Draw(Material, RenderMesh, Camera);
static Material CreateMaterial(void);
static Material InstanceMaterial(const Material);
static void SetMainTexture(Material, Texture);
static void SetShader(Material, const Shader, size_t index);
static void SetColor(Material, const Color);
static void SetColors(Material, const float r, const float g, const float b, const float a);
static Material Load(const char* path);
static bool Save(const Material material, const char* path);

const struct _materialMethods Materials = {
	.Dispose = &Dispose,
	.Create = &Create,
	.Load = &Load,
	.Save = &Save,
	.Draw = &Draw,
	.Instance = &InstanceMaterial,
	.SetMainTexture = &SetMainTexture,
	.SetShader = &SetShader,
	.SetColor = &SetColor,
	.SetColors = &SetColors
};

#define DEFAULT_MATERIAL_SETTINGS (ShaderSettings.UseCameraPerspective | ShaderSettings.BackfaceCulling)

#define MATERIAL_LOADER_BUFFER_SIZE 1024

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
		size_t previousSize = material->Count * sizeof(Shader);
		size_t newSize = desiredCount * sizeof(Shader);

		if (TryRealloc(material->Shaders, previousSize, newSize, (void**)&material->Shaders) is false)
		{
			Shader* newArray = SafeAlloc(newSize);

			CopyShadersTo(material->Shaders, material->Count, newArray, desiredCount);

			SafeFree(material->Shaders);
			material->Shaders = newArray;
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

	return newMaterial;
}

static void PrepareSettings(unsigned int settings, Shader shader, mat4 modelMatrix, mat4 mvpMatrix)
{
	int handle;
	if (Shaders.TryGetUniform(shader, Uniforms.MVP, &handle))
	{
		if (HasFlag(settings, ShaderSettings.UseCameraPerspective))
		{
			glUniformMatrix4fv(handle, 1, false, &mvpMatrix[0][0]);
		}
		else
		{
			glUniformMatrix4fv(handle, 1, false, &modelMatrix[0][0]);
		}
	}

	// check to see if we need to DISABLE back face culling(by default it's on)
	if (HasFlag(settings, ShaderSettings.BackfaceCulling))
	{
		GraphicsDevice.EnableCulling();
	}
	else
	{
		GraphicsDevice.DisableCulling();
	}

	// check to see if we should enable transparency
	if (HasFlag(settings, ShaderSettings.Transparency))
	{
		GraphicsDevice.EnableBlending();
	}
	else
	{
		GraphicsDevice.DisableBlending();
	}
}

static void PerformDraw(Material material, RenderMesh mesh, mat4 modelMatrix, mat4 MVPMatrix)
{
	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];

		if (shader isnt null)
		{
			Shaders.Enable(shader);

			PrepareSettings(shader->Settings, shader, modelMatrix, MVPMatrix);

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

			Shaders.Disable(shader);
		}
	}
}

static void Draw(Material material, RenderMesh mesh, Camera camera)
{
	// check if we should use the camera's perspective
	vec4* modelMatrix = Transforms.Refresh(mesh->Transform);

	vec4* cameraVP = Cameras.Refresh(camera);

	mat4 mvp;
	glm_mat4_mul(cameraVP, modelMatrix, mvp);

	PerformDraw(material, mesh, modelMatrix, mvp);
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

#define TokenCount 3

#define ExportTokenFormat "%s: %s\n"
#define ExportCommentFormat "%s\n"

#define ShaderTokenComment "# the array of shaders that should be loaded for this material"
#define ShaderToken "shaders"
#define ColorTokenComment "# the material base color"
#define ColorToken "color"
#define MainTextureComment "# the main UV texture that should be used for this material"
#define MainTextureToken "mainTexture"

#define MAX_PATH_LENGTH 512

struct _materialDefinition
{
	char** ShaderPaths;
	size_t* ShaderPathLengths;
	size_t ShaderCount;
	Color Color;
	char* MainTexturePath;
};

static const char* Tokens[TokenCount] = {
	ShaderToken,
	ColorToken,
	MainTextureToken
};

static const size_t TokenLengths[TokenCount] = {
	sizeof(ShaderToken),
	sizeof(ColorToken),
	sizeof(MainTextureToken)
};

static bool OnTokenFound(size_t index, const char* buffer, const size_t length, struct _materialDefinition* state)
{
	switch (index)
	{
	case 0: // shader token
		return TryParseStringArray(buffer, length, &state->ShaderPaths, &state->ShaderPathLengths, &state->ShaderCount);
	case 1: // color
		return Vector4s.TryDeserialize(buffer, length, state->Color);
	case 2: //  main texture
		return TryParseString(buffer, length, MAX_PATH_LENGTH, &state->MainTexturePath);
	default:
		return false;
	}
}

const struct _configDefinition MaterialConfigDefinition = {
	.Tokens = (const char**)&Tokens,
	.TokenLengths = (const size_t*)&TokenLengths,
	.CommentCharacter = '#',
	.Count = sizeof(Tokens) / sizeof(&Tokens), // tokens is an array of pointers to the total bytes/sizeof pointer is the count
	.OnTokenFound = &OnTokenFound
};

static Material Load(const char* path)
{
	// create an empty material
	Material material = null;

	struct _materialDefinition state = {
		.Color = {1, 1, 1, 1},
		.ShaderCount = 0,
		.ShaderPathLengths = null,
		.ShaderPaths = null,
		.MainTexturePath = null
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&MaterialConfigDefinition, &state))
	{
		// no point of having a material with no shader
		if (state.ShaderCount isnt 0)
		{
			material = CreateMaterial();

			Vectors4CopyTo(state.Color, material->Color);

			material->Shaders = SafeAlloc(sizeof(Shader) * state.ShaderCount);
			material->Count = state.ShaderCount;

			// import each shader
			for (size_t i = 0; i < state.ShaderCount; i++)
			{
				char* shaderPath = state.ShaderPaths[i];
				size_t shaderPathLength = state.ShaderPathLengths[i];

				// trim path
				size_t newLength = Strings.Trim(shaderPath, shaderPathLength);

				// try not to load empty paths
				// this is a valid path array for example ",,,,,,,,,,,,,,,,,,,"
				if (newLength isnt 0)
				{
					Shader shader = ShaderCompilers.Load(shaderPath);

					material->Shaders[i] = shader;
				}
			}

			// load any textures that were included in the material
			if (state.MainTexturePath isnt null)
			{
				Image image;
				if (Images.TryLoadImage(state.MainTexturePath, &image))
				{
					Texture texture;
					if (Textures.TryCreateTexture(image, &texture))
					{
						Materials.SetMainTexture(material, texture);
					}

					Textures.Dispose(texture);
				}

				Images.Dispose(image);
			}
		}
	}

	if (material is null)
	{
		fprintf(stderr, "Failed to load the material from path: %s", path);
	}

	SafeFree(state.ShaderPathLengths);

	for (size_t i = 0; i < state.ShaderCount; i++)
	{
		SafeFree(state.ShaderPaths[i]);
	}

	SafeFree(state.ShaderPaths);

	return material;
}

static bool Save(const Material material, const char* path)
{
	GuardNotNull(material);
	GuardNotNull(path);

	File file;
	if (Files.TryOpen(path, FileModes.Create, &file) is false)
	{
		return false;
	}

	fprintf(file, ExportCommentFormat, ShaderTokenComment);
	fprintf(file, "%s: ", ShaderToken);

	// print string array
	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];

		if (shader isnt null)
		{
			fprintf(file, "%s,", shader->Name);
		}
	}

	fprintf(file, "\n");

	fprintf(file, ExportCommentFormat, ColorTokenComment);
	fprintf(file, "%s: %f %f %f\n", ColorToken, material->Color[0], material->Color[1], material->Color[2]);

	if (material->MainTexture isnt null)
	{
		fprintf(file, ExportCommentFormat, MainTextureComment);
		fprintf(file, ExportTokenFormat, MainTextureToken, material->MainTexture->Path);
	}

	return Files.TryClose(file);
}