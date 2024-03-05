#pragma once
#include "core/csharp.h"
#include <stdint.h>
#include "engine/graphics/imaging.h"
#include "engine/graphics/textureDefinitions.h"
#include "engine/graphics/colors.h"

typedef unsigned int ClearMask;

struct _clearMasks {
	ClearMask None;
	ClearMask Color;
	ClearMask Stencil;
	ClearMask Depth;
};

extern const struct _clearMasks ClearMasks;

typedef unsigned int ColorBufferType;
struct _bufferTypes {
	ColorBufferType None;
	ColorBufferType FrontLeft;
	ColorBufferType FrontRight;
	ColorBufferType BackLeft;
	ColorBufferType BackRight;
	ColorBufferType Front;
	ColorBufferType Back;
	ColorBufferType Left;
	ColorBufferType Right;
	ColorBufferType Color;
};

extern const struct _bufferTypes ColorBufferTypes;

typedef unsigned int FrameBufferAttachment;

struct _frameBufferAttachments {
	FrameBufferAttachment Depth;
	FrameBufferAttachment Color;
	FrameBufferAttachment DepthStencil;
};

extern const struct _frameBufferAttachments FrameBufferAttachments;

typedef unsigned int FrameBufferComponent;

struct _frameBufferComponents {
	FrameBufferComponent RenderBuffer;
	FrameBufferComponent Texture;
	FrameBufferComponent Cubemap;
};

extern const struct _frameBufferComponents FrameBufferComponents;

typedef parsableValue CullingType;

struct _cullingTypes {
	CullingType None;
	CullingType Front;
	CullingType Back;
};

extern struct _cullingTypes CullingTypes;


#define MAX_CULLING_TYPES sizeof(CullingTypes)/sizeof(CullingType)
#define TryGetCullingType(buffer, length, out_value) ParsableValues.TryGetMemberByName(&CullingTypes, MAX_CULLING_TYPES, buffer, length, out_value)

typedef parsableValue FillMode;

struct _fillModes {
	FillMode Fill;
	FillMode Line;
};

extern struct _fillModes FillModes;

#define MAX_FILL_MODES sizeof(FillModes)/sizeof(FillMode)
#define TryGetFillMode(buffer, length, out_value) ParsableValues.TryGetMemberByName(&FillModes, MAX_CULLING_TYPES, buffer, length, out_value)

typedef parsableValue Comparison;

struct _comparisons {
	Comparison Always;
	Comparison Never;
	Comparison Equal;
	Comparison NotEqual;
	Comparison GreaterThan;
	Comparison LessThan;
	Comparison GreaterThanOrEqual;
	Comparison LessThanOrEqual;
};

extern struct _comparisons Comparisons;

#define MAX_COMPARISONS sizeof(Comparisons)/sizeof(Comparison)
#define TryGetComparison(buffer, length, out_value) ParsableValues.TryGetMemberByName(&Comparisons, MAX_COMPARISONS, buffer, length, out_value)

