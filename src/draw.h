#ifndef DRAW_H
#define DRAW_H

#include "geometry.h"
#include "bitmap.h"

struct Texture_Map {
    char *full_path;
    char *short_name;
    
    int width = 0;
    int height = 0;

    Texture_Format format;

#ifdef RENDER_D3D11
    void *texture = nullptr;
    void *rtv = nullptr;
    void *srv = nullptr;
#endif
};

struct Shader;

struct Mesh;

extern Texture_Map *the_back_buffer;
extern Texture_Map *the_back_depth_buffer;

extern Texture_Map *the_offscreen_buffer;
extern Texture_Map *the_offscreen_depth_buffer;

extern int render_target_width;
extern int render_target_height;

extern Matrix4 view_to_proj_matrix;
extern Matrix4 world_to_view_matrix;
extern Matrix4 object_to_world_matrix;
extern Matrix4 object_to_proj_matrix;

extern Shader *shader_color;
extern Shader *shader_texture;
extern Shader *shader_basic_3d;
extern Shader *shader_msaa_2x;
extern Shader *shader_msaa_4x;
extern Shader *shader_msaa_8x;
extern Shader *shader_text;

void init_draw(bool vsync, bool multisample, int sample_count);
void swap_buffers();
void resize_render_targets(int width, int height);

void clear_render_target(f32 r, f32 g, f32 b, f32 a);

void immediate_begin();
void immediate_flush();
void immediate_vertex(Vector3 position, u32 color, Vector2 uv);
void immediate_quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color);
void immediate_quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector4 color);
void immediate_quad(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color);
void immediate_quad(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, Vector4 color);

void set_vertex_format_to_mesh();
void set_vertex_format_to_immediate();

Texture_Map *create_texture_rendertarget(int width, int height, bool multisample, int num_samples);
Texture_Map *create_texture_depthtarget(Texture_Map *render_target);

Texture_Map *create_texture(Bitmap bitmap);
void update_texture(Texture_Map *map, int x, int y, int width, int height, u8 *data);

void set_render_target(Texture_Map *map);
void set_depth_target(Texture_Map *map);

void set_shader(Shader *shader);
void set_diffuse_texture(Texture_Map *map);

void refresh_transform();
void rendering_2d_right_handed();

void draw_text(struct Font *font, char *text, int x, int y, Vector4 color);

void draw_mesh(Mesh *mesh, Vector3 position, f32 scale);

void draw_game_view();

#endif
