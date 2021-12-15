#pragma once

// options

#ifndef cglm_h
typedef float vec4[4];
typedef float vec3[3];
typedef float vec2[2];
#endif // !cglm_h

typedef int intvec4[4];
typedef int intvec3[3];
typedef int intvec2[2];

typedef struct _face* Face;

struct _face {
	intvec3 Vertices[3];
	intvec3 TextureVertices[3];
	intvec3 Normals[3];
};

typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	size_t VertexCount;
	vec3* Vertices;
	size_t TextureVerticesCount;
	vec3* TextureVertices;
	size_t NormalCount;
	vec3* Normals;
	size_t FaceCount;
	Face* Faces;
};