struct _graphicsDeviceMethods
{
	void (*EnableBlending)(void);
	void (*EnableCulling)(const CullingType);
	void (*DisableBlending)(void);
	void (*DisableCulling)(void);
	void (*EnableStencil)(void);
	void (*DisableStencil)(void);
	void (*EnableStencilWriting)(void);
	void (*DisableStencilWriting)(void);
	void(*SetStencilMask)(const unsigned int mask);
	unsigned int (*GetStencilMask)(void);
	/// <summary>
	/// Sets the function that determins whether or not a fragment gets drawn, a fragment will only be drawn when the Comparison would return true
	/// For example: the stencil value for a fragment is 1, and SetStencilFunction(Comparisons.Equal, 1), would mean the fragment would only be drawn
	/// when it's stencil value is 1, otherwise it's not drawn
	/// </summary>
	void (*SetStencilFull)(const Comparison, const unsigned int valueToCompareTo, const unsigned int mask);
	/// <summary>
	/// Sets the function that determins whether or not a fragment gets drawn, a fragment will only be drawn when the Comparison would return true
	/// For example: the stencil value for a fragment is 1, and SetStencilFunction(Comparisons.Equal, 1), would mean the fragment would only be drawn
	/// when it's stencil value is 1, otherwise it's not drawn
	/// </summary>
	void (*SetStencil)(const Comparison);
	void (*ResetStencilFunction)(void);
	void (*EnableDepthTesting)(void);
	void (*DisableDepthTesting)(void);
	void (*SetDepthTest)(const Comparison);
	/// <summary>
	/// Generates a new texture buffer and binds it to the provided type on the graphics device, returns the handle to the generated texture
	/// </summary>
	unsigned int (*CreateTexture)(TextureType);
	/// <summary>
	/// deletes the provided texture on the graphics device
	/// </summary>
	void (*DeleteTexture)(unsigned int handle);
	/// <summary>
	/// Loads the provided image to the graphics device to the currently bound texture
	/// </summary>
	void (*LoadTexture)(const TextureType, TextureFormat, BufferFormat, Image, unsigned int offset);
	/// <summary>
	/// Loads an empty texture on the graphics device
	/// </summary>
	void (*LoadBufferTexture)(const TextureType, const TextureFormat, const BufferFormat, ulong width, ulong height, unsigned int offset);
	/// <summary>
	/// Modifies the currenly bound texture with the provided setting and value
	/// </summary>
	void (*ModifyTexture)(const TextureType, TextureSetting, const TextureValue);
	void (*ModifyTextureProperty)(TextureType type, TextureSetting setting, const color value);
	/// <summary>
	/// Binds the provided texture to an open texture slot on the graphics device
	/// </summary>
	void (*ActivateTexture)(const TextureType, const unsigned int textureHandle, const int uniformHandle, const unsigned int slot);
	void (*ClearTexture)(const TextureType type);
	unsigned int (*GenerateBuffer)(void);
	void (*DeleteBuffer)(unsigned int handle);
	unsigned int (*GenerateRenderBuffer)(void);
	void (*DeleteRenderBuffer)(unsigned int handle);
	unsigned int (*GenerateFrameBuffer)(void);
	void (*DeleteFrameBuffer)(unsigned int handle);
	// Bindes the provided frame buffer as the active one, all following read and write calls will be done to the provided buffer,
	// if 0 is provided the default buffer will be used
	void (*UseFrameBuffer)(unsigned int handle);
	void (*AttachFrameBufferComponent)(FrameBufferComponent componentType, FrameBufferAttachment attachmentType, unsigned int attachmentHandle);
	/// <summary>
	/// Sets the current render buffer to the provided handle
	/// </summary>
	void (*UseRenderBuffer)(unsigned int handle);
	/// <summary>
	/// Allocs the provided sized render buffer on the graphics device
	/// </summary>
	void (*AllocRenderBuffer)(unsigned int handle, TextureFormat format, ulong width, ulong height);
	/// <summary>
	/// Sets the rendering resolution for draw, reads, and writes on the graphics device
	/// </summary>
	void (*SetResolution)(signed long long int x, signed long long int y, ulong width, ulong height);
	void (*SetReadBuffer)(ColorBufferType);
	void (*SetDrawBuffer)(ColorBufferType);
	// sets the fill mode for polygons, typically used for enabling/disabling wireframe rendering mode
	void (*SetFillMode)(const FillMode);
	/// <summary>
	/// Verifies that cleanup was properly performed before program exit
	/// </summary>
	bool (*TryVerifyCleanup)(void);
	/// <summary>
	/// Clears the currently bound frame buffer
	/// </summary>
	void (*ClearCurrentFrameBuffer)(unsigned int clearMask);
};

extern const struct _graphicsDeviceMethods GraphicsDevice;