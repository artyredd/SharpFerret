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
static void SetAreaTexture(Material, const Texture);
static void SetReflectionTexture(Material, const Texture);

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
	.SetSpecularTexture = &SetSpecularTexture,
	.SetAreaTexture = SetAreaTexture,
	.SetReflectionTexture = SetReflectionTexture
};

#define DEFAULT_MATERIAL_SETTINGS (ShaderSettings.UseCameraPerspective | ShaderSettings.BackfaceCulling)

#define MATERIAL_LOADER_BUFFER_SIZE 1024

TYPE_ID(MaterialShaders);
TYPE_ID(Material);

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

	Memory.Free(material->Shaders, MaterialShadersTypeId);

	Memory.Free(material->Name, Memory.String);

	Textures.Dispose(material->MainTexture);
	Textures.Dispose(material->SpecularTexture);
	Textures.Dispose(material->ReflectionMap);
	Textures.Dispose(material->AreaMap);

	Memory.Free(material, MaterialTypeId);
}

static Material Create(Shader shader, Texture texture)
{
	Memory.RegisterTypeName(nameof(Material), &MaterialTypeId);
	Memory.RegisterTypeName("MaterialShaders", &MaterialTypeId);

	Material material = Memory.Alloc(sizeof(struct _material), MaterialTypeId);

	material->MainTexture = Textures.Instance(texture);

	if (shader isnt null)
	{
		material->Shaders = Memory.Alloc(sizeof(Shader), MaterialShadersTypeId);
		material->Shaders[0] = Shaders.Instance(shader);
		material->Count = 1;
	}

	SetVector4Macro(material->Color, 1, 1, 1, 1);
	SetVector4Macro(material->AmbientColor, 1, 1, 1, 1);
	SetVector4Macro(material->DiffuseColor, 1, 1, 1, 1);
	SetVector4Macro(material->SpecularColor, 1, 1, 1, 1);

	material->Shininess = 0.5f;
	material->Reflectivity = 0.0f;

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

		if (Memory.TryRealloc(material->Shaders, previousSize, newSize, (void**)&material->Shaders) is false)
		{
			Shader* newArray = Memory.Alloc(newSize, MaterialShadersTypeId);

			CopyShadersTo(material->Shaders, material->Count, newArray, desiredCount);

			Memory.Free(material->Shaders, MaterialShadersTypeId);
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

	newMaterial->AreaMap = Textures.Instance(material->AreaMap);

	newMaterial->ReflectionMap = Textures.Instance(material->ReflectionMap);

	if (material->Name isnt null)
	{
		newMaterial->Name = Strings.DuplicateTerminated(material->Name);
	}

	Vectors4CopyTo(material->Color, newMaterial->Color);
	Vectors4CopyTo(material->DiffuseColor, newMaterial->DiffuseColor);
	Vectors4CopyTo(material->SpecularColor, newMaterial->SpecularColor);
	Vectors4CopyTo(material->AmbientColor, newMaterial->AmbientColor);

	newMaterial->Shininess = material->Shininess;
	newMaterial->Reflectivity = material->Reflectivity;

	return newMaterial;
}

// Enables or disables graphics and matrix settings based on what settings are provided by the shader
static void PrepareSettings(Shader shader)
{
	unsigned int settings = shader->Settings;

	// check to see if we need to DISABLE back face culling(by default it's on)
	if (HasFlag(settings, ShaderSettings.BackfaceCulling))
	{
		GraphicsDevice.EnableCulling(shader->CullingType);
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
		GraphicsDevice.SetDepthTest(shader->DepthFunction);
	}
	else
	{
		GraphicsDevice.DisableDepthTesting();
	}

	// set whether or not we should be drawing with wireframes
	GraphicsDevice.SetFillMode(shader->FillMode);
}

static void SetTextureUniform(Shader shader, Uniform uniform, Texture texture, unsigned int slot)
{
	if (texture isnt null)
	{
		int uniformHandle;
		if (Shaders.TryGetUniform(shader, uniform, &uniformHandle))
		{
			GraphicsDevice.ActivateTexture(texture->Type, texture->Handle->Handle, uniformHandle, slot);
		}
	}
}

static void SetMaterialTexture(Shader shader, Uniform textureUniform, Uniform mapUniform, Texture texture, unsigned int slot)
{
	if (Shaders.SetInt(shader, mapUniform, texture isnt null))
	{
		SetTextureUniform(shader, textureUniform, texture, slot);
	}
}

static bool TrySetLightUniforms(Shader shader, Light light, size_t index)
{
	/*
	struct _light{
		bool enabled;
		int lightType;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		float range;
		float radius;
		float edgeSoftness;
		vec3 position;
		vec3 direction;
	};
	*/

	Shaders.SetArrayFieldInt(shader, Uniforms.Lights, index, Uniforms.Light.Enabled, light->Enabled);

	// no sense setting the other ones if it's not enabled
	if (light->Enabled is false)
	{
		return true;
	}

	Shaders.SetArrayFieldInt(shader, Uniforms.Lights, index, Uniforms.Light.Type, light->Type);

	Shaders.SetArrayFieldFloat(shader, Uniforms.Lights, index, Uniforms.Light.Intensity, light->Intensity);

	Shaders.SetArrayFieldVector4(shader, Uniforms.Lights, index, Uniforms.Light.Ambient, light->Ambient);

	Shaders.SetArrayFieldVector4(shader, Uniforms.Lights, index, Uniforms.Light.Diffuse, light->Diffuse);

	Shaders.SetArrayFieldVector4(shader, Uniforms.Lights, index, Uniforms.Light.Specular, light->Specular);

	Shaders.SetArrayFieldFloat(shader, Uniforms.Lights, index, Uniforms.Light.Range, light->Range);

	Shaders.SetArrayFieldFloat(shader, Uniforms.Lights, index, Uniforms.Light.Radius, light->Radius);

	Shaders.SetArrayFieldFloat(shader, Uniforms.Lights, index, Uniforms.Light.EdgeSoftness, light->EdgeSoftness);

	int handle;
	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, Uniforms.Light.Position, &handle))
	{
		vec3 pos;

		// update the position of the light to include it's parent's transform and any rotation
		glm_mat4_mulv3(Transforms.Refresh(light->Transform), light->Transform->Position, 1.0f, pos);

		glUniform3fv(handle, 1, pos);
	}

	if (Shaders.TryGetUniformArrayField(shader, Uniforms.Lights, index, Uniforms.Light.Direction, &handle))
	{
		vec3 direction;
		Transforms.GetDirection(light->Transform, Directions.Back, direction);
		glUniform3fv(handle, 1, direction);
	}

	if (Shaders.TryGetUniformArray(shader, Uniforms.LightShadowMaps, index, &handle))
	{
		Texture texture = light->FrameBuffer->Texture;

		// MAGIC NUMBER ALERT
		// index+4 is used here becuase 0,1,2,3 are all resevered for the current material's texture units
		// all other texture units are used for lighting
		GraphicsDevice.ActivateTexture(texture->Type, texture->Handle->Handle, handle, (int)(index + 4));
	}

	// check to see if we need to load the light matrix into the shader so we can render the RESULTS of a shadowmap
	if (Shaders.TryGetUniformArray(shader, Uniforms.LightViewMatrix, index, &handle))
	{
		glUniformMatrix4fv(handle, 1, false, &light->ViewMatrix[0][0]);
	}

	return true;
}

