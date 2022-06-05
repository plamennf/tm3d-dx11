#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "core.h"

#include <math.h>

union Vector2 {
    struct { f32 x, y; };
    struct { f32 r, g; };
    f32 e[2];
};

inline Vector2 make_vector2(f32 x, f32 y) {
    Vector2 result;

    result.x = x;
    result.y = y;

    return result;
}

inline Vector2 operator+(Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

union Vector3 {
    struct { f32 x, y, z; };
    struct { f32 r, g, b; };
    f32 e[3];
};

inline Vector3 make_vector3(f32 x, f32 y, f32 z) {
    Vector3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

union Vector4 {
    struct { f32 x, y, z, w; };
    struct { f32 r, g, b, a; };
    f32 e[4];
};

inline Vector4 make_vector4(f32 x, f32 y, f32 z, f32 w) {
    Vector4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    
    return result;
}

inline u32 abgr_color(Vector4 v) {
    u32 ir = (u32)(v.r * 255.0f);
    u32 ig = (u32)(v.g * 255.0f);
    u32 ib = (u32)(v.b * 255.0f);
    u32 ia = (u32)(v.a * 255.0f);

    return (ia << 24) | (ib << 16) | (ig << 8) | (ir << 0);
}

union Matrix4 {
    struct {
        f32 _11, _12, _13, _14;
        f32 _21, _22, _23, _24;
        f32 _31, _32, _33, _34;
        f32 _41, _42, _43, _44;
    };
    f32 e[4][4];
};

inline Matrix4 operator*(Matrix4 a, Matrix4 b) {
    Matrix4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float sum = 0.0f;
            for (int e = 0; e < 4; e++) {
                sum += a.e[row][e] * b.e[e][col];
            }
            result.e[row][col] = sum;
        }
    }
    return result;
}

inline Matrix4 matrix4_identity() {
    Matrix4 result = {};

    result._11 = 1.0f;
    result._22 = 1.0f;
    result._33 = 1.0f;
    result._44 = 1.0f;
    
    return result;
}

inline Matrix4 transpose(Matrix4 m) {
    Matrix4 result;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.e[row][col] = m.e[col][row];
        }
    }

    return result;
}

inline Matrix4 make_perspective_projection(f32 aspect_ratio, f32 fov, f32 z_near, f32 z_far) {
    Matrix4 result = {};

    f32 a = 1.0f;
    f32 b = aspect_ratio;
    f32 c = fov;

    f32 n = z_near;
    f32 f = z_far;

    f32 d = (n+f) / (n-f);
    f32 e = (2*f*n) / (n-f);

    result._11 = a*c;
    result._22 = b*c;
    result._33 = d;
    result._34 = e;
    result._43 = -1.0f;
    
    return result;
}

inline Matrix4 make_x_rotation(f32 t) {
    Matrix4 result = matrix4_identity();

    f32 ct = cosf(t);
    f32 st = sinf(t);

    result._22 = ct;
    result._23 = -st;
    result._32 = st;
    result._33 = ct;
    
    return result;
}

inline Matrix4 make_y_rotation(f32 t) {
    Matrix4 result = matrix4_identity();

    f32 ct = cosf(t);
    f32 st = sinf(t);

    result._11 = ct;
    result._31 = -st;
    result._13 = st;
    result._33 = ct;
    
    return result;
}

inline Matrix4 make_z_rotation(f32 t) {
    Matrix4 result = matrix4_identity();

    f32 ct = cosf(t);
    f32 st = sinf(t);

    result._11 = ct;
    result._12 = -st;
    result._21 = st;
    result._22 = ct;
    
    return result;
}

#endif
