#pragma once

#include "geometry.h"

const float TERRAIN_SIZE = 800.0f;
const double TERRAIN_MAX_HEIGHT = 40.0;
const u64 TERRAIN_MAX_PIXEL_COLOR = 256ULL * 256ULL * 256ULL;

struct Texture_Map;
struct Mesh;

struct Terrain_Texture_Pack {
    Texture_Map *background_texture;
    Texture_Map *r_texture;
    Texture_Map *g_texture;
    Texture_Map *b_texture;
};

Terrain_Texture_Pack make_terrain_texture_pack(Texture_Map *background_texture, Texture_Map *r_texture, Texture_Map *g_texture, Texture_Map *b_texture);

struct Terrain {
    float x, z;
    Mesh *mesh;
    Terrain_Texture_Pack texture_pack;
    Texture_Map *blend_map;
    float *heights;
    int num_heights;
};

Terrain *make_terrain(int grid_x, int grid_z, Terrain_Texture_Pack texture_pack, Texture_Map *blend_map, char *height_map_name);
float get_terrain_height_at(Terrain *terrain, float world_x, float world_z);
Terrain *get_terrain_at(Vector3 world_pos);
void draw_terrains();