static void SetLightUniforms(Shader shader, Scene scene)
{
	// if there is no light count in the shader don't set light uniforms for performance
	if (Shaders.SetInt(shader, Uniforms.LightCount, (int)scene->LightCount) is false)
	{
		return;
	}

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

static void Draw(Material material, RenderMesh mesh, Scene scene)
{
	vec4* modelMatrix = Transforms.Refresh(mesh->Transform);

	Cameras.Refresh(scene->MainCamera);

	for (size_t i = 0; i < material->Count; i++)
	{
		Shader shader = material->Shaders[i];

		if (shader isnt null && shader->Enabled)
		{
			Shaders.Enable(shader);

			// turn on or off various settings like blending etc..
			PrepareSettings(shader);

			// set the light uniforms if we need to
			SetLightUniforms(shader, scene);

			// set MVP stuff for shaders
			Shaders.SetMatrix(shader, Uniforms.ModelMatrix, modelMatrix);

			Shaders.SetMatrix(shader, Uniforms.ViewMatrix, scene->MainCamera->State.View);

			Shaders.SetMatrix(shader, Uniforms.ProjectionMatrix, scene->MainCamera->State.Projection);

			// set various commmonly used uniforms in shaders
			Shaders.SetVector3(shader, Uniforms.CameraPosition, scene->MainCamera->Transform->Position);

			// set material if it's used 
			Shaders.SetVector4(shader, Uniforms.Material.Color, material->Color);

			Shaders.SetFloat(shader, Uniforms.Material.Shininess, material->Shininess);

			Shaders.SetFloat(shader, Uniforms.Material.Reflectivity, material->Reflectivity);

			Shaders.SetVector4(shader, Uniforms.Material.Specular, material->SpecularColor);

			Shaders.SetVector4(shader, Uniforms.Material.Diffuse, material->DiffuseColor);

			Shaders.SetVector4(shader, Uniforms.Material.Ambient, material->AmbientColor);

			SetMaterialTexture(shader, Uniforms.Material.DiffuseMap, Uniforms.Material.UseDiffuseMap, material->MainTexture, 0);

			SetMaterialTexture(shader, Uniforms.Material.SpecularMap, Uniforms.Material.UseSpecularMap, material->SpecularTexture, 1);

			SetMaterialTexture(shader, Uniforms.Material.ReflectionMap, Uniforms.Material.UseReflectionMap, material->ReflectionMap, 2);

			SetMaterialTexture(shader, Uniforms.Material.AreaMap, Uniforms.Material.UseAreaMap, material->AreaMap, 3);

			// draw the triangles
			RenderMeshes.Draw(mesh);

			Shaders.Disable(shader);
		}
	}
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

static void SetAreaTexture(Material material, const Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->AreaMap);
	material->AreaMap = Textures.Instance(texture);
}

static void SetReflectionTexture(Material material, const Texture texture)
{
	GuardNotNull(material);
	Textures.Dispose(material->ReflectionMap);
	material->ReflectionMap = Textures.Instance(texture);
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

	SetVector4Macro(material->Color, r, g, b, a);
}

static void SetName(Material material, const char* name)
{
	GuardNotNull(material);
	if (material->Name isnt null)
	{
		Memory.Free(material->Name, Memory.String);
	}

	material->Name = Strings.DuplicateTerminated(name);
}

#define ExportTokenFormat "%s: %s\n"
#define ExportCommentFormat "%s\n"

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
	float Reflectivity;
	char* ReflectionTexturePath;
	char* ReflectionMap;
	char* AreaMap;
};

