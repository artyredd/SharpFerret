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
#include "graphics/scene.h"
#include "math/floats.h"

static void Dispose(Material material);
static Material Create(const Shader shader, const Texture texture);
static void Draw(Material material, RenderMesh mesh, Scene scene);
static Material CreateMaterial(void);
static Material InstanceMaterial(const Material);
static void SetMainTexture(Material, Texture);
static void SetShader(Material, const Shader, size_t index);
static void SetColor(Material, const Color);
static void SetColors(Material, const float r, const float g, const float b, const float a);
static Material Load(const char* path);
static bool Save(const Material material, const char* path);
static void SetName(Material, const char* name);
static void SetSpecularTexture(Material material, Texture texture);

const struct _materialMethods Materials = {
	.Dispose = &Dispose,
	.Create = &Create,
	.CreateMaterial = &CreateMaterial,
	.Load = &Load,
	.Save = &Save,
	.Draw = &Draw,
	.Instance = &InstanceMaterial,
	.SetMainTexture = &SetMainTexture,
	.SetShader = &SetShader,
	.SetColor = &SetColor,
	.SetColors = &SetColors,
	.SetName = &SetName,
	.SetSpecularTexture = &SetSpecularTexture
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

	SafeFree(material->Name);

	Textures.Dispose(material->MainTexture);

	SafeFree(material);
}

static Material Create(Shader shader, Texture texture)
{
	Material material = SafeAlloc(sizeof(struct _material));

	material->MainTexture = Textures.Instance(texture);

	if (shader isnt null)
	{
		material->Shaders = SafeAlloc(sizeof(Shader));
		material->Shaders[0] = Shaders.Instance(shader);
		material->Count = 1;
	}

	SetVector4(material->Color, 1, 1, 1, 1);
	SetVector4(material->AmbientColor, 1, 1, 1, 1);
	SetVector4(material->DiffuseColor, 1, 1, 1, 1);
	SetVector4(material->SpecularColor, 1, 1, 1, 1);

	material->Shininess = 0.5f;

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
	// if there are no shaders to instance return
	if (source->Shaders is null or source->Count is 0) { return; }

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

	newMaterial->SpecularTexture = Textures.Instance(material->SpecularTexture);

	if (material->Name isnt null)
	{
		newMaterial->Name = Strings.DuplicateTerminated(material->Name);
	}

	Vectors4CopyTo(material->Color, newMaterial->Color);
	Vectors4CopyTo(material->DiffuseColor, newMaterial->DiffuseColor);
	Vectors4CopyTo(material->SpecularColor, newMaterial->SpecularColor);
	Vectors4CopyTo(material->AmbientColor, newMaterial->AmbientColor);

	newMaterial->Shininess = material->Shininess;

	return newMaterial;
}

// Enables or disables graphics and matrix settings based on what settings are provided by the shader
static void PrepareSettings(Shader shader, mat4 modelMatrix, mat4 mvpMatrix)
{
	unsigned int settings = shader->Settings;

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

	if (HasFlag(settings, ShaderSettings.UseStencilBuffer))
	{
		if (HasFlag(settings, ShaderSettings.CustomStencilAttributes))
		{
			GraphicsDevice.SetStencilFull(shader->StencilFunction, shader->StencilValue, shader->StencilMask);
		}
		else
		{
			GraphicsDevice.ResetStencilFunction();
		}

		// should this object's fragments write values to the stencil buffer?
		if (HasFlag(settings, ShaderSettings.WriteToStencilBuffer))
		{
			GraphicsDevice.EnableStencilWriting();
		}
		else
		{
			GraphicsDevice.DisableStencilWriting();
		}
	}
	else
	{
		GraphicsDevice.ResetStencilFunction();
	}

	// should this object's fragments use the depth test to determine if they appear over/under other fragments?
	if (HasFlag(settings, ShaderSettings.UseDepthTest))
	{
		GraphicsDevice.EnableDepthTesting();
	}
	else
	{
		GraphicsDevice.DisableDepthTesting();
	}
}

static void SetUniformVector3(Shader shader, Uniform uniform, vec3 vector3)
{
	int handle;
	if (Shaders.TryGetUniform(shader, uniform, &handle))
	{
		glUniform3fv(handle, 1, vector3);
	}
}

