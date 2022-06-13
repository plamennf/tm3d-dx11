#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "general.h"

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

inline Vector2 rotate(Vector2 v, float rotation) {
    float st = sinf(rotation);
    float ct = cosf(rotation);

    float nx = v.x * ct - v.y * st;
    float ny = v.x * st + v.y * ct;

    return { nx, ny };
}

inline float get_length_squared(Vector2 v) {
    return v.x*v.x+v.y*v.y;
}

inline float get_length(Vector2 v) {
    return sqrtf(get_length_squared(v));
}
    
inline Vector2 normalize(Vector2 v) {
    float multiplier = 1.0f / get_length(v);
    v.x *= multiplier;
    v.y *= multiplier;
    return v;
}

inline Vector2 normalize_or_zero(Vector2 v) {
    Vector2 result = {};
    
    float length_squared = get_length_squared(v);
    if (length_squared > 0.0001f * 0.0001f) {
        float multiplier = 1.0f / sqrtf(length_squared);
        result.x = v.x * multiplier;
        result.y = v.y * multiplier;
    }

    return result;
}

inline Vector2 componentwise_product(Vector2 left, Vector2 right) {
    return make_vector2(left.x * right.x, left.y * right.y);
}

inline Vector2 operator+(Vector2 a, Vector2 b) {
    Vector2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

inline Vector2 operator-(Vector2 left, Vector2 right) {
    return make_vector2(left.x - right.x, left.y - right.y);
}

inline Vector2 operator*(Vector2 left, float right) {
    return make_vector2(left.x * right, left.y * right);
}

inline Vector2 operator*(float right, Vector2 left) {
    return make_vector2(left.x * right, left.y * right);
}

inline Vector2 operator/(Vector2 left, Vector2 right) {
    return make_vector2(left.x / right.x, left.y / right.y);
}

inline Vector2 &operator*=(Vector2 &left, float right) {
    left.x *= right;
    left.y *= right;
    return left;
}

inline Vector2 &operator*=(Vector2 &left, Vector2 right) {
    left.x *= right.x;
    left.y *= right.y;
    return left;
}

inline Vector2 &operator/=(Vector2 &left, float right) {
    left.x /= right;
    left.y /= right;
    return left;
}
    
inline Vector2 &operator/=(Vector2 &left, Vector2 right) {
    left.x /= right.x;
    left.y /= right.y;
    return left;
}

inline Vector2 &operator-=(Vector2 &left, float right) {
    left.x -= right;
    left.y -= right;
    return left;
}
    
inline Vector2 &operator-=(Vector2 &left, Vector2 right) {
    left.x -= right.x;
    left.y -= right.y;
    return left;
}

inline Vector2 &operator+=(Vector2 &left, float right) {
    left.x += right;
    left.y += right;
    return left;
}
    
inline Vector2 &operator+=(Vector2 &left, Vector2 right) {
    left.x += right.x;
    left.y += right.y;
    return left;
}

inline Vector2 lerp(Vector2 a, Vector2 b, float t) {
    Vector2 result = (1.0f - t) * a + t * b;
}

inline float distance(Vector2 a, Vector2 b) {
    Vector2 dir = a - b;
    return get_length(dir);
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

inline Vector3 operator+(Vector3 left, Vector3 right) {
    Vector3 result = make_vector3(left.x + right.x, left.y + right.y, left.z + right.z);
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

struct Rectangle2i {
    int x, y, width, height;
};

inline int get_max_x(Rectangle2i rect) {
    return rect.x + rect.width;
}

inline int get_max_y(Rectangle2i rect) {
    return rect.y + rect.height;
}

inline Vector2 get_vec2(float theta) {
    float ct = cosf(theta);
    float st = sinf(theta);

    return make_vector2(ct, st);
}

#endif
