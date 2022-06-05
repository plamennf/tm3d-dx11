#include "draw.h"

#include "font.h"
#include "mesh.h"
#include "os.h"
#include "catalog.h"
#include "entities.h"

#ifdef DEBUG
#include "debug.h"
#endif

Shader *shader_color;
Shader *shader_texture;
Shader *shader_basic_3d;
Shader *shader_msaa_2x;
Shader *shader_msaa_4x;
Shader *shader_msaa_8x;
Shader *shader_text;

static void draw_game_3d();
static void draw_game_2d();

static void draw_glyph(Font *font, Glyph *glyph, int x, int y, Vector4 color) {
    f32 xpos = (f32) (x + glyph->bearing_x);
    f32 ypos = (f32) (y - (glyph->size_y - glyph->bearing_y));
    
    f32 w = (f32) glyph->size_x;
    f32 h = (f32) glyph->size_y;
    
    Vector2 p0 = make_vector2(xpos+0, ypos+h);
    Vector2 p1 = make_vector2(xpos+0, ypos+0);
    Vector2 p2 = make_vector2(xpos+w, ypos+0);
    Vector2 p3 = make_vector2(xpos+w, ypos+h);

    Vector2 uv0 = make_vector2(glyph->min_uv.x, glyph->min_uv.y);
    Vector2 uv1 = make_vector2(glyph->min_uv.x, glyph->max_uv.y);
    Vector2 uv2 = make_vector2(glyph->max_uv.x, glyph->max_uv.y);
    Vector2 uv3 = make_vector2(glyph->max_uv.x, glyph->min_uv.y);
    
    immediate_quad(p0, p1, p2, p3, uv0, uv1, uv2, uv3, color);
}

void draw_text(Font *font, char *text, int x, int y, Vector4 color) {
    int orig_x = x;

    set_shader(shader_text);
    set_diffuse_texture(font->map);

    immediate_begin();

    for (char *at = text; *at;) {
        int codepoint_byte_count = 0;
        int codepoint = get_codepoint(at, &codepoint_byte_count);
        Glyph *glyph = get_or_load_glyph(font, codepoint);
        
        if (codepoint == 0x3f) codepoint_byte_count = 1;

        if (codepoint == '\n') {
            y -= font->character_height;
            x = orig_x;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                draw_glyph(font, glyph, x, y, color);
            }

            x += glyph->advance;

            if (font->has_kerning) {
                int next_codepoint_byte_count = 0;
                int next_codepoint = get_codepoint(at + codepoint_byte_count, &next_codepoint_byte_count);
                x += get_kerning_in_pixels(font, codepoint, next_codepoint);
            }
        }

        at += codepoint_byte_count;
    }
    
    immediate_flush();
}

void draw_game_view() {
    set_render_target(the_offscreen_buffer);
    set_depth_target(the_offscreen_depth_buffer);

    clear_render_target(0.2f, 0.5f, 0.8f, 1.0f);

    draw_game_3d();
    //draw_game_2d();
    
    set_render_target(the_back_buffer);
    set_depth_target(the_back_depth_buffer);

    clear_render_target(0.0f, 0.0f, 0.0f, 1.0f);
    
    extern bool multisampling;
    extern int num_samples;
    if (multisampling) {
        if (num_samples == 2) {
            set_shader(shader_msaa_2x);
        } else if (num_samples == 4) {
            set_shader(shader_msaa_4x);
        } else if (num_samples == 8) {
            set_shader(shader_msaa_8x);
        }
    } else {
        set_shader(shader_texture);
    }

    rendering_2d_right_handed();
    
    set_diffuse_texture(the_offscreen_buffer);
    
    immediate_begin();
    
    Vector2 p0 = make_vector2(0.0f, 0.0f);
    Vector2 p1 = make_vector2((float)render_target_width, 0.0f);
    Vector2 p2 = make_vector2((float)render_target_width, (float)render_target_height);
    Vector2 p3 = make_vector2(0.0f, (float)render_target_height);

    Vector2 uv0 = make_vector2(0.0f, 1.0f);
    Vector2 uv1 = make_vector2(1.0f, 1.0f);
    Vector2 uv2 = make_vector2(1.0f, 0.0f);
    Vector2 uv3 = make_vector2(0.0f, 0.0f);
        
    immediate_quad(p0, p1, p2, p3, uv0, uv1, uv2, uv3, make_vector4(1, 1, 1, 1));
    immediate_flush();
}

static void draw_game_3d() {
    f32 aspect_ratio = (f32)render_target_width / (f32)render_target_height;
    view_to_proj_matrix = make_perspective_projection(aspect_ratio, 70.0f * (PI / 180.0f), 0.1f, 1000.0f);
    refresh_transform();
    
    set_shader(shader_basic_3d);

    {
        Entity_Manager *manager = get_entity_manager();
        
        Guy *guy = manager->guy;
        
        set_diffuse_texture(guy->mesh->map);
        draw_mesh(guy->mesh, guy->position, guy->rotation, guy->scale);
    }
}

static void draw_game_2d() {
    //
    // Draw crosshair
    //
    {
        set_shader(shader_text);
        rendering_2d_right_handed();
        
        const f32 BIG_FONT_SIZE = 0.0725f;
        int font_size = (int) ((BIG_FONT_SIZE * 1.4f) * render_target_height);
        Font *big_font = get_font_at_size("Inconsolata-Regular.ttf", font_size);
        
        char *text = ".";
        int x = render_target_width / 2;
        int y = render_target_height / 2;

        draw_text(big_font, text, x, y, make_vector4(1, 1, 1, 1));
    }
    
#ifdef _DEBUG
    draw_debug_info();
#endif
}
