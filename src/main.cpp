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

#include <stdio.h>

Globals globals = {};

Mesh *mesh;

static Entity_Manager *entity_manager = new Entity_Manager();

Entity_Manager *get_entity_manager() {
    return entity_manager;
}

static void update_time() {
    f64 now_time = os_get_time();
    globals.time_info.current_dt = now_time - globals.time_info.last_time;    
    globals.time_info.last_time = now_time;
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
    
    display_init(1280, 720, "TM3D-DX11");
    init_draw(true, true, 4);

    printf("Back buffer size: %dx%d\n", the_back_buffer->width, the_back_buffer->height);
    
    mesh = load_obj("stall");
    mesh->map = find_or_create_texture("stall");

    globals.time_info.last_time = os_get_time();
    globals.time_info.current_dt = 0.0;

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

        if (globals.time_info.current_dt) {
            if (globals.program_mode == PROGRAM_MODE_GAME) {
                draw_game_view();
            } else if (globals.program_mode == PROGRAM_MODE_MENU) {
                void draw_menu();
                draw_menu();
            }
        }
        
        swap_buffers();
        
        update_time();
    }
}

static void game_init() {
    entity_manager = new Entity_Manager();
    
    Guy *guy = entity_manager->add_guy();
    guy->mesh = load_obj("dragon");
    guy->mesh->map = find_or_create_texture("white");
    guy->position = make_vector3(0, 0, -50);
}

static void simulate_game() {
    simulate_guy(entity_manager->guy);
}
