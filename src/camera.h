#ifndef CAMERA_H
#define CAMERA_H

#include "geometry.h"

struct Guy;

struct Camera {
    Vector3 position = make_vector3(0, 0, 0);
    f32 pitch = 20.0f;
    f32 yaw = 0.0f;
    f32 roll = 0.0f;

    Guy *guy;

    f32 distance_from_player = 50.0f;
    f32 angle_around_player = 0.0f;
};

Camera make_camera(Guy *guy);
void update_camera(Camera *camera);

#endif