TOKEN_LOAD(shaders, struct _materialDefinition*)
{
	return Parsing.TryGetStrings(buffer, length, &state->ShaderPaths, &state->ShaderPathLengths, &state->ShaderCount);
}


TOKEN_SAVE(shaders, Material)
{
	// print string array
	for (size_t i = 0; i < state->Count; i++)
	{
		const Shader shader = state->Shaders[i];

		if (shader isnt null)
		{
			fprintf(stream, "%s,", shader->Name);
		}
	}
}

TOKEN_LOAD(color, struct _materialDefinition*)
{
	return Vector4s.TryDeserialize(buffer, length, state->Color);
}

TOKEN_SAVE(color, Material)
{
	if (Vector4s.TrySerializeStream(stream, state->Color) is false)
	{
		throw(FailedToReadFileException);
	}
}

TOKEN_LOAD(mainTexture, struct _materialDefinition*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_LENGTH, &state->MainTexturePath);
}

TOKEN_SAVE(mainTexture, Material)
{
	if (state->MainTexture isnt null)
	{
		fprintf(stream, "%s",state->MainTexture->Path);
	}
}

TOKEN_LOAD(specularMap, struct _materialDefinition*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_LENGTH, &state->SpecularTexturePath);
}

TOKEN_SAVE(specularMap, Material)
{
	if (state->SpecularTexture isnt null)
	{
		fprintf(stream, "%s", state->SpecularTexture->Path);
	}
}

TOKEN_LOAD(specular, struct _materialDefinition*)
{
	return Vector4s.TryDeserialize(buffer, length, state->Specular);
}

TOKEN_SAVE(specular, Material)
{
	if (Vector4s.TrySerializeStream(stream, state->SpecularColor) is false)
	{
		throw(FailedToSerializeException);
	}
}

TOKEN_LOAD(ambient, struct _materialDefinition*)
{
	return Vector4s.TryDeserialize(buffer, length, state->Ambient);
}

TOKEN_SAVE(ambient, Material)
{
	if (Vector4s.TrySerializeStream(stream, state->AmbientColor) is false)
	{
		throw(FailedToSerializeException);
	}
}

TOKEN_LOAD(shininess, struct _materialDefinition*)
{
	return Floats.TryDeserialize(buffer, length, &state->Shininess);
}

