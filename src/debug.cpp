#include "debug.h"

#include "draw.h"
#include "font.h"
#include "os.h"

const f64 NUM_SECONDS_BETWEEN_UPDATES = 0.05;
static f64 num_seconds_since_last_update;
static int num_frames_since_last_update;
static f64 accumulated_dt;
static f64 dt_for_draw;

void draw_debug_info() {
    f64 dt = globals.time_info.current_dt;
    
    num_seconds_since_last_update += dt;
    num_frames_since_last_update++;
    if (num_seconds_since_last_update >= NUM_SECONDS_BETWEEN_UPDATES) {
        num_seconds_since_last_update = 0;
        
        dt_for_draw = accumulated_dt / num_frames_since_last_update;
        
        accumulated_dt = 0.0;
        num_frames_since_last_update = 0;
    }
    accumulated_dt += globals.time_info.current_dt;
    
    set_shader(shader_text);
    rendering_2d_right_handed();
    
    int font_size = (int) (0.02f * render_target_height);
    Font *font = get_font_at_size("OpenSans-SemiBold.ttf", font_size);

    int y = render_target_height - font->character_height;
    int offset = font->character_height / 20;
    
    {
        char *text = mprintf("%.2lf fps", 1.0 / dt_for_draw);
        defer { delete [] text; };
        
        int x = render_target_width - get_string_width_in_pixels(font, text);
        
        draw_text(font, text, x + offset, y - offset, make_vector4(0, 0, 0, 1));
        draw_text(font, text, x, y, make_vector4(1, 1, 1, 1));
    }

    y -= font->character_height;
    
    {
        Time time = os_get_local_time();
        
        char *text = mprintf("%02i:%02i:%02i", time.hour, time.minute, time.second);
        defer { delete [] text; };
        
        int x = render_target_width - get_string_width_in_pixels(font, text);

        draw_text(font, text, x + offset, y - offset, make_vector4(0, 0, 0, 1));
        draw_text(font, text, x, y, make_vector4(1, 1, 1, 1));        
    }
}
