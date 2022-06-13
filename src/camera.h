#ifndef CAMERA_H
#define CAMERA_H

#include "geometry.h"

struct Camera {
    Vector3 position = make_vector3(0, 0, 0);
    Vector3 target = make_vector3(0, 0, -1);
    Vector3 up = make_vector3(0, 1, 0);
    float pitch, yaw, roll;
    float jump_velocity = 0.0f;
    bool is_on_ground = true;
};

Camera make_camera(Vector3 position, float pitch, float yaw, float roll);
void update_camera(Camera *camera);

#endif