static void SetUniformVector4(Shader shader, Uniform uniform, vec4 vector4)
{
	int handle;
	if (Shaders.TryGetUniform(shader, uniform, &handle))
	{
		glUniform4fv(handle, 1, vector4);
	}
}

static void SetUniformFloat(Shader shader, Uniform uniform, float value)
{
	int handle;
	if (Shaders.TryGetUniform(shader, uniform, &handle))
	{
		glUniform1f(handle, value);
	}
}

static void SetTextureUniform(Shader shader, Uniform uniform, Texture texture)
{
	if (texture isnt null)
	{
		int textureHandle;
		if (Shaders.TryGetUniform(shader, uniform, &textureHandle))
		{
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->Handle->Handle);
			glUniform1i(textureHandle, 0);
			glDisable(GL_TEXTURE_2D);
		}
	}
}

static void SetUniformMatrix4(Shader shader, Uniform uniform, mat4 matrix4)
{
	int handle;
	if (Shaders.TryGetUniform(shader, uniform, &handle))
	{
		glUniformMatrix4fv(handle, 1, false, &matrix4[0][0]);
	}
}

static bool TrySetLightUniforms(Shader shader, Light light, size_t index)
{
	/*
		int lightType;
		vec4 color;
		float intensity;
		float range;
		float radius;
		vec3 position;
	*/
	int handle;
	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "lightType", &handle))
	{
		glUniform1i(handle, light->Type);
	}
	

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "ambient", &handle))
	{
		glUniform4fv(handle, 1, light->Ambient);
	}
	

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "diffuse", &handle))
	{
		glUniform4fv(handle, 1, light->Diffuse);
	}
	

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "specular", &handle))
	{
		glUniform4fv(handle, 1, light->Specular);
	}
	

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "range", &handle))
	{
		glUniform1f(handle, light->Range);
	}

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "radius", &handle))
	{
		glUniform1f(handle, light->Radius);
	}

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, "position", &handle))
	{
		glUniform3fv(handle, 1, light->Transform->Position);
	}
	

	return true;
}

static void SetLightUniforms(Shader shader, Scene scene)
{
	int countHandle;
	if (Shaders.TryGetUniform(shader, Uniforms.LightCount, &countHandle) is false)
	{
		return;
	}

	// set the number of lughts that are going to be within the array
	glUniform1i(countHandle, (GLint)scene->LightCount);

	// set each element of the array
	for (size_t i = 0; i < scene->LightCount; i++)
	{
		Light light = scene->Lights[i];

		if (TrySetLightUniforms(shader, light, i) is false)
		{
			fprintf(stderr, "Failed to set a light uniform for light at index: %lli"NEWLINE, i);
			return;
		}
	}
}

static void PerformDraw(Material material, Scene scene, RenderMesh mesh, mat4 modelMatrix, mat4 MVPMatrix)
{
	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];

		if (shader isnt null)
		{
			Shaders.Enable(shader);

			PrepareSettings(shader, modelMatrix, MVPMatrix);

			SetLightUniforms(shader, scene);

			SetUniformMatrix4(shader, Uniforms.ModelMatrix, modelMatrix);

			SetUniformMatrix4(shader, Uniforms.ViewMatrix, scene->MainCamera->State.View);

			SetUniformMatrix4(shader, Uniforms.ProjectionMatrix, scene->MainCamera->State.Projection);

			SetUniformVector3(shader, Uniforms.CameraPosition, scene->MainCamera->Transform->Position);

			SetUniformVector4(shader, Uniforms.Color, material->Color);

			SetUniformFloat(shader, Uniforms.Shininess, material->Shininess);

			SetUniformVector4(shader, Uniforms.Specular, material->SpecularColor);

			SetUniformVector4(shader, Uniforms.Diffuse, material->DiffuseColor);

			SetUniformVector4(shader, Uniforms.Ambient, material->AmbientColor);

			SetTextureUniform(shader, Uniforms.Texture0, material->MainTexture);

			SetTextureUniform(shader, Uniforms.SpecularMap, material->SpecularTexture);

			// draw the triangles
			RenderMeshes.Draw(mesh);

			Shaders.Disable(shader);
		}
	}
}

