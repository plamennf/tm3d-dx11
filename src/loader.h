#ifndef LOADER_H
#define LOADER_H

#include "geometry.h"

struct Mesh;

Mesh *make_mesh(u32 num_vertices, Vector3 *positions, Vector2 *uvs, Vector3 *normals,
                u32 num_indices, u32 *indices);
Mesh *load_obj(char *filename);

#endif