TOKEN_SAVE(shininess, Material)
{
	Floats.SerializeStream(stream, state->Shininess);
}

TOKEN_LOAD(diffuse, struct _materialDefinition*)
{
	return Vector4s.TryDeserialize(buffer, length, state->Diffuse);
}

TOKEN_SAVE(diffuse, Material)
{
	if (Vector4s.TrySerializeStream(stream, state->DiffuseColor) is false)
	{
		throw(FailedToSerializeException);
	}
}

TOKEN_LOAD(reflectionMap, struct _materialDefinition*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_LENGTH, &state->ReflectionMap);
}

TOKEN_SAVE(reflectionMap, Material)
{
	if (state->ReflectionMap isnt null)
	{
		fprintf(stream, "%s", state->ReflectionMap->Path);
	}
}

TOKEN_LOAD(areaMap, struct _materialDefinition*)
{
	return Parsing.TryGetString(buffer, length, MAX_PATH_LENGTH, &state->AreaMap);
}

TOKEN_SAVE(areaMap, Material)
{
	if (state->AreaMap isnt null)
	{
		fprintf(stream, "%s", state->AreaMap->Path);
	}
}

TOKEN_LOAD(reflectivity, struct _materialDefinition*)
{
	return Floats.TryDeserialize(buffer, length, &state->Reflectivity);
}

TOKEN_SAVE(reflectivity, Material)
{
	Floats.SerializeStream(stream, state->Reflectivity);
}

TOKENS(11) {
	TOKEN(shaders, "# path array delimited by ','; the array of shaders that should be loaded for this material" ),
	TOKEN(color, "# vec4; the material base color" ),
	TOKEN(mainTexture, "# path; the main UV texture that should be used for this material" ),
	TOKEN(specularMap, "# path; the texture that should be used to determine what parts of this material are shiny" ),
	TOKEN(specular, "# vec4; the color that specular highlights should be" ),
	TOKEN(ambient, "# vec4; the ambient color this object should be when exposed to no light" ),
	TOKEN(shininess, "# float [0-255]; How shiny the material should be" ),
	TOKEN(diffuse, "# vec4; the diffuse color of the material" ),
	TOKEN(reflectionMap, "# the 2d texture path that should be used to determine which parts of this material should be reflective" ),
	TOKEN(areaMap, "# the 3d cubemap texture path that should be used to determine what is shown in reflections off of this object" ),
	TOKEN(reflectivity, "# float [0-1]; the reflectivity of this material" ),
};

CONFIG(Material);

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
		.Shininess = 32.0f,
		.Reflectivity = 0.0f,
		.ReflectionTexturePath = null
	};

	if (Configs.TryLoadConfig(path, &MaterialConfigDefinition, &state))
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
			material->Reflectivity = state.Reflectivity;

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

			material->Shaders = Memory.Alloc(sizeof(Shader) * shaderCount, MaterialShadersTypeId);
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
				Texture texture = Textures.Load(state.MainTexturePath);

				if (texture is null)
				{
					throw(FailedToLoadTextureException)
				}

				Materials.SetMainTexture(material, texture);

				Textures.Dispose(texture);
			}

			if (state.SpecularTexturePath isnt null)
			{
				Texture texture = Textures.Load(state.SpecularTexturePath);

				if (texture is null)
				{
					throw(FailedToLoadTextureException)
				}

				Materials.SetSpecularTexture(material, texture);

				Textures.Dispose(texture);
			}

			if (state.ReflectionTexturePath isnt null)
			{
				Texture texture = Textures.Load(state.ReflectionTexturePath);

				if (texture is null)
				{
					throw(FailedToLoadTextureException)
				}

				Materials.SetReflectionTexture(material, texture);

				Textures.Dispose(texture);
			}
		}
	}

	if (material isnt null)
	{
		material->Name = Strings.DuplicateTerminated(path);
	}

	Memory.Free(state.MainTexturePath, Memory.String);

	Memory.Free(state.SpecularTexturePath, Memory.String);

	Memory.Free(state.ShaderPathLengths, Memory.String);

	for (size_t i = 0; i < state.ShaderCount; i++)
	{
		Memory.Free(state.ShaderPaths[i], Memory.String);
	}

	Memory.Free(state.ShaderPaths, Memory.String);

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

	Configs.SaveConfigStream(file, &MaterialConfigDefinition, material);

	return Files.TryClose(file);
}