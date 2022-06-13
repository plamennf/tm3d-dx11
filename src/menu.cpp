#include "general.h"

#include "draw.h"
#include "font.h"
#include "input.h"

enum Menu_Item {
    MENU_RESUME,
    MENU_RENDER_SCALE,
    MENU_QUIT,
    NUM_MENU_ITEMS,
};

static bool asking_for_quit_confirmation;
static int current_menu_choice;
float render_scale_to_draw = 1.0f;

void toggle_menu() {
    if (globals.program_mode == PROGRAM_MODE_GAME) {
        globals.program_mode = PROGRAM_MODE_MENU;
    } else if (globals.program_mode == PROGRAM_MODE_MENU) {
        globals.program_mode = PROGRAM_MODE_GAME;
    }

    asking_for_quit_confirmation = false;
}

static void advance_menu_choice(int delta) {
    int prev_menu_choice = current_menu_choice;
    current_menu_choice += delta;
    if (current_menu_choice < 0) current_menu_choice = 0;
    else if (current_menu_choice >= NUM_MENU_ITEMS) current_menu_choice = NUM_MENU_ITEMS - 1;

    if (prev_menu_choice != current_menu_choice) {
        asking_for_quit_confirmation = false;
    }
}

static void handle_enter() {
    switch (current_menu_choice) {
    case MENU_RESUME:
        toggle_menu();
        break;

    case MENU_RENDER_SCALE: {
        int width = static_cast <int>(render_scale_to_draw * default_offscreen_buffer_width);
        int height = static_cast <int>(render_scale_to_draw * default_offscreen_buffer_height);
        resize_offscreen_buffer(width, height);
        break;
    }

    case MENU_QUIT:
        if (asking_for_quit_confirmation) globals.should_quit = true;
        else asking_for_quit_confirmation = true;
        break;
    }
}

static void draw_item(Font *font, char *text, int y, int item) {
    int x = (render_target_width - get_string_width_in_pixels(font, text)) / 2;
    
    int offset = font->character_height / 40;
    draw_text(font, text, x + offset, y - offset, make_vector4(0.0f, 0.0f, 0.0f, 1.0f));

    Vector4 color = make_vector4(0.4f, 0.4f, 0.4f, 0.4f);
    if (current_menu_choice == item) {
        color = make_vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    draw_text(font, text, x, y, color);
}

void draw_menu() {
    if (is_key_pressed(KEY_UP)) advance_menu_choice(-1);
    if (is_key_pressed(KEY_DOWN)) advance_menu_choice(+1);

    if (current_menu_choice == MENU_RENDER_SCALE) {
        if (is_key_pressed(KEY_LEFT)) {
            render_scale_to_draw -= 0.01f;
            if (render_scale_to_draw < 0.1f) render_scale_to_draw = 0.1f;
        } else if (is_key_pressed(KEY_RIGHT)) {
            render_scale_to_draw += 0.01f;
            if (render_scale_to_draw > 1.0f) render_scale_to_draw = 1.0f;
        }
    }
    
    if (is_key_pressed(KEY_ENTER)) handle_enter();
    
    clear_render_target(0.0f, 0.0f, 0.0f, 1.0f);
    
    set_shader(shader_text);
    rendering_2d_right_handed();

    //
    // Draw title
    //
    {
        int big_font_size = static_cast <int>(0.1f * render_target_height);
        Font *big_font = get_font_at_size("KarminaBoldItalic.otf", big_font_size);
        
        char *text = "ThinMatrix's 3D OpenGL Series";
        int x = (render_target_width - get_string_width_in_pixels(big_font, text)) / 2;
        int y = static_cast <int>(0.85f * render_target_height);
        draw_text(big_font, text, x, y, make_vector4(1.0f, 1.0f, 1.0f, 1.0f));

        text = "ported to Direct3D 11";
        x = (render_target_width - get_string_width_in_pixels(big_font, text)) / 2;
        y -= big_font->character_height;
        draw_text(big_font, text, x, y, make_vector4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    int font_size = static_cast <int>(0.05f * render_target_height);
    Font *font = get_font_at_size("KarminaBold.otf", font_size);
    
    int start_y = static_cast <int>(0.55f * render_target_height);
    int y = start_y;
    
    char *text = "Resume";
    draw_item(font, text, y, MENU_RESUME);
    y -= font->character_height;

    text = mprintf("Render scale: %.2f", render_scale_to_draw);
    draw_item(font, text, y, MENU_RENDER_SCALE);
    y -= font->character_height;
    delete [] text;

    text = "Quit";
    if (asking_for_quit_confirmation) text = "Quit? Are you sure?";
    draw_item(font, text, y, MENU_QUIT);
    y -= font->character_height;
}
