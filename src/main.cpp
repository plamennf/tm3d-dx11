
#include "general.h"
#include "os.h"
#include "display.h"
#include "draw.h"
#include "loader.h"
#include "catalog.h"
#include "mesh.h"
#include "input.h"
#include "entities.h"
#include "hash_table.h"
#include "config.h"

#include <stdio.h>

Globals globals = {};
double global_time_rate = 1.0;
static double last_time;

Mesh *mesh;

static Entity_Manager *entity_manager = new Entity_Manager();

Entity_Manager *get_entity_manager() {
    return entity_manager;
}

static void game_init();
static void main_loop();
static void simulate_game();

int main(int argc, char **argv) {
    {
        char *exe = os_get_path_to_executable();
        defer { delete [] exe; };
        
        auto slash = find_character_from_right(exe, '/');
        slash += 1;

        globals.operating_folder = copy_string(exe);
        globals.operating_folder[get_string_length(exe) - get_string_length(slash)] = 0;

        os_setcwd(globals.operating_folder);
    }
    
    Config config = load_config();
    {
        extern float render_scale_to_draw; // From menu.cpp
        render_scale_to_draw = config.render_scale;
    }
    
    display_init(1280, 720, "TM3D-DX11");
    init_draw(true, true, 4);
    {
        int width = static_cast <int>(config.render_scale * default_offscreen_buffer_width);
        int height = static_cast <int>(config.render_scale * default_offscreen_buffer_height);
        resize_offscreen_buffer(width, height);
    }
    
    mesh = load_obj("stall");
    mesh->map = find_or_create_texture("stall");

    last_time = os_get_time();
    globals.time_info.current_dt = 0.0f;

    game_init();
    main_loop();
    
    return 0;
}

static void main_loop() {
    while (!globals.should_quit) {
        os_poll_events();
        
        if (is_key_pressed(KEY_ESCAPE)) {
            toggle_menu();
        }

        if (is_key_pressed(KEY_F11)) {
            display_toggle_fullscreen();
        }
        
        if (globals.time_info.current_dt) {
            if (globals.program_mode == PROGRAM_MODE_GAME) {
                if (display_has_focus()) {
                    os_hide_cursor();
                } else {
                    os_show_cursor();
                }
                simulate_game();
            }
        }

        if (the_offscreen_buffer && the_offscreen_depth_buffer) {
            set_render_target(the_offscreen_buffer);
            set_depth_target(the_offscreen_depth_buffer);
            if (globals.time_info.current_dt) {
                if (globals.program_mode == PROGRAM_MODE_GAME) {
                    draw_game_view();
                } else if (globals.program_mode == PROGRAM_MODE_MENU) {
                    void draw_menu();
                    draw_menu();
                }
            }
            {
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
        }
            
        swap_buffers();
        
        update_time(0.15f);
    }

    save_config();
}

static void game_init() {
    entity_manager = new Entity_Manager();
    
    Guy *guy = entity_manager->add_guy();
    guy->mesh = load_obj("dragon");
    guy->mesh->map = find_or_create_texture("white");
    guy->position = make_vector3(0, 0, -50);

    camera = make_camera(make_vector3(0, 0, 0), 0, 0, 0);
}

static void simulate_game() {
    update_camera(&camera);
}

void update_time(float dt_max) {
    double now = os_get_time();
    double delta = now - last_time;
    float dilated_dt = static_cast <float>(delta * global_time_rate);

    float clamped_dilated_dt = dilated_dt;
    if (clamped_dilated_dt > dt_max) clamped_dilated_dt = dt_max;

    globals.time_info.current_dt = clamped_dilated_dt;
    globals.time_info.current_time += clamped_dilated_dt;

    globals.time_info.real_world_dt = dilated_dt;
    globals.time_info.real_world_time += dilated_dt;
    
    globals.time_info.ui_dt = static_cast <float>(delta);
    globals.time_info.ui_time += static_cast <float>(delta);

    last_time = now;
}
