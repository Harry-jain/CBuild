#pragma once
#include "vec2.hpp"
#include <cmath>

struct Mat3 {
    // Column-major array: m[col][row]
    float m[3][3];

    Mat3() {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
    }

    static Mat3 translation(float tx, float ty) {
        Mat3 res;
        res.m[2][0] = tx;
        res.m[2][1] = ty;
        return res;
    }

    static Mat3 translation(const Vec2& v) {
        return translation(v.x, v.y);
    }

    static Mat3 scale(float sx, float sy) {
        Mat3 res;
        res.m[0][0] = sx;
        res.m[1][1] = sy;
        return res;
    }

    static Mat3 rotation(float angle_rad) {
        Mat3 res;
        float c = std::cos(angle_rad);
        float s = std::sin(angle_rad);
        res.m[0][0] = c;  res.m[1][0] = -s;
        res.m[0][1] = s;  res.m[1][1] = c;
        return res;
    }

    Mat3 operator*(const Mat3& b) const {
        Mat3 res;
        for (int col = 0; col < 3; col++) {
            for (int row = 0; row < 3; row++) {
                res.m[col][row] = m[0][row] * b.m[col][0] +
                                  m[1][row] * b.m[col][1] +
                                  m[2][row] * b.m[col][2];
            }
        }
        return res;
    }

    Vec2 transform_point(const Vec2& p) const {
        return Vec2(
            m[0][0] * p.x + m[1][0] * p.y + m[2][0],
            m[0][1] * p.x + m[1][1] * p.y + m[2][1]
        );
    }

    Vec2 transform_dir(const Vec2& d) const {
        return Vec2(
            m[0][0] * d.x + m[1][0] * d.y,
            m[0][1] * d.x + m[1][1] * d.y
        );
    }
};
