#include "input.h"

Key_Info key_infos[NUM_KEYS];

bool is_key_down(Key key) {
    return key_infos[key].is_down;
}

bool is_key_pressed(Key key) {
    return key_infos[key].was_down && !key_infos[key].is_down;
}
