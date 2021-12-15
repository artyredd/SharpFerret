#pragma once

// options

#ifndef cglm_h
typedef float vec4[4];
typedef float vec3[3];
typedef float vec2[2];
#endif // !cglm_h

typedef struct _triangle* Triangle;

struct _triangle {
	vec3 Vertices[3];
	vec3 TextureVertices[3];
	vec3 VertexNormals[3];
};

typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	Triangle* Triangles;
};