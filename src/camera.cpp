#include "camera.h"

#if 0
static void calculate_zoom(Camera *camera) {
    f32 zoom_level = get_scroll_offset();
    camera->distance_from_player -= zoom_level;
}

static void calculate_pitch(Camera *camera) {
    if (is_key_down(MOUSE_BUTTON_RIGHT)) {
        float pitch_change = get_mouse_y_offset();
        camera->pitch -= pitch_change;
    }
}

static void calculate_angle_around_player(Camera *camera) {
    if (is_key_down(MOUSE_BUTTON_LEFT)) {
        float angle_change = get_mouse_x_offset();
        camera->angle_around_player -= angle_change;
    }
}

Camera make_camera(Guy *guy) {
    Camera result = {};

    result.guy = guy;
    
    return result;
}

void update_camera(Camera *camera) {
    calculate_zoom(camera);
    calculate_pitch(camera);
    calculate_angle_around_player(camera);
}
#endif
