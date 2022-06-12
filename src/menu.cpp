#include "general.h"

#include "draw.h"
#include "font.h"
#include "input.h"

enum Menu_Item {
    MENU_RESUME,
    MENU_QUIT,
    NUM_MENU_ITEMS,
};

static bool asking_for_quit_confirmation;
static int current_menu_choice;

void toggle_menu() {
    if (globals.program_mode == PROGRAM_MODE_GAME) {
        globals.program_mode = PROGRAM_MODE_MENU;
    } else if (globals.program_mode == PROGRAM_MODE_MENU) {
        globals.program_mode = PROGRAM_MODE_GAME;
    }

    asking_for_quit_confirmation = false;
}

static void advance_menu_choice(int delta) {
    asking_for_quit_confirmation = false;
    
    current_menu_choice += delta;
    if (current_menu_choice < 0) current_menu_choice = 0;
    else if (current_menu_choice >= NUM_MENU_ITEMS) current_menu_choice = NUM_MENU_ITEMS - 1;
}

static void handle_enter() {
    switch (current_menu_choice) {
    case MENU_RESUME:
        toggle_menu();
        break;

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

    if (is_key_pressed(KEY_ENTER)) handle_enter();
    
    set_render_target(the_back_buffer);
    set_depth_target(the_back_depth_buffer);
    clear_render_target(0.0f, 0.0f, 0.0f, 1.0f);
    
    set_shader(shader_text);
    rendering_2d_right_handed();

    //
    // Draw title
    //
    {
        int big_font_size = static_cast <int>(0.1f * render_target_height);
        Font *big_font = get_font_at_size("KarminaBold.otf", big_font_size);

        char *text = "TM3D-DX11";
        int x = (render_target_width - get_string_width_in_pixels(big_font, text)) / 2;
        int y = static_cast <int>(0.85f * render_target_height);
        draw_text(big_font, text, x, y, make_vector4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    int font_size = static_cast <int>(0.05f * render_target_height);
    Font *font = get_font_at_size("KarminaBoldItalic.otf", font_size);
    
    int start_y = static_cast <int>(0.55f * render_target_height);
    int y = start_y;
    
    char *text = "Resume";
    draw_item(font, text, y, MENU_RESUME);
    y -= font->character_height;

    text = "Quit";
    if (asking_for_quit_confirmation) text = "Quit? Are you sure?";
    draw_item(font, text, y, MENU_QUIT);
    y -= font->character_height;
}
