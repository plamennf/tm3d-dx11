#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"

struct Texture_Map;

struct Mesh_Vertex {
    Vector3 position;
    Vector2 uv;
    Vector3 normal;
};

struct Mesh {
    void *vbo;
    void *ibo;
    u32 vertex_count;

    Texture_Map *map;
};

#endif
