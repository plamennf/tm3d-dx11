#ifndef ENTITIES_H
#define ENTITIES_H

#include "array.h"
#include "geometry.h"

struct Mesh;
struct Entity_Manager;
struct Terrain;

struct Entity {
    Entity_Manager *manager = nullptr;
    
    Mesh *mesh = nullptr;

    Vector3 position = make_vector3(0, 0, 0);
    Vector3 rotation = make_vector3(0, 0, 0);
    f32 scale = 1.0f;
};

struct Light : public Entity {
    Vector3 color;
    Vector3 attenutation;
};

struct Guy : public Entity {
    const float RUN_SPEED = 20.0f;
    const float TURN_SPEED = 160.0f;
    const float GRAVITY = -50.0f;
    const float JUMP_POWER = 30.0f;
    const float TERRAIN_HEIGHT = 0.0f;
    
    f32 current_speed = 0.0f;
    f32 current_turn_speed = 0.0f;
    f32 upwards_speed = 0.0f;

    bool is_in_air = false;
};

struct Entity_Manager {
    Array <Light *> lights;
    
    Guy *guy;

    inline Guy *add_guy() {
        guy = new Guy();
        guy->manager = this;
        return guy;
    }
};

#endif
