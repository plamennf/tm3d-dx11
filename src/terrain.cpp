#include "general.h"
#include "terrain.h"
#include "draw.h"
#include "bitmap.h"
#include "array.h"
#include "loader.h"

#include <stb_image.h>

static Array <Terrain *> loaded_terrains;

inline float get_terrain_height(int x, int z, Bitmap bitmap) {
    if (x < 0 || x >= bitmap.height || z < 0 || z >= bitmap.height) {
        return 0.0f;
    }

    double height = static_cast <double>(bitmap.get_rgb(x, z));
    height /= TERRAIN_MAX_PIXEL_COLOR;
    height *= TERRAIN_MAX_HEIGHT;
    height -= TERRAIN_MAX_HEIGHT*0.5;
    return static_cast<float>(height);
}

inline Vector3 calculate_normal(int x, int z, Bitmap bitmap) {
    float height_l = 0.0f;
    float height_r = 0.0f;
    float height_d = 0.0f;
    float height_u = 0.0f;

    if (x == 0) {
        height_l = get_terrain_height(x-1 + bitmap.height - 1, z, bitmap);
        height_r = get_terrain_height(x+1, z, bitmap);
    } else if (x == 1) {
        height_l = get_terrain_height(x-1 + bitmap.height - 1, z, bitmap);
        height_r = get_terrain_height(x+1, z, bitmap);
    } else if (x == bitmap.height) {
        height_r = get_terrain_height(x+1 - bitmap.height, z, bitmap);
        height_l = get_terrain_height(x-1, z, bitmap);
    } else if (x == bitmap.height - 1) {
        height_r = get_terrain_height(x-1 - bitmap.height + 1, z, bitmap);
        height_l = get_terrain_height(x-1, z, bitmap);
    } else {
        height_l = get_terrain_height(x-1, z, bitmap);
        height_r = get_terrain_height(x+1, z, bitmap);
    }

    if (z == 0) {
        height_d = get_terrain_height(x, z-1 + bitmap.height, bitmap);
        height_u = get_terrain_height(x, z+1, bitmap);
    } else if (z == 1) {
        height_d = get_terrain_height(x, z-1 + bitmap.height - 1, bitmap);
        height_u = get_terrain_height(x, z+1, bitmap);
    } else if (z == bitmap.height) {
        height_d = get_terrain_height(x, z-1, bitmap);
        height_u = get_terrain_height(x, z+1 - bitmap.height, bitmap);
    } else if (z == bitmap.height - 1) {
        height_d = get_terrain_height(x, z-1, bitmap);
        height_u = get_terrain_height(x, z+1 - bitmap.height + 1, bitmap);
    } else {
        height_d = get_terrain_height(x, z-1, bitmap);
        height_u = get_terrain_height(x, z+1, bitmap);
    }

    return normalize_or_zero(make_vector3(height_l-height_r, 2.0f, height_d-height_u));
}

static Mesh *generate_terrain(char *height_map, Terrain *terrain) {
    char *full_path = mprintf("data/textures/%s.png", height_map);
    defer { delete [] full_path; };
    Bitmap bitmap;
    bitmap.load_from_file(full_path);
    defer { stbi_image_free(bitmap.data); };

    int TERRAIN_VERTEX_COUNT = static_cast <int>(bitmap.height);
    int count = TERRAIN_VERTEX_COUNT*TERRAIN_VERTEX_COUNT;

    terrain->heights = new float[count];
    terrain->num_heights = count;

    Vector3 *vertices = new Vector3[count];
    defer { delete [] vertices; };
    Vector3 *normals = new Vector3[count];
    defer { delete [] normals; };
    Vector2 *uvs = new Vector2[count];
    defer { delete [] uvs; };

    u32 num_indices = 6*(TERRAIN_VERTEX_COUNT-1)*(TERRAIN_VERTEX_COUNT*1);
    u32 *indices = new u32[num_indices];
    defer { delete [] indices; };

    u32 vertex_pointer = 0;
    for (u32 i = 0; i < TERRAIN_VERTEX_COUNT; i++) {
        for (u32 j = 0; j < TERRAIN_VERTEX_COUNT; j++) {
            vertices[vertex_pointer].x = -static_cast <float>(j)/(static_cast <float>(TERRAIN_VERTEX_COUNT)-1)*TERRAIN_SIZE;
            float height = get_terrain_height(j, i, bitmap);
            terrain->heights[i * TERRAIN_VERTEX_COUNT + j] = height;
            vertices[vertex_pointer].y = height;
            vertices[vertex_pointer].z = -static_cast <float>(i)/(static_cast <float>(TERRAIN_VERTEX_COUNT)-1)*TERRAIN_SIZE;
            normals[vertex_pointer] = calculate_normal(j, i, bitmap);
            uvs[vertex_pointer].x = static_cast <float>(j)/(static_cast <float>(TERRAIN_VERTEX_COUNT)-1);
            uvs[vertex_pointer].y = static_cast <float>(i)/(static_cast <float>(TERRAIN_VERTEX_COUNT)-1);
            vertex_pointer++;
        }
    }

    u32 pointer = 0;
    for (u32 gz = 0; gz < TERRAIN_VERTEX_COUNT - 1; gz++) {
        for (u32 gx = 0; gx < TERRAIN_VERTEX_COUNT - 1; gx++) {
            u32 top_left = (gz*TERRAIN_VERTEX_COUNT)+gx;
            u32 top_right = top_left+1;
            u32 bottom_left = ((gz+1)*TERRAIN_VERTEX_COUNT)+gx;
            u32 bottom_right = bottom_left+1;
            indices[pointer++] = top_left;
            indices[pointer++] = bottom_left;
            indices[pointer++] = top_right;
            indices[pointer++] = top_right;
            indices[pointer++] = bottom_left;
            indices[pointer++] = bottom_right;
        }
    }

    return make_mesh(count, vertices, uvs, normals, num_indices, indices);
}