static void Draw(Material material, RenderMesh mesh, Scene scene)
{
	// check if we should use the camera's perspective
	vec4* modelMatrix = Transforms.Refresh(mesh->Transform);

	vec4* cameraVP = Cameras.Refresh(scene->MainCamera);

	mat4 mvp;
	glm_mat4_mul(cameraVP, modelMatrix, mvp);

	PerformDraw(material, scene, mesh, modelMatrix, mvp);
}

static void SetMainTexture(Material material, Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->MainTexture);
	material->MainTexture = Textures.Instance(texture);
}

static void SetSpecularTexture(Material material, Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->SpecularTexture);
	material->SpecularTexture = Textures.Instance(texture);
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

static void SetName(Material material, const char* name)
{
	GuardNotNull(material);
	if (material->Name isnt null)
	{
		SafeFree(material->Name);
	}

	material->Name = Strings.DuplicateTerminated(name);
}

#define ExportTokenFormat "%s: %s\n"
#define ExportCommentFormat "%s\n"

#define ShaderTokenComment "# path array delimited by ','; the array of shaders that should be loaded for this material"
#define ShaderToken "shaders"
#define ColorTokenComment "# vec4; the material base color"
#define ColorToken "color"
#define MainTextureComment "# path; the main UV texture that should be used for this material"
#define MainTextureToken "mainTexture"
#define SpecularTextureComment "# path; the texture that should be used to determine what parts of this material are shiny"
#define SpecularTextureToken "specularMap"
#define SpecularColorComment "# vec4; the color that specular highlights should be"
#define SpecularColorToken "specular"
#define AmbientColorComment "# vec4; the ambient color this object should be when exposed to no light"
#define AmbientColorToken "ambient"
#define ShininessComment "# float [0-255]; How shiny the material should be"
#define ShininessToken "shininess"
#define DiffuseColorComment "# vec4; the diffuse color of the material"
#define DiffuseColorToken "diffuse"
#define SpecularStrengthComment "# float [0-1]; how strong the specular highlights should be"
#define SpecularStrengthToken "specularStrength"

#define MAX_PATH_LENGTH 512

struct _materialDefinition
{
	char** ShaderPaths;
	size_t* ShaderPathLengths;
	size_t ShaderCount;
	Color Color;
	Color Specular;
	char* SpecularTexturePath;
	char* MainTexturePath;
	Color Ambient;
	Color Diffuse;
	float Shininess;
};

static const char* Tokens[] = {
	ShaderToken,
	ColorToken,
	MainTextureToken,
	SpecularTextureToken,
	SpecularColorToken,
	AmbientColorToken,
	ShininessToken,
	DiffuseColorToken
};

