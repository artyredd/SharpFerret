#include "engine/graphics/colors.h"
#include "core/math/vectors.h"

static bool TryDeserialize(const char* buffer, const size_t length, color* out_vector3);
static bool TrySerialize(char* buffer, const size_t length, const color vector);
static bool TrySerializeStream(File stream, const color vector);

const struct _colors Colors = {
	.Red = {1,0,0,1},
	.Green = {0,1,0,1},
	.Blue = {0,0,1,1},
	.White = {1,1,1,1},
	.Black = {0,0,0,1},
	.TryDeserialize = TryDeserialize,
	.TrySerialize = TrySerialize,
	.TrySerializeStream = TrySerializeStream
};

static bool TryDeserialize(const char* buffer, const size_t length, color* out_vector3)
{
	return Vector4s.TryDeserialize(buffer, length, (vector4*)out_vector3);
}

static bool TrySerialize(char* buffer, const size_t length, const color vector)
{
	return Vector4s.TrySerialize( buffer, length, *(vector4*)&vector);
}

static bool TrySerializeStream(File stream, const color vector)
{
	return Vector4s.TrySerializeStream(stream, *(vector4*)&vector);
}