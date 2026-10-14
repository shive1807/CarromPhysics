#pragma once

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)        const { return {x * s,   y * s};   }

    float dot(const Vec2& o)  const { return x * o.x + y * o.y; }
    float lengthSq()          const { return dot(*this); }
};

struct Circle2D {
    Vec2  position;
    Vec2  velocity;
    float radius = 1.0f;
    float mass   = 1.0f;
};
