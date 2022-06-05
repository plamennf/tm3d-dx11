#include "entities.h"

#include "input.h"

void simulate_guy(Guy *guy) {
    f32 dt = core.time_info.current_dt;
    
    if (is_key_down(KEY_W)) {
        guy->current_speed = -guy->RUN_SPEED;
    } else if (is_key_down(KEY_S)) {
        guy->current_speed = guy->RUN_SPEED;
    } else {
        guy->current_speed = 0.0f;
    }
    
    if (is_key_down(KEY_D)) {
        guy->current_turn_speed = -guy->TURN_SPEED;
    } else if (is_key_down(KEY_A)) {
        guy->current_turn_speed = guy->TURN_SPEED;
    } else {
        guy->current_turn_speed = 0.0f;
    }

    if (is_key_down(KEY_SPACE) && !guy->is_in_air) {
        guy->upwards_speed = guy->JUMP_POWER;
        guy->is_in_air = true;
    }
    
    guy->rotation.y += guy->current_turn_speed * dt;

    f32 distance = guy->current_speed * dt;
    f32 dx = distance * sinf(guy->rotation.y * (PI / 180.0f));
    f32 dz = distance * cosf(guy->rotation.y * (PI / 180.0f));

    guy->position.x += dx;
    guy->position.z += dz;

    guy->upwards_speed += guy->GRAVITY * dt;

    guy->position.y += guy->upwards_speed * dt;

    if (guy->position.y < guy->TERRAIN_HEIGHT) {
        guy->upwards_speed = 0;
        guy->position.y = guy->TERRAIN_HEIGHT;
        guy->is_in_air = false;
    }
}
