#include "general.h"
#include "camera.h"
#include "input.h"
#include "terrain.h"

Camera make_camera(Vector3 position, float pitch, float yaw, float roll) {
    Camera result = {};
    result.position = position;
    result.target = position - make_vector3(0, 0, 1);
    result.up = make_vector3(0, 1, 0);
    result.pitch = pitch;
    result.yaw = yaw;
    result.roll = roll;
    return result;
}

void update_camera(Camera *camera) {
    float dt = globals.time_info.current_dt;

    float sensitivity = 0.1f;
    camera->yaw += get_mouse_pointer_delta_x() * sensitivity;
    camera->pitch += get_mouse_pointer_delta_y() * sensitivity;

    Clamp(&camera->pitch, -89.0f, 89.0f);

    float old_y = camera->position.y;
    float movement_speed = 12.5f;

    Vector3 world_up = make_vector3(0, 1, 0);
    Vector3 right = normalize_or_zero(cross_product(camera->target, world_up));
    Vector3 up = normalize_or_zero(cross_product(right, camera->target));

    camera->target = make_vector3(0, 0, 0);
    camera->target.x = cosf(camera->yaw * (PI / 180.0f)) * cosf(camera->pitch * (PI / 180.0f));
    camera->target.z = sinf(camera->yaw * (PI / 180.0f)) * cosf(camera->pitch * (PI / 180.0f));
    Vector3 camera_target = normalize_or_zero(camera->target);

    if (is_key_down(KEY_W)) camera->position += camera_target * movement_speed * dt;
    else if (is_key_down(KEY_S)) camera->position -= camera_target * movement_speed * dt;

    if (is_key_down(KEY_A)) camera->position -= right * movement_speed * dt;
    else if (is_key_down(KEY_D)) camera->position += right * movement_speed * dt;

    camera->position.y = old_y;

    camera->target.y = sinf(camera->pitch * (PI / 180.0f));
    camera->target = normalize_or_zero(camera->target);

    if (is_key_down(KEY_SPACE)) {
        if (camera->is_on_ground) {
            camera->jump_velocity = 50.0f * dt;
            camera->is_on_ground = false;
        }
    }
    
    camera->jump_velocity -= 1.0f * dt;

    camera->position.y += camera->jump_velocity;

#if 1
    Terrain *terrain = get_terrain_at(camera->position);
    float terrain_height = get_terrain_height_at(terrain, -camera->position.x, -camera->position.z);
    if (camera->position.y < terrain_height + 3.0f) {
        camera->position.y = terrain_height + 3.0f;
        camera->is_on_ground = true;
    }
#else
    if (camera->position.y < 3.0f) {
        camera->position.y = 3.0f;
        camera->is_on_ground = true;
    }
#endif
}