static const size_t TokenLengths[] = {
	sizeof(ShaderToken),
	sizeof(ColorToken),
	sizeof(MainTextureToken),
	sizeof(SpecularTextureToken),
	sizeof(SpecularColorToken),
	sizeof(AmbientColorToken),
	sizeof(ShininessToken),
	sizeof(DiffuseColorToken)
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
	case 3: // specular texture
		return TryParseString(buffer, length, MAX_PATH_LENGTH, &state->SpecularTexturePath);
	case 4: // specular color
		return Vector4s.TryDeserialize(buffer, length, state->Specular);
	case 5: // ambient color
		return Vector4s.TryDeserialize(buffer, length, state->Ambient);
	case 6: // shininess
		return Floats.TryDeserialize(buffer, length, &state->Shininess);
	case 7: // diffuse color
		return Vector4s.TryDeserialize(buffer, length, state->Diffuse);
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

	if (path is null)
	{
		return null;
	}

	struct _materialDefinition state = {
		.Color = {1, 1, 1, 1},
		.Ambient = { 1, 1, 1, 1},
		.Diffuse = { 1, 1, 1, 1 },
		.Specular = { 1, 1, 1, 1 },
		.ShaderCount = 0,
		.ShaderPathLengths = null,
		.ShaderPaths = null,
		.MainTexturePath = null,
		.SpecularTexturePath = null,
		.Shininess = 32.0f
	};

	if (Configs.TryLoadConfig(path, (const ConfigDefinition)&MaterialConfigDefinition, &state))
	{
		// no point of having a material with no shader
		if (state.ShaderCount isnt 0)
		{
			material = CreateMaterial();

			// copy all the colors over to the result material
			Vectors4CopyTo(state.Color, material->Color);
			Vectors4CopyTo(state.Specular, material->SpecularColor);
			Vectors4CopyTo(state.Ambient, material->AmbientColor);
			Vectors4CopyTo(state.Diffuse, material->DiffuseColor);

			material->Shininess = state.Shininess;

			size_t shaderCount = state.ShaderCount;

			// iterate the shader path array and trim each one
			for (size_t i = 0; i < state.ShaderCount; i++)
			{
				char* shaderPath = state.ShaderPaths[i];
				size_t shaderPathLength = state.ShaderPathLengths[i];

				shaderPathLength = Strings.Trim(shaderPath, shaderPathLength);

				state.ShaderPathLengths[i] = shaderPathLength;

				// if the string is only whitespace or empty we should not alloc space for a shader there
				if (shaderPathLength is 0)
				{
					shaderCount--;
				}
			}

			material->Shaders = SafeAlloc(sizeof(Shader) * shaderCount);
			material->Count = shaderCount;

			// becuase we might not add each shader into the array we have to keep i and currentIndex
			size_t currentIndex = 0;

			// import each shader
			for (size_t i = 0; i < state.ShaderCount; i++)
			{
				char* shaderPath = state.ShaderPaths[i];
				size_t shaderPathLength = state.ShaderPathLengths[i];

				// try not to load empty paths
				// this is a valid path array for example ",,,,,,,,,,,,,,,,,,,"
				if (shaderPathLength isnt 0)
				{
					Shader shader = ShaderCompilers.Load(shaderPath);

					material->Shaders[currentIndex++] = shader;
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

			if (state.SpecularTexturePath isnt null)
			{
				Image image;
				if (Images.TryLoadImage(state.SpecularTexturePath, &image))
				{
					Texture texture;
					if (Textures.TryCreateTexture(image, &texture))
					{
						Materials.SetSpecularTexture(material, texture);
					}

					Textures.Dispose(texture);
				}

				Images.Dispose(image);
			}
		}
	}

	if (material isnt null)
	{
		material->Name = Strings.DuplicateTerminated(path);
	}

	SafeFree(state.MainTexturePath);

	SafeFree(state.SpecularTexturePath);

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

	fprintf(file, NEWLINE);

	fprintf(file, ExportCommentFormat, ColorTokenComment);
	fprintf(file, "%s: ", ColorToken);

	if (Vector4s.TrySerializeStream(file, material->Color) is false)
	{
		return false;
	}

	fprintf(file, NEWLINE);

	fprintf(file, ExportCommentFormat, SpecularColorComment);
	fprintf(file, "%s: ", SpecularColorToken);

	if (Vector4s.TrySerializeStream(file, material->SpecularColor) is false)
	{
		return false;
	}

	fprintf(file, NEWLINE);

	fprintf(file, ExportCommentFormat, AmbientColorComment);
	fprintf(file, "%s: ", AmbientColorToken);

	if (Vector4s.TrySerializeStream(file, material->AmbientColor) is false)
	{
		return false;
	}

	fprintf(file, NEWLINE);

	fprintf(file, ExportCommentFormat, DiffuseColorComment);
	fprintf(file, "%s: ", DiffuseColorToken);

	if (Vector4s.TrySerializeStream(file, material->DiffuseColor) is false)
	{
		return false;
	}

	fprintf(file, NEWLINE);

	fprintf(file, ExportCommentFormat, ShininessComment);
	fprintf(file, "%s: %f"NEWLINE, ShininessToken, material->Shininess);

	if (material->MainTexture isnt null)
	{
		fprintf(file, ExportCommentFormat, MainTextureComment);
		fprintf(file, ExportTokenFormat, MainTextureToken, material->MainTexture->Path);
	}

	if (material->SpecularTexture isnt null)
	{
		fprintf(file, ExportCommentFormat, SpecularTextureComment);
		fprintf(file, ExportTokenFormat, SpecularTextureToken, material->SpecularTexture->Path);
	}

	return Files.TryClose(file);
}