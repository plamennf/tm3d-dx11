#include "core.h"
#include "os.h"
#include "display.h"
#include "draw.h"
#include "loader.h"
#include "catalog.h"
#include "mesh.h"
#include "input.h"

#include <stdio.h>

Core core = {};

Mesh *mesh;

static void update_time() {
    f64 now_time = os_get_time();
    core.time_info.current_dt = now_time - core.time_info.last_time;    
    core.time_info.last_time = now_time;
}

int main(int argc, char **argv) {
    {
        char *exe = os_get_path_to_executable();
        defer { delete [] exe; };

        auto slash = find_character_from_right(exe, '/');
        slash += 1;

        core.operating_folder = copy_string(exe);
        core.operating_folder[get_string_length(exe) - get_string_length(slash)] = 0;

        os_setcwd(core.operating_folder);
    }
    
    display_init(1280, 720, "TM3D-DX11");
    init_draw(true, true, 4);

    mesh = load_obj("stall");
    mesh->map = find_or_create_texture("stall");

    core.time_info.last_time = os_get_time();
    core.time_info.current_dt = 0.0;
    while (!core.should_quit) {
        if (!display_is_open()) {
            core.should_quit = true;
            break;
        }
        os_poll_events();

        if (is_key_pressed(KEY_ESCAPE)) {
            core.should_quit = true;
            break;
        }
        
        if (core.time_info.current_dt) {
            draw_game_view();
        }
        
        swap_buffers();

        update_time();
    }
    
    return 0;
}
