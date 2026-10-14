#pragma once

#include <cmath>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    [[nodiscard]] Vec2 operator+(Vec2 o)  const { return {x + o.x, y + o.y}; }
    [[nodiscard]] Vec2 operator-(Vec2 o)  const { return {x - o.x, y - o.y}; }
    [[nodiscard]] Vec2 operator*(float s) const { return {x * s,   y * s};   }

    Vec2& operator+=(Vec2 o)  { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(Vec2 o)  { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s;   y *= s;   return *this; }

    [[nodiscard]] float dot(Vec2 o)  const { return x * o.x + y * o.y; }
    [[nodiscard]] float lengthSq()   const { return dot(*this); }
    [[nodiscard]] float length()     const { return std::sqrt(lengthSq()); }

    [[nodiscard]] Vec2 normalized() const {
        float len = length();
        return len > 0.0f ? *this * (1.0f / len) : Vec2{};
    }
};
