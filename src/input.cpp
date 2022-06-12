#include "input.h"

Key_Info key_infos[NUM_KEYS];

int mouse_pointer_delta_x;
int mouse_pointer_delta_y;

bool is_key_down(Key key) {
    return key_infos[key].is_down;
}

bool is_key_pressed(Key key) {
    return key_infos[key].is_down && key_infos[key].changed;
}

bool was_key_pressed(Key key) {
    return key_infos[key].was_down && !key_infos[key].is_down;
}

int get_mouse_pointer_delta_x() {
    return mouse_pointer_delta_x;
}

int get_mouse_pointer_delta_y() {
    return mouse_pointer_delta_y;
}