Terrain *make_terrain(int grid_x, int grid_z, Terrain_Texture_Pack texture_pack, Texture_Map *blend_map, char *height_map_name) {
    Terrain *result = new Terrain();
    result->texture_pack = texture_pack;
    result->blend_map = blend_map;
    result->x = grid_x * TERRAIN_SIZE;
    result->z = grid_z * TERRAIN_SIZE;
    result->mesh = generate_terrain(height_map_name, result);
    loaded_terrains.add(result);
    return result;
}

float get_terrain_height_at(Terrain *terrain, float world_x, float world_z) {
    if (!terrain->heights) return 0.0f;

    world_x = fabsf(world_x);
    world_z = fabsf(world_z);

    u32 num_heights = sqrtf(terrain->num_heights);
    float terrain_x = fabsf(world_x - terrain->x);
    float terrain_z = fabsf(world_z - terrain->z);
    float grid_square_size = TERRAIN_SIZE / (static_cast <float>(num_heights) - 1);
    int grid_x = static_cast <int>(terrain_x / grid_square_size);
    int grid_z = static_cast <int>(terrain_z / grid_square_size);
    if (grid_x >= num_heights - 1 || grid_z >= num_heights - 1 || grid_x < 0 || grid_z < 0) {
        return 0.0f;
    }

    float x_coord = fmodf(terrain_x, grid_square_size) / grid_square_size;
    float z_coord = fmodf(terrain_z, grid_square_size) / grid_square_size;

    float result;
    if (x_coord <= 1.0f-z_coord) {
        result = get_barycentric(make_vector3(0, terrain->heights[grid_z * num_heights + grid_x], 0),
                                 make_vector3(1, terrain->heights[grid_z * num_heights + grid_x + 1], 0),
                                 make_vector3(0, terrain->heights[(grid_z + 1) * num_heights + grid_x], 1),
                                 make_vector2(x_coord, z_coord));
    } else {
        result = get_barycentric(make_vector3(1, terrain->heights[grid_z * num_heights + grid_x + 1], 0),
                                 make_vector3(1, terrain->heights[(grid_z + 1) * num_heights + grid_x + 1], 1),
                                 make_vector3(0, terrain->heights[(grid_z + 1) * num_heights + grid_x], 1),
                                 make_vector2(x_coord, z_coord));
    }
    return result;
}

Terrain *get_terrain_at(Vector3 world_pos) {
    world_pos.x = -world_pos.x;
    world_pos.z = -world_pos.z;

    int c_grid_x = fabsf(static_cast <int>(world_pos.x / static_cast <float>(TERRAIN_SIZE)));
    int c_grid_z = fabsf(static_cast <int>(world_pos.z / static_cast <float>(TERRAIN_SIZE)));

    for (int i = 0; i < loaded_terrains.count; i++) {
        Terrain *t = loaded_terrains[i];
        int grid_x = static_cast <int>(t->x / static_cast <float>(TERRAIN_SIZE));
        int grid_z = static_cast <int>(t->z / static_cast <float>(TERRAIN_SIZE));
        if (c_grid_x == grid_x && c_grid_z == grid_z) {
            return t;
        }
    }
    
    return nullptr;
}

void draw_terrains() {
    set_shader(shader_terrain);
    
    for (int i = 0; i < loaded_terrains.count; i++) {
        Terrain *terrain = loaded_terrains[i];
        
        set_terrain_textures(terrain->texture_pack);

        draw_mesh(terrain->mesh, make_vector3(terrain->x, 0, terrain->z), make_vector3(0, 0, 0), 1);
    }
}
